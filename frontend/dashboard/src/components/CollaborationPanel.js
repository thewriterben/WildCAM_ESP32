/**
 * CollaborationPanel Component
 * Main integration panel for collaboration features
 * Demonstrates how to use collaboration components together
 */

import React, { useEffect, useState } from 'react';
import { Grid, Paper, Box, Typography, Tab, Tabs } from '@mui/material';
import { UserPresence, TeamChat, TaskManager } from './collaboration';
import WebSocketService from '../services/WebSocketService';

function CollaborationPanel() {
  const [activeTab, setActiveTab] = useState(0);
  const [sessionId, setSessionId] = useState(null);

  useEffect(() => {
    // Generate session ID
    const newSessionId = `session_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    setSessionId(newSessionId);

    // Connect to WebSocket if not already connected
    if (!WebSocketService.getConnectionStatus().connected) {
      WebSocketService.connect();
    }

    // Register user presence
    const user = JSON.parse(localStorage.getItem('user') || '{}');
    if (user.id && WebSocketService.getConnectionStatus().connected) {
      WebSocketService.sendPresence('connect', newSessionId, user.id);
      WebSocketService.joinChannel('general');
    }

    // Update activity when tab changes
    const updateActivity = () => {
      if (user.id) {
        WebSocketService.sendPresence(
          'update',
          newSessionId,
          user.id,
          '/collaboration',
          null
        );
      }
    };

    updateActivity();

    // Cleanup on unmount
    return () => {
      if (user.id) {
        WebSocketService.sendPresence('disconnect', newSessionId, user.id);
        WebSocketService.leaveChannel('general');
      }
    };
  }, []);

  const handleTabChange = (event, newValue) => {
    setActiveTab(newValue);
  };

  return (
    <Box sx={{ p: 3 }}>
      {/* Header */}
      <Paper elevation={1} sx={{ p: 2, mb: 3 }}>
        <Typography variant="h4" gutterBottom>
          Team Collaboration
        </Typography>
        <Typography variant="body2" color="text.secondary">
          Real-time collaboration tools for wildlife monitoring teams
        </Typography>
      </Paper>

      {/* User Presence - Always visible */}
      <Grid container spacing={3}>
        <Grid item xs={12}>
          <UserPresence />
        </Grid>

        {/* Tabbed Interface for other features */}
        <Grid item xs={12}>
          <Paper elevation={2}>
            <Tabs value={activeTab} onChange={handleTabChange} sx={{ borderBottom: 1, borderColor: 'divider' }}>
              <Tab label="Team Chat" />
              <Tab label="Tasks" />
              <Tab label="Field Notes" />
            </Tabs>

            <Box sx={{ p: 3 }}>
              {activeTab === 0 && <TeamChat />}
              {activeTab === 1 && <TaskManager />}
              {activeTab === 2 && (
                <Box sx={{ textAlign: 'center', py: 4 }}>
                  <Typography variant="h6" color="text.secondary">
                    Field Notes Component
                  </Typography>
                  <Typography variant="body2" color="text.secondary" sx={{ mt: 2 }}>
                    Field notes UI coming soon. API is ready at /api/collaboration/field-notes
                  </Typography>
                </Box>
              )}
            </Box>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
}

export default CollaborationPanel;
