/**
 * Team Chat Component - Real-time team communication
 */

import React, { useState, useEffect, useRef } from 'react';
import { 
  Box, Paper, TextField, Button, Typography, List, ListItem, 
  ListItemText, Avatar, Divider 
} from '@mui/material';
import { Send } from '@mui/icons-material';
import WebSocketService from '../services/WebSocketService';
import APIService from '../services/api';

function TeamChat({ detectionId = null }) {
  const [messages, setMessages] = useState([]);
  const [newMessage, setNewMessage] = useState('');
  const [loading, setLoading] = useState(false);
  const messagesEndRef = useRef(null);

  useEffect(() => {
    loadMessages();
    
    // Listen for new messages via WebSocket
    WebSocketService.on('chat_message', (message) => {
      setMessages(prev => [...prev, message]);
      scrollToBottom();
    });

    return () => {
      WebSocketService.off('chat_message');
    };
  }, [detectionId]);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  const loadMessages = async () => {
    try {
      setLoading(true);
      const params = detectionId ? `?detection_id=${detectionId}` : '';
      const response = await fetch(`/api/collaboration/chat${params}`, {
        headers: {
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        }
      });
      const data = await response.json();
      setMessages(data.messages || []);
      setTimeout(scrollToBottom, 100);
    } catch (error) {
      console.error('Failed to load messages:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleSend = async () => {
    if (!newMessage.trim()) return;

    try {
      const response = await fetch('/api/collaboration/chat', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        },
        body: JSON.stringify({
          message: newMessage,
          detection_id: detectionId
        })
      });

      if (response.ok) {
        setNewMessage('');
      }
    } catch (error) {
      console.error('Failed to send message:', error);
    }
  };

  const handleKeyPress = (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSend();
    }
  };

  return (
    <Paper sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      <Box sx={{ p: 2, borderBottom: 1, borderColor: 'divider' }}>
        <Typography variant="h6">
          {detectionId ? 'Detection Discussion' : 'Team Chat'}
        </Typography>
      </Box>

      <Box sx={{ flexGrow: 1, overflow: 'auto', p: 2 }}>
        <List>
          {messages.map((message, index) => (
            <ListItem key={message.id || index} alignItems="flex-start">
              <Avatar sx={{ mr: 2, bgcolor: 'primary.main' }}>
                {message.user_name?.charAt(0).toUpperCase() || '?'}
              </Avatar>
              <ListItemText
                primary={
                  <Typography variant="subtitle2">
                    {message.user_name || 'Unknown User'}
                  </Typography>
                }
                secondary={
                  <>
                    <Typography variant="body2" color="text.primary" sx={{ whiteSpace: 'pre-wrap' }}>
                      {message.message}
                    </Typography>
                    <Typography variant="caption" color="text.secondary">
                      {new Date(message.created_at).toLocaleString()}
                    </Typography>
                  </>
                }
              />
            </ListItem>
          ))}
          <div ref={messagesEndRef} />
        </List>
      </Box>

      <Divider />

      <Box sx={{ p: 2, display: 'flex', gap: 1 }}>
        <TextField
          fullWidth
          multiline
          maxRows={3}
          placeholder="Type a message..."
          value={newMessage}
          onChange={(e) => setNewMessage(e.target.value)}
          onKeyPress={handleKeyPress}
          size="small"
        />
        <Button
          variant="contained"
          endIcon={<Send />}
          onClick={handleSend}
          disabled={!newMessage.trim()}
        >
          Send
        </Button>
      </Box>
    </Paper>
  );
}

export default TeamChat;
