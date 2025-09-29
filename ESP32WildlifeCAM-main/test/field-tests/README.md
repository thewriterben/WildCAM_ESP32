# ESP32 Wildlife CAM - Field Test Suite

This directory contains field deployment test scenarios and validation procedures for real-world ESP32 Wildlife CAM operation.

## Test Scenarios

### Urban Deployment Tests
- **SCN-001**: Residential garden monitoring (3 days)
- **SCN-002**: Urban predator detection (5 days)

### Trail/Park Tests  
- **SCN-003**: Nature trail monitoring (7 days)
- **SCN-004**: Water source monitoring (10 days)

### Wilderness Tests
- **SCN-005**: Remote mountain survey (14 days)
- **SCN-006**: Migration route documentation (30 days)

### Stress Tests
- **SCN-007**: Extreme weather survival (48 hours)
- **SCN-008**: High-frequency capture stress (24 hours)

## Test Execution

```bash
# Execute specific scenario
python3 scripts/field_test_automation.py --scenario SCN-001

# List available scenarios
python3 scripts/field_test_automation.py --list-scenarios

# Monitor active deployment
python3 scripts/field_monitor.py --deployment_id site_001

# Analyze results
python3 scripts/analyze_field_data.py --scenario SCN-001
```

## Data Collection

Field tests collect comprehensive data including:
- Wildlife capture images and metadata
- Environmental sensor readings
- System performance metrics
- Power consumption data
- Error logs and diagnostic information

## Results Analysis

Test results include:
- Wildlife detection accuracy and species identification
- System uptime and reliability metrics
- Power autonomy and solar charging efficiency
- Image quality assessment across conditions
- Environmental resilience validation

## Safety and Legal

- Obtain necessary permissions for deployment locations
- Follow wildlife observation ethics and regulations
- Secure equipment against theft and environmental damage
- Maintain emergency contact information for remote deployments
- Document GPS coordinates and access routes for equipment recovery