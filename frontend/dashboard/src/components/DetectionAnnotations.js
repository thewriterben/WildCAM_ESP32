/**
 * Detection Annotations Component - Comments and bookmarks on wildlife detections
 */

import React, { useState, useEffect } from 'react';
import { 
  Box, Paper, Tabs, Tab, TextField, Button, Typography, List, 
  ListItem, ListItemText, IconButton, Chip, Switch, FormControlLabel
} from '@mui/material';
import { Comment, Bookmark, Add } from '@mui/icons-material';
import WebSocketService from '../services/WebSocketService';

function DetectionAnnotations({ detectionId }) {
  const [tabValue, setTabValue] = useState(0);
  const [comments, setComments] = useState([]);
  const [bookmarks, setBookmarks] = useState([]);
  const [newComment, setNewComment] = useState('');
  const [newBookmark, setNewBookmark] = useState({ title: '', description: '', shared: false });
  const [showBookmarkForm, setShowBookmarkForm] = useState(false);

  useEffect(() => {
    if (detectionId) {
      loadComments();
      loadBookmarks();
    }

    // Listen for new comments via WebSocket
    WebSocketService.on('new_comment', (comment) => {
      if (comment.detection_id === detectionId) {
        setComments(prev => [comment, ...prev]);
      }
    });

    WebSocketService.on('new_bookmark', (bookmark) => {
      if (bookmark.shared) {
        setBookmarks(prev => [bookmark, ...prev]);
      }
    });

    return () => {
      WebSocketService.off('new_comment');
      WebSocketService.off('new_bookmark');
    };
  }, [detectionId]);

  const loadComments = async () => {
    try {
      const response = await fetch(`/api/collaboration/comments?detection_id=${detectionId}`, {
        headers: {
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        }
      });
      const data = await response.json();
      setComments(data.comments || []);
    } catch (error) {
      console.error('Failed to load comments:', error);
    }
  };

  const loadBookmarks = async () => {
    try {
      const response = await fetch(`/api/collaboration/bookmarks?shared=true`, {
        headers: {
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        }
      });
      const data = await response.json();
      // Filter bookmarks for this detection
      const filtered = (data.bookmarks || []).filter(b => b.detection_id === detectionId);
      setBookmarks(filtered);
    } catch (error) {
      console.error('Failed to load bookmarks:', error);
    }
  };

  const handleAddComment = async () => {
    if (!newComment.trim()) return;

    try {
      const response = await fetch('/api/collaboration/comments', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        },
        body: JSON.stringify({
          detection_id: detectionId,
          comment: newComment
        })
      });

      if (response.ok) {
        setNewComment('');
      }
    } catch (error) {
      console.error('Failed to add comment:', error);
    }
  };

  const handleAddBookmark = async () => {
    if (!newBookmark.title.trim()) return;

    try {
      const response = await fetch('/api/collaboration/bookmarks', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        },
        body: JSON.stringify({
          detection_id: detectionId,
          ...newBookmark
        })
      });

      if (response.ok) {
        setNewBookmark({ title: '', description: '', shared: false });
        setShowBookmarkForm(false);
        loadBookmarks();
      }
    } catch (error) {
      console.error('Failed to add bookmark:', error);
    }
  };

  return (
    <Paper sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      <Tabs value={tabValue} onChange={(e, v) => setTabValue(v)} sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tab icon={<Comment />} label="Comments" />
        <Tab icon={<Bookmark />} label="Bookmarks" />
      </Tabs>

      {tabValue === 0 && (
        <Box sx={{ flexGrow: 1, overflow: 'auto', p: 2 }}>
          <Box sx={{ mb: 2 }}>
            <TextField
              fullWidth
              multiline
              rows={2}
              placeholder="Add a comment..."
              value={newComment}
              onChange={(e) => setNewComment(e.target.value)}
              size="small"
            />
            <Button
              variant="contained"
              onClick={handleAddComment}
              disabled={!newComment.trim()}
              sx={{ mt: 1 }}
              size="small"
            >
              Add Comment
            </Button>
          </Box>

          <List>
            {comments.map((comment) => (
              <ListItem key={comment.id} alignItems="flex-start">
                <ListItemText
                  primary={
                    <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                      <Typography variant="subtitle2">
                        {comment.user_name}
                      </Typography>
                      <Typography variant="caption" color="text.secondary">
                        {new Date(comment.created_at).toLocaleString()}
                      </Typography>
                    </Box>
                  }
                  secondary={
                    <Typography variant="body2" sx={{ whiteSpace: 'pre-wrap' }}>
                      {comment.comment}
                    </Typography>
                  }
                />
              </ListItem>
            ))}
          </List>
        </Box>
      )}

      {tabValue === 1 && (
        <Box sx={{ flexGrow: 1, overflow: 'auto', p: 2 }}>
          {!showBookmarkForm && (
            <Button
              variant="outlined"
              startIcon={<Add />}
              onClick={() => setShowBookmarkForm(true)}
              fullWidth
              sx={{ mb: 2 }}
            >
              Add Bookmark
            </Button>
          )}

          {showBookmarkForm && (
            <Box sx={{ mb: 2, p: 2, bgcolor: 'background.default', borderRadius: 1 }}>
              <TextField
                fullWidth
                placeholder="Bookmark title"
                value={newBookmark.title}
                onChange={(e) => setNewBookmark({ ...newBookmark, title: e.target.value })}
                size="small"
                sx={{ mb: 1 }}
              />
              <TextField
                fullWidth
                multiline
                rows={2}
                placeholder="Description (optional)"
                value={newBookmark.description}
                onChange={(e) => setNewBookmark({ ...newBookmark, description: e.target.value })}
                size="small"
                sx={{ mb: 1 }}
              />
              <FormControlLabel
                control={
                  <Switch
                    checked={newBookmark.shared}
                    onChange={(e) => setNewBookmark({ ...newBookmark, shared: e.target.checked })}
                  />
                }
                label="Share with team"
                sx={{ mb: 1 }}
              />
              <Box sx={{ display: 'flex', gap: 1 }}>
                <Button
                  variant="contained"
                  onClick={handleAddBookmark}
                  disabled={!newBookmark.title.trim()}
                  size="small"
                >
                  Save
                </Button>
                <Button
                  variant="outlined"
                  onClick={() => setShowBookmarkForm(false)}
                  size="small"
                >
                  Cancel
                </Button>
              </Box>
            </Box>
          )}

          <List>
            {bookmarks.map((bookmark) => (
              <ListItem key={bookmark.id}>
                <ListItemText
                  primary={
                    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                      <Typography variant="subtitle2">
                        {bookmark.title}
                      </Typography>
                      {bookmark.shared && (
                        <Chip label="Shared" size="small" color="primary" />
                      )}
                    </Box>
                  }
                  secondary={
                    <>
                      {bookmark.description && (
                        <Typography variant="body2" color="text.primary">
                          {bookmark.description}
                        </Typography>
                      )}
                      <Typography variant="caption" color="text.secondary">
                        By {bookmark.user_name} • {new Date(bookmark.created_at).toLocaleString()}
                      </Typography>
                    </>
                  }
                />
              </ListItem>
            ))}
          </List>
        </Box>
      )}
    </Paper>
  );
}

export default DetectionAnnotations;
