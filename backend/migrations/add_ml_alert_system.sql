-- Migration: Add ML Alert System
-- Description: Adds enhanced alert tables with ML features

-- Add ML-enhanced columns to alerts table
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS ml_confidence FLOAT;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS temporal_consistency FLOAT;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS false_positive_score FLOAT;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS priority INTEGER DEFAULT 100;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS notification_sent BOOLEAN DEFAULT FALSE;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS notification_channels JSONB;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS notification_status JSONB;

-- Update severity column to support new levels
ALTER TABLE alerts ALTER COLUMN severity TYPE VARCHAR(20);
COMMENT ON COLUMN alerts.severity IS 'Alert severity: emergency, critical, warning, info';

-- Create alert_feedback table for user feedback and learning
CREATE TABLE IF NOT EXISTS alert_feedback (
    id SERIAL PRIMARY KEY,
    alert_id INTEGER NOT NULL REFERENCES alerts(id) ON DELETE CASCADE,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    is_accurate BOOLEAN NOT NULL,
    species_correction VARCHAR(100),
    confidence_rating INTEGER CHECK (confidence_rating >= 1 AND confidence_rating <= 5),
    notes TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Create indexes for alert_feedback
CREATE INDEX IF NOT EXISTS idx_alert_feedback_alert_id ON alert_feedback(alert_id);
CREATE INDEX IF NOT EXISTS idx_alert_feedback_user_id ON alert_feedback(user_id);
CREATE INDEX IF NOT EXISTS idx_alert_feedback_created_at ON alert_feedback(created_at);

-- Create alert_preferences table for user notification settings
CREATE TABLE IF NOT EXISTS alert_preferences (
    id SERIAL PRIMARY KEY,
    user_id INTEGER UNIQUE NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    
    -- Channel preferences
    email_enabled BOOLEAN DEFAULT TRUE,
    sms_enabled BOOLEAN DEFAULT FALSE,
    push_enabled BOOLEAN DEFAULT TRUE,
    slack_webhook VARCHAR(500),
    discord_webhook VARCHAR(500),
    custom_webhook VARCHAR(500),
    
    -- Alert level filters
    min_severity VARCHAR(20) DEFAULT 'info',
    dangerous_species_only BOOLEAN DEFAULT FALSE,
    min_confidence FLOAT DEFAULT 0.7 CHECK (min_confidence >= 0.0 AND min_confidence <= 1.0),
    
    -- Quiet hours
    quiet_hours_enabled BOOLEAN DEFAULT FALSE,
    quiet_start_hour INTEGER DEFAULT 22 CHECK (quiet_start_hour >= 0 AND quiet_start_hour <= 23),
    quiet_end_hour INTEGER DEFAULT 7 CHECK (quiet_end_hour >= 0 AND quiet_end_hour <= 23),
    
    -- Rate limiting
    max_alerts_per_hour INTEGER DEFAULT 10 CHECK (max_alerts_per_hour > 0),
    batch_alerts BOOLEAN DEFAULT FALSE,
    
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Create index for alert_preferences
CREATE INDEX IF NOT EXISTS idx_alert_preferences_user_id ON alert_preferences(user_id);

-- Create indexes for enhanced alerts queries
CREATE INDEX IF NOT EXISTS idx_alerts_ml_confidence ON alerts(ml_confidence);
CREATE INDEX IF NOT EXISTS idx_alerts_priority ON alerts(priority);
CREATE INDEX IF NOT EXISTS idx_alerts_severity_created_at ON alerts(severity, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_alerts_camera_severity ON alerts(camera_id, severity);

-- Add comments for documentation
COMMENT ON TABLE alert_feedback IS 'User feedback on alert accuracy for ML learning';
COMMENT ON TABLE alert_preferences IS 'User notification preferences and settings';
COMMENT ON COLUMN alerts.ml_confidence IS 'Enhanced ML confidence score from ensemble methods';
COMMENT ON COLUMN alerts.temporal_consistency IS 'Temporal consistency score across frames';
COMMENT ON COLUMN alerts.false_positive_score IS 'Predicted false positive probability';
COMMENT ON COLUMN alerts.priority IS 'Numeric priority for alert routing';
