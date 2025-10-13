# Collaboration Features Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         Frontend (React)                         │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │  TeamChat    │  │UserPresence  │  │ Detection    │          │
│  │  Component   │  │  Component   │  │ Annotations  │          │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
│         │                  │                  │                  │
│         └──────────────────┴──────────────────┘                  │
│                           │                                      │
│                    ┌──────▼──────┐                              │
│                    │  WebSocket  │                              │
│                    │   Service   │                              │
│                    └──────┬──────┘                              │
└───────────────────────────┼──────────────────────────────────────┘
                            │
                    ┌───────▼────────┐
                    │  Socket.IO     │
                    │  Connection    │
                    └───────┬────────┘
                            │
┌───────────────────────────┼──────────────────────────────────────┐
│                  Backend (Flask + Socket.IO)                     │
├───────────────────────────┴──────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              WebSocket Event Handlers                   │   │
│  │  • user_connected    • cursor_move                      │   │
│  │  • chat_message      • new_comment                      │   │
│  │  • new_bookmark      • disconnect                       │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────▼────────────────────────────────────┐   │
│  │            REST API Endpoints (JWT Auth)                │   │
│  │  • GET/POST /api/collaboration/comments                 │   │
│  │  • GET/POST /api/collaboration/chat                     │   │
│  │  • GET/POST /api/collaboration/bookmarks                │   │
│  │  • GET      /api/collaboration/presence                 │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────▼────────────────────────────────────┐   │
│  │              SQLAlchemy Models                          │   │
│  │  • DetectionComment  • Bookmark  • ChatMessage          │   │
│  └────────────────────┬────────────────────────────────────┘   │
└────────────────────────┼─────────────────────────────────────────┘
                         │
                    ┌────▼─────┐
                    │PostgreSQL│
                    │ Database │
                    └──────────┘
```

## Data Flow

### Sending a Chat Message

```
User Types Message
       │
       ▼
TeamChat Component
       │
       ▼
POST /api/collaboration/chat (JWT Token)
       │
       ▼
Backend validates JWT
       │
       ▼
Create ChatMessage in DB
       │
       ▼
Emit 'chat_message' via Socket.IO
       │
       ▼
All connected clients receive message
       │
       ▼
Update UI in real-time
```

### Adding a Comment

```
User Comments on Detection
       │
       ▼
DetectionAnnotations Component
       │
       ▼
POST /api/collaboration/comments (JWT Token)
       │
       ▼
Backend validates user + detection exists
       │
       ▼
Create DetectionComment in DB
       │
       ▼
Emit 'new_comment' via Socket.IO
       │
       ▼
All users viewing that detection see comment
```

### User Presence

```
User Logs In
       │
       ▼
WebSocket connects
       │
       ▼
Backend updates User.last_login
       │
       ▼
Emit 'user_connected' event
       │
       ▼
All clients refresh active user list
       │
       ▼
UserPresence component polls every 30s
       │
       ▼
GET /api/collaboration/presence
       │
       ▼
Returns users active in last 5 minutes
```

## Database Schema

### detection_comments
```sql
id              SERIAL PRIMARY KEY
detection_id    INTEGER → wildlife_detections(id)
user_id         INTEGER → users(id)
comment         TEXT
created_at      TIMESTAMP
updated_at      TIMESTAMP
```

### bookmarks
```sql
id              SERIAL PRIMARY KEY
detection_id    INTEGER → wildlife_detections(id)
user_id         INTEGER → users(id)
title           VARCHAR(200)
description     TEXT
shared          BOOLEAN (shared with team)
created_at      TIMESTAMP
```

### chat_messages
```sql
id              SERIAL PRIMARY KEY
user_id         INTEGER → users(id)
message         TEXT
detection_id    INTEGER → wildlife_detections(id) [optional]
created_at      TIMESTAMP
```

## Authentication Flow

All collaboration endpoints require JWT authentication:

```
1. User logs in via /api/auth/login
2. Receives access_token
3. Stores token in localStorage
4. Includes in all API requests:
   Authorization: Bearer <token>
5. Backend validates token and extracts user_id
6. Associates actions with authenticated user
```

## Role-Based Access

Extended existing user roles:
- **Admin**: Full access to all features
- **Researcher**: Can comment, bookmark, chat
- **Viewer**: Read-only access

Roles enforced via JWT claims and decorator functions:
```python
@jwt_required()
@researcher_required
def endpoint():
    # Only researchers and admins can access
```

## Real-time Events

### Socket.IO Events

**Client → Server:**
- `cursor_move` - Share cursor position (optional feature)

**Server → Client:**
- `connect` - Client connected successfully
- `disconnect` - Client disconnected
- `chat_message` - New team chat message
- `new_comment` - Comment added to detection
- `new_bookmark` - Shared bookmark created
- `user_connected` - User joined dashboard
- `cursor_update` - Another user's cursor moved

## Security Measures

1. **JWT Authentication**: All endpoints protected
2. **User Identity**: Actions tied to authenticated user
3. **Input Validation**: Data validated before DB insertion
4. **SQL Injection**: SQLAlchemy ORM prevents SQL injection
5. **CORS**: Configured for specific origins
6. **Rate Limiting**: Can be added via Flask-Limiter

## Performance Optimizations

1. **Database Indexes**: Added on foreign keys and query fields
2. **Pagination**: Chat messages limited to last 50 by default
3. **Lazy Loading**: Messages/comments loaded on demand
4. **WebSocket**: Efficient bidirectional communication
5. **Caching**: Active users cached with 30s refresh

## Scalability Considerations

Current implementation suitable for:
- **Users**: 10-50 concurrent users
- **Messages**: Thousands per day
- **Comments**: Hundreds per detection
- **Bookmarks**: Thousands total

For larger scale:
- Add Redis for WebSocket pub/sub
- Implement message queue (RabbitMQ/Celery)
- Database connection pooling
- CDN for static assets
- Load balancing across servers

## Testing Strategy

Automated tests cover:
- ✅ Model structure validation
- ✅ API endpoint existence
- ✅ Frontend component creation
- ✅ WebSocket handler registration

Manual testing required:
- [ ] Real-time message delivery
- [ ] Multi-user scenarios
- [ ] Network failure recovery
- [ ] JWT token expiration
- [ ] Database constraint validation

## Monitoring

Recommended monitoring points:
1. WebSocket connection count
2. Database query performance
3. API endpoint response times
4. Error rates and exceptions
5. User activity metrics

## Deployment

1. **Database Migration**:
   ```bash
   psql -d wildlife_db -f backend/migrations/add_collaboration_tables.sql
   ```

2. **Environment Variables**:
   ```
   DATABASE_URL=postgresql://...
   JWT_SECRET_KEY=your-secret-key
   REDIS_URL=redis://localhost:6379  # For future scaling
   ```

3. **Start Backend**:
   ```bash
   cd backend
   python app.py
   ```

4. **Start Frontend**:
   ```bash
   cd frontend/dashboard
   npm install
   npm start
   ```

## Future Enhancements

Not implemented (beyond minimal scope):
- Video/audio calling (WebRTC)
- File sharing and uploads
- Advanced cursor tracking with visual avatars
- Operational Transformation for conflict-free editing
- Mobile app integration
- Email notifications
- Advanced analytics on collaboration patterns
- Integration with external tools (Slack, Teams)
