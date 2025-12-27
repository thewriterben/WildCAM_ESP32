# IMX500 Model Deployment Guide

This guide covers deploying AI models to the Sony IMX500 sensor on Arducam Pivistation 5.

## Model Requirements

### Input Specifications
- **Format:** ONNX (Open Neural Network Exchange)
- **Input Shape:** (1, 3, 640, 640) - Batch, Channels, Height, Width
- **Color Space:** RGB
- **Normalization:** 0-255 (uint8) or 0.0-1.0 (float32)
- **ONNX Version:** 1.12 or later

### Output Specifications
- **Detection Format:** Bounding boxes + class probabilities
- **Supported Architectures:**
  - YOLOv5 (recommended)
  - YOLOv8
  - SSD MobileNet
  - Custom detection models

### Performance Constraints
- **Model Size:** < 50MB recommended
- **Inference Time:** Target <10ms for 30fps
- **Quantization:** INT8 required for optimal performance

## Pre-trained Models

### WildCAM Wildlife Detector v1

Official pre-trained model for wildlife monitoring.

**Download:**
```bash
cd /usr/share/imx500-models
sudo wget https://models.wildcam.io/imx500/wildlife_detector_v1.rpk
sudo wget https://models.wildcam.io/imx500/wildlife_detector_v1.yaml  # Model config
```

**Supported Species (30):**
- Mammals: deer, elk, moose, bear, wolf, coyote, fox, rabbit, raccoon, bobcat, mountain_lion, wild_boar, beaver, porcupine, badger, skunk, opossum
- Birds: turkey, eagle, owl, hawk
- Other: human, vehicle, domestic_animal

**Performance:**
- mAP@0.5: 0.82
- Inference: 6-8ms
- False Positive Rate: <5%

### Community Models

Browse community-contributed models:
```bash
# List available models
curl https://models.wildcam.io/imx500/list

# Download specific model
wget https://models.wildcam.io/imx500/models/[model_name].rpk
```

## Converting Custom Models

### Step 1: Train ONNX Model

Example using YOLOv5:

```bash
# Clone YOLOv5
git clone https://github.com/ultralytics/yolov5
cd yolov5

# Install dependencies
pip install -r requirements.txt

# Train on custom dataset
python train.py --img 640 --batch 16 --epochs 100 \
                --data custom_wildlife.yaml \
                --weights yolov5s.pt

# Export to ONNX
python export.py --weights runs/train/exp/weights/best.pt \
                 --include onnx --simplify
```

### Step 2: Convert ONNX to RPK

Using Python API:

```python
from edge.arducam.utils.model_converter import ModelConverter

converter = ModelConverter()

# Convert with INT8 quantization
success = converter.convert_onnx_to_rpk(
    onnx_path="yolov5s.onnx",
    output_path="/usr/share/imx500-models/custom_wildlife.rpk",
    quantization="int8",
    input_shape=(1, 3, 640, 640),
    calibration_data="calibration_images/",  # Optional: improve quantization
    optimization_level=2
)

if success:
    print("Conversion successful!")
else:
    print("Conversion failed - check logs")
```

Using command line:

```bash
# Assuming imx500-converter is installed
python3 -m edge.arducam.utils.model_converter \
    --input yolov5s.onnx \
    --output custom_wildlife.rpk \
    --quantization int8 \
    --input-shape 1,3,640,640
```

### Step 3: Validate Model

```python
from edge.arducam.utils.model_converter import ModelConverter

converter = ModelConverter()
validation = converter.validate_rpk("/usr/share/imx500-models/custom_wildlife.rpk")

print(f"Valid: {validation['valid']}")
print(f"Size: {validation.get('size_mb', 0):.2f} MB")
```

### Step 4: Test Deployment

```python
from edge.arducam.inference import IMX500WildlifeDetector

# Create detector with custom model
config = {
    'model_path': '/usr/share/imx500-models/custom_wildlife.rpk',
    'confidence_threshold': 0.5,
    'inference_fps': 30
}

detector = IMX500WildlifeDetector(config)

if detector.initialize():
    print("Model loaded successfully!")
    detector.start()
else:
    print("Failed to load model")
```

## Calibration for Quantization

INT8 quantization requires calibration data for optimal accuracy.

### Prepare Calibration Dataset

```bash
# Create calibration directory
mkdir -p calibration_images

# Add representative images (100-500 images recommended)
# Images should cover:
# - Various lighting conditions
# - Different species
# - Various backgrounds
# - Different weather conditions
```

### Run Calibration

```python
converter.convert_onnx_to_rpk(
    onnx_path="model.onnx",
    output_path="model_int8.rpk",
    quantization="int8",
    calibration_data="calibration_images/"  # Point to calibration images
)
```

## Optimization Tips

### Model Architecture

1. **Use efficient architectures:**
   - YOLOv5s/n (small/nano) for IMX500
   - Avoid heavy backbones (ResNet-101, etc.)
   - Prefer MobileNet, EfficientNet

2. **Reduce model complexity:**
   ```python
   # YOLOv5 example - use smaller model
   python export.py --weights yolov5n.pt  # Nano model
   ```

3. **Prune unnecessary classes:**
   - Remove classes not needed for deployment
   - Reduces model size and improves speed

### Quantization Settings

```python
# Experiment with quantization types
quantization_options = ["int8", "fp16", "mixed"]

for quant in quantization_options:
    converter.convert_onnx_to_rpk(
        onnx_path="model.onnx",
        output_path=f"model_{quant}.rpk",
        quantization=quant
    )
    # Test and compare accuracy vs. speed
```

### Input Resolution

```python
# Test different input sizes
resolutions = [(1, 3, 416, 416), (1, 3, 512, 512), (1, 3, 640, 640)]

for shape in resolutions:
    converter.convert_onnx_to_rpk(
        onnx_path="model.onnx",
        output_path=f"model_{shape[2]}x{shape[3]}.rpk",
        input_shape=shape
    )
```

**Trade-offs:**
- 416x416: Faster inference, lower accuracy
- 640x640: Balanced (recommended)
- Higher resolutions: May not fit in IMX500 memory

## Model Performance Benchmarks

| Model | Input Size | Inference (ms) | mAP@0.5 | Size (MB) |
|-------|------------|----------------|---------|-----------|
| YOLOv5n | 640x640 | 5-6 | 0.75 | 3.8 |
| YOLOv5s | 640x640 | 7-8 | 0.82 | 7.2 |
| YOLOv8n | 640x640 | 6-7 | 0.78 | 4.1 |
| Custom Wildlife v1 | 640x640 | 6-8 | 0.82 | 6.5 |

*Benchmarks on Raspberry Pi CM5 with IMX500*

## Troubleshooting

### Conversion Fails

**Error: "Model too large"**
```python
# Solution: Use smaller model or reduce input size
converter.convert_onnx_to_rpk(
    onnx_path="model.onnx",
    output_path="model.rpk",
    input_shape=(1, 3, 416, 416)  # Smaller input
)
```

**Error: "Unsupported operator"**
- Some ONNX operators not supported by IMX500
- Simplify model: `python export.py --simplify`
- Remove custom layers

### Poor Accuracy After Quantization

```python
# Use calibration data
converter.convert_onnx_to_rpk(
    onnx_path="model.onnx",
    output_path="model.rpk",
    quantization="int8",
    calibration_data="calibration_images/",  # Add this
    optimization_level=1  # Lower optimization
)
```

### Model Won't Load

Check model file:
```bash
# Verify file exists and has reasonable size
ls -lh /usr/share/imx500-models/model.rpk

# Check permissions
sudo chmod 644 /usr/share/imx500-models/model.rpk
```

Enable debug logging:
```python
import logging
logging.basicConfig(level=logging.DEBUG)

detector = IMX500WildlifeDetector(config)
detector.initialize()  # Will show detailed error messages
```

## Training Custom Models

### Dataset Preparation

1. **Collect Images:**
   - Minimum 500 images per class
   - Various lighting, weather, backgrounds
   - Mix of camera traps, photos, video frames

2. **Annotation:**
   - Use tools: LabelImg, CVAT, Roboflow
   - Format: YOLO, COCO, or Pascal VOC
   - Quality > quantity

3. **Dataset Structure:**
   ```
   dataset/
   ├── images/
   │   ├── train/
   │   ├── val/
   │   └── test/
   └── labels/
       ├── train/
       ├── val/
       └── test/
   ```

### Training Pipeline

```python
# data.yaml
train: dataset/images/train
val: dataset/images/val
nc: 20  # Number of classes
names: ['deer', 'elk', 'moose', ...]

# Train
python train.py --img 640 --batch 16 --epochs 100 \
                --data data.yaml --weights yolov5s.pt

# Validate
python val.py --data data.yaml --weights runs/train/exp/weights/best.pt

# Export
python export.py --weights runs/train/exp/weights/best.pt --include onnx
```

## Model Versioning

```bash
# Organize models by version
/usr/share/imx500-models/
├── wildlife_detector_v1.0.rpk
├── wildlife_detector_v1.1.rpk
├── wildlife_detector_v2.0.rpk
└── active -> wildlife_detector_v2.0.rpk  # Symlink to active model
```

Update configuration to use symlink:
```yaml
inference:
  model_path: "/usr/share/imx500-models/active"
```

## Resources

- **ONNX Documentation:** https://onnx.ai/
- **YOLOv5:** https://github.com/ultralytics/yolov5
- **YOLOv8:** https://github.com/ultralytics/ultralytics
- **Model Zoo:** https://models.wildcam.io
- **IMX500 SDK:** https://docs.arducam.com/AI-Camera-Solutions/

## Support

For model deployment issues:
1. Check logs: `sudo journalctl -u wildcam-arducam -n 100`
2. Test in simulation mode first
3. Create issue: https://github.com/thewriterben/WildCAM_ESP32/issues
4. Community forum: https://community.wildcam.io
