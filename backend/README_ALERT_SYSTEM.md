# Advanced ML Alert System - Quick Start Guide

## Overview

The Advanced ML Alert System provides intelligent wildlife monitoring alerts with machine learning-based false positive reduction and context-aware prioritization.

## Key Features

✅ **ML-Based False Positive Reduction** - Pattern learning from historical data  
✅ **Context-Aware Prioritization** - 4-tier severity system (info, warning, critical, emergency)  
✅ **Anomaly Detection** - Statistical detection of unusual behavior patterns  
✅ **Time-Series Forecasting** - Predictive activity analysis  
✅ **User Feedback Learning** - Continuous improvement from user input  
✅ **Alert Correlation** - Automatic grouping and deduplication  
✅ **Comprehensive API** - Full REST API for alert management  
✅ **Background Processing** - Celery-based async task processing  

## Quick Start

### 1. Install Dependencies

```bash
cd backend
pip install -r requirements.txt
```

### 2. Run Database Migration

```bash
# Apply the alert system schema
psql -U wildlife_user -d wildlife_db -f migrations/versions/add_ml_alert_system.sql
```

### 3. Configure Environment

Create `.env` file:

```bash
DATABASE_URL=postgresql://wildlife_user:password@localhost:5432/wildlife_db
REDIS_URL=redis://localhost:6379
SECRET_KEY=your-secret-key
JWT_SECRET_KEY=your-jwt-secret
```

### 4. Start Celery Worker

```bash
celery -A tasks.alert_tasks worker --loglevel=info
```

### 5. Start Flask Application

```bash
python app.py
```

### 6. Run Demo

```bash
python examples/alert_system_demo.py
```

## Usage Examples

### Create Alert from Detection

```python
from services.alert_service import AlertService

alert_service = AlertService()

detection_data = {
    'detected_species': 'bear',
    'confidence_score': 0.89,
    'timestamp': datetime.utcnow(),
    'camera_id': 1,
    'weather_conditions': {'temperature': 20.0, 'humidity': 60.0}
}

alert = alert_service.create_alert(detection_data)
```

### Submit User Feedback

```python
alert_service.process_feedback(
    alert_id=123,
    user_id=1,
    is_false_positive=False,
    rating=5,
    notes="Accurate bear detection"
)
```

### Create Alert Rule

```bash
curl -X POST http://localhost:5000/api/alerts/rules \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "name": "Critical Species Alerts",
    "species_filter": ["bear", "wolf", "mountain_lion"],
    "min_confidence": 0.7,
    "severity_levels": ["critical", "emergency"],
    "email_enabled": true,
    "suppress_false_positives": true
  }'
```

### Get Active Alerts

```bash
curl http://localhost:5000/api/alerts?severity=critical&resolved=false \
  -H "Authorization: Bearer $TOKEN"
```

## API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/alerts` | GET | Get alerts with filtering |
| `/api/alerts/<id>` | GET | Get specific alert |
| `/api/alerts/<id>/acknowledge` | POST | Acknowledge alert |
| `/api/alerts/<id>/resolve` | POST | Resolve alert |
| `/api/alerts/<id>/feedback` | POST | Submit feedback |
| `/api/alerts/rules` | GET/POST | Manage alert rules |
| `/api/alerts/rules/<id>` | PUT/DELETE | Update/delete rule |
| `/api/alerts/analytics` | GET | Get alert analytics |
| `/api/alerts/stats` | GET | Get current statistics |

## Running Tests

```bash
pytest tests/test_alert_system.py -v
```

**Test Results**: 26/26 passing ✅

## Architecture

```
backend/
├── ml/
│   ├── alert_classifier.py      # ML-based alert classification
│   └── anomaly_detector.py      # Behavior anomaly detection
├── services/
│   └── alert_service.py         # Alert creation and management
├── tasks/
│   └── alert_tasks.py           # Background processing tasks
├── routes/
│   └── alerts.py                # REST API endpoints
└── models.py                    # Database models
```

## Performance Metrics

The system tracks:
- **Volume**: Total, filtered, and sent alerts
- **Quality**: False positive rate, ML accuracy
- **Response**: Acknowledgment and resolution times
- **ML**: Pattern learning progress, prediction accuracy

## Monitoring

Check ML accuracy:
```bash
curl http://localhost:5000/api/alerts/stats \
  -H "Authorization: Bearer $TOKEN"
```

View daily metrics:
```bash
curl http://localhost:5000/api/alerts/analytics?days=7 \
  -H "Authorization: Bearer $TOKEN"
```

## Configuration

### Alert Priorities

- **Emergency**: Critical dangerous species (bear, tiger) with >85% confidence
- **Critical**: Dangerous species (wolf, mountain lion) with >75% confidence
- **Warning**: Moderate confidence detections or rare species
- **Info**: Low confidence or common species

### Environmental Filtering

Alerts are automatically filtered in:
- Extreme temperatures: < -5°C or > 40°C
- High wind: > 40 km/h
- Low visibility: < 10m

### Temporal Analysis

Activity scoring:
- Dawn/Dusk (5-8 AM, 5-8 PM): High activity (0.9)
- Midday (9 AM-4 PM): Moderate activity (0.5)
- Night (9 PM-4 AM): Significant activity (0.7)

## Troubleshooting

### High False Positive Rate
1. Lower confidence threshold in alert rules
2. Enable environmental filtering
3. Provide more user feedback on misclassified alerts

### Missing Alerts
1. Verify alert rules are active
2. Check if alerts are being filtered
3. Review min_confidence settings

### Slow Alert Delivery
1. Check Celery worker status
2. Monitor Redis queue size
3. Scale background workers if needed

## Documentation

- **Full Documentation**: `docs/ALERT_SYSTEM.md`
- **API Reference**: See `routes/alerts.py`
- **Tests**: See `tests/test_alert_system.py`
- **Examples**: See `examples/alert_system_demo.py`

## Support

- GitHub Issues: [WildCAM_ESP32 Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Documentation: See `docs/` directory

## License

Part of WildCAM ESP32 Wildlife Monitoring System
