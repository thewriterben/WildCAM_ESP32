# ML Deployment Checklist

Use this checklist to ensure proper ML model deployment to ESP32 Wildlife Camera.

## Pre-Deployment

### 1. Training Environment Setup

- [ ] Python 3.8+ installed
- [ ] Dependencies installed (`pip install -r backend/requirements.txt`)
- [ ] TensorFlow 2.13+ working
- [ ] Dataset directory structure prepared
- [ ] Training configuration file created

### 2. Dataset Preparation

- [ ] Images collected for target species
- [ ] Minimum 500 images per species (recommended: 1000+)
- [ ] Images organized in train/validation/test splits
- [ ] Class balance verified (similar number of images per species)
- [ ] Image quality checked (resolution, lighting, clarity)
- [ ] Dataset augmentation configured

### 3. Hardware Verification

- [ ] ESP32 board identified (ESP32-CAM, ESP32-S3, etc.)
- [ ] PSRAM availability confirmed (8MB recommended)
- [ ] Flash memory size checked (minimum 4MB)
- [ ] Camera module tested and working
- [ ] Power supply adequate (5V 2A recommended)

## Model Training

### 4. Training Process

- [ ] Training script executed successfully
- [ ] Training converged (loss decreasing, accuracy increasing)
- [ ] Validation accuracy > 85% (target: > 90%)
- [ ] No significant overfitting detected
- [ ] Model saved in Keras format (.h5)

### 5. Model Conversion

- [ ] Model converted to TensorFlow Lite
- [ ] INT8 quantization applied
- [ ] Quantized model validated
- [ ] Model size < 15 MB
- [ ] Accuracy after quantization checked (< 2% loss acceptable)

### 6. Model Evaluation

- [ ] Test set accuracy measured
- [ ] Inference time benchmarked (target: < 1000ms)
- [ ] Memory usage profiled (target: < 2MB)
- [ ] Per-class accuracy reviewed
- [ ] Confusion matrix analyzed
- [ ] Edge cases identified

## Deployment Preparation

### 7. Model Files

- [ ] TFLite model file ready (`.tflite`)
- [ ] Model metadata file created (`model_metadata.json`)
- [ ] Class labels file prepared (`class_labels.json`)
- [ ] Model README documentation written
- [ ] Version number assigned

### 8. Model Validation

- [ ] Model loads in TensorFlow Lite interpreter
- [ ] Test inference runs successfully
- [ ] Input/output shapes verified
- [ ] Data types correct (uint8 for quantized)
- [ ] File integrity checked (checksum)

### 9. Deployment Package

- [ ] Deployment directory created
- [ ] Model file copied to deployment location
- [ ] Metadata files included
- [ ] Installation script prepared
- [ ] Deployment README written

## ESP32 Deployment

### 10. Firmware Preparation

- [ ] Firmware source code updated (if needed)
- [ ] Model path configured correctly
- [ ] PlatformIO environment selected
- [ ] Dependencies verified in `platformio.ini`
- [ ] Build flags checked (PSRAM, etc.)

### 11. Model Upload

- [ ] Model copied to ESP32 filesystem
  - Via SD card, or
  - Via SPIFFS/LittleFS, or
  - Embedded in firmware
- [ ] File permissions set correctly
- [ ] Available storage space verified
- [ ] File integrity on device confirmed

### 12. Firmware Upload

- [ ] Firmware compiled successfully
- [ ] No compilation errors or warnings
- [ ] Firmware uploaded to ESP32
- [ ] Serial monitor connected
- [ ] Boot messages verified
- [ ] Model loading confirmed

## Testing and Validation

### 13. Basic Functionality

- [ ] Camera initialization successful
- [ ] Model loads without errors
- [ ] First inference completes
- [ ] Results format correct
- [ ] Memory usage within limits
- [ ] No crashes or reboots

### 14. Performance Testing

- [ ] Inference time measured (< 1000ms target)
- [ ] Memory usage monitored (< 2MB PSRAM target)
- [ ] Power consumption checked (< 200mA target)
- [ ] Frame capture time measured
- [ ] End-to-end latency verified
- [ ] Sustained operation tested (1+ hour)

### 15. Accuracy Validation

- [ ] Test images captured
- [ ] Classification results reviewed
- [ ] Confidence scores reasonable (> 0.7 for positives)
- [ ] False positive rate acceptable
- [ ] False negative rate acceptable
- [ ] Edge cases tested (low light, partial views, etc.)

### 16. Integration Testing

- [ ] Motion detection triggers classification
- [ ] Results logged correctly
- [ ] Data collection working
- [ ] Alert system functioning (if applicable)
- [ ] Storage management working
- [ ] Power management integrated

## Production Deployment

### 17. Field Testing

- [ ] Device tested in target environment
- [ ] Various lighting conditions tested
- [ ] Weather resistance verified (if outdoor)
- [ ] Battery life measured
- [ ] Real wildlife tested (if possible)
- [ ] Long-term stability confirmed (24+ hours)

### 18. Documentation

- [ ] Deployment date recorded
- [ ] Model version documented
- [ ] Hardware configuration noted
- [ ] Known issues documented
- [ ] Troubleshooting guide updated
- [ ] User manual updated (if applicable)

### 19. Monitoring Setup

- [ ] Logging enabled and working
- [ ] Performance metrics collected
- [ ] Error tracking configured
- [ ] Remote monitoring enabled (if applicable)
- [ ] Alert system configured
- [ ] Backup/recovery procedures tested

### 20. Maintenance Plan

- [ ] Update schedule defined
- [ ] Backup procedures documented
- [ ] Rollback procedure tested
- [ ] Support contacts identified
- [ ] Maintenance schedule created
- [ ] Performance baseline established

## Post-Deployment

### 21. Performance Monitoring

- [ ] Daily detection rate tracked
- [ ] Classification accuracy monitored
- [ ] System uptime tracked
- [ ] Error rates monitored
- [ ] Performance trends analyzed
- [ ] User feedback collected

### 22. Model Improvement

- [ ] Misclassifications logged
- [ ] New species/scenarios identified
- [ ] Training data gaps identified
- [ ] Model retraining planned
- [ ] A/B testing prepared (if applicable)
- [ ] Continuous learning pipeline active

### 23. System Optimization

- [ ] Bottlenecks identified
- [ ] Optimization opportunities found
- [ ] Power consumption optimized
- [ ] Memory usage optimized
- [ ] Inference speed improved
- [ ] Model size reduced (if needed)

## Troubleshooting Checklist

### If Model Won't Load

- [ ] File path correct
- [ ] File exists on device
- [ ] File not corrupted (check size)
- [ ] Sufficient memory available
- [ ] Format is TensorFlow Lite
- [ ] Compatible with TFLite Micro version

### If Inference Fails

- [ ] Input image format correct
- [ ] Input dimensions match model
- [ ] Input data type correct (uint8/float32)
- [ ] Tensor arena size sufficient
- [ ] Memory not fragmented
- [ ] No memory leaks

### If Accuracy Is Low

- [ ] Model properly trained
- [ ] Quantization validated
- [ ] Environmental factors considered
- [ ] Camera settings optimized
- [ ] Image preprocessing correct
- [ ] Confidence threshold appropriate

### If Performance Is Slow

- [ ] INT8 quantization applied
- [ ] PSRAM enabled and used
- [ ] CPU frequency adequate (240MHz)
- [ ] Model architecture optimized
- [ ] Preprocessing optimized
- [ ] No memory swapping

## Version Control

**Checklist Version:** 1.0  
**Last Updated:** 2024-01-15  
**Compatible With:** WildCAM ESP32 v2.0+

## Notes

- This checklist should be completed for each model deployment
- Keep a copy of completed checklist with deployment documentation
- Review and update checklist based on deployment experience
- Share lessons learned with team

## References

- [ML Integration Guide](ml_integration_guide.md)
- [Quick Start Guide](ml_quickstart.md)
- [ML Workflow](ML_WORKFLOW.md)
- [Main README](../ML_INTEGRATION_README.md)

---

**Deployment Date:** _______________  
**Model Version:** _______________  
**Deployed By:** _______________  
**Verified By:** _______________  
**Status:** [ ] Pass [ ] Fail [ ] Conditional  
**Notes:** _______________________________________________
