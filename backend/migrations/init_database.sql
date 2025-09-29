-- ESP32 Wildlife Camera Database Schema
-- PostgreSQL initialization script

-- Create database (run this as superuser)
-- CREATE DATABASE wildlife_db;
-- CREATE USER wildlife_user WITH ENCRYPTED PASSWORD 'secure_password';
-- GRANT ALL PRIVILEGES ON DATABASE wildlife_db TO wildlife_user;

-- Connect to wildlife_db before running the following:

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Users table for authentication
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(80) UNIQUE NOT NULL,
    email VARCHAR(120) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    first_name VARCHAR(50),
    last_name VARCHAR(50),
    role VARCHAR(20) DEFAULT 'user' CHECK (role IN ('admin', 'user', 'researcher')),
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);

-- Cameras table for device registration
CREATE TABLE IF NOT EXISTS cameras (
    id SERIAL PRIMARY KEY,
    device_id VARCHAR(50) UNIQUE NOT NULL,
    name VARCHAR(100),
    location_name VARCHAR(200),
    latitude DOUBLE PRECISION,
    longitude DOUBLE PRECISION,
    altitude DOUBLE PRECISION,
    last_seen TIMESTAMP,
    battery_level DOUBLE PRECISION,
    solar_voltage DOUBLE PRECISION,
    temperature DOUBLE PRECISION,
    humidity DOUBLE PRECISION,
    firmware_version VARCHAR(20),
    hardware_version VARCHAR(20),
    config JSONB,
    status VARCHAR(20) DEFAULT 'active' CHECK (status IN ('active', 'inactive', 'maintenance')),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Species reference table
CREATE TABLE IF NOT EXISTS species (
    id SERIAL PRIMARY KEY,
    common_name VARCHAR(100) NOT NULL,
    scientific_name VARCHAR(100),
    category VARCHAR(50),
    conservation_status VARCHAR(30),
    description TEXT,
    habitat VARCHAR(200),
    behavior_notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Wildlife detections table
CREATE TABLE IF NOT EXISTS wildlife_detections (
    id SERIAL PRIMARY KEY,
    camera_id INTEGER REFERENCES cameras(id) ON DELETE CASCADE,
    species_id INTEGER REFERENCES species(id),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
    image_url VARCHAR(500),
    thumbnail_url VARCHAR(500),
    
    -- AI Classification results
    detected_species VARCHAR(100),
    confidence_score DOUBLE PRECISION CHECK (confidence_score >= 0 AND confidence_score <= 1),
    bounding_box JSONB,
    additional_detections JSONB,
    
    -- Manual verification
    verified BOOLEAN DEFAULT FALSE,
    verified_species VARCHAR(100),
    verified_by INTEGER REFERENCES users(id),
    verification_notes TEXT,
    verification_timestamp TIMESTAMP,
    
    -- Context data
    weather_conditions JSONB,
    motion_trigger_data JSONB,
    metadata JSONB,
    
    -- Flags
    is_false_positive BOOLEAN DEFAULT FALSE,
    is_research_grade BOOLEAN DEFAULT FALSE,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Alerts table
CREATE TABLE IF NOT EXISTS alerts (
    id SERIAL PRIMARY KEY,
    camera_id INTEGER REFERENCES cameras(id),
    detection_id INTEGER REFERENCES wildlife_detections(id),
    alert_type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) DEFAULT 'info' CHECK (severity IN ('critical', 'warning', 'info')),
    title VARCHAR(200) NOT NULL,
    message TEXT,
    data JSONB,
    
    -- Status tracking
    acknowledged BOOLEAN DEFAULT FALSE,
    acknowledged_by INTEGER REFERENCES users(id),
    acknowledged_at TIMESTAMP,
    resolved BOOLEAN DEFAULT FALSE,
    resolved_at TIMESTAMP,
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Analytics data table for aggregated metrics
CREATE TABLE IF NOT EXISTS analytics_data (
    id SERIAL PRIMARY KEY,
    metric_type VARCHAR(50) NOT NULL,
    time_period VARCHAR(20) NOT NULL CHECK (time_period IN ('hourly', 'daily', 'weekly', 'monthly')),
    timestamp TIMESTAMP NOT NULL,
    camera_id INTEGER REFERENCES cameras(id),
    species_id INTEGER REFERENCES species(id),
    value DOUBLE PRECISION NOT NULL,
    metadata JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create indexes for better performance
CREATE INDEX IF NOT EXISTS idx_cameras_device_id ON cameras(device_id);
CREATE INDEX IF NOT EXISTS idx_cameras_status ON cameras(status);
CREATE INDEX IF NOT EXISTS idx_cameras_last_seen ON cameras(last_seen);

CREATE INDEX IF NOT EXISTS idx_detections_camera_id ON wildlife_detections(camera_id);
CREATE INDEX IF NOT EXISTS idx_detections_timestamp ON wildlife_detections(timestamp);
CREATE INDEX IF NOT EXISTS idx_detections_species ON wildlife_detections(detected_species);
CREATE INDEX IF NOT EXISTS idx_detections_verified ON wildlife_detections(verified);
CREATE INDEX IF NOT EXISTS idx_detections_confidence ON wildlife_detections(confidence_score);

CREATE INDEX IF NOT EXISTS idx_alerts_camera_id ON alerts(camera_id);
CREATE INDEX IF NOT EXISTS idx_alerts_type ON alerts(alert_type);
CREATE INDEX IF NOT EXISTS idx_alerts_severity ON alerts(severity);
CREATE INDEX IF NOT EXISTS idx_alerts_acknowledged ON alerts(acknowledged);
CREATE INDEX IF NOT EXISTS idx_alerts_created_at ON alerts(created_at);

CREATE INDEX IF NOT EXISTS idx_analytics_metric_type ON analytics_data(metric_type);
CREATE INDEX IF NOT EXISTS idx_analytics_timestamp ON analytics_data(timestamp);
CREATE INDEX IF NOT EXISTS idx_analytics_camera_id ON analytics_data(camera_id);

-- Grant permissions to wildlife_user
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO wildlife_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO wildlife_user;