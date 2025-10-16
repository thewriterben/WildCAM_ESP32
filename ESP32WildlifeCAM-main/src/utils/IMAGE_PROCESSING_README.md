# Image Processing Module - JPEG Decoding and Image Resizing

## Overview

This module provides JPEG decoding and image processing functionality for the ESP32WildlifeCAM project. It includes:

- **JPEG Decoding**: Using the TJpgDec library for efficient JPEG decompression on ESP32
- **Image Resizing**: High-quality bilinear interpolation for AI preprocessing and thumbnail generation
- **Memory Management**: Safe allocation and deallocation of image buffers

## Features

### JPEG Decoding

The `decodeJPEG()` function decodes JPEG images to RGB888 format:

```cpp
#include "utils/image_utils.h"

uint16_t width, height;
uint8_t* rgbData = nullptr;

bool success = ImageUtils::decodeJPEG(
    jpegData,    // JPEG data buffer
    jpegSize,    // Size of JPEG data
    &width,      // Output: image width
    &height,     // Output: image height
    &rgbData     // Output: RGB buffer (caller must free)
);

if (success) {
    // Use the RGB data
    // rgbData contains width * height * 3 bytes (RGB888)
    
    // Free when done
    ImageUtils::freeDecodedBuffer(rgbData);
}
```

**Key Features:**
- Decodes JPEG images of any size (limited by available memory)
- Outputs RGB888 format (3 bytes per pixel)
- Uses PSRAM when available for large images
- Handles decoding errors gracefully
- Returns actual dimensions of decoded image

### Image Resizing with Bilinear Interpolation

The `scaleImage()` function resizes images using bilinear interpolation:

```cpp
// Source image: 640x480 RGB
uint8_t* srcImage = ...; // 640 * 480 * 3 bytes

// Allocate destination buffer: 224x224 RGB
uint16_t dstWidth = 224;
uint16_t dstHeight = 224;
uint8_t* dstImage = (uint8_t*)malloc(dstWidth * dstHeight * 3);

bool success = ImageUtils::scaleImage(
    srcImage, 640, 480,      // Source image and dimensions
    dstImage, dstWidth, dstHeight  // Destination buffer and dimensions
);

if (success) {
    // Use resized image
}

free(dstImage);
```

**Key Features:**
- High-quality bilinear interpolation algorithm
- Supports both upscaling and downscaling
- Smooth gradients and reduced aliasing
- Optimized for AI model input preparation
- Suitable for thumbnail generation

### Complete Preprocessing Pipeline

For AI models, use the complete preprocessing pipeline:

```cpp
camera_fb_t* frame = esp_camera_fb_get();

ImageUtils::PreprocessResult result = ImageUtils::preprocessFrameForModel(
    frame,
    224,  // Target width (model input)
    224   // Target height (model input)
);

if (result.success) {
    // result.tensorData contains normalized float data [-1, 1]
    // result.tensorSize is the number of float values
    // Feed to TensorFlow Lite model
    
    ImageUtils::freePreprocessResult(result);
}

esp_camera_fb_return(frame);
```

## API Reference

### Functions

#### `bool decodeJPEG(const uint8_t* jpegData, size_t jpegSize, uint16_t* outWidth, uint16_t* outHeight, uint8_t** outRgb)`

Decodes a JPEG image to RGB888 format.

**Parameters:**
- `jpegData`: Pointer to JPEG data buffer
- `jpegSize`: Size of JPEG data in bytes
- `outWidth`: Pointer to receive image width
- `outHeight`: Pointer to receive image height
- `outRgb`: Pointer to receive allocated RGB buffer (must be freed by caller)

**Returns:** `true` on success, `false` on error

**Notes:**
- Allocates memory using `heap_caps_malloc()` with PSRAM preference
- Caller must free the RGB buffer using `freeDecodedBuffer()`
- RGB data is in RGB888 format: `[R, G, B, R, G, B, ...]`

---

#### `void freeDecodedBuffer(uint8_t* rgbBuffer)`

Frees memory allocated by `decodeJPEG()`.

**Parameters:**
- `rgbBuffer`: Pointer to RGB buffer to free (can be NULL)

**Notes:**
- Safe to call with NULL pointer
- Should be used to free buffers allocated by `decodeJPEG()`

---

#### `bool scaleImage(const uint8_t* srcRgb, uint16_t srcWidth, uint16_t srcHeight, uint8_t* dstRgb, uint16_t dstWidth, uint16_t dstHeight)`

Scales an RGB888 image using bilinear interpolation.

**Parameters:**
- `srcRgb`: Source RGB data
- `srcWidth`: Source image width
- `srcHeight`: Source image height
- `dstRgb`: Destination RGB buffer (must be pre-allocated)
- `dstWidth`: Target width
- `dstHeight`: Target height

**Returns:** `true` on success, `false` on error

**Notes:**
- Destination buffer must be pre-allocated: `dstWidth * dstHeight * 3` bytes
- Uses bilinear interpolation for high quality
- Works for both upscaling and downscaling

---

#### `PreprocessResult preprocessFrameForModel(camera_fb_t* frame, uint16_t targetWidth, uint16_t targetHeight)`

Complete preprocessing pipeline: decode JPEG → scale → normalize.

**Parameters:**
- `frame`: Camera frame buffer (JPEG format)
- `targetWidth`: Target width for model input
- `targetHeight`: Target height for model input

**Returns:** `PreprocessResult` structure with:
- `success`: true if preprocessing succeeded
- `tensorData`: Normalized float data [-1, 1]
- `tensorSize`: Number of float values
- `processingTime`: Time taken in milliseconds
- `errorMessage`: Error description if failed

**Notes:**
- Output is ready for TensorFlow Lite models
- Free result using `freePreprocessResult()`

## Implementation Details

### TJpgDec Library

The module uses the [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder) library for JPEG decoding:

- Optimized for embedded systems
- Low memory footprint
- Supports progressive and baseline JPEG
- Outputs RGB565, which we convert to RGB888

### Bilinear Interpolation Algorithm

The bilinear interpolation algorithm provides smooth scaling:

```
For each destination pixel (x, y):
  1. Calculate corresponding source position (sx, sy) as floating-point
  2. Find four nearest source pixels: (x1,y1), (x2,y1), (x1,y2), (x2,y2)
  3. Interpolate horizontally between top two pixels
  4. Interpolate horizontally between bottom two pixels
  5. Interpolate vertically between the two horizontal results
```

This produces much better quality than nearest-neighbor interpolation, especially for:
- Downscaling (reduces aliasing and pixelation)
- Upscaling (smoother gradients)
- AI preprocessing (better feature preservation)

### Memory Management

The module uses ESP32's memory allocation capabilities:

- **JPEG Decoding**: Uses `MALLOC_CAP_SPIRAM` preferentially for large buffers
- **Scaling**: Uses standard heap for intermediate buffers
- **Error Handling**: All allocations are checked, resources freed on error

**Best Practices:**
```cpp
// Always check return values
if (ImageUtils::decodeJPEG(...)) {
    // Success - use the data
    
    // Always free when done
    ImageUtils::freeDecodedBuffer(rgbData);
}
```

## Testing

The module includes comprehensive unit tests in `test/test_image_processing.cpp`:

### Test Coverage

- ✅ Valid JPEG decoding
- ✅ Invalid input handling (null pointers, zero sizes)
- ✅ Memory management (no leaks)
- ✅ Image upscaling and downscaling
- ✅ Bilinear interpolation quality
- ✅ Edge cases (zero dimensions, boundary conditions)

### Running Tests

Using PlatformIO:

```bash
# Run tests on native platform
pio test -e native

# Run tests on ESP32 hardware
pio test -e esp32cam --upload-port /dev/ttyUSB0
```

## Performance

Typical performance on ESP32-CAM (AI-Thinker):

| Operation | Input Size | Output Size | Time |
|-----------|------------|-------------|------|
| JPEG Decode | 640x480 JPEG | 640x480 RGB | ~150-200ms |
| Bilinear Scale | 640x480 RGB | 224x224 RGB | ~80-100ms |
| Complete Pipeline | 640x480 JPEG | 224x224 Tensor | ~250-300ms |

**Optimization Tips:**
- Use lower JPEG quality for faster decoding
- Resize directly from camera resolution to model size
- Consider using ESP32-S3 for faster processing (dual-core, higher clock)

## Usage Examples

### Example 1: Decode Camera Frame

```cpp
#include "utils/image_utils.h"

void processCameraFrame() {
    camera_fb_t* frame = esp_camera_fb_get();
    if (!frame) {
        Serial.println("Failed to capture frame");
        return;
    }
    
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    if (ImageUtils::decodeJPEG(frame->buf, frame->len, &width, &height, &rgbData)) {
        Serial.printf("Decoded %dx%d image\n", width, height);
        
        // Process RGB data here...
        
        ImageUtils::freeDecodedBuffer(rgbData);
    }
    
    esp_camera_fb_return(frame);
}
```

### Example 2: Create Thumbnail

```cpp
void createThumbnail(const uint8_t* jpegData, size_t jpegSize) {
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    // Decode JPEG
    if (!ImageUtils::decodeJPEG(jpegData, jpegSize, &width, &height, &rgbData)) {
        return;
    }
    
    // Create 128x128 thumbnail
    const uint16_t thumbSize = 128;
    uint8_t* thumbnail = (uint8_t*)malloc(thumbSize * thumbSize * 3);
    
    if (ImageUtils::scaleImage(rgbData, width, height, thumbnail, thumbSize, thumbSize)) {
        // Save or transmit thumbnail
        saveThumbnail(thumbnail, thumbSize, thumbSize);
    }
    
    free(thumbnail);
    ImageUtils::freeDecodedBuffer(rgbData);
}
```

### Example 3: Prepare for AI Model

```cpp
void classifyWildlife() {
    camera_fb_t* frame = esp_camera_fb_get();
    
    // Preprocess for MobileNet (224x224 input)
    ImageUtils::PreprocessResult result = 
        ImageUtils::preprocessFrameForModel(frame, 224, 224);
    
    if (result.success) {
        // Feed to TensorFlow Lite model
        tflite_model->input(0)->data.f = result.tensorData;
        tflite_interpreter->Invoke();
        
        // Get predictions...
        
        ImageUtils::freePreprocessResult(result);
    }
    
    esp_camera_fb_return(frame);
}
```

## Dependencies

- **TJpg_Decoder** (`bodmer/TJpg_Decoder@^1.0.8`): JPEG decoding
- **esp_camera**: ESP32 camera driver
- **esp_heap_caps**: ESP32 memory allocation

## Future Enhancements

Potential improvements for future versions:

- [ ] Hardware-accelerated JPEG decoding (ESP32-S3)
- [ ] Additional interpolation methods (bicubic, Lanczos)
- [ ] Color space conversions (RGB ↔ YUV)
- [ ] Image filters (sharpen, blur, edge detection)
- [ ] SIMD optimizations for faster scaling
- [ ] Support for PNG/BMP formats

## License

This module is part of the ESP32WildlifeCAM project.

## References

- [TJpg_Decoder Library](https://github.com/Bodmer/TJpg_Decoder)
- [Bilinear Interpolation](https://en.wikipedia.org/wiki/Bilinear_interpolation)
- [ESP32 Camera Driver](https://github.com/espressif/esp32-camera)
