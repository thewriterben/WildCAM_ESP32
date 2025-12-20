/**
 * WildCAM Dashboard - Main Application Component
 * Advanced React-based monitoring interface for ESP32 WildCAM v2.0
 * 
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

import React, { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { Box, AppBar, Toolbar, Typography, Drawer, List, ListItem, ListItemIcon, ListItemText, Badge } from '@mui/material';
import { Dashboard, PhotoCamera, Analytics, Settings, DeviceHub, Map, Timeline, Assessment } from '@mui/icons-material';

// Import dashboard components
import DashboardHome from './components/DashboardHome';
import LiveDetections from './components/LiveDetections';
import SpeciesAnalytics from './components/SpeciesAnalytics';
import CameraManagement from './components/CameraManagement';
import DataAnalytics from './components/DataAnalytics';

// Import WebSocket service
import WebSocketService from './services/WebSocketService';
import APIService from './services/APIService';

// Import styles
import './App.css';

const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#2e7d32', // Forest green
    },
    secondary: {
      main: '#8bc34a', // Light green
    },
    background: {
      default: '#121212',
      paper: '#1e1e1e',
    },
    text: {
      primary: '#ffffff',
      secondary: '#b0b0b0',
    },
  },
  typography: {
    h4: {
      fontWeight: 600,
    },
    h6: {
      fontWeight: 500,
    },
  },
});

const drawerWidth = 240;

function App() {
  // Application state
  const [selectedPage, setSelectedPage] = useState('dashboard');
  const [realtimeData, setRealtimeData] = useState({
    detections: [],
    deviceStatuses: {},
    systemHealth: {},
  });
  const [notificationCount, setNotificationCount] = useState(0);
  const [connectionStatus, setConnectionStatus] = useState('disconnected');

  // Initialize services on component mount
  useEffect(() => {
    // Initialize WebSocket connection
    WebSocketService.connect('ws://localhost:5000');
    
    // Set up WebSocket event handlers
    WebSocketService.on('connect', () => {
      console.log('Connected to WildCAM API');
      setConnectionStatus('connected');
    });

    WebSocketService.on('disconnect', () => {
      console.log('Disconnected from WildCAM API');
      setConnectionStatus('disconnected');
    });

    WebSocketService.on('new_detection', (detection) => {
      console.log('New wildlife detection:', detection);
      setRealtimeData(prev => ({
        ...prev,
        detections: [detection, ...prev.detections.slice(0, 49)] // Keep last 50 detections
      }));
      setNotificationCount(prev => prev + 1);
      
      // Show browser notification for rare species
      if (detection.confidence > 0.9 && detection.species !== 'common_bird') {
        showNotification(`Rare ${detection.species} detected!`, {
          body: `Confidence: ${(detection.confidence * 100).toFixed(1)}%`,
          icon: '/wildlife-icon.png'
        });
      }
    });

    WebSocketService.on('device_status_update', (status) => {
      setRealtimeData(prev => ({
        ...prev,
        deviceStatuses: {
          ...prev.deviceStatuses,
          [status.device_id]: status
        }
      }));
    });

    // Load initial data
    loadInitialData();

    // Set up periodic data refresh
    const refreshInterval = setInterval(loadInitialData, 30000); // Every 30 seconds

    // Cleanup on unmount
    return () => {
      clearInterval(refreshInterval);
      WebSocketService.disconnect();
    };
  }, []);

  const loadInitialData = async () => {
    try {
      // Placeholder for initial data loading
      console.log('Loading initial data...');
    } catch (error) {
      console.error('Failed to load initial data:', error);
    }
  };

  const showNotification = (title, options) => {
    if ('Notification' in window && Notification.permission === 'granted') {
      new Notification(title, options);
    } else if ('Notification' in window && Notification.permission !== 'denied') {
      Notification.requestPermission().then(permission => {
        if (permission === 'granted') {
          new Notification(title, options);
        }
      });
    }
  };

  const menuItems = [
    { id: 'dashboard', label: 'Dashboard', icon: <Dashboard /> },
    { id: 'detections', label: 'Live Detections', icon: <PhotoCamera /> },
    { id: 'analytics', label: 'Species Analytics', icon: <Analytics /> },
    { id: 'data-analytics', label: 'Data Analytics', icon: <Assessment /> },
    { id: 'devices', label: 'Device Management', icon: <DeviceHub /> },
    { id: 'map', label: 'Map View', icon: <Map /> },
    { id: 'timeline', label: 'Activity Timeline', icon: <Timeline /> },
    { id: 'settings', label: 'Settings', icon: <Settings /> },
  ];

  const handleMenuClick = (pageId) => {
    setSelectedPage(pageId);
    if (pageId === 'detections') {
      setNotificationCount(0); // Clear notification count when viewing detections
    }
  };

  const renderSelectedPage = () => {
    switch (selectedPage) {
      case 'dashboard':
        return <DashboardHome realtimeData={realtimeData} />;
      case 'detections':
        return <LiveDetections />;
      case 'analytics':
        return <SpeciesAnalytics />;
      case 'data-analytics':
        return <DataAnalytics />;
      case 'devices':
        return <CameraManagement />;
      case 'map':
        return <div>Map View - Geographic distribution of wildlife detections</div>;
      case 'timeline':
        return <div>Activity Timeline - Historical wildlife activity patterns</div>;
      case 'settings':
        return <div>System Settings - Configuration and preferences</div>;
      default:
        return <DashboardHome realtimeData={realtimeData} />;
    }
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Box sx={{ display: 'flex', height: '100vh' }}>
        {/* Navigation Drawer */}
        <Drawer
          variant="permanent"
          sx={{
            width: drawerWidth,
            flexShrink: 0,
            '& .MuiDrawer-paper': {
              width: drawerWidth,
              boxSizing: 'border-box',
              bgcolor: 'background.paper',
            },
          }}
        >
          <Toolbar>
            <Typography variant="h6" noWrap component="div" sx={{ color: 'primary.main', fontWeight: 'bold' }}>
              🦌 WildCAM v2.0
            </Typography>
          </Toolbar>
          
          <List>
            {menuItems.map((item) => (
              <ListItem
                button
                key={item.id}
                selected={selectedPage === item.id}
                onClick={() => handleMenuClick(item.id)}
                sx={{
                  '&.Mui-selected': {
                    backgroundColor: 'primary.main',
                    '&:hover': {
                      backgroundColor: 'primary.dark',
                    },
                  },
                }}
              >
                <ListItemIcon sx={{ color: selectedPage === item.id ? 'white' : 'text.secondary' }}>
                  {item.id === 'detections' && notificationCount > 0 ? (
                    <Badge badgeContent={notificationCount} color="error">
                      {item.icon}
                    </Badge>
                  ) : (
                    item.icon
                  )}
                </ListItemIcon>
                <ListItemText 
                  primary={item.label} 
                  sx={{ color: selectedPage === item.id ? 'white' : 'text.primary' }}
                />
              </ListItem>
            ))}
          </List>

          {/* Connection Status */}
          <Box sx={{ mt: 'auto', p: 2 }}>
            <Typography variant="caption" sx={{ color: connectionStatus === 'connected' ? 'success.main' : 'error.main' }}>
              ● {connectionStatus === 'connected' ? 'Connected' : 'Disconnected'}
            </Typography>
          </Box>
        </Drawer>

        {/* Main Content */}
        <Box component="main" sx={{ flexGrow: 1, bgcolor: 'background.default', overflow: 'auto' }}>
          <AppBar 
            position="sticky" 
            sx={{ 
              zIndex: (theme) => theme.zIndex.drawer - 1,
              bgcolor: 'background.paper',
              boxShadow: 1,
            }}
          >
            <Toolbar>
              <Typography variant="h6" noWrap component="div" sx={{ flexGrow: 1, color: 'text.primary' }}>
                {menuItems.find(item => item.id === selectedPage)?.label || 'Dashboard'}
              </Typography>
              
              {/* System Status Indicators */}
              <Box sx={{ display: 'flex', gap: 2, alignItems: 'center' }}>
                <Typography variant="caption" sx={{ color: 'text.secondary' }}>
                  Active Devices: {Object.keys(realtimeData.deviceStatuses).length}
                </Typography>
                <Typography variant="caption" sx={{ color: 'text.secondary' }}>
                  Recent Detections: {realtimeData.detections.length}
                </Typography>
              </Box>
            </Toolbar>
          </AppBar>

          {/* Page Content */}
          <Box sx={{ p: 3 }}>
            {renderSelectedPage()}
          </Box>
        </Box>
      </Box>
    </ThemeProvider>
  );
}

export default App;