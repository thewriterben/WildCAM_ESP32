/**
 * @file jpeg_decoder.h
 * @brief JPEG decoding functionality for image processing
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides JPEG decompression using TJpgDec library for AI preprocessing
 * and thumbnail generation. Handles memory allocation and error management.
 */

#ifndef JPEG_DECODER_H
#define JPEG_DECODER_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Decode JPEG image data to RGB888 format
 * 
 * Decodes a JPEG compressed image into raw RGB888 format suitable for
 * AI preprocessing and image manipulation. Allocates memory for the
 * output buffer which must be freed using freeDecodedBuffer().
 * 
 * @param jpeg_data Pointer to JPEG compressed data
 * @param size Size of JPEG data in bytes
 * @param rgb_output Pointer to receive allocated RGB buffer (must be freed)
 * @param width Pointer to receive decoded image width
 * @param height Pointer to receive decoded image height
 * @return true if decoding successful, false on error
 * 
 * @note The caller is responsible for freeing the rgb_output buffer using
 *       freeDecodedBuffer() when finished with the data.
 * 
 * @example
 * uint8_t* rgb_buffer = nullptr;
 * int width, height;
 * if (decodeJPEG(jpeg_data, jpeg_size, &rgb_buffer, &width, &height)) {
 *     // Process RGB buffer
 *     freeDecodedBuffer(rgb_buffer);
 * }
 */
bool decodeJPEG(const uint8_t* jpeg_data, size_t size, uint8_t** rgb_output, 
                int* width, int* height);

/**
 * @brief Free memory allocated by decodeJPEG()
 * 
 * Releases the RGB buffer allocated during JPEG decoding. This must be
 * called for every successful decodeJPEG() call to prevent memory leaks.
 * 
 * @param buffer Pointer to RGB buffer from decodeJPEG()
 * 
 * @note It is safe to call this function with nullptr
 */
void freeDecodedBuffer(uint8_t* buffer);

/**
 * @brief Resize image using bilinear interpolation
 * 
 * Resizes an RGB888 image to target dimensions using high-quality bilinear
 * interpolation. Allocates memory for the output buffer which must be freed
 * using freeDecodedBuffer().
 * 
 * @param input_data Input RGB888 image data
 * @param input_width Input image width
 * @param input_height Input image height
 * @param target_width Desired output width
 * @param target_height Desired output height
 * @param output_data Pointer to receive allocated resized RGB buffer
 * @return true if resize successful, false on error
 * 
 * @note Uses bilinear interpolation for smooth results
 * @note The output buffer must be freed using freeDecodedBuffer()
 * 
 * @example
 * uint8_t* resized_buffer = nullptr;
 * if (resizeImageBilinear(rgb_data, 640, 480, 224, 224, &resized_buffer)) {
 *     // Use resized buffer for AI inference
 *     freeDecodedBuffer(resized_buffer);
 * }
 */
bool resizeImageBilinear(const uint8_t* input_data, int input_width, int input_height,
                         int target_width, int target_height, uint8_t** output_data);

#endif // JPEG_DECODER_H
