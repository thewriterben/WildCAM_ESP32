/**
 * Camera Management Component
 * Manage camera network, configuration, and status monitoring
 */

import React, { useState, useEffect } from 'react';
import {
  Grid,
  Card,
  CardContent,
  Typography,
  Box,
  Chip,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  Switch,
  FormControlLabel,
  List,
  ListItem,
  ListItemAvatar,
  ListItemText,
  Avatar,
  IconButton,
  Fab,
  LinearProgress,
  Alert,
  Divider,
  Slider,
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
  Add,
  Edit,
  Delete,
  Settings,
  Refresh,
  LocationOn,
  PowerSettingsNew,
} from '@mui/icons-material';
import { MapContainer, TileLayer, Marker, Popup } from 'react-leaflet';

import apiService from '../services/api';
import WebSocketService from '../services/WebSocketService';

function CameraManagement() {
  const [cameras, setCameras] = useState([]);
  const [loading, setLoading] = useState(true);
  const [selectedCamera, setSelectedCamera] = useState(null);
  const [configDialog, setConfigDialog] = useState(false);
  const [addDialog, setAddDialog] = useState(false);
  const [newCamera, setNewCamera] = useState({
    device_id: '',
    name: '',
    location_name: '',
    latitude: '',
    longitude: '',
    altitude: '',
  });
  const [cameraConfig, setCameraConfig] = useState({
    motion_sensitivity: 75,
    night_vision: true,
    photo_quality: 'high',
    detection_threshold: 0.7,
    capture_interval: 30,
    power_save_mode: false,
  });

  useEffect(() => {
    loadCameras();
    
    // Subscribe to real-time camera updates
    WebSocketService.subscribeToDeviceUpdates(handleDeviceUpdate);
    
    return () => {
      WebSocketService.off('device_status_update', handleDeviceUpdate);
    };
  }, []);

  const handleDeviceUpdate = (status) => {
    setCameras(prev => prev.map(camera =>
      camera.device_id === status.device_id
        ? { ...camera, ...status }
        : camera
    ));
  };

  const loadCameras = async () => {
    try {
      setLoading(true);
      const response = await apiService.getCameras();
      setCameras(response.cameras);
    } catch (error) {
      console.error('Failed to load cameras:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleAddCamera = async () => {
    try {
      await apiService.registerCamera(newCamera);
      setAddDialog(false);
      setNewCamera({
        device_id: '',
        name: '',
        location_name: '',
        latitude: '',
        longitude: '',
        altitude: '',
      });
      loadCameras();
    } catch (error) {
      console.error('Failed to add camera:', error);
    }
  };

  const handleConfigSave = async () => {
    try {
      // Send configuration to camera via WebSocket
      WebSocketService.sendCameraConfig(selectedCamera.device_id, cameraConfig);
      setConfigDialog(false);
      setSelectedCamera(null);
    } catch (error) {
      console.error('Failed to save config:', error);
    }
  };

  const handleCameraSelect = (camera) => {
    setSelectedCamera(camera);
    setCameraConfig(camera.config || {
      motion_sensitivity: 75,
      night_vision: true,
      photo_quality: 'high',
      detection_threshold: 0.7,
      capture_interval: 30,
      power_save_mode: false,
    });
    setConfigDialog(true);
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

  const getSignalIcon = (camera) => {
    const lastSeen = new Date(camera.last_seen);
    const now = new Date();
    const minutesAgo = (now - lastSeen) / (1000 * 60);
    
    if (minutesAgo < 5) return <SignalWifi4Bar color="success" />;
    return <SignalWifiOff color="error" />;
  };

  const formatTimestamp = (timestamp) => {
    return new Date(timestamp).toLocaleString();
  };

  return (
    <Box sx={{ p: 2 }}>
      {/* Header */}
      <Box sx={{ mb: 3, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <Typography variant="h4" component="h1">
          Camera Network Management
        </Typography>
        
        <Box sx={{ display: 'flex', gap: 2 }}>
          <Button
            variant="outlined"
            startIcon={<Refresh />}
            onClick={loadCameras}
            disabled={loading}
          >
            Refresh
          </Button>
          
          <Fab
            color="primary"
            onClick={() => setAddDialog(true)}
            sx={{ ml: 2 }}
          >
            <Add />
          </Fab>
        </Box>
      </Box>

      {/* Camera Grid */}
      <Grid container spacing={3}>
        {cameras.map((camera) => (
          <Grid item xs={12} md={6} lg={4} key={camera.id}>
            <Card sx={{ height: '100%' }}>
              <CardContent>
                {/* Camera Header */}
                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'start', mb: 2 }}>
                  <Box>
                    <Typography variant="h6" noWrap>
                      {camera.name || `Camera ${camera.device_id}`}
                    </Typography>
                    <Typography variant="body2" color="textSecondary">
                      ID: {camera.device_id}
                    </Typography>
                  </Box>
                  
                  <Box sx={{ display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                    <Chip 
                      label={camera.status} 
                      color={getStatusColor(camera.status)} 
                      size="small" 
                      sx={{ mb: 1 }}
                    />
                    {getSignalIcon(camera)}
                  </Box>
                </Box>

                {/* Battery and Power */}
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                  {getBatteryIcon(camera.battery_level)}
                  <Box sx={{ ml: 1, flex: 1 }}>
                    <Typography variant="body2">
                      Battery: {camera.battery_level}%
                    </Typography>
                    <LinearProgress 
                      variant="determinate" 
                      value={camera.battery_level || 0}
                      color={camera.battery_level >= 30 ? 'primary' : 'error'}
                      sx={{ mt: 0.5 }}
                    />
                  </Box>
                </Box>

                {/* Environmental Data */}
                <Box sx={{ display: 'flex', justifyContent: 'space-between', mb: 2 }}>
                  {camera.temperature && (
                    <Box sx={{ display: 'flex', alignItems: 'center' }}>
                      <Thermostat fontSize="small" />
                      <Typography variant="body2" sx={{ ml: 0.5 }}>
                        {camera.temperature}°C
                      </Typography>
                    </Box>
                  )}
                  
                  {camera.humidity && (
                    <Box sx={{ display: 'flex', alignItems: 'center' }}>
                      <Opacity fontSize="small" />
                      <Typography variant="body2" sx={{ ml: 0.5 }}>
                        {camera.humidity}%
                      </Typography>
                    </Box>
                  )}
                  
                  {camera.solar_voltage && (
                    <Box sx={{ display: 'flex', alignItems: 'center' }}>
                      <PowerSettingsNew fontSize="small" />
                      <Typography variant="body2" sx={{ ml: 0.5 }}>
                        {camera.solar_voltage}V
                      </Typography>
                    </Box>
                  )}
                </Box>

                {/* Location */}
                {camera.location_name && (
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                    <LocationOn fontSize="small" />
                    <Typography variant="body2" sx={{ ml: 0.5 }}>
                      {camera.location_name}
                    </Typography>
                  </Box>
                )}

                {/* Last Seen */}
                <Typography variant="caption" color="textSecondary">
                  Last seen: {camera.last_seen && formatTimestamp(camera.last_seen)}
                </Typography>

                {/* Actions */}
                <Box sx={{ mt: 2, display: 'flex', justifyContent: 'space-between' }}>
                  <Button
                    size="small"
                    startIcon={<Settings />}
                    onClick={() => handleCameraSelect(camera)}
                  >
                    Configure
                  </Button>
                  
                  <Button
                    size="small"
                    startIcon={<PhotoCamera />}
                    disabled={camera.status !== 'active'}
                  >
                    Live View
                  </Button>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        ))}
      </Grid>

      {/* Add Camera Dialog */}
      <Dialog open={addDialog} onClose={() => setAddDialog(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Add New Camera</DialogTitle>
        <DialogContent>
          <Grid container spacing={2} sx={{ mt: 1 }}>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Device ID"
                value={newCamera.device_id}
                onChange={(e) => setNewCamera(prev => ({ ...prev, device_id: e.target.value }))}
                required
              />
            </Grid>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Camera Name"
                value={newCamera.name}
                onChange={(e) => setNewCamera(prev => ({ ...prev, name: e.target.value }))}
              />
            </Grid>
            <Grid item xs={12}>
              <TextField
                fullWidth
                label="Location Name"
                value={newCamera.location_name}
                onChange={(e) => setNewCamera(prev => ({ ...prev, location_name: e.target.value }))}
              />
            </Grid>
            <Grid item xs={4}>
              <TextField
                fullWidth
                label="Latitude"
                type="number"
                value={newCamera.latitude}
                onChange={(e) => setNewCamera(prev => ({ ...prev, latitude: e.target.value }))}
              />
            </Grid>
            <Grid item xs={4}>
              <TextField
                fullWidth
                label="Longitude"
                type="number"
                value={newCamera.longitude}
                onChange={(e) => setNewCamera(prev => ({ ...prev, longitude: e.target.value }))}
              />
            </Grid>
            <Grid item xs={4}>
              <TextField
                fullWidth
                label="Altitude (m)"
                type="number"
                value={newCamera.altitude}
                onChange={(e) => setNewCamera(prev => ({ ...prev, altitude: e.target.value }))}
              />
            </Grid>
          </Grid>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setAddDialog(false)}>Cancel</Button>
          <Button onClick={handleAddCamera} variant="contained">Add Camera</Button>
        </DialogActions>
      </Dialog>

      {/* Camera Configuration Dialog */}
      <Dialog open={configDialog} onClose={() => setConfigDialog(false)} maxWidth="md" fullWidth>
        <DialogTitle>
          Camera Configuration - {selectedCamera?.name || selectedCamera?.device_id}
        </DialogTitle>
        <DialogContent>
          {selectedCamera && (
            <Box sx={{ mt: 2 }}>
              {/* Motion Detection */}
              <Typography variant="h6" gutterBottom>Motion Detection</Typography>
              <Box sx={{ mb: 3 }}>
                <Typography gutterBottom>Motion Sensitivity: {cameraConfig.motion_sensitivity}%</Typography>
                <Slider
                  value={cameraConfig.motion_sensitivity}
                  onChange={(e, value) => setCameraConfig(prev => ({ ...prev, motion_sensitivity: value }))}
                  min={10}
                  max={100}
                  step={5}
                  marks
                  valueLabelDisplay="auto"
                />
              </Box>

              <Divider sx={{ my: 2 }} />

              {/* Image Settings */}
              <Typography variant="h6" gutterBottom>Image Settings</Typography>
              <Grid container spacing={2} sx={{ mb: 3 }}>
                <Grid item xs={12} sm={6}>
                  <TextField
                    fullWidth
                    select
                    label="Photo Quality"
                    value={cameraConfig.photo_quality}
                    onChange={(e) => setCameraConfig(prev => ({ ...prev, photo_quality: e.target.value }))}
                    SelectProps={{ native: true }}
                  >
                    <option value="low">Low</option>
                    <option value="medium">Medium</option>
                    <option value="high">High</option>
                  </TextField>
                </Grid>
                <Grid item xs={12} sm={6}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={cameraConfig.night_vision}
                        onChange={(e) => setCameraConfig(prev => ({ ...prev, night_vision: e.target.checked }))}
                      />
                    }
                    label="Night Vision"
                  />
                </Grid>
              </Grid>

              <Divider sx={{ my: 2 }} />

              {/* AI Detection */}
              <Typography variant="h6" gutterBottom>AI Detection</Typography>
              <Box sx={{ mb: 3 }}>
                <Typography gutterBottom>Detection Threshold: {cameraConfig.detection_threshold}</Typography>
                <Slider
                  value={cameraConfig.detection_threshold}
                  onChange={(e, value) => setCameraConfig(prev => ({ ...prev, detection_threshold: value }))}
                  min={0.1}
                  max={1.0}
                  step={0.1}
                  marks
                  valueLabelDisplay="auto"
                />
              </Box>

              <Divider sx={{ my: 2 }} />

              {/* Power Management */}
              <Typography variant="h6" gutterBottom>Power Management</Typography>
              <Grid container spacing={2}>
                <Grid item xs={12} sm={6}>
                  <TextField
                    fullWidth
                    label="Capture Interval (seconds)"
                    type="number"
                    value={cameraConfig.capture_interval}
                    onChange={(e) => setCameraConfig(prev => ({ ...prev, capture_interval: parseInt(e.target.value) }))}
                    inputProps={{ min: 10, max: 3600 }}
                  />
                </Grid>
                <Grid item xs={12} sm={6}>
                  <FormControlLabel
                    control={
                      <Switch
                        checked={cameraConfig.power_save_mode}
                        onChange={(e) => setCameraConfig(prev => ({ ...prev, power_save_mode: e.target.checked }))}
                      />
                    }
                    label="Power Save Mode"
                  />
                </Grid>
              </Grid>
            </Box>
          )}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setConfigDialog(false)}>Cancel</Button>
          <Button onClick={handleConfigSave} variant="contained">Save Configuration</Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
}

export default CameraManagement;