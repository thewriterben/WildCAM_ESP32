-- Collaboration Features Migration
-- Adds tables for comments, bookmarks, and chat messages

-- Detection Comments Table
CREATE TABLE IF NOT EXISTS detection_comments (
    id SERIAL PRIMARY KEY,
    detection_id INTEGER NOT NULL REFERENCES wildlife_detections(id) ON DELETE CASCADE,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    comment TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_detection_comments_detection ON detection_comments(detection_id);
CREATE INDEX idx_detection_comments_user ON detection_comments(user_id);

-- Bookmarks Table
CREATE TABLE IF NOT EXISTS bookmarks (
    id SERIAL PRIMARY KEY,
    detection_id INTEGER NOT NULL REFERENCES wildlife_detections(id) ON DELETE CASCADE,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    title VARCHAR(200) NOT NULL,
    description TEXT,
    shared BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_bookmarks_detection ON bookmarks(detection_id);
CREATE INDEX idx_bookmarks_user ON bookmarks(user_id);
CREATE INDEX idx_bookmarks_shared ON bookmarks(shared);

-- Chat Messages Table
CREATE TABLE IF NOT EXISTS chat_messages (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    message TEXT NOT NULL,
    detection_id INTEGER REFERENCES wildlife_detections(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_chat_messages_user ON chat_messages(user_id);
CREATE INDEX idx_chat_messages_detection ON chat_messages(detection_id);
CREATE INDEX idx_chat_messages_created ON chat_messages(created_at DESC);

-- Update trigger for detection_comments updated_at
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

CREATE TRIGGER update_detection_comments_updated_at 
    BEFORE UPDATE ON detection_comments
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();
