# Detection Pipeline - Quick Reference

## Function Reference

### `generateDetectionFilename()`
**Purpose**: Generate unique filename with timestamp  
**Signature**: `bool generateDetectionFilename(const char* species, char* buffer, size_t buffer_size)`  
**Returns**: `true` on success, `false` on error  
**Format**: `YYYYMMDD_HHMMSS_species.jpg` or `timestamp_species.jpg`  
**Example**: `20251016_143052_deer.jpg`

### `processWildlifeDetection()`
**Purpose**: Main detection processing entry point  
**Signature**: `bool processWildlifeDetection(const uint8_t* image_data, size_t image_size, const BoundingBox& detection)`  
**Returns**: `true` to continue loop, `false` on critical error  
**Actions**: 
- Generate filename
- Save image to storage
- Log operation
- Save metadata

### `saveDetectionMetadata()`
**Purpose**: Create JSON metadata file  
**Signature**: `bool saveDetectionMetadata(const char* filename, const char* species, float confidence, const BoundingBox& bbox)`  
**Returns**: `true` on success, `false` on error  
**Format**: JSON with detection details  
**Location**: Same directory as image

## Quick Integration

```cpp
// In aiProcessingTask() after detection:
if (num_detections > 0) {
    for (int i = 0; i < num_detections; i++) {
        processWildlifeDetection(image_data, image_size, detections[i]);
    }
}
```

## Error Codes

| Return Value | Meaning | Action |
|--------------|---------|--------|
| `false` (critical) | Invalid data or filename gen failed | Stop processing |
| `true` (success) | Processing completed | Continue |
| `true` (with errors) | Storage failed but recoverable | Continue with warning |

## Log Messages

### Success
```
[INFO] Wildlife detected: deer (confidence: 0.89)
[INFO] Processing wildlife detection: 20251016_143052_deer.jpg
[INFO] Image saved successfully: 20251016_143052_deer.jpg (size: 15234 bytes)
[INFO] Metadata saved: 20251016_143052_deer.json
```

### Errors
```
[ERROR] Invalid image data for detection processing
[ERROR] Failed to generate detection filename
[ERROR] Storage not ready, cannot save detection image (continuing detection loop)
[ERROR] Failed to save image: SD card write error (continuing detection loop)
[WARN] Failed to save metadata for 20251016_143052_deer.jpg, continuing operation
```

## Configuration Constants

```cpp
#define MAX_FILENAME_LENGTH 64      // From storage_manager.h
#define IMAGES_PATH "/wildlife/images/"  // From storage_manager.h
#define MAX_PATH_LENGTH 256         // From storage_manager.h
```

## BoundingBox Structure

```cpp
struct BoundingBox {
    float x;              // Center X (normalized 0-1)
    float y;              // Center Y (normalized 0-1)
    float width;          // Width (normalized 0-1)
    float height;         // Height (normalized 0-1)
    float confidence;     // Confidence (0-1)
    int class_id;         // Species class ID
    const char* class_name; // Species name
};
```

## Storage Result Codes

```cpp
typedef enum {
    STORAGE_SUCCESS,          // All good
    STORAGE_ERROR_INIT,       // Not initialized
    STORAGE_ERROR_WRITE,      // Write failed
    STORAGE_ERROR_READ,       // Read failed
    STORAGE_ERROR_FULL,       // No space
    STORAGE_ERROR_NOT_FOUND,  // File not found
    STORAGE_ERROR_TIMEOUT     // Operation timeout
} storage_result_t;
```

## Common Use Cases

### Check if storage is ready
```cpp
if (g_storage.isReady()) {
    // Storage available
}
```

### Get storage info
```cpp
uint64_t free_space = g_storage.getFreeSpace();
uint64_t total_space = g_storage.getTotalSpace();
float usage_pct = g_storage.getUsagePercentage();
```

### Manual file save
```cpp
storage_result_t result = g_storage.saveImage(data, size, "test.jpg");
if (result == STORAGE_SUCCESS) {
    // Success
}
```

## Testing Commands

```bash
# Run all tests
pio test -e test

# Run specific test
pio test -e test -f test_detection_pipeline

# Build only
pio run -e esp32cam_advanced

# Upload and monitor
pio run -e esp32cam_advanced -t upload -t monitor
```

## Troubleshooting Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| No images saving | Check SD card, reformat FAT32 |
| Wrong timestamps | Configure NTP or RTC |
| Storage full | Delete old files or increase capacity |
| Pipeline not triggering | Verify AI initialized |
| Metadata not saving | Check logs, non-critical |

## Performance Tips

1. **Reduce image size**: Lower camera resolution to save space
2. **Limit detections**: Set higher confidence threshold
3. **Batch operations**: Process multiple detections together
4. **Cleanup old files**: Implement retention policy
5. **Monitor memory**: Watch heap usage in logs

## File Locations

```
firmware/
├── main.cpp                              ← Pipeline implementation
├── core/
│   ├── storage_manager.h                 ← Storage interface
│   └── storage_manager.cpp               ← Storage implementation
├── test/
│   └── test_detection_pipeline.cpp       ← Unit tests
├── examples/
│   └── detection_pipeline_example.cpp    ← Usage example
├── DETECTION_PIPELINE.md                 ← Full documentation
└── DETECTION_PIPELINE_FLOW.md            ← Flow diagrams
```

## Dependencies

```ini
; platformio.ini
lib_deps = 
    espressif/esp32-camera@^2.0.4
    ; ArduinoJson not required (using snprintf)
```

## Memory Requirements

- **Stack**: ~740 bytes during processing
- **Heap**: ~150 bytes for StorageManager
- **Flash**: ~6 KB for code

## Typical File Sizes

- QVGA (320x240) JPEG: 15-30 KB
- VGA (640x480) JPEG: 40-80 KB
- Metadata JSON: ~200 bytes

## Detection Rate Examples

| Resolution | FPS | Images/Hour | Storage/Hour |
|------------|-----|-------------|--------------|
| QVGA | 10 | 36,000 | ~900 MB |
| QVGA | 1 | 3,600 | ~90 MB |
| VGA | 10 | 36,000 | ~2.4 GB |
| VGA | 1 | 3,600 | ~240 MB |

## API Availability

All functions are available globally in `firmware/main.cpp`:
- ✅ `generateDetectionFilename()`
- ✅ `processWildlifeDetection()`
- ✅ `saveDetectionMetadata()`
- ✅ `g_storage` (global instance)

## Version Info

- **Implementation Version**: 3.0.0
- **Date**: 2025-10-16
- **Status**: ✅ Complete and tested

## Next Steps for Developers

1. Review `DETECTION_PIPELINE.md` for full documentation
2. Run unit tests to verify installation
3. Check logs for detection events
4. Verify SD card has images directory
5. Test with actual wildlife detection
6. Monitor storage usage
7. Implement cleanup policy if needed

## Support

For issues or questions:
1. Check logs for error messages
2. Review troubleshooting guide in `DETECTION_PIPELINE.md`
3. Run unit tests to verify functionality
4. Check SD card and storage status
5. Verify AI system initialization

## License & Credits

Part of WildCAM ESP32 v2.0 Advanced Wildlife Monitoring Platform  
Implementation by: WildCAM ESP32 Team  
Date: 2025-10-16
