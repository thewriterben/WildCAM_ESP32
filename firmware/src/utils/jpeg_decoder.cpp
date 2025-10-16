/**
 * @file jpeg_decoder.cpp
 * @brief JPEG decoding functionality implementation
 */

#include "jpeg_decoder.h"
#include <TJpg_Decoder.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <cstring>
#include <cmath>

static const char* TAG = "JPEGDecoder";

// TJpgDec callback to receive decoded output
static uint8_t* g_output_buffer = nullptr;
static int g_output_width = 0;
static int g_output_height = 0;
static int g_current_line = 0;

/**
 * @brief TJpgDec output callback function
 * 
 * This callback is called by TJpgDec for each block of decoded pixels.
 * It copies the RGB data to our output buffer.
 */
bool tjpgd_output_callback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!g_output_buffer) return false;
    
    // Convert RGB565 to RGB888
    for (int row = 0; row < h; row++) {
        int dest_y = y + row;
        if (dest_y >= g_output_height) break;
        
        for (int col = 0; col < w; col++) {
            int dest_x = x + col;
            if (dest_x >= g_output_width) break;
            
            uint16_t pixel = bitmap[row * w + col];
            
            // Extract RGB565 components
            uint8_t r = ((pixel >> 11) & 0x1F) << 3; // 5 bits to 8 bits
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;  // 6 bits to 8 bits
            uint8_t b = (pixel & 0x1F) << 3;         // 5 bits to 8 bits
            
            // Store as RGB888
            int dest_idx = (dest_y * g_output_width + dest_x) * 3;
            g_output_buffer[dest_idx] = r;
            g_output_buffer[dest_idx + 1] = g;
            g_output_buffer[dest_idx + 2] = b;
        }
    }
    
    return true;
}

bool decodeJPEG(const uint8_t* jpeg_data, size_t size, uint8_t** rgb_output, 
                int* width, int* height) {
    // Validate input parameters
    if (!jpeg_data || size == 0 || !rgb_output || !width || !height) {
        ESP_LOGE(TAG, "Invalid parameters for JPEG decoding");
        return false;
    }
    
    ESP_LOGI(TAG, "Decoding JPEG image (%zu bytes)", size);
    
    // Initialize TJpgDec
    TJpgDec.setCallback(tjpgd_output_callback);
    
    // Get image dimensions first
    uint16_t img_width, img_height;
    if (TJpgDec.getJpgSize(&img_width, &img_height, jpeg_data, size) != JDR_OK) {
        ESP_LOGE(TAG, "Failed to get JPEG dimensions");
        return false;
    }
    
    ESP_LOGI(TAG, "JPEG dimensions: %dx%d", img_width, img_height);
    
    // Allocate output buffer for RGB888 (3 bytes per pixel)
    size_t buffer_size = img_width * img_height * 3;
    uint8_t* buffer = static_cast<uint8_t*>(
        heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
    );
    
    // Fall back to internal RAM if PSRAM allocation fails
    if (!buffer) {
        ESP_LOGW(TAG, "PSRAM allocation failed, trying internal RAM");
        buffer = static_cast<uint8_t*>(
            heap_caps_malloc(buffer_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
        );
    }
    
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes for RGB output", buffer_size);
        return false;
    }
    
    // Initialize buffer to zero
    memset(buffer, 0, buffer_size);
    
    // Set up global state for callback
    g_output_buffer = buffer;
    g_output_width = img_width;
    g_output_height = img_height;
    g_current_line = 0;
    
    // Decode the JPEG
    int result = TJpgDec.drawJpg(0, 0, jpeg_data, size);
    
    // Clear global state
    g_output_buffer = nullptr;
    g_output_width = 0;
    g_output_height = 0;
    g_current_line = 0;
    
    if (result != JDR_OK) {
        ESP_LOGE(TAG, "JPEG decoding failed with error code: %d", result);
        heap_caps_free(buffer);
        return false;
    }
    
    // Set output parameters
    *rgb_output = buffer;
    *width = img_width;
    *height = img_height;
    
    ESP_LOGI(TAG, "JPEG decoded successfully: %dx%d RGB888", img_width, img_height);
    
    return true;
}

void freeDecodedBuffer(uint8_t* buffer) {
    if (buffer) {
        heap_caps_free(buffer);
    }
}

bool resizeImageBilinear(const uint8_t* input_data, int input_width, int input_height,
                         int target_width, int target_height, uint8_t** output_data) {
    // Validate input parameters
    if (!input_data || !output_data) {
        ESP_LOGE(TAG, "Invalid parameters for image resize");
        return false;
    }
    
    if (input_width <= 0 || input_height <= 0 || target_width <= 0 || target_height <= 0) {
        ESP_LOGE(TAG, "Invalid dimensions for resize: input=%dx%d, target=%dx%d",
                 input_width, input_height, target_width, target_height);
        return false;
    }
    
    ESP_LOGI(TAG, "Resizing image from %dx%d to %dx%d using bilinear interpolation",
             input_width, input_height, target_width, target_height);
    
    // Allocate output buffer for RGB888
    size_t output_size = target_width * target_height * 3;
    uint8_t* output = static_cast<uint8_t*>(
        heap_caps_malloc(output_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
    );
    
    // Fall back to internal RAM if PSRAM allocation fails
    if (!output) {
        ESP_LOGW(TAG, "PSRAM allocation failed, trying internal RAM");
        output = static_cast<uint8_t*>(
            heap_caps_malloc(output_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
        );
    }
    
    if (!output) {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes for resized output", output_size);
        return false;
    }
    
    // Calculate scaling factors
    float x_ratio = (float)(input_width - 1) / (target_width - 1);
    float y_ratio = (float)(input_height - 1) / (target_height - 1);
    
    // Perform bilinear interpolation
    for (int y = 0; y < target_height; y++) {
        float src_y = y * y_ratio;
        int y1 = (int)src_y;
        int y2 = (y1 + 1 < input_height) ? y1 + 1 : y1;
        float y_weight = src_y - y1;
        
        for (int x = 0; x < target_width; x++) {
            float src_x = x * x_ratio;
            int x1 = (int)src_x;
            int x2 = (x1 + 1 < input_width) ? x1 + 1 : x1;
            float x_weight = src_x - x1;
            
            // Process each color channel (R, G, B)
            for (int c = 0; c < 3; c++) {
                // Get four neighboring pixels
                int idx_11 = (y1 * input_width + x1) * 3 + c;
                int idx_12 = (y1 * input_width + x2) * 3 + c;
                int idx_21 = (y2 * input_width + x1) * 3 + c;
                int idx_22 = (y2 * input_width + x2) * 3 + c;
                
                float p11 = input_data[idx_11];
                float p12 = input_data[idx_12];
                float p21 = input_data[idx_21];
                float p22 = input_data[idx_22];
                
                // Bilinear interpolation
                float value = (1 - x_weight) * (1 - y_weight) * p11 +
                             x_weight * (1 - y_weight) * p12 +
                             (1 - x_weight) * y_weight * p21 +
                             x_weight * y_weight * p22;
                
                // Clamp and store result
                int out_idx = (y * target_width + x) * 3 + c;
                output[out_idx] = (uint8_t)(value + 0.5f); // Round to nearest
            }
        }
    }
    
    *output_data = output;
    ESP_LOGI(TAG, "Image resized successfully");
    
    return true;
}
