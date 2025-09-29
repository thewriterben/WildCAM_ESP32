-- Sample data for ESP32 Wildlife Camera system
-- Use for development and testing

-- Insert sample species data
INSERT INTO species (common_name, scientific_name, category, conservation_status, description, habitat) VALUES
('White-tailed Deer', 'Odocoileus virginianus', 'mammal', 'least_concern', 'Common North American deer species', 'Forests, grasslands, agricultural areas'),
('Red Fox', 'Vulpes vulpes', 'mammal', 'least_concern', 'Adaptable carnivore found across many habitats', 'Forests, grasslands, urban areas'),
('American Black Bear', 'Ursus americanus', 'mammal', 'least_concern', 'Large omnivorous bear native to North America', 'Forests, swamps, mountains'),
('Wild Turkey', 'Meleagris gallopavo', 'bird', 'least_concern', 'Large ground-dwelling bird', 'Forests, grasslands'),
('Great Blue Heron', 'Ardea herodias', 'bird', 'least_concern', 'Large wading bird', 'Wetlands, shores, rivers'),
('Eastern Gray Squirrel', 'Sciurus carolinensis', 'mammal', 'least_concern', 'Common tree squirrel', 'Deciduous and mixed forests'),
('Raccoon', 'Procyon lotor', 'mammal', 'least_concern', 'Highly adaptable omnivore', 'Forests, urban areas, wetlands'),
('Northern Cardinal', 'Cardinalis cardinalis', 'bird', 'least_concern', 'Bright red songbird', 'Woodlands, gardens, parks'),
('American Robin', 'Turdus migratorius', 'bird', 'least_concern', 'Common thrush species', 'Parks, gardens, forests'),
('Coyote', 'Canis latrans', 'mammal', 'least_concern', 'Adaptable canid', 'Prairies, forests, urban areas')
ON CONFLICT (common_name) DO NOTHING;

-- Insert sample admin user (password: admin123)
INSERT INTO users (username, email, password_hash, first_name, last_name, role) VALUES
('admin', 'admin@wildcam.local', 'pbkdf2:sha256:260000$vZFLPMJZc5H7Q8Nr$8c5cca4b7c7b5b7b6c6d8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a', 'Admin', 'User', 'admin'),
('researcher', 'researcher@wildcam.local', 'pbkdf2:sha256:260000$vZFLPMJZc5H7Q8Nr$8c5cca4b7c7b5b7b6c6d8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a', 'Research', 'User', 'researcher'),
('viewer', 'viewer@wildcam.local', 'pbkdf2:sha256:260000$vZFLPMJZc5H7Q8Nr$8c5cca4b7c7b5b7b6c6d8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a', 'View', 'User', 'user')
ON CONFLICT (username) DO NOTHING;

-- Insert sample cameras
INSERT INTO cameras (device_id, name, location_name, latitude, longitude, altitude, firmware_version, hardware_version, config, status) VALUES
('ESP32-CAM-001', 'Forest Trail Camera', 'Main Forest Trail', 45.5236, -122.6750, 150.0, '2.1.0', 'v1.2', '{"motion_sensitivity": 75, "night_vision": true, "photo_quality": "high"}', 'active'),
('ESP32-CAM-002', 'Creek Side Monitor', 'Salmon Creek', 45.5289, -122.6711, 120.0, '2.1.0', 'v1.2', '{"motion_sensitivity": 60, "night_vision": true, "photo_quality": "medium"}', 'active'),
('ESP32-CAM-003', 'Meadow Observer', 'Oak Meadow', 45.5195, -122.6834, 180.0, '2.0.5', 'v1.1', '{"motion_sensitivity": 85, "night_vision": false, "photo_quality": "high"}', 'maintenance'),
('ESP32-CAM-004', 'Ridge Watcher', 'Pine Ridge', 45.5334, -122.6622, 220.0, '2.1.0', 'v1.2', '{"motion_sensitivity": 70, "night_vision": true, "photo_quality": "high"}', 'active')
ON CONFLICT (device_id) DO NOTHING;

-- Update camera status with realistic data
UPDATE cameras SET 
    last_seen = CURRENT_TIMESTAMP - (RANDOM() * INTERVAL '2 hours'),
    battery_level = 65.0 + (RANDOM() * 30),
    solar_voltage = 12.0 + (RANDOM() * 2),
    temperature = 15.0 + (RANDOM() * 20),
    humidity = 45.0 + (RANDOM() * 30)
WHERE device_id = 'ESP32-CAM-001';

UPDATE cameras SET 
    last_seen = CURRENT_TIMESTAMP - (RANDOM() * INTERVAL '1 hour'),
    battery_level = 80.0 + (RANDOM() * 15),
    solar_voltage = 13.2 + (RANDOM() * 1),
    temperature = 18.0 + (RANDOM() * 15),
    humidity = 55.0 + (RANDOM() * 25)
WHERE device_id = 'ESP32-CAM-002';

UPDATE cameras SET 
    last_seen = CURRENT_TIMESTAMP - INTERVAL '4 hours',
    battery_level = 25.0,
    solar_voltage = 10.5,
    temperature = 12.0,
    humidity = 70.0
WHERE device_id = 'ESP32-CAM-003';

UPDATE cameras SET 
    last_seen = CURRENT_TIMESTAMP - (RANDOM() * INTERVAL '30 minutes'),
    battery_level = 90.0 + (RANDOM() * 10),
    solar_voltage = 13.8 + (RANDOM() * 0.5),
    temperature = 20.0 + (RANDOM() * 10),
    humidity = 40.0 + (RANDOM() * 35)
WHERE device_id = 'ESP32-CAM-004';

-- Insert sample wildlife detections
INSERT INTO wildlife_detections (camera_id, species_id, timestamp, detected_species, confidence_score, bounding_box, weather_conditions, metadata) 
SELECT 
    c.id,
    s.id,
    CURRENT_TIMESTAMP - (RANDOM() * INTERVAL '7 days'),
    s.common_name,
    0.7 + (RANDOM() * 0.3),
    jsonb_build_object('x', floor(RANDOM() * 200), 'y', floor(RANDOM() * 150), 'width', 80 + floor(RANDOM() * 100), 'height', 60 + floor(RANDOM() * 80)),
    jsonb_build_object('temperature', 15 + (RANDOM() * 20), 'humidity', 40 + (RANDOM() * 40), 'conditions', 'clear'),
    jsonb_build_object('motion_level', floor(RANDOM() * 100), 'processing_time_ms', floor(500 + RANDOM() * 2000))
FROM cameras c
CROSS JOIN species s
WHERE RANDOM() < 0.3  -- Only create detections for ~30% of camera-species combinations
LIMIT 50;

-- Insert some sample alerts
INSERT INTO alerts (camera_id, alert_type, severity, title, message) VALUES
((SELECT id FROM cameras WHERE device_id = 'ESP32-CAM-003'), 'low_battery', 'warning', 'Low Battery - Meadow Observer', 'Battery level has dropped to 25%'),
((SELECT id FROM cameras WHERE device_id = 'ESP32-CAM-003'), 'offline', 'critical', 'Camera Offline - Meadow Observer', 'No updates received for over 4 hours'),
((SELECT id FROM cameras WHERE device_id = 'ESP32-CAM-001'), 'rare_species', 'info', 'Black Bear Detected', 'High confidence black bear detection on Forest Trail Camera');

-- Insert sample analytics data
INSERT INTO analytics_data (metric_type, time_period, timestamp, camera_id, species_id, value) 
SELECT 
    'species_count',
    'daily',
    DATE_TRUNC('day', CURRENT_TIMESTAMP) - (RANDOM() * INTERVAL '30 days'),
    c.id,
    s.id,
    floor(RANDOM() * 5) + 1
FROM cameras c
CROSS JOIN species s
WHERE RANDOM() < 0.2
LIMIT 100;

-- Add hourly activity analytics
INSERT INTO analytics_data (metric_type, time_period, timestamp, value) 
SELECT 
    'hourly_activity',
    'hourly',
    DATE_TRUNC('hour', CURRENT_TIMESTAMP) - (generate_series(0, 167) * INTERVAL '1 hour'), -- Last 7 days
    floor(RANDOM() * 10) + 1
FROM generate_series(0, 167);

COMMIT;