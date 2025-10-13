# Real-time Collaboration Features Documentation

## Overview

This document describes the foundational real-time collaboration features implemented for the WildCAM ESP32 wildlife monitoring system. These features enable multiple researchers, conservationists, and field workers to collaborate effectively on wildlife monitoring projects.

## Architecture

### Backend Components

1. **Collaboration Service** (`backend/collaboration.py`)
   - Manages user presence and session tracking
   - Handles real-time event broadcasting via WebSocket
   - Coordinates collaborative features

2. **Database Models** (`backend/models.py`)
   - `UserSession`: Tracks active user sessions and presence
   - `Annotation`: Collaborative annotations on detections
   - `ChatMessage`: Team chat messages with threading support
   - `SharedBookmark`: Shared bookmarks and favorites
   - `Task`: Project tasks for field work coordination
   - `FieldNote`: Shared field notes and observations

3. **API Endpoints** (`backend/app.py`)
   - `/api/collaboration/users/active` - Get active users
   - `/api/collaboration/annotations` - Manage annotations
   - `/api/collaboration/chat` - Team chat
   - `/api/collaboration/bookmarks` - Shared bookmarks
   - `/api/collaboration/tasks` - Task management
   - `/api/collaboration/field-notes` - Field notes

### Frontend Components

1. **Services**
   - `collaborationApi.js`: API client for collaboration features
   - `WebSocketService.js`: Enhanced with collaboration event handlers

2. **React Components**
   - `UserPresence`: Display active users with role indicators
   - `TeamChat`: Real-time team chat with channels
   - `TaskManager`: Task creation and tracking

## Features Implemented

### 1. User Presence Awareness

**Purpose**: Track which team members are currently active and what they're viewing.

**Usage**:
```javascript
import { UserPresence } from './components/collaboration';

function Dashboard() {
  return (
    <div>
      <UserPresence />
      {/* Other dashboard content */}
    </div>
  );
}
```

**Features**:
- Real-time active user list
- Role-based color coding (admin, researcher, observer, field worker)
- Current page/view tracking
- Automatic session cleanup for inactive users

### 2. Team Chat

**Purpose**: Enable team communication with channel-based organization.

**Usage**:
```javascript
import { TeamChat } from './components/collaboration';

function CollaborationPanel() {
  return <TeamChat />;
}
```

**Features**:
- Multiple channels (general, detections, field-work)
- Real-time message delivery
- User avatars with role indicators
- Unread message counts per channel
- @mention support (backend ready, UI to be enhanced)

### 3. Annotations

**Purpose**: Allow team members to annotate wildlife detections collaboratively.

**Backend API**:
```python
# Get annotations for a detection
GET /api/collaboration/annotations?detection_id=123

# Create annotation
POST /api/collaboration/annotations
{
  "detection_id": 123,
  "annotation_type": "marker",
  "content": "Notice the unique markings",
  "position": {"x": 150, "y": 200},
  "metadata": {"color": "#ff0000"}
}
```

**Annotation Types**:
- `comment`: Text comments
- `marker`: Point markers on images
- `region`: Area highlighting
- `correction`: Species identification corrections

### 4. Shared Bookmarks

**Purpose**: Create and share important detections or camera locations with the team.

**API Usage**:
```python
# Get all bookmarks
GET /api/collaboration/bookmarks

# Create bookmark
POST /api/collaboration/bookmarks
{
  "title": "Rare Species Sighting",
  "description": "Tiger spotted near waterhole",
  "detection_id": 456,
  "tags": ["tiger", "rare", "priority"],
  "is_shared": true,
  "shared_with": ["all"]
}
```

### 5. Task Management

**Purpose**: Coordinate field work and data analysis tasks among team members.

**Usage**:
```javascript
import { TaskManager } from './components/collaboration';

function FieldWorkPanel() {
  return <TaskManager />;
}
```

**Features**:
- Task creation with priority levels (low, medium, high, urgent)
- Task types: maintenance, data_collection, verification, analysis
- Assignment to team members
- Status tracking: pending, in_progress, completed, cancelled
- Real-time notifications for assigned tasks

### 6. Field Notes

**Purpose**: Share observations and notes about specific detections or cameras.

**API Usage**:
```python
# Get field notes for a camera
GET /api/collaboration/field-notes?camera_id=789

# Create field note
POST /api/collaboration/field-notes
{
  "title": "Unusual Behavior Pattern",
  "content": "Observed nocturnal activity during daylight hours...",
  "note_type": "observation",
  "camera_id": 789,
  "tags": ["behavior", "unusual"]
}
```

## WebSocket Events

### Client → Server

```javascript
// Connect user presence
socket.emit('user_presence', {
  action: 'connect',
  session_id: 'uuid',
  user_id: 123
});

// Update activity
socket.emit('user_presence', {
  action: 'update',
  session_id: 'uuid',
  current_page: '/detections/456',
  cursor_position: {x: 100, y: 200}
});

// Join chat channel
socket.emit('join_channel', {
  channel: 'detections'
});
```

### Server → Client

```javascript
// Presence updates
socket.on('presence_update', (data) => {
  console.log('Active users:', data.active_users);
});

// Cursor tracking
socket.on('cursor_update', (data) => {
  console.log('User cursor:', data.username, data.position);
});

// New chat message
socket.on('chat_message', (data) => {
  console.log('New message:', data.message);
});

// Annotation created
socket.on('annotation_created', (data) => {
  console.log('New annotation:', data.annotation);
});

// Task updates
socket.on('task_created', (data) => {
  console.log('New task:', data.task);
});

// Mentions
socket.on('mention_notification', (data) => {
  console.log('You were mentioned:', data.message);
});
```

## Role-Based Access Control

The system supports different user roles:

- **admin**: Full access to all features
- **researcher**: Data analysis and annotation capabilities
- **observer**: View and basic collaboration features
- **field_worker**: Task execution and field notes

Roles are managed in the User model and enforced at the API level.

## Database Migrations

After cloning the repository, run migrations to create collaboration tables:

```bash
cd backend
flask db upgrade
```

Or manually create tables in development:

```python
from app import app, db
with app.app_context():
    db.create_all()
```

## Integration Example

Here's a complete example of adding collaboration features to an existing page:

```javascript
import React, { useEffect } from 'react';
import { Grid } from '@mui/material';
import { UserPresence, TeamChat, TaskManager } from './components/collaboration';
import WebSocketService from './services/WebSocketService';

function Dashboard() {
  useEffect(() => {
    // Connect to WebSocket
    WebSocketService.connect();
    
    // Register user presence
    const sessionId = generateSessionId();
    const userId = getCurrentUserId();
    
    WebSocketService.sendPresence('connect', sessionId, userId);
    
    // Join general channel
    WebSocketService.joinChannel('general');
    
    // Update activity on page changes
    WebSocketService.sendPresence(
      'update',
      sessionId,
      userId,
      '/dashboard',
      null
    );
    
    return () => {
      WebSocketService.sendPresence('disconnect', sessionId, userId);
      WebSocketService.leaveChannel('general');
    };
  }, []);

  return (
    <Grid container spacing={3}>
      <Grid item xs={12}>
        <UserPresence />
      </Grid>
      
      <Grid item xs={12} md={6}>
        <TeamChat />
      </Grid>
      
      <Grid item xs={12} md={6}>
        <TaskManager />
      </Grid>
    </Grid>
  );
}
```

## Future Enhancements

While this implementation provides foundational collaboration features, the following enhancements could be added:

### Near-term
- Annotation overlay UI for image viewing
- Rich text support in chat and notes
- File attachments in chat
- Email/push notifications for mentions
- Export chat/notes as reports

### Medium-term
- Video/audio calling integration (WebRTC)
- Screen sharing for remote assistance
- Collaborative drawing tools
- Advanced task dependencies and workflows
- Integration with external tools (Slack, Teams, etc.)

### Long-term
- Operational Transformation (OT) for real-time co-editing
- AI-powered species identification workflow
- Automated report generation
- Mobile app support
- Offline-first architecture with sync

## Security Considerations

1. **Authentication**: All API endpoints require JWT authentication
2. **Authorization**: Role-based access control enforced at API level
3. **WebSocket Security**: Session validation and user verification
4. **Data Privacy**: Audit logging for all collaborative actions
5. **Input Validation**: All user inputs sanitized and validated

## Performance

- **WebSocket**: Scales to hundreds of concurrent users per server
- **Database**: Indexed queries for fast presence and message retrieval
- **Caching**: Redis can be integrated for session management
- **Pagination**: All list endpoints support pagination

## Testing

Run backend tests:
```bash
cd backend
pytest tests/test_collaboration.py
```

Run frontend tests:
```bash
cd frontend/dashboard
npm test
```

## Support

For questions or issues with collaboration features:
1. Check this documentation
2. Review the API endpoint implementations
3. Test with the included example components
4. Open an issue on GitHub with details

## License

Same as the main WildCAM ESP32 project.
