/**
 * UserPresence Component
 * Displays active users and their current activity
 */

import React, { useState, useEffect } from 'react';
import {
  Box,
  Paper,
  Typography,
  Avatar,
  AvatarGroup,
  Chip,
  Tooltip,
  List,
  ListItem,
  ListItemAvatar,
  ListItemText,
  Badge,
  IconButton,
  Collapse
} from '@mui/material';
import {
  People,
  ExpandMore,
  ExpandLess,
  Circle
} from '@mui/icons-material';

import WebSocketService from '../../services/WebSocketService';
import collaborationApi from '../../services/collaborationApi';

function UserPresence() {
  const [activeUsers, setActiveUsers] = useState([]);
  const [expanded, setExpanded] = useState(false);

  useEffect(() => {
    // Load initial active users
    loadActiveUsers();

    // Subscribe to presence updates
    WebSocketService.subscribeToPresenceUpdates(handlePresenceUpdate);

    return () => {
      WebSocketService.off('presence_update', handlePresenceUpdate);
    };
  }, []);

  const loadActiveUsers = async () => {
    try {
      const data = await collaborationApi.getActiveUsers();
      setActiveUsers(data.active_users || []);
    } catch (error) {
      console.error('Error loading active users:', error);
    }
  };

  const handlePresenceUpdate = (data) => {
    setActiveUsers(data.active_users || []);
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

  const getInitials = (username) => {
    return username
      .split(' ')
      .map(n => n[0])
      .join('')
      .toUpperCase()
      .substring(0, 2);
  };

  return (
    <Paper elevation={2} sx={{ p: 2, mb: 2 }}>
      <Box 
        sx={{ 
          display: 'flex', 
          alignItems: 'center', 
          justifyContent: 'space-between',
          mb: expanded ? 2 : 0
        }}
      >
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          <People />
          <Typography variant="h6">
            Active Users ({activeUsers.length})
          </Typography>
        </Box>
        
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          <AvatarGroup max={4}>
            {activeUsers.slice(0, 4).map((user) => (
              <Tooltip key={user.user_id} title={user.username}>
                <Badge
                  overlap="circular"
                  anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
                  badgeContent={
                    <Circle 
                      sx={{ 
                        fontSize: 10, 
                        color: '#4caf50'
                      }} 
                    />
                  }
                >
                  <Avatar
                    sx={{
                      bgcolor: getRoleColor(user.role),
                      width: 32,
                      height: 32,
                      fontSize: 14
                    }}
                  >
                    {getInitials(user.username)}
                  </Avatar>
                </Badge>
              </Tooltip>
            ))}
          </AvatarGroup>
          
          <IconButton size="small" onClick={() => setExpanded(!expanded)}>
            {expanded ? <ExpandLess /> : <ExpandMore />}
          </IconButton>
        </Box>
      </Box>

      <Collapse in={expanded}>
        <List dense>
          {activeUsers.map((user) => (
            <ListItem key={user.user_id}>
              <ListItemAvatar>
                <Badge
                  overlap="circular"
                  anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
                  badgeContent={
                    <Circle 
                      sx={{ 
                        fontSize: 10, 
                        color: '#4caf50'
                      }} 
                    />
                  }
                >
                  <Avatar
                    sx={{
                      bgcolor: getRoleColor(user.role),
                      width: 40,
                      height: 40
                    }}
                  >
                    {getInitials(user.username)}
                  </Avatar>
                </Badge>
              </ListItemAvatar>
              <ListItemText
                primary={user.username}
                secondary={
                  <Box sx={{ display: 'flex', gap: 1, mt: 0.5 }}>
                    <Chip 
                      label={user.role} 
                      size="small" 
                      sx={{ 
                        bgcolor: getRoleColor(user.role),
                        color: 'white',
                        fontSize: 10
                      }}
                    />
                    {user.current_page && (
                      <Chip 
                        label={user.current_page} 
                        size="small" 
                        variant="outlined"
                        sx={{ fontSize: 10 }}
                      />
                    )}
                  </Box>
                }
              />
            </ListItem>
          ))}
        </List>
      </Collapse>
    </Paper>
  );
}

export default UserPresence;
