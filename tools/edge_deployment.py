"""
Tools for deploying ML models to edge devices
"""
import tensorflow as tf
import numpy as np
from pathlib import Path

class EdgeMLDeployment:
    """
    Optimize and deploy models for edge computing
    """
    
    @staticmethod
    def optimize_for_esp32(model_path):
        """
        Convert TensorFlow model for ESP32 deployment
        Using TensorFlow Lite Micro
        """
        # Load the model
        model = tf.keras.models.load_model(model_path)
        
        # Representative dataset for quantization
        def representative_dataset():
            for _ in range(100):
                data = np.random.rand(1, 224, 224, 3).astype(np.float32)
                yield [data]
        
        # Convert to TFLite with int8 quantization
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        converter.optimizations = [tf.lite.Optimize.DEFAULT]
        converter.representative_dataset = representative_dataset
        converter.target_spec.supported_types = [tf.int8]
        converter.inference_input_type = tf.uint8
        converter.inference_output_type = tf.uint8
        
        tflite_model = converter.convert()
        
        # Save the model
        tflite_path = Path(model_path).stem + '_esp32.tflite'
        with open(tflite_path, 'wb') as f:
            f.write(tflite_model)
            
        # Generate C array for embedding
        self.generate_c_array(tflite_model, Path(model_path).stem)
        
        return tflite_path
    
    @staticmethod
    def generate_c_array(model_bytes, model_name):
        """Generate C header file for ESP32"""
        hex_array = ', '.join([f'0x{b:02x}' for b in model_bytes])
        
        c_code = f"""
#ifndef {model_name.upper()}_MODEL_H
#define {model_name.upper()}_MODEL_H

const unsigned char {model_name}_model[] = {{
    {hex_array}
}};

const unsigned int {model_name}_model_len = {len(model_bytes)};

#endif
"""
        
        with open(f'{model_name}_model.h', 'w') as f:
            f.write(c_code)

class OpenMVIntegration:
    """
    Integration with OpenMV Cam for smart camera modules
    Alternative/complement to ESP32-CAM
    """
    
    @staticmethod
    def generate_openmv_script():
        """Generate OpenMV Python script for wildlife detection"""
        
        script = '''
# OpenMV Wildlife Detection Script
import sensor, image, time, nn, uos, gc
import tf

# Initialize camera
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_windowing((240, 240))
sensor.skip_frames(time=2000)

# Load TensorFlow Lite model
net = tf.load("wildlife_model.tflite", load_to_fb=True)

# Motion detection variables
old_frame = sensor.snapshot()
motion_threshold = 1000

while True:
    img = sensor.snapshot()
    
    # Motion detection
    diff = img.difference(old_frame)
    stats = diff.get_statistics()
    
    if stats.mean() > motion_threshold:
        # Run inference
        obj = net.classify(img)[0]
        
        # Get top prediction
        predictions = obj.output()
        max_idx = predictions.index(max(predictions))
        confidence = predictions[max_idx]
        
        if confidence > 0.7:
            # Save image with detection
            img.save("/sd/capture_%d.jpg" % time.ticks_ms())
            
            # Log detection
            with open("/sd/detections.csv", "a") as f:
                f.write("%d,%d,%.2f\\n" % (time.ticks_ms(), max_idx, confidence))
    
    old_frame = img
    gc.collect()
'''
        return script