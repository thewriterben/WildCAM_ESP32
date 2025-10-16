# Wildlife Detection Processing Pipeline

## Overview
The wildlife detection processing pipeline automatically saves detected wildlife images and metadata when the AI system identifies animals. This feature is implemented in `firmware/main.cpp` and integrates with the YOLO-tiny AI detector.

## Features

### 1. Automatic Image Saving
- **Location**: Images are saved to `/wildlife/images/` on SD card or LittleFS
- **Format**: JPEG images captured from the camera
- **Filename**: `YYYYMMDD_HHMMSS_species.jpg` (e.g., `20251016_143052_deer.jpg`)

### 2. Metadata Files
Each saved image has an accompanying JSON metadata file containing:
- **filename**: Original image filename
- **species**: Detected species name
- **confidence**: AI detection confidence (0.0-1.0)
- **timestamp**: System timestamp in milliseconds
- **bounding_box**: Normalized coordinates (x, y, width, height as 0.0-1.0)
- **class_id**: Numeric species class identifier

Example metadata file (`20251016_143052_deer.json`):
```json
{
  "filename": "20251016_143052_deer.jpg",
  "species": "deer",
  "confidence": 0.895,
  "timestamp": 3456789,
  "bounding_box": {
    "x": 0.450,
    "y": 0.320,
    "width": 0.280,
    "height": 0.450
  },
  "class_id": 1
}
```

### 3. Robust Error Handling
- Storage failures are logged but don't interrupt detection
- Continues operation even when SD card is unavailable
- Falls back to LittleFS if SD card fails
- Metadata save failures are non-critical

## Storage Requirements

### SD Card Setup (Recommended)
1. Format SD card as FAT32
2. Insert into ESP32-CAM SD slot
3. System automatically creates `/wildlife/images/` directory

### LittleFS Fallback
- Used when SD card unavailable
- Limited space (~1.5MB typical)
- Automatically formatted if needed

## Configuration

The detection pipeline uses the following defaults:
- **Storage path**: `/wildlife/images/`
- **Metadata format**: JSON
- **Timestamp source**: RTC/NTP (falls back to millis())

## Usage

The pipeline automatically activates when:
1. AI system is initialized (`system_state.ai_initialized == true`)
2. Camera captures a frame
3. YOLO detector identifies wildlife
4. Confidence exceeds threshold

No manual intervention required - it's fully automatic!

## Monitoring

Check logs for detection events:
```
[INFO] Wildlife detected: deer (confidence: 0.89)
[INFO] Processing wildlife detection: 20251016_143052_deer.jpg
[INFO] Image saved successfully: 20251016_143052_deer.jpg (size: 15234 bytes)
[INFO] Metadata saved: 20251016_143052_deer.json
```

## Error Messages

Common error messages and solutions:

| Error | Cause | Solution |
|-------|-------|----------|
| "Storage not ready" | SD card not detected | Check SD card connection |
| "Failed to save image" | Storage full or write error | Free up space or format SD card |
| "Failed to save metadata" | Metadata write failure | Check storage, non-critical error |
| "Failed to generate filename" | Buffer overflow | Internal error, check logs |

## Performance

- **Processing time**: ~50-100ms per detection
- **Storage overhead**: ~15-30KB per image + ~200 bytes metadata
- **Detection rate**: Up to 10 FPS when AI enabled

## Troubleshooting

### No images saving
1. Check SD card is properly inserted
2. Verify SD card is formatted (FAT32)
3. Check free space on SD card
4. Review serial logs for error messages

### Only metadata saving (no images)
1. Image size may exceed available memory
2. Check PSRAM availability
3. Verify camera initialization

### Timestamps incorrect
1. Configure NTP server in WiFi settings
2. Or use RTC module for accurate time
3. Falls back to millis() if time unavailable

## Advanced Configuration

### Custom Storage Path
Modify `IMAGES_PATH` in `storage_manager.h`:
```cpp
#define IMAGES_PATH "/custom/path"
```

### Disable Metadata
Comment out metadata saving in `processWildlifeDetection()`:
```cpp
// if (!saveDetectionMetadata(...)) { ... }
```

### Change Filename Format
Modify `generateDetectionFilename()` function in `main.cpp`

## API Reference

### `generateDetectionFilename()`
```cpp
bool generateDetectionFilename(const char* species, char* buffer, size_t buffer_size)
```
Generates unique filename with timestamp.

### `processWildlifeDetection()`
```cpp
bool processWildlifeDetection(const uint8_t* image_data, size_t image_size, 
                              const BoundingBox& detection)
```
Main detection processing function - saves image and metadata.

### `saveDetectionMetadata()`
```cpp
bool saveDetectionMetadata(const char* filename, const char* species, 
                           float confidence, const BoundingBox& bbox)
```
Saves JSON metadata file for a detection.

## Testing

Run unit tests to verify functionality:
```bash
pio test -e test
```

Tests cover:
- Filename generation
- Metadata creation
- Error handling
- Multiple detections
- Storage failure scenarios

## Future Enhancements

Potential improvements for future versions:
- Configurable retention policies (auto-delete old files)
- Image compression options
- Remote sync to cloud storage
- Species-specific storage paths
- Detection statistics dashboard
- Thumbnail generation
