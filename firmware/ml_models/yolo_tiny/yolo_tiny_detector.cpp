/**
 * @file yolo_tiny_detector.cpp
 * @brief YOLO-tiny implementation for ESP32 real-time object detection
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

#include "yolo_tiny_detector.h"
#include <math.h>
#include <algorithm>

bool YOLOTinyDetector::initialize(const unsigned char* model_data) {
    // Allocate memory for tensor arena
    tensor_arena_ = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!tensor_arena_) {
        Serial.println("Failed to allocate tensor arena memory");
        return false;
    }
    
    // Load the model
    model_ = tflite::GetModel(model_data);
    if (model_->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model schema version %d not supported. Expected %d\n", 
                     model_->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    
    // Create resolver for operations
    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddReshape();
    resolver.AddSoftmax();
    resolver.AddLogistic();
    resolver.AddAdd();
    resolver.AddMul();
    resolver.AddPad();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver_ = &resolver;
    
    // Create interpreter
    static tflite::MicroInterpreter static_interpreter(
        model_, *resolver_, tensor_arena_, kTensorArenaSize);
    interpreter_ = &static_interpreter;
    
    // Allocate tensors
    TfLiteStatus allocate_status = interpreter_->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        Serial.println("Failed to allocate tensors");
        return false;
    }
    
    // Get input and output tensors
    input_tensor_ = interpreter_->input(0);
    output_tensor_ = interpreter_->output(0);
    
    // Verify input tensor dimensions
    if (input_tensor_->dims->size != 4 ||
        input_tensor_->dims->data[1] != kInputHeight ||
        input_tensor_->dims->data[2] != kInputWidth ||
        input_tensor_->dims->data[3] != kInputChannels) {
        Serial.println("Model input tensor dimensions don't match expected");
        return false;
    }
    
    Serial.println("YOLO-tiny detector initialized successfully");
    return true;
}

int YOLOTinyDetector::detect(const uint8_t* image_data, BoundingBox* detections, int max_detections) {
    if (!isInitialized()) {
        return 0;
    }
    
    // Preprocess image data into model input tensor
    preprocessImage(image_data, kInputWidth, kInputHeight, 
                   input_tensor_->data.uint8);
    
    // Run inference
    TfLiteStatus invoke_status = interpreter_->Invoke();
    if (invoke_status != kTfLiteOk) {
        Serial.println("Inference failed");
        return 0;
    }
    
    // Post-process output to get bounding boxes
    float* raw_output = output_tensor_->data.f;
    int num_detections = postprocessOutput(raw_output, detections, max_detections);
    
    // Apply Non-Maximum Suppression
    if (num_detections > 1) {
        num_detections = applyNMS(detections, num_detections);
    }
    
    return num_detections;
}

void YOLOTinyDetector::preprocessImage(const uint8_t* src_image, int src_width, int src_height, uint8_t* dst_image) {
    // Simple resize and normalize (0-255 to 0-1 range)
    for (int y = 0; y < kInputHeight; y++) {
        for (int x = 0; x < kInputWidth; x++) {
            for (int c = 0; c < kInputChannels; c++) {
                int src_x = (x * src_width) / kInputWidth;
                int src_y = (y * src_height) / kInputHeight;
                int src_idx = (src_y * src_width + src_x) * kInputChannels + c;
                int dst_idx = (y * kInputWidth + x) * kInputChannels + c;
                
                // Normalize to 0-1 range for model input
                dst_image[dst_idx] = src_image[src_idx];
            }
        }
    }
}

int YOLOTinyDetector::postprocessOutput(float* raw_output, BoundingBox* detections, int max_detections) {
    int num_detections = 0;
    
    // YOLO-tiny output format: [batch, grid_h, grid_w, anchors * (5 + num_classes)]
    // For simplicity, assume 5x5 grid with 1 anchor box and 20 classes
    const int grid_size = 5;
    const int num_anchors = 1;
    const int num_classes = 20;
    const int output_size = 5 + num_classes; // x, y, w, h, confidence + classes
    
    for (int grid_y = 0; grid_y < grid_size && num_detections < max_detections; grid_y++) {
        for (int grid_x = 0; grid_x < grid_size && num_detections < max_detections; grid_x++) {
            for (int anchor = 0; anchor < num_anchors && num_detections < max_detections; anchor++) {
                int base_idx = (grid_y * grid_size + grid_x) * num_anchors * output_size + anchor * output_size;
                
                float confidence = raw_output[base_idx + 4];
                if (confidence < kConfidenceThreshold) {
                    continue;
                }
                
                // Find best class
                int best_class = 0;
                float best_class_prob = raw_output[base_idx + 5];
                for (int c = 1; c < num_classes; c++) {
                    float class_prob = raw_output[base_idx + 5 + c];
                    if (class_prob > best_class_prob) {
                        best_class = c;
                        best_class_prob = class_prob;
                    }
                }
                
                float final_confidence = confidence * best_class_prob;
                if (final_confidence < kConfidenceThreshold) {
                    continue;
                }
                
                // Extract bounding box coordinates
                float x = (raw_output[base_idx + 0] + grid_x) / grid_size;
                float y = (raw_output[base_idx + 1] + grid_y) / grid_size;
                float w = exp(raw_output[base_idx + 2]) / grid_size;
                float h = exp(raw_output[base_idx + 3]) / grid_size;
                
                // Store detection
                detections[num_detections].x = x;
                detections[num_detections].y = y;
                detections[num_detections].width = w;
                detections[num_detections].height = h;
                detections[num_detections].confidence = final_confidence;
                detections[num_detections].class_id = best_class;
                detections[num_detections].class_name = getClassName(best_class);
                
                num_detections++;
            }
        }
    }
    
    return num_detections;
}

int YOLOTinyDetector::applyNMS(BoundingBox* detections, int num_detections) {
    // Sort detections by confidence (descending)
    std::sort(detections, detections + num_detections, 
              [](const BoundingBox& a, const BoundingBox& b) {
                  return a.confidence > b.confidence;
              });
    
    // Apply NMS
    std::vector<bool> suppressed(num_detections, false);
    int kept = 0;
    
    for (int i = 0; i < num_detections; i++) {
        if (suppressed[i]) continue;
        
        // Keep this detection
        if (kept != i) {
            detections[kept] = detections[i];
        }
        kept++;
        
        // Suppress overlapping detections
        for (int j = i + 1; j < num_detections; j++) {
            if (suppressed[j]) continue;
            
            // Calculate IoU (Intersection over Union)
            float x1 = std::max(detections[i].x - detections[i].width/2, 
                               detections[j].x - detections[j].width/2);
            float y1 = std::max(detections[i].y - detections[i].height/2, 
                               detections[j].y - detections[j].height/2);
            float x2 = std::min(detections[i].x + detections[i].width/2, 
                               detections[j].x + detections[j].width/2);
            float y2 = std::min(detections[i].y + detections[i].height/2, 
                               detections[j].y + detections[j].height/2);
            
            float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);
            float area_i = detections[i].width * detections[i].height;
            float area_j = detections[j].width * detections[j].height;
            float union_area = area_i + area_j - intersection;
            
            float iou = intersection / union_area;
            if (iou > kNMSThreshold) {
                suppressed[j] = true;
            }
        }
    }
    
    return kept;
}

const char* YOLOTinyDetector::getClassName(int class_id) {
    if (class_id >= 0 && class_id < 20) {
        return class_names_[class_id];
    }
    return "unknown";
}

YOLOTinyDetector::~YOLOTinyDetector() {
    if (tensor_arena_) {
        heap_caps_free(tensor_arena_);
        tensor_arena_ = nullptr;
    }
}