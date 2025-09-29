# Test Data for ESP32WildlifeCAM

This directory contains test data sets and configurations for validating the wildlife monitoring system.

## Directory Structure

```
test_data/
├── images/
│   ├── wildlife_samples/     # Sample wildlife images for testing
│   ├── motion_test/          # Images for motion detection testing
│   ├── classification_test/  # Images for AI classification testing
│   └── night_vision/         # Night vision test images
├── scenarios/
│   ├── basic_workflow.json   # Basic system workflow test
│   ├── stress_test.json      # System stress testing scenarios
│   └── edge_cases.json       # Edge case testing scenarios
├── configurations/
│   ├── test_esp32_cam.h      # Test configuration for ESP32-CAM
│   └── test_esp32_s3.h       # Test configuration for ESP32-S3
└── validation/
    ├── performance_benchmarks.json
    └── accuracy_metrics.json
```

## Test Image Categories

### Wildlife Samples
- **mammals_large/**: Deer, elk, bear, etc.
- **mammals_small/**: Squirrels, rabbits, etc.
- **birds_large/**: Hawks, eagles, owls
- **birds_small/**: Songbirds, etc.
- **false_positives/**: Non-animal motion triggers

### Motion Detection Test Images
- **static_scenes/**: No motion baseline images
- **vegetation_movement/**: Wind-blown vegetation
- **lighting_changes/**: Shadow and sunlight variations
- **weather_conditions/**: Rain, snow, fog effects

### AI Classification Test Set
Each category includes:
- 50+ labeled images per species
- Various lighting conditions
- Different angles and distances
- Seasonal variations
- Environmental contexts

## Test Scenarios

### Basic Workflow Test
```json
{
  "name": "Basic Wildlife Detection Workflow",
  "description": "Tests complete motion detection to image capture workflow",
  "steps": [
    "Initialize system",
    "Simulate motion trigger",
    "Capture image",
    "Validate storage",
    "Check system health"
  ],
  "expected_duration_seconds": 30,
  "success_criteria": {
    "motion_detected": true,
    "image_captured": true,
    "file_stored": true,
    "system_stable": true
  }
}
```

### Stress Test Scenarios
- Continuous operation for 24+ hours
- Rapid motion detection events
- Low battery conditions
- High temperature operation
- Storage near capacity

### Edge Cases
- Simultaneous motion detection
- Network connectivity loss
- SD card failure
- Power fluctuations
- Extreme weather conditions

## Performance Benchmarks

### Response Time Metrics
```json
{
  "motion_detection_response_ms": {
    "target": 500,
    "acceptable": 1000,
    "maximum": 2000
  },
  "image_capture_time_ms": {
    "target": 1000,
    "acceptable": 2000,
    "maximum": 5000
  },
  "ai_classification_time_ms": {
    "target": 2000,
    "acceptable": 5000,
    "maximum": 10000
  }
}
```

### Accuracy Targets
```json
{
  "motion_detection_accuracy": {
    "true_positive_rate": 0.95,
    "false_positive_rate": 0.05,
    "precision": 0.90,
    "recall": 0.95
  },
  "species_classification_accuracy": {
    "overall_accuracy": 0.85,
    "per_species_minimum": 0.75,
    "confidence_threshold": 0.80
  }
}
```

## Usage Instructions

### Running Tests with Sample Data

1. **Copy test configurations:**
   ```bash
   cp examples/configurations/test_data/configurations/test_esp32_cam.h src/config/
   ```

2. **Load test scenarios:**
   ```bash
   python3 scripts/load_test_scenario.py examples/configurations/test_data/scenarios/basic_workflow.json
   ```

3. **Run validation:**
   ```bash
   ./scripts/integration_test.sh --test-data-path=examples/configurations/test_data/
   ```

### Creating Custom Test Data

1. **Image Preparation:**
   - Resize images to match camera resolution
   - Ensure proper lighting and contrast
   - Label images with species and confidence scores

2. **Scenario Definition:**
   ```json
   {
     "name": "Custom Test Scenario",
     "test_images": ["path/to/image1.jpg", "path/to/image2.jpg"],
     "expected_results": {
       "species": "target_species",
       "confidence_min": 0.8
     }
   }
   ```

3. **Validation Metrics:**
   - Define success criteria
   - Set performance thresholds
   - Specify test duration and conditions

## Test Data Generation

### Automated Test Data Creation
```python
# Generate synthetic test scenarios
python3 scripts/generate_test_data.py --count=100 --species=deer,rabbit,bird
```

### Real-world Data Collection
```bash
# Collect field data for testing
./scripts/field_data_collection.sh --duration=24h --location=test_site
```

## Validation Checklist

- [ ] Test images load correctly
- [ ] Motion detection triggers appropriately
- [ ] Camera captures are successful
- [ ] AI classification provides expected results
- [ ] System performance meets benchmarks
- [ ] Error handling works correctly
- [ ] Resource usage is within limits

## Contributing Test Data

When contributing new test data:

1. Ensure images are properly licensed
2. Include metadata (location, date, conditions)
3. Provide ground truth labels
4. Document any special conditions
5. Test with multiple hardware configurations

## Notes

- Test images should be representative of real deployment conditions
- Include challenging scenarios (low light, weather, occlusion)
- Validate test data regularly to ensure relevance
- Consider privacy and ethical implications of wildlife imagery
- Update test data as system capabilities evolve