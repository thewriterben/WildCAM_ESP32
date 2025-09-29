/**
 * Main Dashboard Home Component
 * Real-time overview of wildlife monitoring system
 */

import React, { useState, useEffect } from 'react';
import {
  Grid,
  Card,
  CardContent,
  Typography,
  Box,
  Chip,
  CircularProgress,
  Alert,
  Avatar,
  List,
  ListItem,
  ListItemAvatar,
  ListItemText,
  Paper,
  LinearProgress,
} from '@mui/material';
import {
  PhotoCamera,
  Battery20,
  Battery80,
  BatteryFull,
  SignalWifi4Bar,
  SignalWifiOff,
  Thermostat,
  Opacity,
  Timeline,
  Analytics,
} from '@mui/icons-material';
import { PieChart, Pie, Cell, BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

import apiService from '../services/api';
import WebSocketService from '../services/WebSocketService';

const COLORS = ['#0088FE', '#00C49F', '#FFBB28', '#FF8042', '#8884d8'];

function DashboardHome() {
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [dashboardData, setDashboardData] = useState({
    cameras: [],
    recentDetections: [],
    speciesData: [],
    activityData: [],
    systemStats: {
      totalCameras: 0,
      activeCameras: 0,
      totalDetections: 0,
      speciesCount: 0,
    },
  });

  useEffect(() => {
    loadDashboardData();
    
    // Subscribe to real-time updates
    WebSocketService.subscribeToDetections(handleNewDetection);
    WebSocketService.subscribeToDeviceUpdates(handleDeviceUpdate);
    
    return () => {
      WebSocketService.off('new_detection', handleNewDetection);
      WebSocketService.off('device_status_update', handleDeviceUpdate);
    };
  }, []);

  const loadDashboardData = async () => {
    try {
      setLoading(true);
      setError(null);

      // Load all dashboard data concurrently
      const [camerasRes, detectionsRes, speciesRes, activityRes] = await Promise.all([
        apiService.getCameras(),
        apiService.getDetections({ per_page: 10 }),
        apiService.getSpeciesAnalytics(30),
        apiService.getActivityPatterns(7),
      ]);

      setDashboardData({
        cameras: camerasRes.cameras || [],
        recentDetections: detectionsRes.detections || [],
        speciesData: speciesRes.species_data || [],
        activityData: activityRes.hourly_activity || [],
        systemStats: {
          totalCameras: camerasRes.count || 0,
          activeCameras: (camerasRes.cameras || []).filter(c => c.status === 'active').length,
          totalDetections: detectionsRes.total || 0,
          speciesCount: (speciesRes.species_data || []).length,
        },
      });
    } catch (err) {
      console.error('Failed to load dashboard data:', err);
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  const handleNewDetection = (detection) => {
    setDashboardData(prev => ({
      ...prev,
      recentDetections: [detection, ...prev.recentDetections.slice(0, 9)],
      systemStats: {
        ...prev.systemStats,
        totalDetections: prev.systemStats.totalDetections + 1,
      },
    }));
  };

  const handleDeviceUpdate = (status) => {
    setDashboardData(prev => ({
      ...prev,
      cameras: prev.cameras.map(camera =>
        camera.device_id === status.device_id
          ? { ...camera, ...status }
          : camera
      ),
    }));
  };

  const getBatteryIcon = (level) => {
    if (level >= 80) return <BatteryFull color="success" />;
    if (level >= 30) return <Battery80 color="warning" />;
    return <Battery20 color="error" />;
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'active': return 'success';
      case 'inactive': return 'error';
      case 'maintenance': return 'warning';
      default: return 'default';
    }
  };

  const formatTimestamp = (timestamp) => {
    return new Date(timestamp).toLocaleString();
  };

  if (loading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <CircularProgress size={60} />
      </Box>
    );
  }

  if (error) {
    return (
      <Alert severity="error" sx={{ mb: 2 }}>
        Error loading dashboard data: {error}
      </Alert>
    );
  }

  return (
    <Box sx={{ flexGrow: 1 }}>
      {/* System Overview Cards */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center">
                <PhotoCamera color="primary" sx={{ mr: 2 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Active Cameras
                  </Typography>
                  <Typography variant="h4">
                    {dashboardData.systemStats.activeCameras}/{dashboardData.systemStats.totalCameras}
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center">
                <Timeline color="secondary" sx={{ mr: 2 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Total Detections
                  </Typography>
                  <Typography variant="h4">
                    {dashboardData.systemStats.totalDetections}
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center">
                <Analytics color="info" sx={{ mr: 2 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Species Detected
                  </Typography>
                  <Typography variant="h4">
                    {dashboardData.systemStats.speciesCount}
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>

        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center">
                <SignalWifi4Bar color="success" sx={{ mr: 2 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    System Status
                  </Typography>
                  <Typography variant="h6" color="success.main">
                    Operational
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      <Grid container spacing={3}>
        {/* Camera Status */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Camera Network Status
              </Typography>
              <List>
                {dashboardData.cameras.slice(0, 5).map((camera) => (
                  <ListItem key={camera.id}>
                    <ListItemAvatar>
                      <Avatar>
                        <PhotoCamera />
                      </Avatar>
                    </ListItemAvatar>
                    <ListItemText
                      primary={camera.name || `Camera ${camera.device_id}`}
                      secondary={
                        <Box>
                          <Box display="flex" alignItems="center" gap={1} mb={1}>
                            <Chip 
                              label={camera.status} 
                              color={getStatusColor(camera.status)} 
                              size="small" 
                            />
                            {camera.battery_level && getBatteryIcon(camera.battery_level)}
                            <Typography variant="caption">
                              {camera.battery_level}%
                            </Typography>
                          </Box>
                          <Box display="flex" gap={2}>
                            {camera.temperature && (
                              <Box display="flex" alignItems="center">
                                <Thermostat fontSize="small" />
                                <Typography variant="caption">
                                  {camera.temperature}°C
                                </Typography>
                              </Box>
                            )}
                            {camera.humidity && (
                              <Box display="flex" alignItems="center">
                                <Opacity fontSize="small" />
                                <Typography variant="caption">
                                  {camera.humidity}%
                                </Typography>
                              </Box>
                            )}
                          </Box>
                          <Typography variant="caption" color="textSecondary">
                            Last seen: {camera.last_seen && formatTimestamp(camera.last_seen)}
                          </Typography>
                        </Box>
                      }
                    />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Recent Detections */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Recent Wildlife Detections
              </Typography>
              <List>
                {dashboardData.recentDetections.map((detection, index) => (
                  <ListItem key={detection.id || index}>
                    <ListItemAvatar>
                      <Avatar 
                        src={detection.thumbnail_url} 
                        sx={{ width: 56, height: 56 }}
                      >
                        <PhotoCamera />
                      </Avatar>
                    </ListItemAvatar>
                    <ListItemText
                      primary={detection.detected_species || 'Unknown Species'}
                      secondary={
                        <Box>
                          <Typography variant="body2">
                            Confidence: {((detection.confidence_score || 0) * 100).toFixed(1)}%
                          </Typography>
                          <Typography variant="caption" color="textSecondary">
                            {formatTimestamp(detection.timestamp)}
                          </Typography>
                        </Box>
                      }
                    />
                  </ListItem>
                ))}
              </List>
            </CardContent>
          </Card>
        </Grid>

        {/* Species Distribution */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Species Distribution (30 days)
              </Typography>
              {dashboardData.speciesData.length > 0 ? (
                <ResponsiveContainer width="100%" height={300}>
                  <PieChart>
                    <Pie
                      data={dashboardData.speciesData}
                      cx="50%"
                      cy="50%"
                      labelLine={false}
                      label={({ species, percent }) => `${species} ${(percent * 100).toFixed(0)}%`}
                      outerRadius={80}
                      fill="#8884d8"
                      dataKey="count"
                    >
                      {dashboardData.speciesData.map((entry, index) => (
                        <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                      ))}
                    </Pie>
                    <Tooltip />
                  </PieChart>
                </ResponsiveContainer>
              ) : (
                <Typography color="textSecondary">No species data available</Typography>
              )}
            </CardContent>
          </Card>
        </Grid>

        {/* Activity Patterns */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Hourly Activity Pattern (7 days)
              </Typography>
              {dashboardData.activityData.length > 0 ? (
                <ResponsiveContainer width="100%" height={300}>
                  <BarChart data={dashboardData.activityData}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis dataKey="hour" />
                    <YAxis />
                    <Tooltip />
                    <Bar dataKey="count" fill="#8884d8" />
                  </BarChart>
                </ResponsiveContainer>
              ) : (
                <Typography color="textSecondary">No activity data available</Typography>
              )}
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
}

export default DashboardHome;