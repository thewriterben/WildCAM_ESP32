# CSV Export - Integration Guide

This guide helps developers integrate the CSV export functionality with actual detection data sources.

## Current Implementation Status

### ✅ What's Working
- CSV endpoint (`/api/export/detections.csv`)
- Streaming response mechanism
- CSV formatting and escaping
- Query parameter parsing
- Record limiting (10,000 max)
- HTTP headers and filename generation
- Logging

### 🔄 What Needs Integration
- Reading actual detection data from storage
- Applying date range filters to queries
- Integration with WildlifeClassifier history
- Integration with StorageManager

## Data Flow Architecture

```
┌─────────────────┐
│  Web Request    │
│  /api/export/   │
│  detections.csv │
└────────┬────────┘
         │
         v
┌─────────────────────────────────┐
│  handleAPIExportDetectionsCSV   │
│  - Parse query parameters       │
│  - Setup streaming response     │
└────────┬────────────────────────┘
         │
         v
┌─────────────────────────────────┐
│  Query Detection Data Source    │
│  ┌─────────────────────────┐   │
│  │ Option 1: StorageManager│   │
│  │ Option 2: Log Files     │   │
│  │ Option 3: Database      │   │
│  └─────────────────────────┘   │
└────────┬────────────────────────┘
         │
         v
┌─────────────────────────────────┐
│  Generate CSV Rows              │
│  - Iterate records              │
│  - Apply date filters           │
│  - Enforce record limit         │
│  - Format and escape fields     │
└────────┬────────────────────────┘
         │
         v
┌─────────────────────────────────┐
│  Stream to Client               │
│  - Chunked response             │
│  - Proper headers               │
└─────────────────────────────────┘
```

## Integration Option 1: StorageManager

### Step 1: Define Detection Record Structure

Add to `storage_manager.h`:

```cpp
/**
 * @brief Detection record for CSV export
 */
struct DetectionRecord {
    time_t timestamp;
    String species;
    float confidence;
    String imagePath;
    double gpsLat;
    double gpsLon;
    uint8_t batteryLevel;
};
```

### Step 2: Add Query Method

Add to `storage_manager.h`:

```cpp
/**
 * @brief Query detection records with optional date filtering
 * @param startTimestamp Start time filter (0 for no filter)
 * @param endTimestamp End time filter (0 for no filter)
 * @param maxRecords Maximum records to return
 * @param records Output array for records
 * @return Number of records retrieved
 */
static uint32_t queryDetections(time_t startTimestamp, 
                               time_t endTimestamp, 
                               uint32_t maxRecords,
                               DetectionRecord* records);
```

### Step 3: Implement Query Method

Add to `storage_manager.cpp`:

```cpp
uint32_t StorageManager::queryDetections(time_t startTimestamp,
                                        time_t endTimestamp,
                                        uint32_t maxRecords,
                                        DetectionRecord* records) {
    uint32_t count = 0;
    
    // Open detections directory
    File dir = SD_MMC.open("/detections", FILE_READ);
    if (!dir) {
        ESP_LOGE("StorageManager", "Failed to open detections directory");
        return 0;
    }
    
    // Iterate through detection files
    while (File file = dir.openNextFile()) {
        if (count >= maxRecords) break;
        
        // Parse detection metadata file
        if (String(file.name()).endsWith(".json")) {
            DetectionRecord record = parseDetectionFile(file);
            
            // Apply date filter
            if (startTimestamp > 0 && record.timestamp < startTimestamp) continue;
            if (endTimestamp > 0 && record.timestamp > endTimestamp) continue;
            
            records[count++] = record;
        }
    }
    
    return count;
}

DetectionRecord StorageManager::parseDetectionFile(File& file) {
    DetectionRecord record = {};
    
    // Parse JSON metadata
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, file);
    
    record.timestamp = doc["timestamp"];
    record.species = doc["species"].as<String>();
    record.confidence = doc["confidence"];
    record.imagePath = doc["imagePath"].as<String>();
    record.gpsLat = doc["gps"]["lat"];
    record.gpsLon = doc["gps"]["lon"];
    record.batteryLevel = doc["battery"];
    
    return record;
}
```

### Step 4: Update CSV Export Handler

Replace mock data generation in `enhanced_web_server.cpp`:

```cpp
// Instead of mock data:
// for (uint32_t i = 0; i < recordsToAdd; i++) { ... }

// Use actual data:
const uint32_t BATCH_SIZE = 50;
static DetectionRecord recordBuffer[BATCH_SIZE];
static uint32_t bufferIndex = 0;
static bool dataFetched = false;

if (!dataFetched) {
    // Parse date range strings to timestamps
    time_t startTimestamp = parseDateString(startDate);
    time_t endTimestamp = parseDateString(endDate);
    
    // Query actual records
    uint32_t fetched = StorageManager::queryDetections(
        startTimestamp, 
        endTimestamp, 
        MAX_RECORDS,
        recordBuffer
    );
    
    dataFetched = true;
}

// Generate CSV from actual records
while (bufferIndex < BATCH_SIZE && recordCount < MAX_RECORDS) {
    DetectionRecord& record = recordBuffer[bufferIndex++];
    
    String timestampStr = String(record.timestamp);
    String confidenceStr = String(record.confidence, 2);
    String batteryStr = String(record.batteryLevel);
    String latStr = String(record.gpsLat, 6);
    String lonStr = String(record.gpsLon, 6);
    
    chunk += generateCSVRow(timestampStr, record.species, confidenceStr,
                           record.imagePath, latStr, lonStr, batteryStr);
    
    recordCount++;
}
```

## Integration Option 2: Log Files

### Step 1: Create Detection Logger

Add to `detection_logger.h`:

```cpp
class DetectionLogger {
public:
    static bool logDetection(const DetectionRecord& record);
    static bool readDetections(const String& logPath, 
                              DetectionRecord* records,
                              uint32_t maxRecords,
                              time_t startTime = 0,
                              time_t endTime = 0);
};
```

### Step 2: Log Format

Use CSV format for detection logs:

```
# detections.log format
timestamp,species,confidence,image_path,gps_lat,gps_lon,battery_level
1729113600,deer,0.92,/images/wildlife_1729113600.jpg,45.500000,-122.700000,85
```

### Step 3: Update CSV Export

```cpp
// Read from log file
File logFile = SD_MMC.open("/logs/detections.log", FILE_READ);
if (!logFile) {
    ESP_LOGE(TAG, "Failed to open detection log");
    return 0;
}

// Skip header line
logFile.readStringUntil('\n');

// Read and filter records
while (logFile.available() && recordCount < MAX_RECORDS) {
    String line = logFile.readStringUntil('\n');
    
    // Parse CSV line
    DetectionRecord record = parseCSVLine(line);
    
    // Apply date filter
    if (startTimestamp > 0 && record.timestamp < startTimestamp) continue;
    if (endTimestamp > 0 && record.timestamp > endTimestamp) continue;
    
    // Add to output CSV
    chunk += generateCSVRowFromRecord(record);
    recordCount++;
}
```

## Integration Option 3: Database (SQLite/InfluxDB)

### Step 1: Define Database Schema

SQLite schema:

```sql
CREATE TABLE detections (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    species TEXT NOT NULL,
    confidence REAL NOT NULL,
    image_path TEXT NOT NULL,
    gps_lat REAL,
    gps_lon REAL,
    battery_level INTEGER,
    INDEX idx_timestamp (timestamp)
);
```

### Step 2: Query Implementation

```cpp
#include <sqlite3.h>

uint32_t queryDetectionsFromDB(time_t startTime, 
                               time_t endTime,
                               uint32_t maxRecords,
                               DetectionRecord* records) {
    sqlite3* db;
    sqlite3_open("/sdcard/detections.db", &db);
    
    const char* sql = 
        "SELECT timestamp, species, confidence, image_path, "
        "       gps_lat, gps_lon, battery_level "
        "FROM detections "
        "WHERE (? = 0 OR timestamp >= ?) "
        "  AND (? = 0 OR timestamp <= ?) "
        "ORDER BY timestamp DESC "
        "LIMIT ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, startTime);
    sqlite3_bind_int64(stmt, 2, startTime);
    sqlite3_bind_int64(stmt, 3, endTime);
    sqlite3_bind_int64(stmt, 4, endTime);
    sqlite3_bind_int(stmt, 5, maxRecords);
    
    uint32_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < maxRecords) {
        records[count].timestamp = sqlite3_column_int64(stmt, 0);
        records[count].species = String((const char*)sqlite3_column_text(stmt, 1));
        records[count].confidence = sqlite3_column_double(stmt, 2);
        records[count].imagePath = String((const char*)sqlite3_column_text(stmt, 3));
        records[count].gpsLat = sqlite3_column_double(stmt, 4);
        records[count].gpsLon = sqlite3_column_double(stmt, 5);
        records[count].batteryLevel = sqlite3_column_int(stmt, 6);
        count++;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return count;
}
```

## Date String Parsing

Add helper function to parse ISO 8601 date strings:

```cpp
time_t EnhancedWebServer::parseDateString(const String& dateStr) {
    if (dateStr.isEmpty()) return 0;
    
    struct tm timeinfo = {};
    
    // Parse YYYY-MM-DD format
    if (sscanf(dateStr.c_str(), "%d-%d-%d", 
               &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday) == 3) {
        timeinfo.tm_year -= 1900;  // Years since 1900
        timeinfo.tm_mon -= 1;       // Months since January
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        
        return mktime(&timeinfo);
    }
    
    return 0;
}
```

## Integration with WildlifeClassifier

### Step 1: Add Detection History to Classifier

Add to `wildlife_classifier.h`:

```cpp
struct DetectionHistory {
    time_t timestamp;
    ClassificationResult result;
    float batteryLevel;
    double gpsLat;
    double gpsLon;
    String imagePath;
};

class WildlifeClassifier {
private:
    std::vector<DetectionHistory> detectionHistory;
    const uint32_t MAX_HISTORY = 1000;
    
public:
    void addToHistory(const ClassificationResult& result, 
                     const String& imagePath,
                     float batteryLevel,
                     double gpsLat, 
                     double gpsLon);
    
    std::vector<DetectionHistory> getHistory(time_t startTime = 0, 
                                            time_t endTime = 0,
                                            uint32_t maxRecords = 10000);
};
```

### Step 2: Update CSV Export

```cpp
if (g_wildlifeClassifier) {
    // Get detection history
    auto history = g_wildlifeClassifier->getHistory(startTimestamp, 
                                                    endTimestamp, 
                                                    MAX_RECORDS);
    
    // Generate CSV rows
    for (const auto& detection : history) {
        if (recordCount >= MAX_RECORDS) break;
        
        String timestampStr = String(detection.timestamp);
        String species = WildlifeClassifier::getSpeciesName(detection.result.species);
        String confidenceStr = String(detection.result.confidence, 2);
        String latStr = String(detection.gpsLat, 6);
        String lonStr = String(detection.gpsLon, 6);
        String batteryStr = String((int)detection.batteryLevel);
        
        chunk += generateCSVRow(timestampStr, species, confidenceStr,
                               detection.imagePath, latStr, lonStr, batteryStr);
        
        recordCount++;
    }
}
```

## Testing Integration

### Unit Test Template

```cpp
#include <unity.h>
#include "../firmware/src/web/enhanced_web_server.h"
#include "../ESP32WildlifeCAM-main/src/data/storage_manager.h"

void test_csv_export_with_real_data() {
    // Setup
    StorageManager::initialize();
    
    // Create test detection records
    DetectionRecord testRecords[5];
    // ... populate test records
    
    // Mock query function
    // ... 
    
    // Generate CSV
    EnhancedWebServer server;
    // ... test CSV generation
    
    TEST_ASSERT_TRUE(csvGenerated);
}
```

### Integration Test Checklist

- [ ] CSV export works with empty dataset
- [ ] CSV export handles single record
- [ ] CSV export respects record limit (10,000)
- [ ] Date filtering correctly filters records
- [ ] CSV escaping works with special characters
- [ ] Large exports don't cause memory issues
- [ ] Concurrent requests are handled properly
- [ ] Invalid date parameters handled gracefully

## Performance Optimization

### Chunked Reading

For large datasets, implement chunked reading:

```cpp
const uint32_t CHUNK_SIZE = 100;
static uint32_t offset = 0;

// Read next chunk from storage
DetectionRecord chunk[CHUNK_SIZE];
uint32_t chunkSize = StorageManager::queryDetections(
    startTimestamp, endTimestamp, 
    CHUNK_SIZE, chunk, offset);

offset += chunkSize;
```

### Caching

For frequently accessed date ranges:

```cpp
struct CachedExport {
    String startDate;
    String endDate;
    String csvData;
    time_t cacheTime;
};

// Check cache before querying
if (cacheHit) {
    return cachedData;
}
```

## Error Handling

Add error cases:

```cpp
void EnhancedWebServer::handleAPIExportDetectionsCSV(AsyncWebServerRequest* request) {
    // Validate date parameters
    if (!validateDateFormat(startDate)) {
        ESP_LOGE(TAG, "Invalid start date format: %s", startDate.c_str());
        request->send(400, "application/json", 
                     "{\"error\":\"Invalid start date format. Use YYYY-MM-DD\"}");
        return;
    }
    
    // Check storage availability
    if (!SD_MMC.begin()) {
        ESP_LOGE(TAG, "SD card not available");
        request->send(503, "application/json", 
                     "{\"error\":\"Storage not available\"}");
        return;
    }
    
    // Handle empty dataset
    if (recordCount == 0) {
        ESP_LOGW(TAG, "No detection records found");
        // Still send CSV with header but no data
    }
}
```

## Monitoring and Logging

Add detailed logging:

```cpp
ESP_LOGI(TAG, "CSV export started - Range: %s to %s", 
         startDate.c_str(), endDate.c_str());
ESP_LOGI(TAG, "Queried %d records from storage", recordCount);
ESP_LOGI(TAG, "CSV export completed - %d records sent, %d bytes transferred", 
         recordCount, bytesTransferred);
ESP_LOGW(TAG, "CSV export hit record limit (%d)", MAX_RECORDS);
```

## Migration Path

1. **Phase 1**: Use current mock data implementation for testing
2. **Phase 2**: Implement log file reading
3. **Phase 3**: Add StorageManager integration
4. **Phase 4**: Implement database queries (if needed)
5. **Phase 5**: Add caching and optimization

## Documentation Updates

When integration is complete, update:
- CSV_EXPORT_README.md - Update "Production Integration" section
- API documentation - Add actual data source details
- User guide - Update usage examples with real data
