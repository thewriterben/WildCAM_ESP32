# Detection Pipeline Quick Reference

## Function Signatures

```cpp
// Generate unique timestamped filename
bool generateDetectionFilename(const char* species, char* buffer, size_t buffer_size);

// Save metadata JSON file
bool saveDetectionMetadata(const char* filename, const char* species, 
                          float confidence, const BoundingBox& bbox);

// Main detection processing orchestration
bool processWildlifeDetection(const uint8_t* image_data, size_t image_size, 
                              const BoundingBox& detection);

// Get persistent detection counter
uint32_t getDetectionCounter();
```

## Usage in AI Detection Loop

```cpp
// In aiProcessingTask()
if (num_detections > 0) {
    for (int i = 0; i < num_detections; i++) {
        // Log detection
        Logger::info("Wildlife detected: %s (confidence: %.2f)", 
                   detections[i].class_name, detections[i].confidence);
        
        // Process and save
        processWildlifeDetection(image_data, image_size, detections[i]);
    }
}
```

## Key Features

### Automatic Retry
- **3 attempts** for image and metadata saves
- **Exponential backoff**: 100ms → 200ms → 400ms
- **Graceful degradation** on persistent failure

### Storage Management
- Checks free space before saving
- Handles SD card full gracefully
- Falls back to LittleFS if SD unavailable

### Persistent Counter
```cpp
// Stored in NVS - survives reboots
uint32_t count = getDetectionCounter();  // Read
// Automatically incremented after successful save
```

### Metadata Fields
```json
{
  "filename": "20251016_143025_deer.jpg",
  "species": "deer",
  "confidence": 0.956,
  "timestamp": "2025-10-16 14:30:25",
  "timestamp_millis": 1234567890,
  "bounding_box": { "x": 0.25, "y": 0.35, "width": 0.30, "height": 0.40 },
  "class_id": 1,
  "battery_voltage": 4.12,
  "gps_coordinates": { "latitude": 0.0, "longitude": 0.0 }
}
```

## Error Handling

### Return Values
- `false`: Invalid input (stops processing)
- `true`: Success OR graceful degradation (continues detection)

### Common Scenarios
```cpp
// Storage full - continues detection
Insufficient storage space: 512KB free, 2MB required

// Save retry - automatic
Image save failed (attempt 1/3): Write timeout
Image save failed (attempt 2/3): Write timeout
Image saved successfully (attempt 3/3)

// Storage unavailable - continues detection
Storage not ready, cannot save detection image
```

## System Status

### Check Detection Stats
```
> status
Total Detections: 42
Storage Ready: YES
Storage Usage: 67.3% (1024 MB free)
```

## Configuration

### Storage Paths
- Images: `/wildlife/images/`
- Metadata: `/wildlife/logs/`

### Limits
- Max retries: 3
- Retry backoff: 100ms base (exponential)
- Filename buffer: 64 bytes
- Metadata buffer: 768 bytes

## Testing

### Run Tests
```bash
pio test -e test -f test_detection_pipeline
```

### Test Coverage
- Filename generation (valid, null, overflow)
- Metadata creation (all fields)
- Detection processing (valid, invalid)
- Storage failure handling
- Retry logic verification
- Counter persistence

## Troubleshooting

### Detection not saving
1. Check storage initialization: `g_storage.isReady()`
2. Check free space: `g_storage.getFreeSpace()`
3. Review logs for retry attempts

### Counter not incrementing
1. Verify NVS initialized in setup()
2. Check for STORAGE_SUCCESS before increment
3. Use `getDetectionCounter()` to verify

### Metadata missing fields
1. Ensure `system_state.battery_level` is updated
2. Check `getLocalTime()` success for timestamps

## Performance

| Operation | Duration |
|-----------|----------|
| Filename generation | <1ms |
| Storage space check | <10ms |
| Image save | 100-400ms |
| Metadata save | 10-50ms |
| **Total per detection** | **<500ms** |

## Memory Usage

- **Stack only** - No heap allocations
- Filename buffer: 64 bytes
- Metadata buffer: 768 bytes
- **Total**: ~832 bytes per detection

---

**See Also**: [DETECTION_PIPELINE_IMPLEMENTATION.md](DETECTION_PIPELINE_IMPLEMENTATION.md)
