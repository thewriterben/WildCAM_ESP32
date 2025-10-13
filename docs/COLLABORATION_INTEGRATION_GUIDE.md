# Collaboration Features Integration Guide

This guide provides step-by-step instructions for developers to integrate and extend the collaboration features in the WildCAM ESP32 system.

## Quick Start

### 1. Setup Database

The collaboration features require new database tables. Run the migration:

```bash
cd backend
flask db upgrade
```

Or in development, create all tables:

```python
from app import app, db
with app.app_context():
    db.create_all()
```

### 2. Start Backend Server

```bash
cd backend
# Install dependencies if needed
pip install -r requirements.txt

# Run the server
python app.py
```

The server will start on `http://localhost:5000` with WebSocket support.

### 3. Start Frontend Dashboard

```bash
cd frontend/dashboard
# Install dependencies if needed
npm install

# Start development server
npm start
```

The dashboard will open at `http://localhost:3000`.

## Integration Steps

### Adding Collaboration to an Existing Page

1. **Import the components:**

```javascript
import { UserPresence, TeamChat, TaskManager } from './components/collaboration';
import WebSocketService from './services/WebSocketService';
```

2. **Initialize WebSocket connection:**

```javascript
useEffect(() => {
  // Connect if not already connected
  if (!WebSocketService.getConnectionStatus().connected) {
    WebSocketService.connect();
  }

  // Get user info from localStorage (set during login)
  const user = JSON.parse(localStorage.getItem('user') || '{}');
  const sessionId = `session_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;

  // Register presence
  if (user.id) {
    WebSocketService.sendPresence('connect', sessionId, user.id);
  }

  return () => {
    if (user.id) {
      WebSocketService.sendPresence('disconnect', sessionId, user.id);
    }
  };
}, []);
```

3. **Add components to your layout:**

```javascript
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
```

### Complete Example: Enhanced Detection View

Here's how to add collaboration features to a detection details page:

```javascript
import React, { useEffect, useState } from 'react';
import { useParams } from 'react-router-dom';
import { Grid, Paper, Box, Typography, Divider } from '@mui/material';
import { UserPresence } from './components/collaboration';
import WebSocketService from './services/WebSocketService';
import collaborationApi from './services/collaborationApi';

function DetectionDetails() {
  const { detectionId } = useParams();
  const [annotations, setAnnotations] = useState([]);
  const [fieldNotes, setFieldNotes] = useState([]);

  useEffect(() => {
    // Setup presence
    const user = JSON.parse(localStorage.getItem('user') || '{}');
    const sessionId = `session_${Date.now()}`;
    
    if (user.id) {
      WebSocketService.sendPresence(
        'connect',
        sessionId,
        user.id,
        `/detections/${detectionId}`
      );
    }

    // Load annotations and notes
    loadCollaborationData();

    // Subscribe to real-time updates
    WebSocketService.subscribeToAnnotations(handleNewAnnotation);
    WebSocketService.subscribeToFieldNotes(handleNewNote);

    return () => {
      if (user.id) {
        WebSocketService.sendPresence('disconnect', sessionId, user.id);
      }
      WebSocketService.off('annotation_created', handleNewAnnotation);
      WebSocketService.off('field_note_created', handleNewNote);
    };
  }, [detectionId]);

  const loadCollaborationData = async () => {
    try {
      const [annots, notes] = await Promise.all([
        collaborationApi.getAnnotations(detectionId),
        collaborationApi.getFieldNotes(null, detectionId)
      ]);
      setAnnotations(annots);
      setFieldNotes(notes);
    } catch (error) {
      console.error('Error loading collaboration data:', error);
    }
  };

  const handleNewAnnotation = (data) => {
    if (data.detection_id === parseInt(detectionId)) {
      setAnnotations(prev => [...prev, data.annotation]);
    }
  };

  const handleNewNote = (data) => {
    if (data.note.detection_id === parseInt(detectionId)) {
      setFieldNotes(prev => [...prev, data.note]);
    }
  };

  const handleAddAnnotation = async (type, content, position) => {
    try {
      await collaborationApi.createAnnotation(
        detectionId,
        type,
        content,
        position
      );
    } catch (error) {
      console.error('Error creating annotation:', error);
    }
  };

  return (
    <Box sx={{ p: 3 }}>
      <Grid container spacing={3}>
        {/* Active users watching this detection */}
        <Grid item xs={12}>
          <UserPresence />
        </Grid>

        {/* Detection image and details */}
        <Grid item xs={12} md={8}>
          <Paper sx={{ p: 2 }}>
            {/* Detection image with annotation overlay */}
            <Typography variant="h6">Detection #{detectionId}</Typography>
            {/* ... detection content ... */}
          </Paper>
        </Grid>

        {/* Collaboration sidebar */}
        <Grid item xs={12} md={4}>
          {/* Annotations */}
          <Paper sx={{ p: 2, mb: 2 }}>
            <Typography variant="h6" gutterBottom>
              Annotations ({annotations.length})
            </Typography>
            <Divider sx={{ mb: 2 }} />
            {annotations.map(ann => (
              <Box key={ann.id} sx={{ mb: 2 }}>
                <Typography variant="caption" color="text.secondary">
                  {ann.annotation_type}
                </Typography>
                <Typography variant="body2">{ann.content}</Typography>
              </Box>
            ))}
          </Paper>

          {/* Field Notes */}
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6" gutterBottom>
              Field Notes ({fieldNotes.length})
            </Typography>
            <Divider sx={{ mb: 2 }} />
            {fieldNotes.map(note => (
              <Box key={note.id} sx={{ mb: 2 }}>
                <Typography variant="subtitle2">{note.title}</Typography>
                <Typography variant="body2">{note.content}</Typography>
              </Box>
            ))}
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
}

export default DetectionDetails;
```

## API Usage Examples

### Creating a Task

```javascript
import collaborationApi from './services/collaborationApi';

async function createMaintenanceTask() {
  try {
    const task = await collaborationApi.createTask(
      'Replace Camera Battery',           // title
      'Camera 5 battery at 15%',         // description
      'maintenance',                      // task_type
      'high',                            // priority
      42,                                // assigned_to (user_id)
      5,                                 // camera_id
      null,                              // detection_id
      '2024-01-15T10:00:00'             // due_date
    );
    console.log('Task created:', task);
  } catch (error) {
    console.error('Error:', error);
  }
}
```

### Sending a Chat Message with @Mentions

```javascript
async function sendMessageWithMention() {
  try {
    const message = await collaborationApi.sendChatMessage(
      'Hey @john, can you verify this detection?',
      'detections',        // channel
      null,               // parent_id (for threading)
      [42]                // mentions (array of user IDs)
    );
    console.log('Message sent:', message);
  } catch (error) {
    console.error('Error:', error);
  }
}
```

### Creating a Shared Bookmark

```javascript
async function bookmarkImportantDetection() {
  try {
    const bookmark = await collaborationApi.createBookmark(
      'Rare Tiger Sighting',              // title
      'First tiger sighting in 5 years', // description
      456,                                // detection_id
      null,                               // camera_id
      ['tiger', 'rare', 'important'],    // tags
      true,                               // is_shared
      ['all']                             // shared_with
    );
    console.log('Bookmark created:', bookmark);
  } catch (error) {
    console.error('Error:', error);
  }
}
```

### Adding Field Notes

```javascript
async function addFieldObservation() {
  try {
    const note = await collaborationApi.createFieldNote(
      'Unusual Behavior Observed',                 // title
      'Animal showed signs of distress...',        // content
      'observation',                               // note_type
      7,                                           // camera_id
      null,                                        // detection_id
      ['behavior', 'urgent'],                      // tags
      []                                           // attachments
    );
    console.log('Field note created:', note);
  } catch (error) {
    console.error('Error:', error);
  }
}
```

## Extending the System

### Adding a New Collaboration Feature

1. **Backend - Add Model** (models.py):

```python
class ReviewRequest(db.Model):
    """Peer review requests for detections"""
    __tablename__ = 'review_requests'
    
    id = db.Column(db.Integer, primary_key=True)
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    requested_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    assigned_to = db.Column(db.Integer, db.ForeignKey('users.id'))
    status = db.Column(db.String(20), default='pending')
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
```

2. **Backend - Add Service Method** (collaboration.py):

```python
def request_review(self, detection_id, requested_by, assigned_to):
    """Request peer review for a detection"""
    try:
        review = ReviewRequest(
            detection_id=detection_id,
            requested_by=requested_by,
            assigned_to=assigned_to
        )
        db.session.add(review)
        db.session.commit()
        
        # Notify assigned reviewer
        self.socketio.emit('review_requested', {
            'review': review.to_dict()
        }, room=f'user_{assigned_to}')
        
        return review.to_dict()
    except Exception as e:
        logger.error(f"Error requesting review: {e}")
        db.session.rollback()
        return None
```

3. **Backend - Add API Endpoint** (app.py):

```python
@app.route('/api/collaboration/review-requests', methods=['POST'])
@jwt_required()
def create_review_request():
    user_id = get_jwt_identity()
    data = request.get_json()
    
    review = collab_service.request_review(
        detection_id=data['detection_id'],
        requested_by=user_id,
        assigned_to=data['assigned_to']
    )
    
    if review:
        return jsonify({'review': review}), 201
    return jsonify({'error': 'Failed to create review request'}), 500
```

4. **Frontend - Add API Method** (collaborationApi.js):

```javascript
requestReview: async (detectionId, assignedTo) => {
  try {
    const response = await axios.post(
      `${API_BASE}/collaboration/review-requests`,
      { detection_id: detectionId, assigned_to: assignedTo },
      { headers: getAuthHeaders() }
    );
    return response.data.review;
  } catch (error) {
    console.error('Request review error:', error);
    throw error;
  }
}
```

5. **Frontend - Create Component** (ReviewRequests.js):

```javascript
import React, { useState, useEffect } from 'react';
import collaborationApi from '../services/collaborationApi';

function ReviewRequests() {
  const [reviews, setReviews] = useState([]);
  
  useEffect(() => {
    loadReviews();
  }, []);
  
  // ... component implementation
}
```

## Testing

### Backend Tests

Create `backend/tests/test_collaboration.py`:

```python
import pytest
from app import create_app, db
from models import User, UserSession

@pytest.fixture
def client():
    app, socketio = create_app('testing')
    with app.test_client() as client:
        with app.app_context():
            db.create_all()
            yield client
            db.drop_all()

def test_active_users(client):
    # Create test user
    # Login and get token
    # Call active users endpoint
    # Assert response
    pass
```

### Frontend Tests

Create `frontend/dashboard/src/components/collaboration/__tests__/UserPresence.test.js`:

```javascript
import { render, screen } from '@testing-library/react';
import UserPresence from '../UserPresence';

jest.mock('../../services/WebSocketService');
jest.mock('../../services/collaborationApi');

test('renders active users count', () => {
  render(<UserPresence />);
  expect(screen.getByText(/Active Users/i)).toBeInTheDocument();
});
```

## Performance Considerations

### Backend
- Use Redis for session caching: `pip install redis`
- Implement rate limiting for API endpoints
- Use database connection pooling
- Add indexes on foreign keys

### Frontend
- Implement virtual scrolling for large chat histories
- Debounce cursor position updates
- Use React.memo for expensive components
- Implement pagination for task lists

## Security Best Practices

1. **Always validate user identity in WebSocket handlers**
2. **Sanitize user inputs** to prevent XSS attacks
3. **Implement rate limiting** on collaboration endpoints
4. **Use HTTPS/WSS** in production
5. **Audit log all collaborative actions**
6. **Implement role-based permissions** for sensitive operations

## Troubleshooting

### WebSocket Connection Issues

If WebSocket won't connect:

1. Check CORS settings in `app.py`
2. Verify SocketIO version compatibility
3. Check firewall/proxy settings
4. Use browser dev tools to inspect WebSocket frames

### Database Migration Errors

If migration fails:

```bash
# Reset migrations (development only!)
rm -rf migrations/
flask db init
flask db migrate -m "Initial migration"
flask db upgrade
```

### Real-time Updates Not Working

1. Verify WebSocket connection status
2. Check that event handlers are registered
3. Confirm room/channel joins
4. Check browser console for errors

## Production Deployment

### Environment Variables

```bash
export DATABASE_URL=postgresql://user:pass@localhost/wildlife_db
export REDIS_URL=redis://localhost:6379
export SECRET_KEY=your-secret-key
export JWT_SECRET_KEY=your-jwt-secret
```

### Nginx Configuration for WebSocket

```nginx
location /socket.io {
    proxy_pass http://localhost:5000;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
    proxy_set_header Host $host;
    proxy_cache_bypass $http_upgrade;
}
```

### Scaling Considerations

For production with multiple servers:

1. **Use Redis for Socket.IO adapter**:
```python
from flask_socketio import SocketIO
import redis

redis_client = redis.Redis.from_url(app.config['REDIS_URL'])
socketio = SocketIO(app, message_queue=app.config['REDIS_URL'])
```

2. **Implement sticky sessions** in load balancer
3. **Use separate WebSocket servers** if needed
4. **Monitor connection counts** and memory usage

## Support and Resources

- Main Documentation: `/docs/COLLABORATION_FEATURES.md`
- API Reference: Check endpoint docstrings in `app.py`
- Component Examples: `/frontend/dashboard/src/components/CollaborationPanel.js`
- GitHub Issues: For bugs and feature requests

## Future Roadmap

Planned enhancements:
- [ ] Annotation overlay UI for images
- [ ] WebRTC video/audio calls
- [ ] File attachment support
- [ ] Rich text editor for notes
- [ ] Mobile app integration
- [ ] Offline support with sync
- [ ] Advanced analytics dashboard
