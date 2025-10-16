# CSV Export Functionality

## Overview
The CSV export functionality allows users to download wildlife detection data from the ESP32 WildCAM web server in CSV format. This feature is designed for data analysis, reporting, and integration with external tools.

## Endpoint Details

### URL
```
GET /api/export/detections.csv
```

### Response
- **Content-Type**: `text/csv`
- **Content-Disposition**: `attachment; filename="detections_YYYYMMDD.csv"`
- **Status Codes**:
  - `200 OK`: Successful export
  - `400 Bad Request`: Invalid query parameters
  - `500 Internal Server Error`: Server-side error during export

### Query Parameters

| Parameter | Type | Required | Description | Example |
|-----------|------|----------|-------------|---------|
| `start` | String | No | Start date for filtering (ISO 8601) | `2025-01-01` |
| `end` | String | No | End date for filtering (ISO 8601) | `2025-12-31` |

## CSV Format

### Columns
The exported CSV contains the following columns:

1. **Timestamp**: Unix timestamp or ISO 8601 datetime string
2. **Species**: Detected species name (e.g., "deer", "fox", "raccoon")
3. **Confidence**: Detection confidence score (0.00 to 1.00)
4. **Image_Path**: Path to the captured image file
5. **GPS_Lat**: GPS latitude coordinate
6. **GPS_Lon**: GPS longitude coordinate
7. **Battery_Level**: Battery level percentage at time of detection

### Example CSV Output
```csv
Timestamp,Species,Confidence,Image_Path,GPS_Lat,GPS_Lon,Battery_Level
1729113600,deer,0.92,/images/wildlife_1729113600.jpg,45.500000,-122.700000,85
1729110000,fox,0.89,/images/wildlife_1729110000.jpg,45.501000,-122.701000,84
1729106400,raccoon,0.85,/images/wildlife_1729106400.jpg,45.502000,-122.702000,83
1729102800,bird,0.78,/images/wildlife_1729102800.jpg,45.503000,-122.703000,82
```

## Features

### ✅ Implemented

1. **Streaming Response**: Uses AsyncWebServer's chunked response for efficient memory usage
2. **CSV Escaping**: Properly escapes fields containing commas, quotes, or newlines
3. **Dynamic Filename**: Generates filename with current date (e.g., `detections_20251016.csv`)
4. **Record Limit**: Maximum 10,000 records per export to ensure performance
5. **Logging**: Logs export events and record counts for monitoring
6. **Query Parameter Parsing**: Accepts and parses date range query parameters
7. **Proper Headers**: Sets correct HTTP headers for CSV file download

### 🔄 Planned Enhancements

1. **Date Range Filtering**: Apply date range filters to actual data queries
2. **Database Integration**: Query real detection database instead of mock data
3. **Authentication**: Add authentication/authorization for exports
4. **Compression**: Optional gzip compression for large exports
5. **Multiple Formats**: Support JSON, XML, and other export formats

## Usage Examples

### Basic Export (Browser)
Simply navigate to the endpoint in your browser:
```
http://192.168.1.100/api/export/detections.csv
```
The browser will prompt you to download the file.

### Export with cURL
```bash
# Basic export
curl -O http://192.168.1.100/api/export/detections.csv

# Export with date range
curl -O "http://192.168.1.100/api/export/detections.csv?start=2025-01-01&end=2025-12-31"

# Save to specific filename
curl -o my_detections.csv http://192.168.1.100/api/export/detections.csv
```

### Export with wget
```bash
wget http://192.168.1.100/api/export/detections.csv
```

### Python Example
```python
import requests

# Basic export
response = requests.get('http://192.168.1.100/api/export/detections.csv')
with open('detections.csv', 'wb') as f:
    f.write(response.content)

# Export with date range
params = {'start': '2025-01-01', 'end': '2025-12-31'}
response = requests.get('http://192.168.1.100/api/export/detections.csv', params=params)
with open('detections_filtered.csv', 'wb') as f:
    f.write(response.content)
```

### JavaScript Example
```javascript
// Using fetch API
fetch('http://192.168.1.100/api/export/detections.csv')
  .then(response => response.blob())
  .then(blob => {
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'detections.csv';
    document.body.appendChild(a);
    a.click();
    window.URL.revokeObjectURL(url);
  });
```

## Implementation Details

### Architecture
The CSV export functionality is implemented in the `EnhancedWebServer` class:

- **Header File**: `ESP32WildlifeCAM-main/firmware/src/web/enhanced_web_server.h`
- **Implementation**: `ESP32WildlifeCAM-main/firmware/src/web/enhanced_web_server.cpp`

### Key Functions

#### `handleAPIExportDetectionsCSV(AsyncWebServerRequest* request)`
Main endpoint handler that:
1. Parses query parameters
2. Sets up chunked response with CSV headers
3. Streams detection data in CSV format
4. Enforces 10,000 record limit
5. Logs export events

#### `escapeCSVField(const String& field)`
Escapes CSV fields according to RFC 4180:
- Wraps fields containing commas, quotes, or newlines in double quotes
- Escapes double quotes by doubling them (`"` becomes `""`)

#### `generateCSVRow(...)`
Generates a single CSV row from detection data:
- Takes individual field values
- Applies CSV escaping
- Returns formatted CSV row string

### Memory Management
The implementation uses chunked responses to minimize memory usage:
- Data is streamed in chunks rather than building the entire CSV in memory
- Each chunk contains approximately 50 records
- Efficient for large datasets and ESP32's limited RAM

### Performance Considerations
- **Maximum Records**: 10,000 records per export
- **Chunk Size**: ~50 records per chunk
- **Memory Usage**: ~2-4 KB per chunk
- **Streaming**: No buffering of entire dataset

## Integration with Detection System

### Current Implementation
The current implementation generates mock detection data for demonstration purposes. This includes:
- Cycling through common species (deer, fox, raccoon, bird, squirrel, rabbit)
- Random confidence scores (0.70 - 0.95)
- Mock GPS coordinates
- Mock battery levels

### Production Integration
To integrate with actual detection data:

1. **Query Detection Database**:
   ```cpp
   // Replace mock data generation with database query
   StorageManager::DetectionRecord* records = 
       StorageManager::queryDetections(startDate, endDate, MAX_RECORDS);
   ```

2. **Read Detection Log Files**:
   ```cpp
   // Read from detection log files
   File logFile = SD_MMC.open("/logs/detections.log", FILE_READ);
   while (logFile.available() && recordCount < MAX_RECORDS) {
       DetectionRecord record = parseLogLine(logFile.readStringUntil('\n'));
       // Generate CSV row from record
   }
   ```

3. **Use WildlifeClassifier History**:
   ```cpp
   // Access classification history
   if (g_wildlifeClassifier) {
       auto stats = g_wildlifeClassifier->getStatistics();
       // Generate CSV from statistics
   }
   ```

## Testing

### Manual Testing Checklist
- [ ] Basic export downloads successfully
- [ ] Filename includes current date in YYYYMMDD format
- [ ] CSV contains proper header row
- [ ] CSV data is properly formatted
- [ ] Fields with special characters are escaped correctly
- [ ] Date range parameters are parsed (logs show filter values)
- [ ] Export is limited to 10,000 records maximum
- [ ] Export events are logged
- [ ] Proper Content-Type header is set
- [ ] Content-Disposition triggers file download

### Automated Testing
For automated testing, implement unit tests that:
1. Verify CSV escaping logic
2. Test row generation with various input combinations
3. Validate query parameter parsing
4. Check record limit enforcement
5. Verify header generation

## Security Considerations

### Current Implementation
- No authentication required
- No rate limiting
- Public endpoint

### Recommended Security Measures
1. **Authentication**: Add authentication check before allowing export
   ```cpp
   if (!authenticateRequest(request)) {
       request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
       return;
   }
   ```

2. **Rate Limiting**: Implement rate limiting to prevent abuse
3. **Input Validation**: Validate date parameters to prevent injection
4. **Access Logging**: Log all export requests with IP addresses
5. **HTTPS**: Use HTTPS for encrypted data transmission

## Troubleshooting

### Common Issues

#### Export Returns Empty File
- Check if detection data exists in the system
- Verify date range parameters are correct
- Check ESP32 logs for error messages

#### CSV Contains Garbled Data
- Verify character encoding (should be UTF-8)
- Check for proper CSV escaping
- Ensure AsyncWebServer is configured correctly

#### Browser Doesn't Download File
- Verify Content-Disposition header is set correctly
- Check browser console for errors
- Try with a different browser or curl

#### Export Fails or Times Out
- Check ESP32 memory usage
- Verify WiFi connection stability
- Reduce record limit if needed
- Check for AsyncWebServer task watchdog issues

## Logging

The CSV export functionality logs the following events:

```
ESP_LOGI: "CSV export request received"
ESP_LOGI: "Start date filter: YYYY-MM-DD"
ESP_LOGI: "End date filter: YYYY-MM-DD"
ESP_LOGI: "Starting CSV export as file: detections_YYYYMMDD.csv"
ESP_LOGI: "CSV export completed with N records"
```

Monitor these logs to track export usage and debug issues.

## API Documentation

For integration with the main API documentation, add the following to your API docs:

```yaml
/api/export/detections.csv:
  get:
    summary: Export detection data as CSV
    description: Downloads wildlife detection data in CSV format
    parameters:
      - name: start
        in: query
        required: false
        schema:
          type: string
          format: date
        description: Start date for filtering (YYYY-MM-DD)
      - name: end
        in: query
        required: false
        schema:
          type: string
          format: date
        description: End date for filtering (YYYY-MM-DD)
    responses:
      200:
        description: Successful export
        content:
          text/csv:
            schema:
              type: string
              format: binary
        headers:
          Content-Disposition:
            schema:
              type: string
              example: attachment; filename="detections_20251016.csv"
```

## Contributing

When contributing to the CSV export functionality:

1. **Maintain Backward Compatibility**: Don't break existing CSV format
2. **Add Tests**: Include unit tests for new features
3. **Update Documentation**: Keep this README up to date
4. **Follow CSV RFC 4180**: Adhere to CSV standard for escaping
5. **Consider Memory**: Keep memory usage minimal for ESP32
6. **Log Events**: Add appropriate logging for debugging

## Related Files

- `enhanced_web_server.h`: Header file with declarations
- `enhanced_web_server.cpp`: Implementation file
- `storage_manager.h`: Storage management interface (future integration)
- `wildlife_classifier.h`: Wildlife classification interface (future integration)

## References

- [RFC 4180 - Common Format and MIME Type for CSV Files](https://tools.ietf.org/html/rfc4180)
- [AsyncWebServer Documentation](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

## Changelog

### Version 1.0.0 (2025-10-16)
- Initial implementation of CSV export endpoint
- Streaming response for efficient memory usage
- Proper CSV escaping and formatting
- Date range query parameters
- 10,000 record limit
- Dynamic filename generation
- Export event logging
