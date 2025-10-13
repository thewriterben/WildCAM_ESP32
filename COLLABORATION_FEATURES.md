# Real-time Collaboration Features

This document describes the minimal collaboration features implemented in the WildCAM ESP32 system to enable team-based wildlife monitoring and research.

## Features Implemented

### 1. User Presence System
- Real-time tracking of active users on the dashboard
- Visual indicators showing who's currently online
- Updates every 30 seconds and on user connect/disconnect events
- Located in the top navigation bar

**Component**: `frontend/dashboard/src/components/UserPresence.js`

### 2. Team Chat
- Real-time text messaging between team members
- Global chat for general team communication
- Threaded discussions on specific wildlife detections
- Messages delivered instantly via WebSocket
- Message history preserved in database

**Component**: `frontend/dashboard/src/components/TeamChat.js`  
**API Endpoints**:
- `GET /api/collaboration/chat` - Retrieve messages
- `POST /api/collaboration/chat` - Send new message

### 3. Detection Comments
- Collaborative annotation of wildlife detections
- Add comments to discuss specific sightings
- View all comments on a detection with timestamps
- Real-time updates when team members add comments

**Component**: `frontend/dashboard/src/components/DetectionAnnotations.js`  
**API Endpoints**:
- `GET /api/collaboration/comments?detection_id={id}` - Get comments
- `POST /api/collaboration/comments` - Add comment

### 4. Shared Bookmarks
- Mark important wildlife detections for the team
- Add descriptive titles and notes
- Share bookmarks with entire team or keep private
- Quick access to key research moments

**API Endpoints**:
- `GET /api/collaboration/bookmarks` - Get bookmarks
- `POST /api/collaboration/bookmarks` - Create bookmark

### 5. Role-Based Access Control (RBAC)
Extended the existing user system with three roles:
- **Admin**: Full system access, user management
- **Researcher**: Can analyze data, add comments, create bookmarks
- **Viewer**: Read-only access to dashboard and data

Roles are assigned during user registration and enforced via JWT tokens.

## Database Schema

### New Tables

**detection_comments**
- Stores collaborative comments on wildlife detections
- Links to users and detections with foreign keys
- Includes timestamps for audit trail

**bookmarks**
- Stores important detection bookmarks
- Supports shared (team) and private bookmarks
- Links to users and detections

**chat_messages**
- Stores team chat messages
- Optional link to specific detections for threaded discussions
- Indexed for fast retrieval

### Migration
Run the SQL migration script to add these tables:
```bash
psql -d wildlife_db -f backend/migrations/add_collaboration_tables.sql
```

## WebSocket Events

### Client to Server
- `cursor_move` - Share cursor position for collaborative viewing (basic implementation)

### Server to Client
- `chat_message` - New chat message from any user
- `new_comment` - New comment on a detection
- `new_bookmark` - New shared bookmark created
- `user_connected` - User joined the dashboard
- `cursor_update` - Another user's cursor position

## API Authentication

All collaboration endpoints require JWT authentication:
```javascript
headers: {
  'Authorization': `Bearer ${access_token}`
}
```

## Usage Examples

### Adding a Comment
```javascript
POST /api/collaboration/comments
{
  "detection_id": 123,
  "comment": "This looks like a juvenile deer"
}
```

### Sending a Chat Message
```javascript
POST /api/collaboration/chat
{
  "message": "Great catch on camera 5!",
  "detection_id": 123  // Optional: thread to specific detection
}
```

### Creating a Shared Bookmark
```javascript
POST /api/collaboration/bookmarks
{
  "detection_id": 123,
  "title": "Rare Species - Gray Wolf",
  "description": "First sighting in this region",
  "shared": true
}
```

## Frontend Integration

### Access Team Chat
Navigate to "Team Chat" from the sidebar menu. The chat interface shows:
- All team messages in chronological order
- User avatars and names
- Timestamps
- Real-time updates

### View Active Users
User presence indicators appear in the top-right of the navigation bar showing:
- Number of online users
- User avatars (up to 5)
- Badge indicator for online status

### Add Comments to Detections
When viewing a wildlife detection:
1. Open the DetectionAnnotations component
2. Switch to "Comments" tab
3. Type your comment and click "Add Comment"
4. Comments appear instantly for all team members

## Performance Considerations

- WebSocket connections maintained for real-time updates
- Comments and bookmarks cached locally with server sync
- Active user list refreshed every 30 seconds
- Chat history limited to last 50 messages by default

## Security

- All endpoints protected with JWT authentication
- User identity verified for all actions
- Comments and bookmarks linked to user accounts
- Audit trail maintained with timestamps

## Future Enhancements (Not Implemented)

The following features were considered but not implemented to maintain minimal changes:
- Real-time cursor tracking with visual indicators
- Canvas-based annotation tools for drawing on images
- Voice/Video calling (WebRTC)
- Screen sharing
- Advanced project management
- Operational Transformation for conflict resolution
- Advanced presence indicators (page location, idle status)

## Testing

Test the collaboration features:

1. Start the backend server:
```bash
cd backend
python app.py
```

2. Start the frontend:
```bash
cd frontend/dashboard
npm start
```

3. Open multiple browser windows
4. Login with different users
5. Send chat messages and verify real-time delivery
6. Add comments to detections
7. Create shared bookmarks

## Troubleshooting

**WebSocket not connecting:**
- Verify backend is running on port 5000
- Check CORS configuration
- Ensure JWT token is valid

**Messages not appearing:**
- Check browser console for errors
- Verify WebSocket connection status
- Reload the page to reconnect

**Permission errors:**
- Ensure user is logged in
- Check JWT token is included in requests
- Verify user role has necessary permissions
