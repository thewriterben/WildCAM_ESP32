# Phase 2 Implementation Guide: Enhanced Real-time Features

## Overview

This guide provides detailed implementation instructions for Phase 2 enhancements to the WildCAM_ESP32 collaboration system. Phase 2 focuses on enhancing the existing real-time features with advanced synchronization, improved annotations, and richer communication.

## Prerequisites

- Phase 1 collaboration features must be fully deployed
- PostgreSQL 12+ with Phase 1 schema
- Redis 6+ for caching and pub/sub
- Node.js 16+ and Python 3.9+
- React 18+ frontend

## Feature 1: Advanced Dashboard Synchronization

### 1.1 Synchronized Viewport Navigation

**Backend Changes** (`backend/collaboration.py`):

```python
class ViewportState:
    """Manages synchronized viewport state"""
    
    def __init__(self):
        self.room_viewports = {}  # room_id -> viewport_state
    
    def update_viewport(self, room_id, user_id, viewport_data):
        """
        Update viewport for a room
        
        Args:
            room_id: Unique room identifier
            user_id: User updating viewport
            viewport_data: {
                'zoom': float,
                'center_x': float,
                'center_y': float,
                'rotation': float,
                'locked_by': user_id or None
            }
        """
        if room_id not in self.room_viewports:
            self.room_viewports[room_id] = {
                'state': viewport_data,
                'updated_by': user_id,
                'updated_at': datetime.utcnow()
            }
        else:
            # Check if viewport is locked by another user
            current = self.room_viewports[room_id]['state']
            if current.get('locked_by') and current['locked_by'] != user_id:
                raise ValueError(f"Viewport locked by user {current['locked_by']}")
            
            self.room_viewports[room_id] = {
                'state': viewport_data,
                'updated_by': user_id,
                'updated_at': datetime.utcnow()
            }
        
        return self.room_viewports[room_id]
    
    def get_viewport(self, room_id):
        """Get current viewport state for a room"""
        return self.room_viewports.get(room_id)
```

**API Endpoint** (add to `backend/app.py`):

```python
@app.route('/api/collaboration/viewport/<room_id>', methods=['GET', 'PUT'])
@jwt_required()
def handle_viewport(room_id):
    """Get or update viewport state"""
    user_id = get_jwt_identity()
    
    if request.method == 'GET':
        viewport = collab_service.viewport_state.get_viewport(room_id)
        return jsonify({'viewport': viewport}), 200
    
    elif request.method == 'PUT':
        data = request.get_json()
        try:
            viewport = collab_service.viewport_state.update_viewport(
                room_id, user_id, data
            )
            
            # Broadcast to room members
            socketio.emit('viewport_update', {
                'room_id': room_id,
                'viewport': viewport
            }, room=room_id)
            
            return jsonify({'viewport': viewport}), 200
        except ValueError as e:
            return jsonify({'error': str(e)}), 403
```

**Frontend Hook** (`frontend/dashboard/src/hooks/useSharedViewport.js`):

```javascript
import { useState, useEffect, useCallback } from 'react';
import WebSocketService from '../services/WebSocketService';
import collaborationApi from '../services/collaborationApi';

export const useSharedViewport = (roomId, isLeader = false) => {
  const [viewport, setViewport] = useState({
    zoom: 1.0,
    center_x: 0,
    center_y: 0,
    rotation: 0,
    locked_by: null
  });
  const [isLocked, setIsLocked] = useState(false);

  useEffect(() => {
    // Load initial viewport state
    loadViewport();

    // Subscribe to viewport updates
    WebSocketService.on('viewport_update', handleViewportUpdate);

    return () => {
      WebSocketService.off('viewport_update', handleViewportUpdate);
    };
  }, [roomId]);

  const loadViewport = async () => {
    try {
      const response = await collaborationApi.getViewport(roomId);
      if (response.viewport) {
        setViewport(response.viewport.state);
        setIsLocked(response.viewport.state.locked_by !== null);
      }
    } catch (error) {
      console.error('Error loading viewport:', error);
    }
  };

  const handleViewportUpdate = (data) => {
    if (data.room_id === roomId) {
      setViewport(data.viewport.state);
      setIsLocked(data.viewport.state.locked_by !== null);
    }
  };

  const updateViewport = useCallback(async (newViewport) => {
    try {
      await collaborationApi.updateViewport(roomId, newViewport);
    } catch (error) {
      console.error('Error updating viewport:', error);
    }
  }, [roomId]);

  const lockViewport = useCallback(async (lock = true) => {
    if (!isLeader) {
      throw new Error('Only leaders can lock viewport');
    }
    
    const newViewport = {
      ...viewport,
      locked_by: lock ? 'current_user' : null
    };
    
    await updateViewport(newViewport);
  }, [viewport, isLeader, updateViewport]);

  return {
    viewport,
    isLocked,
    updateViewport,
    lockViewport,
    canControl: isLeader || !isLocked
  };
};
```

### 1.2 Multi-User Cursor Tracking

**Database Schema** (add to migrations):

```sql
CREATE TABLE IF NOT EXISTS cursor_positions (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id),
    room_id VARCHAR(100) NOT NULL,
    position_x FLOAT NOT NULL,
    position_y FLOAT NOT NULL,
    viewport_x FLOAT,
    viewport_y FLOAT,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_room_user (room_id, user_id),
    INDEX idx_updated (updated_at)
);
```

**WebSocket Event Handler** (add to `backend/app.py`):

```python
@socketio.on('cursor_move')
def handle_cursor_move(data):
    """Handle cursor position updates (throttled on client)"""
    user_id = session.get('user_id')
    room_id = data.get('room_id')
    position = data.get('position')
    
    if not all([user_id, room_id, position]):
        return
    
    # Store in Redis with TTL (more efficient than DB for ephemeral data)
    cursor_key = f"cursor:{room_id}:{user_id}"
    redis_client.setex(
        cursor_key, 
        10,  # 10 second TTL
        json.dumps({
            'user_id': user_id,
            'username': session.get('username'),
            'position': position,
            'timestamp': datetime.utcnow().isoformat()
        })
    )
    
    # Broadcast to room (excluding sender)
    emit('cursor_update', {
        'user_id': user_id,
        'username': session.get('username'),
        'position': position
    }, room=room_id, skip_sid=request.sid)
```

**Frontend Component** (`frontend/dashboard/src/components/collaboration/CursorOverlay.js`):

```javascript
import React, { useState, useEffect, useCallback } from 'react';
import { Box } from '@mui/material';
import WebSocketService from '../../services/WebSocketService';
import { throttle } from 'lodash';

const CursorOverlay = ({ roomId, containerRef }) => {
  const [cursors, setCursors] = useState(new Map());

  useEffect(() => {
    WebSocketService.on('cursor_update', handleCursorUpdate);

    // Clean up stale cursors
    const cleanupInterval = setInterval(() => {
      const now = Date.now();
      setCursors(prev => {
        const updated = new Map(prev);
        for (let [userId, data] of updated) {
          if (now - data.timestamp > 5000) {
            updated.delete(userId);
          }
        }
        return updated;
      });
    }, 1000);

    return () => {
      WebSocketService.off('cursor_update', handleCursorUpdate);
      clearInterval(cleanupInterval);
    };
  }, []);

  const handleCursorUpdate = useCallback((data) => {
    setCursors(prev => {
      const updated = new Map(prev);
      updated.set(data.user_id, {
        username: data.username,
        position: data.position,
        timestamp: Date.now()
      });
      return updated;
    });
  }, []);

  const sendCursorPosition = useCallback(
    throttle((e) => {
      if (!containerRef.current) return;
      
      const rect = containerRef.current.getBoundingClientRect();
      const position = {
        x: e.clientX - rect.left,
        y: e.clientY - rect.top
      };

      WebSocketService.emit('cursor_move', {
        room_id: roomId,
        position
      });
    }, 50), // Throttle to 20 updates per second
    [roomId, containerRef]
  );

  useEffect(() => {
    const container = containerRef.current;
    if (!container) return;

    container.addEventListener('mousemove', sendCursorPosition);
    return () => {
      container.removeEventListener('mousemove', sendCursorPosition);
    };
  }, [sendCursorPosition, containerRef]);

  return (
    <Box
      sx={{
        position: 'absolute',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        pointerEvents: 'none',
        zIndex: 1000
      }}
    >
      {Array.from(cursors.entries()).map(([userId, data]) => (
        <Cursor
          key={userId}
          username={data.username}
          position={data.position}
        />
      ))}
    </Box>
  );
};

const Cursor = ({ username, position }) => {
  const color = `hsl(${username.charCodeAt(0) * 137.5 % 360}, 70%, 60%)`;

  return (
    <Box
      sx={{
        position: 'absolute',
        left: position.x,
        top: position.y,
        transform: 'translate(-4px, -4px)',
        pointerEvents: 'none',
        transition: 'left 0.1s, top 0.1s'
      }}
    >
      {/* Cursor pointer */}
      <svg width="24" height="24" viewBox="0 0 24 24">
        <path
          d="M5,3 L19,12 L12,13 L9,21 Z"
          fill={color}
          stroke="white"
          strokeWidth="1"
        />
      </svg>
      
      {/* Username label */}
      <Box
        sx={{
          position: 'absolute',
          left: 20,
          top: 0,
          backgroundColor: color,
          color: 'white',
          padding: '2px 8px',
          borderRadius: '4px',
          fontSize: '12px',
          fontWeight: 'bold',
          whiteSpace: 'nowrap',
          boxShadow: '0 2px 4px rgba(0,0,0,0.2)'
        }}
      >
        {username}
      </Box>
    </Box>
  );
};

export default CursorOverlay;
```

### 1.3 Real-time Activity Feed

**Backend Model** (add to `backend/models.py`):

```python
class ActivityLog(db.Model):
    """Activity feed for collaboration events"""
    __tablename__ = 'activity_logs'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    activity_type = db.Column(db.String(50), nullable=False)
    entity_type = db.Column(db.String(50))  # detection, annotation, task, etc.
    entity_id = db.Column(db.Integer)
    description = db.Column(db.Text)
    metadata = db.Column(JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Relationships
    user = db.relationship('User', backref='activities')
    
    # Indexes
    __table_args__ = (
        db.Index('idx_activity_created', 'created_at'),
        db.Index('idx_activity_type', 'activity_type'),
        db.Index('idx_entity', 'entity_type', 'entity_id'),
    )
```

**API Endpoint**:

```python
@app.route('/api/collaboration/activity', methods=['GET'])
@jwt_required()
def get_activity_feed():
    """Get recent activity feed"""
    limit = request.args.get('limit', 50, type=int)
    offset = request.args.get('offset', 0, type=int)
    activity_type = request.args.get('type')
    
    query = ActivityLog.query
    
    if activity_type:
        query = query.filter_by(activity_type=activity_type)
    
    activities = query.order_by(
        ActivityLog.created_at.desc()
    ).limit(limit).offset(offset).all()
    
    return jsonify({
        'activities': [{
            'id': a.id,
            'user': {
                'id': a.user.id,
                'username': a.user.username
            },
            'type': a.activity_type,
            'entity_type': a.entity_type,
            'entity_id': a.entity_id,
            'description': a.description,
            'metadata': a.metadata,
            'created_at': a.created_at.isoformat()
        } for a in activities]
    }), 200
```

## Feature 2: Enhanced Annotation System

### 2.1 Canvas-based Drawing Tools

**Frontend Canvas Component** (`frontend/dashboard/src/components/collaboration/AnnotationCanvas.js`):

```javascript
import React, { useRef, useEffect, useState } from 'react';
import { Box, ToggleButtonGroup, ToggleButton } from '@mui/material';
import {
  Create,
  RadioButtonUnchecked,
  CropSquare,
  Timeline,
  TextFields
} from '@mui/icons-material';

const AnnotationCanvas = ({ imageUrl, existingAnnotations, onAnnotationCreate }) => {
  const canvasRef = useRef(null);
  const [tool, setTool] = useState('pointer');
  const [isDrawing, setIsDrawing] = useState(false);
  const [currentAnnotation, setCurrentAnnotation] = useState(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    
    // Load and draw image
    const img = new Image();
    img.onload = () => {
      canvas.width = img.width;
      canvas.height = img.height;
      ctx.drawImage(img, 0, 0);
      
      // Draw existing annotations
      drawAnnotations(ctx, existingAnnotations);
    };
    img.src = imageUrl;
  }, [imageUrl, existingAnnotations]);

  const drawAnnotations = (ctx, annotations) => {
    annotations.forEach(annotation => {
      ctx.save();
      ctx.strokeStyle = annotation.color || '#ff0000';
      ctx.lineWidth = 3;
      ctx.fillStyle = annotation.color ? `${annotation.color}33` : '#ff000033';

      switch (annotation.type) {
        case 'rectangle':
          ctx.strokeRect(
            annotation.x,
            annotation.y,
            annotation.width,
            annotation.height
          );
          if (annotation.filled) {
            ctx.fillRect(
              annotation.x,
              annotation.y,
              annotation.width,
              annotation.height
            );
          }
          break;

        case 'circle':
          ctx.beginPath();
          ctx.arc(
            annotation.x,
            annotation.y,
            annotation.radius,
            0,
            2 * Math.PI
          );
          ctx.stroke();
          if (annotation.filled) {
            ctx.fill();
          }
          break;

        case 'line':
          ctx.beginPath();
          ctx.moveTo(annotation.x1, annotation.y1);
          ctx.lineTo(annotation.x2, annotation.y2);
          ctx.stroke();
          break;

        case 'polygon':
          if (annotation.points && annotation.points.length > 0) {
            ctx.beginPath();
            ctx.moveTo(annotation.points[0].x, annotation.points[0].y);
            annotation.points.slice(1).forEach(point => {
              ctx.lineTo(point.x, point.y);
            });
            ctx.closePath();
            ctx.stroke();
            if (annotation.filled) {
              ctx.fill();
            }
          }
          break;
      }

      ctx.restore();
    });
  };

  const handleMouseDown = (e) => {
    if (tool === 'pointer') return;

    const rect = canvasRef.current.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    setIsDrawing(true);
    setCurrentAnnotation({
      type: tool,
      startX: x,
      startY: y,
      x,
      y
    });
  };

  const handleMouseMove = (e) => {
    if (!isDrawing || tool === 'pointer') return;

    const rect = canvasRef.current.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Redraw canvas with current annotation
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    
    // Clear and redraw
    const img = new Image();
    img.src = imageUrl;
    ctx.drawImage(img, 0, 0);
    drawAnnotations(ctx, existingAnnotations);

    // Draw current annotation
    ctx.save();
    ctx.strokeStyle = '#ff0000';
    ctx.lineWidth = 3;

    switch (tool) {
      case 'rectangle':
        const width = x - currentAnnotation.startX;
        const height = y - currentAnnotation.startY;
        ctx.strokeRect(currentAnnotation.startX, currentAnnotation.startY, width, height);
        break;

      case 'circle':
        const radius = Math.sqrt(
          Math.pow(x - currentAnnotation.startX, 2) +
          Math.pow(y - currentAnnotation.startY, 2)
        );
        ctx.beginPath();
        ctx.arc(currentAnnotation.startX, currentAnnotation.startY, radius, 0, 2 * Math.PI);
        ctx.stroke();
        break;

      case 'line':
        ctx.beginPath();
        ctx.moveTo(currentAnnotation.startX, currentAnnotation.startY);
        ctx.lineTo(x, y);
        ctx.stroke();
        break;
    }

    ctx.restore();
  };

  const handleMouseUp = (e) => {
    if (!isDrawing) return;

    const rect = canvasRef.current.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Create annotation data
    let annotation = {
      type: tool,
      color: '#ff0000'
    };

    switch (tool) {
      case 'rectangle':
        annotation = {
          ...annotation,
          x: Math.min(currentAnnotation.startX, x),
          y: Math.min(currentAnnotation.startY, y),
          width: Math.abs(x - currentAnnotation.startX),
          height: Math.abs(y - currentAnnotation.startY)
        };
        break;

      case 'circle':
        annotation = {
          ...annotation,
          x: currentAnnotation.startX,
          y: currentAnnotation.startY,
          radius: Math.sqrt(
            Math.pow(x - currentAnnotation.startX, 2) +
            Math.pow(y - currentAnnotation.startY, 2)
          )
        };
        break;

      case 'line':
        annotation = {
          ...annotation,
          x1: currentAnnotation.startX,
          y1: currentAnnotation.startY,
          x2: x,
          y2: y
        };
        break;
    }

    // Callback with annotation
    onAnnotationCreate(annotation);

    setIsDrawing(false);
    setCurrentAnnotation(null);
  };

  return (
    <Box>
      <ToggleButtonGroup
        value={tool}
        exclusive
        onChange={(e, newTool) => newTool && setTool(newTool)}
        sx={{ mb: 2 }}
      >
        <ToggleButton value="pointer">
          <Create />
        </ToggleButton>
        <ToggleButton value="rectangle">
          <CropSquare />
        </ToggleButton>
        <ToggleButton value="circle">
          <RadioButtonUnchecked />
        </ToggleButton>
        <ToggleButton value="line">
          <Timeline />
        </ToggleButton>
      </ToggleButtonGroup>

      <canvas
        ref={canvasRef}
        onMouseDown={handleMouseDown}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
        style={{
          border: '1px solid #ccc',
          cursor: tool === 'pointer' ? 'default' : 'crosshair',
          maxWidth: '100%'
        }}
      />
    </Box>
  );
};

export default AnnotationCanvas;
```

### 2.2 Annotation Version History

**Database Schema** (add to migrations):

```sql
CREATE TABLE IF NOT EXISTS annotation_history (
    id SERIAL PRIMARY KEY,
    annotation_id INTEGER NOT NULL REFERENCES annotations(id) ON DELETE CASCADE,
    version INTEGER NOT NULL,
    content TEXT,
    position JSON,
    metadata JSON,
    modified_by INTEGER REFERENCES users(id),
    modified_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    change_description TEXT,
    UNIQUE(annotation_id, version),
    INDEX idx_annotation_version (annotation_id, version DESC)
);
```

**Backend Service Method**:

```python
def update_annotation_with_history(self, annotation_id, user_id, updates):
    """Update annotation and maintain version history"""
    annotation = Annotation.query.get(annotation_id)
    if not annotation:
        return None
    
    # Create history entry with current state
    current_version = AnnotationHistory.query.filter_by(
        annotation_id=annotation_id
    ).count()
    
    history = AnnotationHistory(
        annotation_id=annotation_id,
        version=current_version + 1,
        content=annotation.content,
        position=annotation.position,
        metadata=annotation.metadata,
        modified_by=user_id,
        change_description=updates.get('change_description', 'Updated annotation')
    )
    
    db.session.add(history)
    
    # Update annotation
    if 'content' in updates:
        annotation.content = updates['content']
    if 'position' in updates:
        annotation.position = updates['position']
    if 'metadata' in updates:
        annotation.metadata = updates['metadata']
    
    annotation.updated_at = datetime.utcnow()
    
    db.session.commit()
    
    return {
        'id': annotation.id,
        'version': current_version + 1,
        'content': annotation.content,
        'position': annotation.position,
        'metadata': annotation.metadata
    }
```

## Feature 3: Rich Communication Features

### 3.1 Rich Text Editor Integration

**Install Dependencies**:

```bash
cd frontend/dashboard
npm install @tiptap/react @tiptap/starter-kit @tiptap/extension-mention
```

**Rich Text Component** (`frontend/dashboard/src/components/collaboration/RichTextEditor.js`):

```javascript
import React from 'react';
import { useEditor, EditorContent } from '@tiptap/react';
import StarterKit from '@tiptap/starter-kit';
import Mention from '@tiptap/extension-mention';
import { Box, IconButton, Divider } from '@mui/material';
import {
  FormatBold,
  FormatItalic,
  FormatListBulleted,
  FormatListNumbered,
  Code
} from '@mui/icons-material';

const RichTextEditor = ({ value, onChange, mentions = [] }) => {
  const editor = useEditor({
    extensions: [
      StarterKit,
      Mention.configure({
        HTMLAttributes: {
          class: 'mention',
        },
        suggestion: {
          items: ({ query }) => {
            return mentions
              .filter(item => 
                item.label.toLowerCase().startsWith(query.toLowerCase())
              )
              .slice(0, 5);
          },
        },
      }),
    ],
    content: value,
    onUpdate: ({ editor }) => {
      onChange(editor.getHTML());
    },
  });

  if (!editor) {
    return null;
  }

  return (
    <Box sx={{ border: '1px solid #ccc', borderRadius: 1 }}>
      <Box sx={{ p: 1, display: 'flex', gap: 1, flexWrap: 'wrap' }}>
        <IconButton
          size="small"
          onClick={() => editor.chain().focus().toggleBold().run()}
          color={editor.isActive('bold') ? 'primary' : 'default'}
        >
          <FormatBold />
        </IconButton>
        <IconButton
          size="small"
          onClick={() => editor.chain().focus().toggleItalic().run()}
          color={editor.isActive('italic') ? 'primary' : 'default'}
        >
          <FormatItalic />
        </IconButton>
        <Divider orientation="vertical" flexItem />
        <IconButton
          size="small"
          onClick={() => editor.chain().focus().toggleBulletList().run()}
          color={editor.isActive('bulletList') ? 'primary' : 'default'}
        >
          <FormatListBulleted />
        </IconButton>
        <IconButton
          size="small"
          onClick={() => editor.chain().focus().toggleOrderedList().run()}
          color={editor.isActive('orderedList') ? 'primary' : 'default'}
        >
          <FormatListNumbered />
        </IconButton>
        <Divider orientation="vertical" flexItem />
        <IconButton
          size="small"
          onClick={() => editor.chain().focus().toggleCode().run()}
          color={editor.isActive('code') ? 'primary' : 'default'}
        >
          <Code />
        </IconButton>
      </Box>
      <Divider />
      <EditorContent 
        editor={editor}
        style={{
          padding: '12px',
          minHeight: '100px',
          maxHeight: '300px',
          overflowY: 'auto'
        }}
      />
    </Box>
  );
};

export default RichTextEditor;
```

## Testing Phase 2 Features

### Unit Tests

```python
# tests/test_viewport_sync.py
import pytest
from backend.collaboration import ViewportState

def test_viewport_update():
    vs = ViewportState()
    viewport = vs.update_viewport('room1', 'user1', {
        'zoom': 1.5,
        'center_x': 100,
        'center_y': 200
    })
    
    assert viewport['state']['zoom'] == 1.5
    assert viewport['updated_by'] == 'user1'

def test_viewport_lock():
    vs = ViewportState()
    
    # User 1 locks viewport
    vs.update_viewport('room1', 'user1', {
        'zoom': 1.0,
        'locked_by': 'user1'
    })
    
    # User 2 cannot update
    with pytest.raises(ValueError):
        vs.update_viewport('room1', 'user2', {
            'zoom': 2.0'
        })
```

### Integration Tests

```javascript
// frontend/dashboard/src/components/collaboration/__tests__/CursorOverlay.test.js
import { render, screen, fireEvent } from '@testing-library/react';
import CursorOverlay from '../CursorOverlay';

describe('CursorOverlay', () => {
  it('should display remote cursors', () => {
    const containerRef = { current: document.createElement('div') };
    const { rerender } = render(
      <CursorOverlay roomId="test-room" containerRef={containerRef} />
    );

    // Simulate receiving cursor update
    WebSocketService.emit('cursor_update', {
      user_id: 'user1',
      username: 'TestUser',
      position: { x: 100, y: 100 }
    });

    rerender(<CursorOverlay roomId="test-room" containerRef={containerRef} />);
    
    expect(screen.getByText('TestUser')).toBeInTheDocument();
  });
});
```

## Deployment

### Database Migration

```bash
# Run Phase 2 migrations
psql -U wildlife_user -d wildlife_db -f backend/migrations/phase2_schema.sql
```

### Configuration

Add to `.env`:

```env
# Phase 2 Configuration
REDIS_URL=redis://localhost:6379/0
CURSOR_UPDATE_THROTTLE_MS=50
VIEWPORT_SYNC_ENABLED=true
ACTIVITY_FEED_LIMIT=100
```

### Performance Tuning

```python
# backend/config.py additions
class ProductionConfig(Config):
    # Redis configuration for cursor positions
    REDIS_CURSOR_TTL = 10  # seconds
    
    # Viewport sync settings
    VIEWPORT_LOCK_TIMEOUT = 300  # 5 minutes
    
    # Activity feed pagination
    ACTIVITY_FEED_PAGE_SIZE = 50
```

## Monitoring

### Metrics to Track

- Cursor update latency (p50, p95, p99)
- Viewport sync propagation time
- Annotation creation/update latency
- Activity feed query performance
- Redis memory usage for ephemeral data

### Dashboards

Create Grafana dashboards for:
1. Real-time collaboration metrics
2. WebSocket connection health
3. Redis performance
4. User activity patterns

## Next Steps

After completing Phase 2:
1. User acceptance testing with research teams
2. Performance optimization based on metrics
3. Begin Phase 3 planning (WebRTC integration)
4. Documentation updates

## Support

For questions or issues:
- Review [Collaboration Features Documentation](./COLLABORATION_FEATURES.md)
- Check [Architecture Guide](./COMPREHENSIVE_COLLABORATION_ARCHITECTURE.md)
- Open an issue on GitHub
