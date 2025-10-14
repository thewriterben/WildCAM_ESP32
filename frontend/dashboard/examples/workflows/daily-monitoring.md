# Daily Monitoring Workflow

**Daily routine for wildlife monitoring power users**

## Morning Check (5-10 minutes)

### 1. Dashboard Overview
```
Press: D (Dashboard)
```

**Check:**
- [ ] Total active cameras (should match expected)
- [ ] Overnight detection count
- [ ] Battery levels (flag any < 20%)
- [ ] System health indicators
- [ ] Any alert notifications

**Action Items:**
- Note cameras with low battery for field visit
- Review any system alerts
- Check storage capacity warnings

### 2. Review New Detections
```
Press: L (Live Detections)
Filter: Last 24 hours
```

**Tasks:**
- [ ] Scan thumbnails for interesting captures
- [ ] Verify species identification accuracy
- [ ] Flag any unusual behavior or rare species
- [ ] Check detection confidence scores
- [ ] Note any camera positioning issues

**Quality Check:**
```javascript
// In browser console
const lowConfidence = detections.filter(d => d.confidence < 0.7);
console.log(`Low confidence detections: ${lowConfidence.length}`);
```

### 3. Species Activity Summary
```
Press: A (Analytics)
Time Range: Last 24 hours
```

**Review:**
- [ ] Species distribution pie chart
- [ ] Hourly activity patterns
- [ ] Compare to previous day
- [ ] Note any pattern changes
- [ ] Identify peak activity times

**Export Daily Summary:**
```javascript
// Export yesterday's data
const analytics = await apiService.getSpeciesAnalytics(1);
downloadJSON(analytics, `daily_summary_${today}.json`);
```

## Midday Check (3-5 minutes)

### 1. System Health
```
Press: C (Camera Management)
Sort by: Battery Level
```

**Quick Scan:**
- [ ] Any cameras offline?
- [ ] Battery levels declining normally?
- [ ] Storage capacity adequate?
- [ ] Network connectivity stable?

### 2. Real-Time Monitoring
```
Press: L (Live Detections)
Enable: Auto-refresh
```

**Watch for:**
- New detections coming in
- System responsiveness
- Detection quality

## Evening Review (10-15 minutes)

### 1. Daily Analytics
```
Press: A (Analytics)
Time Range: Today
```

**Analyze:**
- [ ] Total detections for the day
- [ ] Species diversity index
- [ ] Detection success rate
- [ ] Activity peak times
- [ ] Comparison to weekly average

### 2. Camera Performance
```
Press: C (Camera Management)
```

**Review Each Camera:**
```javascript
cameras.forEach(cam => {
  console.log(`${cam.name}:
    Detections: ${cam.daily_detections}
    Battery: ${cam.battery_level}%
    Storage: ${cam.storage_used}/${cam.storage_total}
    Uptime: ${cam.uptime_hours}h
  `);
});
```

### 3. Data Quality Check
```
Press: L (Live Detections)
```

**Check for:**
- [ ] False positives (confidence < 0.5)
- [ ] Missed detections (obvious animals not detected)
- [ ] Image quality issues
- [ ] Timestamp accuracy
- [ ] GPS coordinate consistency

### 4. Prepare Daily Report

**Generate Summary:**
```javascript
const dailyReport = {
  date: new Date().toISOString().split('T')[0],
  totalDetections: getTotalDetections(),
  speciesCount: getUniqueSpecies().length,
  topSpecies: getTopSpecies(5),
  cameraStatus: getCameraHealthSummary(),
  alerts: getActiveAlerts(),
  notes: getUserNotes()
};

downloadJSON(dailyReport, `daily_report_${today}.json`);
```

## Weekly Tasks (Fridays)

### 1. Weekly Analytics Review
```
Press: A (Analytics)
Time Range: Last 7 days
```

**Generate Reports:**
- [ ] Species frequency trends
- [ ] Activity pattern analysis
- [ ] Camera performance metrics
- [ ] Detection accuracy stats
- [ ] System uptime report

### 2. Camera Maintenance Planning
```
Press: C (Camera Management)
```

**Identify:**
- Cameras needing battery replacement
- Cameras needing SD card maintenance
- Cameras with positioning issues
- Firmware updates needed

**Create Maintenance Schedule:**
```javascript
const maintenanceNeeded = cameras.filter(cam => 
  cam.battery_level < 30 || 
  cam.storage_used / cam.storage_total > 0.85 ||
  cam.days_since_maintenance > 30
);

exportMaintenanceList(maintenanceNeeded);
```

### 3. Data Backup
```bash
# Export all detection data
npm run export-data -- --days=7 --format=csv

# Backup to external drive
rsync -av exports/ /mnt/backup/wildcam/
```

## Monthly Tasks (1st of month)

### 1. Comprehensive Analysis
```
Press: A (Analytics)
Time Range: Last 30 days
```

**Generate:**
- [ ] Monthly species report
- [ ] Population trend analysis
- [ ] Seasonal pattern comparison
- [ ] Camera network performance
- [ ] Cost per detection metrics

### 2. System Optimization
```
Review:
- API response times
- Database query performance
- Storage utilization trends
- Network bandwidth usage
```

### 3. Update Documentation
- [ ] Update species observations log
- [ ] Document unusual behaviors
- [ ] Record environmental changes
- [ ] Update camera network map
- [ ] Archive old data

## Quick Reference Commands

### Keyboard Shortcuts
```
D - Dashboard Home
L - Live Detections
A - Analytics
C - Camera Management
M - Map View
S - Settings
R - Refresh Current Page
T - Toggle Theme
/ - Focus Search
? - Show Help
```

### Common Filters
```
Species: deer, bear, fox, raccoon, etc.
Date: today, yesterday, last-7-days, last-30-days
Confidence: >0.8, >0.5, <0.5
Camera: CAM-001, CAM-002, etc.
```

### Quick Actions
```javascript
// Refresh dashboard
wildcam.refresh();

// Get today's stats
wildcam.getStats({ days: 1 });

// Export detections
wildcam.exportDetections({ 
  from: '2024-01-01',
  to: '2024-01-31',
  format: 'csv'
});

// Check camera health
wildcam.getCameraHealth();
```

## Alerts to Watch

### Critical (Immediate Action)
- 🔴 Camera offline > 2 hours
- 🔴 Battery < 10%
- 🔴 Storage > 95% full
- 🔴 System errors

### Warning (Action Soon)
- 🟡 Battery < 30%
- 🟡 Storage > 80% full
- 🟡 Low detection confidence
- 🟡 Camera maintenance due

### Info (Monitor)
- 🟢 Normal operations
- 🟢 Detection uploaded
- 🟢 Configuration updated
- 🟢 Firmware updated

## Tips for Efficiency

1. **Use Keyboard Shortcuts**: Navigate 3x faster
2. **Set Up Auto-Refresh**: Stay updated without manual refresh
3. **Create Custom Filters**: Save common search criteria
4. **Use Batch Operations**: Update multiple cameras at once
5. **Export Regularly**: Keep offline backups
6. **Monitor Trends**: Look for patterns, not just numbers
7. **Document Anomalies**: Note unusual observations
8. **Schedule Maintenance**: Proactive > Reactive

## Troubleshooting Common Issues

### Issue: Missing Detections
```
Check:
1. Camera power and connectivity
2. Motion sensitivity settings
3. Detection threshold too high
4. SD card full
5. Firmware version
```

### Issue: Poor Image Quality
```
Check:
1. Lens cleanliness
2. Photo quality setting
3. Lighting conditions
4. Camera positioning
5. Focus calibration
```

### Issue: False Positives
```
Adjust:
1. Lower motion sensitivity
2. Increase detection threshold
3. Review camera placement
4. Check for vegetation movement
5. Update AI model
```

---

**Version**: 1.0  
**Last Updated**: 2025-10-14  
**Author**: WildCAM ESP32 Team
