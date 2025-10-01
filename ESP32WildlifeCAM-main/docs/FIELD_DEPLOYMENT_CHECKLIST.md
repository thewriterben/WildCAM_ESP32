# Field Deployment Checklist - WildCAM ESP32

## Pre-Deployment Testing

### Hardware Verification
- [ ] Camera captures clear images (test in lab)
- [ ] PIR sensor detects motion reliably (5-7 meter range)
- [ ] LoRa module transmits/receives (test with partner node)
- [ ] Battery fully charged (measure voltage: 4.2V for LiPo)
- [ ] Solar panel generates current (test in sunlight)
- [ ] All connectors secured (hot glue if needed)
- [ ] Enclosure is weatherproof (spray test recommended)

### Software Testing
- [ ] Firmware uploaded successfully
- [ ] AI classifier initializes without errors
- [ ] Alert system responds to dangerous species
- [ ] Motion pattern analyzer filters false positives
- [ ] LoRa mesh connects to network
- [ ] Statistics reporting works correctly
- [ ] Power management functioning (check sleep mode)

### System Calibration
- [ ] Camera focus adjusted for target distance
- [ ] PIR sensitivity tuned (not too sensitive)
- [ ] Confidence thresholds set appropriately
- [ ] Alert cooldown period configured
- [ ] GPS coordinates programmed
- [ ] Time zone set correctly

---

## Field Installation

### Site Selection
- [ ] Wildlife trail or feeding area identified
- [ ] Clear camera view (no obstructions)
- [ ] PIR sensor coverage optimized
- [ ] Solar panel faces south (northern hemisphere)
- [ ] Tree/post mounting location selected
- [ ] Height: 1-2 meters above ground
- [ ] Angle: 10-20° downward tilt

### Physical Installation
- [ ] Camera mounted securely (weatherproof case)
- [ ] Enclosure sealed (check all gaskets)
- [ ] Solar panel positioned for maximum sun
- [ ] Antenna positioned vertically (LoRa)
- [ ] Cable routing protected from animals
- [ ] Mounting straps/brackets secured
- [ ] Ground clearance adequate for snow/rain

### Initial Power-On
- [ ] System boots successfully
- [ ] Camera initializes (check LED indicator)
- [ ] PIR sensor responsive (test with motion)
- [ ] LoRa connects to mesh network
- [ ] Battery voltage normal (>3.7V)
- [ ] Solar charging active (check current)
- [ ] No error messages in logs

---

## Configuration

### AI Settings
```cpp
Classifier Confidence:     [____0.70____] (0.60-0.80 recommended)
Alert Threshold:           [____0.70____] (0.60-0.80 recommended)
Consecutive Detections:    [____2_______] (2-3 recommended)
Pattern Sensitivity:       [____0.6_____] (0.5-0.7 recommended)
```

### Power Management
- [ ] Low power mode: [ ] Enabled  [X] Disabled
- [ ] Motion check interval: [____1000____] ms
- [ ] Deep sleep enabled: [X] Yes  [ ] No
- [ ] Sleep duration: [____300_____] seconds (5 min)
- [ ] Wake on PIR: [X] Enabled  [ ] Disabled

### Network Settings
- [ ] Node ID: [_______________]
- [ ] LoRa frequency: [____915____] MHz (US: 915, EU: 868)
- [ ] TX power: [____17____] dBm (10-20)
- [ ] Spreading factor: [____7_____] (7-12)
- [ ] GPS coordinates: Lat [__________] Lon [__________]

### Alert Configuration
- [ ] Email notifications: [ ] Enabled
- [ ] SMS alerts: [ ] Enabled
- [ ] LoRa mesh: [X] Enabled
- [ ] Local alarm: [ ] Enabled
- [ ] Alert cooldown: [____60____] seconds

---

## 24-Hour Test Period

### Day 1: Monitor Closely

**Hour 0-4: Initial Testing**
- [ ] First motion detection successful
- [ ] AI classification working
- [ ] No false positives in first hour
- [ ] Battery level stable or charging
- [ ] Temperature within range (0-40°C)

**Hour 4-8: Pattern Observation**
- [ ] Multiple detections logged
- [ ] Alert system functioning
- [ ] LoRa transmissions successful
- [ ] No system crashes or resets
- [ ] Memory usage stable

**Hour 8-12: Performance Check**
- [ ] Review detection statistics
- [ ] Check false positive rate (<10%)
- [ ] Verify battery charging cycle
- [ ] Test night vision (if equipped)
- [ ] Confirm data logging works

**Hour 12-24: Overnight Monitoring**
- [ ] System survives overnight
- [ ] Night detections working
- [ ] Battery maintained charge
- [ ] No weather-related issues
- [ ] Morning statistics review

### Day 1 Statistics Target
```
Total Detections:     10-50 (depends on location)
Animal Detections:    5-30
False Positives:      <10%
Alerts Generated:     0-5
System Uptime:        100%
Battery Voltage:      >3.7V
```

---

## Week 1: Extended Testing

### Daily Checks (Days 2-7)

**Every 24 Hours:**
- [ ] Review detection log
- [ ] Check battery voltage
- [ ] Verify solar charging
- [ ] Test LoRa connectivity
- [ ] Review false positive rate
- [ ] Check for condensation in enclosure
- [ ] Verify camera focus still good

### Weekly Review (End of Week 1)

**System Health:**
- [ ] No hardware failures
- [ ] Battery maintains >70% charge
- [ ] Camera still clean and focused
- [ ] Enclosure remains sealed
- [ ] Mounting still secure

**Performance Metrics:**
```
Week 1 Statistics:
Total Detections:     [________]
Animal Detections:    [________]
Species Identified:   [________]
Dangerous Species:    [________]
Alerts Sent:          [________]
False Positives:      [________] (<5% target)
Avg Battery Level:    [________] (>70%)
System Uptime:        [________] (>95%)
```

**Data Quality:**
- [ ] Images are clear and focused
- [ ] Classifications accurate (spot check)
- [ ] Timestamps correct
- [ ] GPS coordinates accurate
- [ ] All data fields populated

---

## Monthly Maintenance

### Physical Inspection
- [ ] Clean camera lens
- [ ] Check enclosure seals
- [ ] Inspect solar panel (remove debris)
- [ ] Verify mounting integrity
- [ ] Check for animal damage
- [ ] Test PIR sensor range
- [ ] Verify antenna condition

### System Maintenance
- [ ] Download logs and images
- [ ] Update firmware if available
- [ ] Review and adjust thresholds
- [ ] Clear old data files
- [ ] Backup important detections
- [ ] Test emergency alerts
- [ ] Verify clock accuracy

### Battery Care
- [ ] Measure battery voltage
- [ ] Check charging current
- [ ] Clean solar panel
- [ ] Verify charge controller working
- [ ] Replace battery if <80% capacity
- [ ] Test with full load

### Network Health
- [ ] Test LoRa range
- [ ] Verify mesh connectivity
- [ ] Check signal strength (RSSI)
- [ ] Update routing tables
- [ ] Test alert propagation
- [ ] Verify GPS lock

---

## Troubleshooting Guide

### Camera Issues

**No Images Captured:**
1. Check camera connection
2. Verify power supply
3. Test camera module
4. Check firmware settings
5. Review error logs

**Blurry Images:**
1. Adjust focus manually
2. Check for dirt/moisture
3. Verify mounting stability
4. Test different times of day
5. Consider lens upgrade

### Detection Issues

**Too Many False Positives:**
1. Increase confidence threshold (0.75-0.80)
2. Require more consecutive detections (3)
3. Increase pattern sensitivity (0.7)
4. Adjust PIR position/sensitivity
5. Review camera placement

**Missing Real Detections:**
1. Lower confidence threshold (0.65)
2. Reduce consecutive detections (1-2)
3. Decrease pattern sensitivity (0.5)
4. Check PIR sensor range
5. Verify camera has clear view

### Power Problems

**Battery Draining:**
1. Check for light leaks (PIR always on)
2. Verify deep sleep working
3. Test solar panel output
4. Check for short circuits
5. Consider larger battery

**Not Charging:**
1. Verify solar panel connection
2. Check charge controller
3. Test in direct sunlight
4. Measure panel output voltage
5. Check battery terminals

### Network Issues

**LoRa Not Connecting:**
1. Verify frequency settings
2. Check antenna connection
3. Increase TX power
4. Adjust spreading factor
5. Test line-of-sight
6. Check for interference

**Alerts Not Sending:**
1. Verify LoRa connectivity
2. Check message queue size
3. Review transmission logs
4. Test with simple message
5. Increase retry count

---

## Emergency Procedures

### System Failure
1. Note error codes/messages
2. Document conditions (weather, etc.)
3. Check power supply first
4. Attempt system reboot
5. Retrieve logs if possible
6. Contact support with details

### Dangerous Wildlife Encounter
1. Do not approach the camera
2. Note the species and behavior
3. Alert park rangers/authorities
4. Review camera footage remotely
5. Wait for official clearance
6. Document encounter details

### Equipment Theft/Vandalism
1. Document the scene
2. Report to authorities
3. Check for recovered data
4. Review security measures
5. Consider backup units
6. Update insurance claim

---

## Success Criteria

### Deployment Considered Successful When:

**Technical Performance:**
- [ ] 95%+ system uptime
- [ ] <5% false positive rate
- [ ] Battery maintains >70% charge
- [ ] All species correctly identified
- [ ] Alerts delivered within 2 seconds
- [ ] No hardware failures

**Data Quality:**
- [ ] Clear, focused images
- [ ] Accurate timestamps
- [ ] Complete metadata
- [ ] Reliable classifications
- [ ] Comprehensive logs

**Operational:**
- [ ] Easy to maintain
- [ ] Weatherproof in all conditions
- [ ] Minimal intervention needed
- [ ] Cost-effective operation
- [ ] Meets project objectives

---

## Long-Term Monitoring

### Quarterly Review
- [ ] Analyze 3-month data trends
- [ ] Compare to baseline metrics
- [ ] Identify seasonal patterns
- [ ] Update species models
- [ ] Optimize configurations
- [ ] Plan hardware upgrades

### Annual Review
- [ ] Full system evaluation
- [ ] Hardware replacement planning
- [ ] Software updates
- [ ] Network expansion consideration
- [ ] Budget review
- [ ] Research publication planning

---

## Support Resources

- **Documentation**: /docs folder
- **Examples**: /examples folder  
- **GitHub Issues**: Report problems
- **Community Forum**: Get help
- **Email Support**: wildlife-support@wildcam.org

---

## Deployment Log

**Location**: ________________________________________________

**Date Deployed**: ____________________

**Deployed By**: ________________________________________________

**GPS Coordinates**: Lat ________________ Lon ________________

**Initial Configuration**:
- Firmware Version: ________________
- Classifier Threshold: ________________
- Alert Threshold: ________________
- Battery Capacity: ________________

**24-Hour Test Results**: [ ] Pass  [ ] Fail  [ ] Needs Adjustment

**Week 1 Status**: [ ] Operational  [ ] Issues  [ ] Needs Service

**Month 1 Status**: [ ] Operational  [ ] Issues  [ ] Needs Service

**Notes**:
________________________________________________________________
________________________________________________________________
________________________________________________________________

---

**Checklist Version**: 1.0  
**Last Updated**: 2024  
**Next Review**: Quarterly
