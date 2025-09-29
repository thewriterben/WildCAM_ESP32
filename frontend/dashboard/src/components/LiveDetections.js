/**
 * Live Detections Component
 * Real-time wildlife detection feed with filtering and verification
 */

import React, { useState, useEffect } from 'react';
import {
  Grid,
  Card,
  CardContent,
  CardMedia,
  Typography,
  Box,
  Chip,
  TextField,
  MenuItem,
  Pagination,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  IconButton,
  Avatar,
  List,
  ListItem,
  ListItemAvatar,
  ListItemText,
  Fab,
  Badge,
  CircularProgress,
} from '@mui/material';
import {
  PhotoCamera,
  Verified,
  Close,
  FilterList,
  Refresh,
  ZoomIn,
  CheckCircle,
  Cancel,
  Info,
} from '@mui/icons-material';

import apiService from '../services/api';
import WebSocketService from '../services/WebSocketService';

function LiveDetections() {
  const [detections, setDetections] = useState([]);
  const [loading, setLoading] = useState(true);
  const [selectedDetection, setSelectedDetection] = useState(null);
  const [filters, setFilters] = useState({
    species: '',
    camera_id: '',
    verified_only: false,
    start_date: '',
    end_date: '',
  });
  const [cameras, setCameras] = useState([]);
  const [currentPage, setCurrentPage] = useState(1);
  const [totalPages, setTotalPages] = useState(1);
  const [newDetectionCount, setNewDetectionCount] = useState(0);

  useEffect(() => {
    loadDetections();
    loadCameras();
    
    // Subscribe to real-time updates
    WebSocketService.subscribeToDetections(handleNewDetection);
    
    return () => {
      WebSocketService.off('new_detection', handleNewDetection);
    };
  }, [currentPage, filters]);

  const handleNewDetection = (detection) => {
    setNewDetectionCount(prev => prev + 1);
    // Don't automatically add to list to avoid disrupting user's viewing
    // Instead show a notification badge
  };

  const loadDetections = async () => {
    try {
      setLoading(true);
      const params = {
        page: currentPage,
        per_page: 20,
        ...filters,
      };
      
      const response = await apiService.getDetections(params);
      setDetections(response.detections);
      setTotalPages(response.pages);
    } catch (error) {
      console.error('Failed to load detections:', error);
    } finally {
      setLoading(false);
    }
  };

  const loadCameras = async () => {
    try {
      const response = await apiService.getCameras();
      setCameras(response.cameras);
    } catch (error) {
      console.error('Failed to load cameras:', error);
    }
  };

  const handleFilterChange = (field, value) => {
    setFilters(prev => ({
      ...prev,
      [field]: value,
    }));
    setCurrentPage(1);
  };

  const handleRefresh = () => {
    setNewDetectionCount(0);
    loadDetections();
  };

  const handleDetectionClick = (detection) => {
    setSelectedDetection(detection);
  };

  const handleVerification = async (detectionId, verifiedSpecies) => {
    try {
      // TODO: Implement verification API call
      console.log(`Verifying detection ${detectionId} as ${verifiedSpecies}`);
      // Update local state
      setDetections(prev => prev.map(d => 
        d.id === detectionId 
          ? { ...d, verified: true, verified_species: verifiedSpecies }
          : d
      ));
      setSelectedDetection(null);
    } catch (error) {
      console.error('Failed to verify detection:', error);
    }
  };

  const formatTimestamp = (timestamp) => {
    return new Date(timestamp).toLocaleString();
  };

  const getConfidenceColor = (confidence) => {
    if (confidence >= 0.9) return 'success';
    if (confidence >= 0.7) return 'warning';
    return 'error';
  };

  return (
    <Box sx={{ p: 2 }}>
      {/* Header with filters and refresh */}
      <Box sx={{ mb: 3, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <Typography variant="h4" component="h1">
          Live Wildlife Detections
        </Typography>
        
        <Box sx={{ display: 'flex', gap: 2, alignItems: 'center' }}>
          <Badge badgeContent={newDetectionCount} color="primary">
            <Fab 
              color="primary" 
              size="small" 
              onClick={handleRefresh}
              disabled={loading}
            >
              <Refresh />
            </Fab>
          </Badge>
        </Box>
      </Box>

      {/* Filters */}
      <Card sx={{ mb: 3 }}>
        <CardContent>
          <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
            <FilterList sx={{ mr: 1 }} />
            <Typography variant="h6">Filters</Typography>
          </Box>
          
          <Grid container spacing={2}>
            <Grid item xs={12} sm={6} md={3}>
              <TextField
                fullWidth
                label="Species"
                value={filters.species}
                onChange={(e) => handleFilterChange('species', e.target.value)}
                placeholder="Search species..."
              />
            </Grid>
            
            <Grid item xs={12} sm={6} md={3}>
              <TextField
                fullWidth
                select
                label="Camera"
                value={filters.camera_id}
                onChange={(e) => handleFilterChange('camera_id', e.target.value)}
              >
                <MenuItem value="">All Cameras</MenuItem>
                {cameras.map((camera) => (
                  <MenuItem key={camera.id} value={camera.id}>
                    {camera.name || `Camera ${camera.device_id}`}
                  </MenuItem>
                ))}
              </TextField>
            </Grid>
            
            <Grid item xs={12} sm={6} md={3}>
              <TextField
                fullWidth
                type="date"
                label="Start Date"
                value={filters.start_date}
                onChange={(e) => handleFilterChange('start_date', e.target.value)}
                InputLabelProps={{ shrink: true }}
              />
            </Grid>
            
            <Grid item xs={12} sm={6} md={3}>
              <TextField
                fullWidth
                type="date"
                label="End Date"
                value={filters.end_date}
                onChange={(e) => handleFilterChange('end_date', e.target.value)}
                InputLabelProps={{ shrink: true }}
              />
            </Grid>
          </Grid>
        </CardContent>
      </Card>

      {/* Detection grid */}
      {loading ? (
        <Box display="flex" justifyContent="center" py={4}>
          <CircularProgress />
        </Box>
      ) : (
        <>
          <Grid container spacing={3}>
            {detections.map((detection) => (
              <Grid item xs={12} sm={6} md={4} lg={3} key={detection.id}>
                <Card 
                  sx={{ 
                    cursor: 'pointer',
                    transition: 'transform 0.2s',
                    '&:hover': {
                      transform: 'translateY(-4px)',
                      boxShadow: 4,
                    }
                  }}
                  onClick={() => handleDetectionClick(detection)}
                >
                  <CardMedia
                    component="img"
                    height="200"
                    image={detection.thumbnail_url || detection.image_url || '/placeholder-wildlife.jpg'}
                    alt={detection.detected_species}
                    sx={{ objectFit: 'cover' }}
                  />
                  
                  <CardContent>
                    <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'start', mb: 1 }}>
                      <Typography variant="h6" noWrap>
                        {detection.detected_species || 'Unknown'}
                      </Typography>
                      
                      {detection.verified && (
                        <Verified color="success" fontSize="small" />
                      )}
                    </Box>
                    
                    <Box sx={{ mb: 1 }}>
                      <Chip
                        label={`${((detection.confidence_score || 0) * 100).toFixed(1)}%`}
                        color={getConfidenceColor(detection.confidence_score)}
                        size="small"
                      />
                    </Box>
                    
                    <Typography variant="body2" color="textSecondary" noWrap>
                      Camera: {cameras.find(c => c.id === detection.camera_id)?.name || 'Unknown'}
                    </Typography>
                    
                    <Typography variant="caption" color="textSecondary">
                      {formatTimestamp(detection.timestamp)}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
            ))}
          </Grid>

          {/* Pagination */}
          {totalPages > 1 && (
            <Box sx={{ display: 'flex', justifyContent: 'center', mt: 4 }}>
              <Pagination
                count={totalPages}
                page={currentPage}
                onChange={(event, page) => setCurrentPage(page)}
                color="primary"
                showFirstButton
                showLastButton
              />
            </Box>
          )}
        </>
      )}

      {/* Detection detail dialog */}
      <Dialog
        open={!!selectedDetection}
        onClose={() => setSelectedDetection(null)}
        maxWidth="md"
        fullWidth
      >
        {selectedDetection && (
          <>
            <DialogTitle>
              <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <Typography variant="h6">
                  Wildlife Detection Details
                </Typography>
                <IconButton onClick={() => setSelectedDetection(null)}>
                  <Close />
                </IconButton>
              </Box>
            </DialogTitle>
            
            <DialogContent dividers>
              <Grid container spacing={2}>
                <Grid item xs={12} md={6}>
                  <img
                    src={selectedDetection.image_url || '/placeholder-wildlife.jpg'}
                    alt={selectedDetection.detected_species}
                    style={{ width: '100%', height: 'auto', borderRadius: 8 }}
                  />
                </Grid>
                
                <Grid item xs={12} md={6}>
                  <List>
                    <ListItem>
                      <ListItemAvatar>
                        <Avatar>
                          <PhotoCamera />
                        </Avatar>
                      </ListItemAvatar>
                      <ListItemText
                        primary="Species"
                        secondary={selectedDetection.detected_species || 'Unknown'}
                      />
                    </ListItem>
                    
                    <ListItem>
                      <ListItemAvatar>
                        <Avatar>
                          <Info />
                        </Avatar>
                      </ListItemAvatar>
                      <ListItemText
                        primary="Confidence"
                        secondary={`${((selectedDetection.confidence_score || 0) * 100).toFixed(1)}%`}
                      />
                    </ListItem>
                    
                    <ListItem>
                      <ListItemAvatar>
                        <Avatar>
                          <PhotoCamera />
                        </Avatar>
                      </ListItemAvatar>
                      <ListItemText
                        primary="Camera"
                        secondary={cameras.find(c => c.id === selectedDetection.camera_id)?.name || 'Unknown'}
                      />
                    </ListItem>
                    
                    <ListItem>
                      <ListItemAvatar>
                        <Avatar>
                          {selectedDetection.verified ? <CheckCircle /> : <Cancel />}
                        </Avatar>
                      </ListItemAvatar>
                      <ListItemText
                        primary="Verification Status"
                        secondary={selectedDetection.verified ? 'Verified' : 'Unverified'}
                      />
                    </ListItem>
                  </List>
                </Grid>
              </Grid>
            </DialogContent>
            
            <DialogActions>
              <Button onClick={() => setSelectedDetection(null)}>
                Close
              </Button>
              {!selectedDetection.verified && (
                <>
                  <Button
                    color="success"
                    onClick={() => handleVerification(selectedDetection.id, selectedDetection.detected_species)}
                  >
                    Verify as {selectedDetection.detected_species}
                  </Button>
                  <Button
                    color="error"
                    onClick={() => handleVerification(selectedDetection.id, 'false_positive')}
                  >
                    Mark as False Positive
                  </Button>
                </>
              )}
            </DialogActions>
          </>
        )}
      </Dialog>
    </Box>
  );
}

export default LiveDetections;