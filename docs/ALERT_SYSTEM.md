# Advanced ML Alert System

## Overview

The Advanced ML Alert System is an intelligent wildlife monitoring alert platform that uses machine learning to dramatically reduce false positives and provide context-aware notifications. The system integrates anomaly detection, environmental context analysis, and user feedback learning to continuously improve alert accuracy.

## Architecture

### Core Components

1. **Alert Classifier** (`backend/ml/alert_classifier.py`)
   - ML-based false positive detection
   - Context-aware alert prioritization
   - Pattern learning from historical data
   - Confidence scoring

2. **Anomaly Detector** (`backend/ml/anomaly_detector.py`)
   - Behavior anomaly detection
   - Time-series forecasting
   - Activity baseline establishment
   - Temporal pattern analysis

3. **Alert Service** (`backend/services/alert_service.py`)
   - Alert creation and evaluation
   - Deduplication and correlation
   - Metrics tracking
   - User feedback processing

4. **Background Tasks** (`backend/tasks/alert_tasks.py`)
   - Asynchronous alert processing
   - Notification delivery
   - Webhook integration
   - Scheduled maintenance tasks

5. **API Routes** (`backend/routes/alerts.py`)
   - RESTful alert management
   - Alert rule configuration
   - Analytics endpoints
   - Feedback submission

## Features

### Smart Alert Engine

#### ML-Based False Positive Reduction
The system uses pattern matching to identify and filter false positives:

```python
from services.alert_service import AlertService

alert_service = AlertService()

detection_data = {
    'detected_species': 'deer',
    'confidence_score': 0.75,
    'timestamp': datetime.utcnow(),
    'weather_conditions': {'temperature': 20.0, 'humidity': 60.0},
    'motion_trigger_data': {'motion_level': 0.7}
}

alert = alert_service.create_alert(detection_data)
# Alert is automatically evaluated and filtered if needed
```

#### Context-Aware Prioritization
Alerts are prioritized based on multiple factors:
- **Emergency**: Critical dangerous species (bear, tiger) with high confidence (>85%)
- **Critical**: Dangerous species (wolf, mountain lion) with good confidence (>75%)
- **Warning**: Moderate confidence detections or rare species
- **Info**: Low confidence or common species

#### Environmental Context Integration
The system filters alerts based on environmental conditions:
- Extreme temperatures (-5°C to 40°C)
- High wind speeds (>40 km/h)
- Low visibility (<10m)
- Poor air quality

#### Temporal Analysis
Activity is analyzed by time of day:
- **Dawn/Dusk (5-8 AM, 5-8 PM)**: High activity periods (score: 0.9)
- **Midday (9 AM-4 PM)**: Moderate activity (score: 0.5)
- **Night (9 PM-4 AM)**: Significant but variable (score: 0.7)

### Multi-Tier Alert System

Alerts are organized into four severity levels:
1. **Info**: Routine detections, informational only
2. **Warning**: Significant detections requiring attention
3. **Critical**: Dangerous species or unusual patterns
4. **Emergency**: Immediate threat requiring urgent response

### Alert Correlation and Deduplication

The system automatically:
- Groups related alerts within a 10-minute window
- Identifies and marks duplicate alerts
- Creates correlation groups for similar detections
- Prevents alert fatigue through intelligent batching

### Behavioral Pattern Recognition

#### Anomaly Detection
Detects unusual wildlife behavior:
- High/low activity anomalies (z-score > 2.5)
- Temporal anomalies (activity at unusual times)
- Baseline comparison over 24-hour windows

#### Time-Series Forecasting
Predicts expected activity levels:
- Hourly pattern learning
- Confidence interval calculation
- Unexpected activity detection

### Learning from User Feedback

Users can provide feedback on alerts:

```python
alert_service.process_feedback(
    alert_id=123,
    user_id=1,
    is_false_positive=True,
    rating=2,
    notes="Wind triggered motion sensor"
)
```

The system learns from this feedback to improve future classifications.

## API Usage

### Get Active Alerts

```bash
GET /api/alerts?severity=critical&resolved=false&limit=50
```

Response:
```json
{
  "alerts": [
    {
      "id": 123,
      "title": "⚠️ CRITICAL: Wolf detected",
      "severity": "critical",
      "priority": "high",
      "ml_confidence": 0.82,
      "false_positive_score": 0.15,
      "anomaly_score": 0.3,
      "created_at": "2024-01-15T14:30:00Z"
    }
  ],
  "total": 1,
  "limit": 50,
  "offset": 0
}
```

### Submit Alert Feedback

```bash
POST /api/alerts/123/feedback
Content-Type: application/json

{
  "is_false_positive": false,
  "rating": 5,
  "notes": "Accurate detection, wolf confirmed"
}
```

### Create Alert Rule

```bash
POST /api/alerts/rules
Content-Type: application/json

{
  "name": "Critical Species Alerts",
  "species_filter": ["bear", "wolf", "mountain_lion"],
  "min_confidence": 0.7,
  "severity_levels": ["critical", "emergency"],
  "time_of_day": [6, 7, 8, 17, 18, 19, 20],
  "email_enabled": true,
  "push_enabled": true,
  "suppress_false_positives": true
}
```

### Get Alert Analytics

```bash
GET /api/alerts/analytics?days=7&camera_id=5
```

Response:
```json
{
  "metrics": [
    {
      "date": "2024-01-15",
      "total_alerts": 45,
      "filtered_alerts": 12,
      "sent_alerts": 33,
      "false_positive_count": 3,
      "ml_accuracy": 0.91
    }
  ],
  "ml_performance": {
    "accuracy": 0.91,
    "false_positive_rate": 0.09,
    "total_feedback": 150
  }
}
```

## Database Schema

### Alert Model
```sql
CREATE TABLE alerts (
    id SERIAL PRIMARY KEY,
    camera_id INTEGER REFERENCES cameras(id),
    detection_id INTEGER REFERENCES wildlife_detections(id),
    alert_type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) DEFAULT 'info',
    priority VARCHAR(20) DEFAULT 'normal',
    title VARCHAR(200) NOT NULL,
    message TEXT,
    data JSONB,
    
    -- ML fields
    ml_confidence FLOAT DEFAULT 0.5,
    false_positive_score FLOAT DEFAULT 0.0,
    anomaly_score FLOAT DEFAULT 0.0,
    context_data JSONB,
    
    -- Filtering
    is_filtered BOOLEAN DEFAULT FALSE,
    filter_reason VARCHAR(200),
    duplicate_of INTEGER REFERENCES alerts(id),
    correlation_group VARCHAR(50),
    
    -- Status
    acknowledged BOOLEAN DEFAULT FALSE,
    resolved BOOLEAN DEFAULT FALSE,
    
    -- Feedback
    user_rating INTEGER,
    is_false_positive BOOLEAN,
    
    created_at TIMESTAMP DEFAULT NOW()
);
```

### AlertRule Model
```sql
CREATE TABLE alert_rules (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) NOT NULL,
    camera_id INTEGER REFERENCES cameras(id),
    name VARCHAR(100) NOT NULL,
    
    -- Conditions
    species_filter JSONB,
    min_confidence FLOAT DEFAULT 0.7,
    severity_levels JSONB,
    time_of_day JSONB,
    
    -- Delivery
    email_enabled BOOLEAN DEFAULT TRUE,
    push_enabled BOOLEAN DEFAULT TRUE,
    webhook_url VARCHAR(500),
    
    -- Smart filtering
    enable_ml_filtering BOOLEAN DEFAULT TRUE,
    suppress_false_positives BOOLEAN DEFAULT TRUE,
    
    is_active BOOLEAN DEFAULT TRUE
);
```

## Background Processing

### Celery Tasks

The system uses Celery for asynchronous processing:

```python
from tasks.alert_tasks import process_detection_alert

# Queue alert processing
process_detection_alert.delay(detection_id=456)
```

Available tasks:
- `process_detection_alert`: Evaluate and create alerts from detections
- `deliver_alert_notifications`: Send notifications to users
- `send_webhook_notification`: Trigger webhooks
- `update_daily_metrics`: Calculate daily statistics
- `cleanup_old_alerts`: Remove old resolved alerts

### Scheduled Tasks

Configure periodic tasks in Celery beat:

```python
from celery.schedules import crontab

app.conf.beat_schedule = {
    'update-metrics-daily': {
        'task': 'tasks.update_daily_metrics',
        'schedule': crontab(hour=0, minute=0),
    },
    'cleanup-old-alerts': {
        'task': 'tasks.cleanup_old_alerts',
        'schedule': crontab(hour=2, minute=0),
        'args': (90,)  # Keep 90 days
    }
}
```

## Performance Metrics

The system tracks comprehensive performance metrics:

### Volume Metrics
- Total alerts generated
- Filtered vs sent alerts
- Alert frequency by camera/species

### Quality Metrics
- False positive rate
- True positive rate
- User confirmation rate
- ML prediction accuracy

### Response Metrics
- Average acknowledgment time
- Average resolution time
- Unacknowledged alert count

### ML Performance
- Classification accuracy
- Pattern learning progress
- Feature importance

## Integration Guide

### With Dangerous Species Alert System

```python
from services.alert_service import AlertService
from ai.dangerous_species_alert import DangerousSpeciesAlertSystem

alert_service = AlertService()
dangerous_species = DangerousSpeciesAlertSystem()

# Process dangerous species detection
if dangerous_species.isEnabled():
    result = classifier.classify(image)
    if result.isDangerous():
        alert = alert_service.create_alert({
            'detected_species': result.species,
            'confidence_score': result.confidence,
            'timestamp': datetime.utcnow(),
            'camera_id': camera_id
        }, alert_type='dangerous_species')
```

### With Environmental Sensors

```python
from sensors.environmental_integration import getEnvironmentalData

# Include environmental context in alerts
env_data = getEnvironmentalData()
detection_data['weather_conditions'] = {
    'temperature': env_data.temperature,
    'humidity': env_data.humidity,
    'wind_speed': env_data.wind_speed
}

alert = alert_service.create_alert(detection_data)
```

## Best Practices

1. **Alert Rule Configuration**
   - Start with conservative thresholds (min_confidence > 0.7)
   - Enable ML filtering and false positive suppression
   - Configure time windows to avoid alert fatigue
   - Use species filters to focus on priority animals

2. **Feedback Provision**
   - Provide feedback on both false positives and true positives
   - Include detailed notes for edge cases
   - Rate alert usefulness to help improve prioritization
   - Regular feedback improves ML accuracy significantly

3. **Webhook Integration**
   - Implement retry logic on receiving end
   - Validate payload signatures for security
   - Handle rate limits appropriately
   - Log webhook failures for troubleshooting

4. **Performance Optimization**
   - Use Redis caching for frequently accessed alerts
   - Enable alert batching for high-volume cameras
   - Configure appropriate cleanup schedules
   - Monitor database query performance

5. **Testing**
   - Test alert rules with historical data
   - Validate ML filtering effectiveness
   - Monitor false positive rates
   - A/B test new alert algorithms

## Troubleshooting

### High False Positive Rate
1. Lower confidence threshold in alert rules
2. Enable environmental filtering
3. Review and provide feedback on misclassified alerts
4. Check motion detection sensitivity settings

### Missing Alerts
1. Verify alert rules are active
2. Check if alerts are being filtered
3. Review min_confidence settings
4. Examine time_of_day restrictions

### Slow Alert Delivery
1. Check Celery worker status
2. Monitor Redis queue size
3. Review webhook endpoint performance
4. Scale background workers if needed

### Inaccurate ML Predictions
1. Provide more user feedback
2. Review pattern learning statistics
3. Check feature extraction accuracy
4. Consider model retraining

## Future Enhancements

- [ ] Deep learning models for advanced classification
- [ ] Multi-camera alert correlation
- [ ] Predictive alerting based on patterns
- [ ] Mobile app push notifications
- [ ] Integration with wildlife databases
- [ ] Real-time alert streaming via WebSocket
- [ ] Custom alert templates
- [ ] Alert scheduling and snoozing
- [ ] Geographic clustering
- [ ] Species-specific alert tuning

## Support

For issues or questions:
- GitHub Issues: [WildCAM_ESP32 Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Documentation: See `docs/` directory
- API Reference: `/api/docs` endpoint
