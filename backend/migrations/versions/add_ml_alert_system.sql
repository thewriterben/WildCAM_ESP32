-- Migration: Add ML Alert System Schema
-- Description: Extends alerts table and adds new tables for ML-based alert system
-- Version: 2024.01.15

-- Extend alerts table with ML fields
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS priority VARCHAR(20) DEFAULT 'normal';
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS ml_confidence FLOAT DEFAULT 0.5;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS false_positive_score FLOAT DEFAULT 0.0;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS anomaly_score FLOAT DEFAULT 0.0;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS context_data JSONB;

-- Add filtering and correlation fields
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS is_filtered BOOLEAN DEFAULT FALSE;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS filter_reason VARCHAR(200);
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS duplicate_of INTEGER REFERENCES alerts(id);
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS correlation_group VARCHAR(50);

-- Add feedback fields
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS user_rating INTEGER;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS is_false_positive BOOLEAN;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS feedback_notes TEXT;
ALTER TABLE alerts ADD COLUMN IF NOT EXISTS updated_at TIMESTAMP DEFAULT NOW();

-- Update severity column to include new levels
ALTER TABLE alerts ALTER COLUMN severity TYPE VARCHAR(20);
-- Note: Update application to handle: info, warning, critical, emergency

-- Create alert_feedback table
CREATE TABLE IF NOT EXISTS alert_feedback (
    id SERIAL PRIMARY KEY,
    alert_id INTEGER NOT NULL REFERENCES alerts(id) ON DELETE CASCADE,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    feedback_type VARCHAR(50) NOT NULL,
    rating INTEGER CHECK (rating >= 1 AND rating <= 5),
    is_correct BOOLEAN,
    notes TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_alert_feedback_alert_id ON alert_feedback(alert_id);
CREATE INDEX IF NOT EXISTS idx_alert_feedback_user_id ON alert_feedback(user_id);

-- Create alert_rules table
CREATE TABLE IF NOT EXISTS alert_rules (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    camera_id INTEGER REFERENCES cameras(id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    
    -- Rule conditions
    species_filter JSONB,
    min_confidence FLOAT DEFAULT 0.7 CHECK (min_confidence >= 0 AND min_confidence <= 1),
    severity_levels JSONB,
    time_of_day JSONB,
    days_of_week JSONB,
    
    -- Alert delivery preferences
    email_enabled BOOLEAN DEFAULT TRUE,
    sms_enabled BOOLEAN DEFAULT FALSE,
    push_enabled BOOLEAN DEFAULT TRUE,
    webhook_url VARCHAR(500),
    
    -- Smart filtering
    enable_ml_filtering BOOLEAN DEFAULT TRUE,
    suppress_false_positives BOOLEAN DEFAULT TRUE,
    batch_similar_alerts BOOLEAN DEFAULT TRUE,
    batch_window_minutes INTEGER DEFAULT 30,
    
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_alert_rules_user_id ON alert_rules(user_id);
CREATE INDEX IF NOT EXISTS idx_alert_rules_camera_id ON alert_rules(camera_id);
CREATE INDEX IF NOT EXISTS idx_alert_rules_active ON alert_rules(is_active);

-- Create alert_metrics table
CREATE TABLE IF NOT EXISTS alert_metrics (
    id SERIAL PRIMARY KEY,
    date DATE NOT NULL,
    camera_id INTEGER REFERENCES cameras(id) ON DELETE CASCADE,
    
    -- Volume metrics
    total_alerts INTEGER DEFAULT 0,
    filtered_alerts INTEGER DEFAULT 0,
    sent_alerts INTEGER DEFAULT 0,
    
    -- Quality metrics
    false_positive_count INTEGER DEFAULT 0,
    true_positive_count INTEGER DEFAULT 0,
    user_confirmed_count INTEGER DEFAULT 0,
    
    -- Response metrics
    avg_acknowledgment_time_seconds FLOAT,
    avg_resolution_time_seconds FLOAT,
    unacknowledged_count INTEGER DEFAULT 0,
    
    -- ML performance
    ml_accuracy FLOAT,
    false_positive_rate FLOAT,
    false_negative_rate FLOAT,
    
    metadata JSONB,
    created_at TIMESTAMP DEFAULT NOW(),
    
    UNIQUE(date, camera_id)
);

CREATE INDEX IF NOT EXISTS idx_alert_metrics_date ON alert_metrics(date);
CREATE INDEX IF NOT EXISTS idx_alert_metrics_camera_id ON alert_metrics(camera_id);

-- Add indexes for improved query performance
CREATE INDEX IF NOT EXISTS idx_alerts_is_filtered ON alerts(is_filtered);
CREATE INDEX IF NOT EXISTS idx_alerts_correlation_group ON alerts(correlation_group);
CREATE INDEX IF NOT EXISTS idx_alerts_duplicate_of ON alerts(duplicate_of);
CREATE INDEX IF NOT EXISTS idx_alerts_ml_confidence ON alerts(ml_confidence);
CREATE INDEX IF NOT EXISTS idx_alerts_false_positive_score ON alerts(false_positive_score);
CREATE INDEX IF NOT EXISTS idx_alerts_priority ON alerts(priority);
CREATE INDEX IF NOT EXISTS idx_alerts_updated_at ON alerts(updated_at);

-- Create function to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Create trigger for alerts table
DROP TRIGGER IF EXISTS update_alerts_updated_at ON alerts;
CREATE TRIGGER update_alerts_updated_at
    BEFORE UPDATE ON alerts
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Create trigger for alert_rules table
DROP TRIGGER IF EXISTS update_alert_rules_updated_at ON alert_rules;
CREATE TRIGGER update_alert_rules_updated_at
    BEFORE UPDATE ON alert_rules
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Grant permissions (adjust as needed for your setup)
-- GRANT SELECT, INSERT, UPDATE, DELETE ON alerts TO wildlife_app;
-- GRANT SELECT, INSERT, UPDATE, DELETE ON alert_feedback TO wildlife_app;
-- GRANT SELECT, INSERT, UPDATE, DELETE ON alert_rules TO wildlife_app;
-- GRANT SELECT, INSERT, UPDATE, DELETE ON alert_metrics TO wildlife_app;

-- Insert default alert rules for existing users (optional)
-- INSERT INTO alert_rules (user_id, name, description)
-- SELECT id, 'Default Alert Rule', 'Automatically created rule for existing user'
-- FROM users
-- WHERE NOT EXISTS (SELECT 1 FROM alert_rules WHERE alert_rules.user_id = users.id);

COMMIT;
