/**
 * @file yolo_tiny_detector.h
 * @brief YOLO-tiny implementation for ESP32 real-time object detection
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

#ifndef YOLO_TINY_DETECTOR_H
#define YOLO_TINY_DETECTOR_H

#include <Arduino.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_log.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>

/**
 * @brief YOLO-tiny detection bounding box structure
 */
struct BoundingBox {
    float x;        // Center X coordinate (normalized 0-1)
    float y;        // Center Y coordinate (normalized 0-1) 
    float width;    // Box width (normalized 0-1)
    float height;   // Box height (normalized 0-1)
    float confidence; // Detection confidence (0-1)
    int class_id;   // Species/object class ID
    const char* class_name; // Human-readable class name
};

/**
 * @brief YOLO-tiny detector class for wildlife detection
 */
class YOLOTinyDetector {
private:
    // TensorFlow Lite components
    const tflite::Model* model_;
    tflite::MicroInterpreter* interpreter_;
    tflite::MicroOpResolver* resolver_;
    
    // Model input/output tensors
    TfLiteTensor* input_tensor_;
    TfLiteTensor* output_tensor_;
    
    // Memory allocation for model
    uint8_t* tensor_arena_;
    static const int kTensorArenaSize = 200 * 1024; // 200KB for YOLO-tiny
    
    // Model configuration
    static const int kInputWidth = 160;   // Optimized for ESP32
    static const int kInputHeight = 160;
    static const int kInputChannels = 3;
    static const int kMaxDetections = 10;
    static const float kConfidenceThreshold = 0.5f;
    static const float kNMSThreshold = 0.4f;
    
    // Wildlife class names (expandable)
    const char* class_names_[20] = {
        "deer", "bear", "bird", "squirrel", "rabbit",
        "wolf", "cougar", "fox", "raccoon", "elk",
        "moose", "turkey", "eagle", "hawk", "owl",
        "coyote", "bobcat", "opossum", "skunk", "beaver"
    };
    
public:
    /**
     * @brief Initialize YOLO-tiny detector
     * @param model_data Pointer to model data in flash memory
     * @return true if initialization successful
     */
    bool initialize(const unsigned char* model_data);
    
    /**
     * @brief Run inference on camera frame
     * @param image_data RGB image data (160x160x3)
     * @param detections Output array for detected objects
     * @param max_detections Maximum number of detections to return
     * @return Number of detections found
     */
    int detect(const uint8_t* image_data, BoundingBox* detections, int max_detections);
    
    /**
     * @brief Pre-process image for model input
     * @param src_image Source image data
     * @param src_width Source image width
     * @param src_height Source image height
     * @param dst_image Destination processed image
     */
    void preprocessImage(const uint8_t* src_image, int src_width, int src_height, uint8_t* dst_image);
    
    /**
     * @brief Post-process model output to extract bounding boxes
     * @param raw_output Raw model output tensor
     * @param detections Output detections array
     * @param max_detections Maximum detections to return
     * @return Number of valid detections
     */
    int postprocessOutput(float* raw_output, BoundingBox* detections, int max_detections);
    
    /**
     * @brief Apply Non-Maximum Suppression to remove duplicate detections
     * @param detections Input/output detections array
     * @param num_detections Number of input detections
     * @return Number of detections after NMS
     */
    int applyNMS(BoundingBox* detections, int num_detections);
    
    /**
     * @brief Get class name from class ID
     * @param class_id Class ID from detection
     * @return Human-readable class name
     */
    const char* getClassName(int class_id);
    
    /**
     * @brief Check if detector is initialized
     * @return true if ready for inference
     */
    bool isInitialized() const { return interpreter_ != nullptr; }
    
    /**
     * @brief Get model input dimensions
     * @param width Output width
     * @param height Output height
     * @param channels Output channels
     */
    void getInputDimensions(int* width, int* height, int* channels) const {
        *width = kInputWidth;
        *height = kInputHeight; 
        *channels = kInputChannels;
    }
    
    /**
     * @brief Clean up resources
     */
    ~YOLOTinyDetector();
};

#endif // YOLO_TINY_DETECTOR_H