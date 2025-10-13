/**
 * TeamChat Component
 * Real-time team chat with channel support
 */

import React, { useState, useEffect, useRef } from 'react';
import {
  Box,
  Paper,
  Typography,
  TextField,
  IconButton,
  List,
  ListItem,
  Avatar,
  Chip,
  Tabs,
  Tab,
  Badge
} from '@mui/material';
import {
  Send,
  Chat,
  AttachFile
} from '@mui/icons-material';
import moment from 'moment';

import WebSocketService from '../../services/WebSocketService';
import collaborationApi from '../../services/collaborationApi';

function TeamChat() {
  const [messages, setMessages] = useState([]);
  const [newMessage, setNewMessage] = useState('');
  const [currentChannel, setCurrentChannel] = useState('general');
  const [unreadCounts, setUnreadCounts] = useState({});
  const messagesEndRef = useRef(null);

  const channels = [
    { id: 'general', label: 'General' },
    { id: 'detections', label: 'Detections' },
    { id: 'field-work', label: 'Field Work' }
  ];

  useEffect(() => {
    loadMessages(currentChannel);
    WebSocketService.joinChannel(currentChannel);

    // Subscribe to new messages
    WebSocketService.subscribeToChatMessages(handleNewMessage);

    return () => {
      WebSocketService.leaveChannel(currentChannel);
      WebSocketService.off('chat_message', handleNewMessage);
    };
  }, [currentChannel]);

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const loadMessages = async (channel) => {
    try {
      const msgs = await collaborationApi.getChatMessages(channel, 50);
      setMessages(msgs);
      
      // Clear unread count for this channel
      setUnreadCounts(prev => ({
        ...prev,
        [channel]: 0
      }));
    } catch (error) {
      console.error('Error loading messages:', error);
    }
  };

  const handleNewMessage = (data) => {
    if (data.channel === currentChannel) {
      setMessages(prev => [...prev, data]);
    } else {
      // Increment unread count for other channels
      setUnreadCounts(prev => ({
        ...prev,
        [data.channel]: (prev[data.channel] || 0) + 1
      }));
    }
  };

  const handleSendMessage = async () => {
    if (!newMessage.trim()) return;

    try {
      await collaborationApi.sendChatMessage(newMessage, currentChannel);
      setNewMessage('');
    } catch (error) {
      console.error('Error sending message:', error);
    }
  };

  const handleKeyPress = (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      handleSendMessage();
    }
  };

  const handleChannelChange = (event, newChannel) => {
    setCurrentChannel(newChannel);
  };

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  const getInitials = (username) => {
    return username
      .split(' ')
      .map(n => n[0])
      .join('')
      .toUpperCase()
      .substring(0, 2);
  };

  const getRoleColor = (role) => {
    const colors = {
      admin: '#f44336',
      researcher: '#2196f3',
      observer: '#4caf50',
      field_worker: '#ff9800',
      user: '#9e9e9e'
    };
    return colors[role] || colors.user;
  };

  return (
    <Paper elevation={2} sx={{ height: 600, display: 'flex', flexDirection: 'column' }}>
      {/* Header */}
      <Box sx={{ p: 2, borderBottom: 1, borderColor: 'divider' }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 2 }}>
          <Chat />
          <Typography variant="h6">Team Chat</Typography>
        </Box>
        
        <Tabs value={currentChannel} onChange={handleChannelChange}>
          {channels.map(channel => (
            <Tab
              key={channel.id}
              value={channel.id}
              label={
                <Badge badgeContent={unreadCounts[channel.id] || 0} color="error">
                  {channel.label}
                </Badge>
              }
            />
          ))}
        </Tabs>
      </Box>

      {/* Messages */}
      <Box sx={{ flexGrow: 1, overflow: 'auto', p: 2 }}>
        <List>
          {messages.map((msg, index) => (
            <ListItem
              key={msg.id || index}
              alignItems="flex-start"
              sx={{ mb: 2 }}
            >
              <Avatar
                sx={{
                  bgcolor: msg.user ? getRoleColor(msg.user.role) : '#9e9e9e',
                  mr: 2
                }}
              >
                {msg.user ? getInitials(msg.user.username) : '?'}
              </Avatar>
              
              <Box sx={{ flex: 1 }}>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 0.5 }}>
                  <Typography variant="subtitle2">
                    {msg.user ? msg.user.username : 'Unknown'}
                  </Typography>
                  {msg.user && (
                    <Chip
                      label={msg.user.role}
                      size="small"
                      sx={{
                        bgcolor: getRoleColor(msg.user.role),
                        color: 'white',
                        fontSize: 10,
                        height: 20
                      }}
                    />
                  )}
                  <Typography variant="caption" color="text.secondary">
                    {moment(msg.created_at).format('HH:mm')}
                  </Typography>
                </Box>
                
                <Typography variant="body2" sx={{ whiteSpace: 'pre-wrap' }}>
                  {msg.message}
                </Typography>
              </Box>
            </ListItem>
          ))}
          <div ref={messagesEndRef} />
        </List>
      </Box>

      {/* Input */}
      <Box sx={{ p: 2, borderTop: 1, borderColor: 'divider' }}>
        <Box sx={{ display: 'flex', gap: 1 }}>
          <TextField
            fullWidth
            size="small"
            placeholder="Type a message..."
            value={newMessage}
            onChange={(e) => setNewMessage(e.target.value)}
            onKeyPress={handleKeyPress}
            multiline
            maxRows={3}
          />
          <IconButton color="primary" onClick={handleSendMessage}>
            <Send />
          </IconButton>
        </Box>
      </Box>
    </Paper>
  );
}

export default TeamChat;
