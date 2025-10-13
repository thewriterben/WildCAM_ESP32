/**
 * User Presence Component - Show who's currently viewing the dashboard
 */

import React, { useState, useEffect } from 'react';
import { 
  Box, Chip, Avatar, Tooltip, Badge, Typography 
} from '@mui/material';
import { Group } from '@mui/icons-material';
import WebSocketService from '../services/WebSocketService';

function UserPresence() {
  const [activeUsers, setActiveUsers] = useState([]);

  useEffect(() => {
    loadActiveUsers();
    
    // Refresh every 30 seconds
    const interval = setInterval(loadActiveUsers, 30000);

    // Listen for user connection events
    WebSocketService.on('user_connected', (data) => {
      loadActiveUsers();
    });

    return () => {
      clearInterval(interval);
      WebSocketService.off('user_connected');
    };
  }, []);

  const loadActiveUsers = async () => {
    try {
      const response = await fetch('/api/collaboration/presence', {
        headers: {
          'Authorization': `Bearer ${localStorage.getItem('access_token')}`
        }
      });
      const data = await response.json();
      setActiveUsers(data.active_users || []);
    } catch (error) {
      console.error('Failed to load active users:', error);
    }
  };

  return (
    <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
      <Group sx={{ color: 'text.secondary' }} />
      <Typography variant="caption" color="text.secondary">
        {activeUsers.length} online
      </Typography>
      <Box sx={{ display: 'flex', gap: 0.5, ml: 1 }}>
        {activeUsers.slice(0, 5).map((user) => (
          <Tooltip key={user.id} title={user.username}>
            <Badge
              overlap="circular"
              anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
              variant="dot"
              sx={{
                '& .MuiBadge-badge': {
                  backgroundColor: '#44b700',
                  color: '#44b700',
                  boxShadow: '0 0 0 2px #1e1e1e',
                }
              }}
            >
              <Avatar 
                sx={{ 
                  width: 28, 
                  height: 28, 
                  fontSize: '0.875rem',
                  bgcolor: 'secondary.main' 
                }}
              >
                {user.username && user.username.length > 0
                  ? user.username.charAt(0).toUpperCase()
                  : '?'}
              </Avatar>
            </Badge>
          </Tooltip>
        ))}
        {activeUsers.length > 5 && (
          <Chip 
            label={`+${activeUsers.length - 5}`} 
            size="small" 
            sx={{ height: 28 }}
          />
        )}
      </Box>
    </Box>
  );
}

export default UserPresence;
