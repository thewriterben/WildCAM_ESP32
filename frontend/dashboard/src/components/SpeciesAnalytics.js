/**
 * Species Analytics Component
 * Advanced analytics and insights for wildlife monitoring
 */

import React, { useState, useEffect } from 'react';
import {
  Grid,
  Card,
  CardContent,
  Typography,
  Box,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Button,
  List,
  ListItem,
  ListItemAvatar,
  ListItemText,
  Avatar,
  Chip,
  Paper,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  CircularProgress,
} from '@mui/material';
import {
  Timeline,
  TrendingUp,
  TrendingDown,
  Assessment,
  DateRange,
  PhotoCamera,
  Schedule,
} from '@mui/icons-material';
import {
  PieChart,
  Pie,
  Cell,
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  LineChart,
  Line,
  Area,
  AreaChart,
  RadarChart,
  PolarGrid,
  PolarAngleAxis,
  PolarRadiusAxis,
  Radar,
} from 'recharts';

import apiService from '../services/api';

const COLORS = ['#0088FE', '#00C49F', '#FFBB28', '#FF8042', '#8884d8', '#82ca9d', '#ffc658', '#ff7300'];

function SpeciesAnalytics() {
  const [loading, setLoading] = useState(true);
  const [timeRange, setTimeRange] = useState(30); // days
  const [analyticsData, setAnalyticsData] = useState({
    speciesDistribution: [],
    activityPatterns: [],
    trends: [],
    topSpecies: [],
    camerasData: [],
  });
  const [detectionStats, setDetectionStats] = useState({
    totalDetections: 0,
    uniqueSpecies: 0,
    averageConfidence: 0,
    verificationRate: 0,
  });

  useEffect(() => {
    loadAnalytics();
  }, [timeRange]);

  const loadAnalytics = async () => {
    try {
      setLoading(true);
      
      // Load analytics data
      const [speciesRes, activityRes, detectionsRes] = await Promise.all([
        apiService.getSpeciesAnalytics(timeRange),
        apiService.getActivityPatterns(timeRange),
        apiService.getDetections({ per_page: 1000 }),
      ]);

      // Process species distribution
      const speciesDistribution = speciesRes.species_data.map(item => ({
        name: item.species,
        value: item.count,
        confidence: item.avg_confidence,
      }));

      // Process activity patterns for 24-hour view
      const hourlyActivity = Array.from({ length: 24 }, (_, hour) => {
        const activity = activityRes.hourly_activity.find(a => a.hour === hour);
        return {
          hour: `${hour}:00`,
          detections: activity ? activity.count : 0,
        };
      });

      // Calculate statistics
      const totalDetections = detectionsRes.total || 0;
      const uniqueSpecies = new Set(detectionsRes.detections?.map(d => d.detected_species)).size;
      const avgConfidence = detectionsRes.detections?.reduce((sum, d) => sum + (d.confidence_score || 0), 0) / 
                           (detectionsRes.detections?.length || 1);
      const verifiedCount = detectionsRes.detections?.filter(d => d.verified).length || 0;

      // Generate trend data (simulated for demo)
      const trendData = Array.from({ length: timeRange }, (_, i) => {
        const date = new Date();
        date.setDate(date.getDate() - (timeRange - i - 1));
        return {
          date: date.toISOString().split('T')[0],
          detections: Math.floor(Math.random() * 20) + 5,
          species: Math.floor(Math.random() * 8) + 2,
        };
      });

      // Top species with additional metrics
      const topSpecies = speciesDistribution
        .sort((a, b) => b.value - a.value)
        .slice(0, 10)
        .map((species, index) => ({
          ...species,
          rank: index + 1,
          trend: Math.random() > 0.5 ? 'up' : 'down',
          change: (Math.random() * 30 - 15).toFixed(1), // % change
        }));

      setAnalyticsData({
        speciesDistribution,
        activityPatterns: hourlyActivity,
        trends: trendData,
        topSpecies,
      });

      setDetectionStats({
        totalDetections,
        uniqueSpecies,
        averageConfidence: avgConfidence * 100,
        verificationRate: (verifiedCount / totalDetections) * 100,
      });

    } catch (error) {
      console.error('Failed to load analytics:', error);
    } finally {
      setLoading(false);
    }
  };

  const CustomTooltip = ({ active, payload, label }) => {
    if (active && payload && payload.length) {
      return (
        <Paper sx={{ p: 2, bgcolor: 'background.paper', border: 1, borderColor: 'divider' }}>
          <Typography variant="subtitle2">{label}</Typography>
          {payload.map((entry, index) => (
            <Typography key={index} style={{ color: entry.color }}>
              {entry.name}: {entry.value}
            </Typography>
          ))}
        </Paper>
      );
    }
    return null;
  };

  if (loading) {
    return (
      <Box display="flex" justifyContent="center" alignItems="center" minHeight="400px">
        <CircularProgress size={60} />
      </Box>
    );
  }

  return (
    <Box sx={{ p: 2 }}>
      {/* Header */}
      <Box sx={{ mb: 3, display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <Typography variant="h4" component="h1">
          Species Analytics & Insights
        </Typography>
        
        <FormControl variant="outlined" size="small" sx={{ minWidth: 120 }}>
          <InputLabel>Time Range</InputLabel>
          <Select
            value={timeRange}
            label="Time Range"
            onChange={(e) => setTimeRange(e.target.value)}
          >
            <MenuItem value={7}>7 Days</MenuItem>
            <MenuItem value={30}>30 Days</MenuItem>
            <MenuItem value={90}>90 Days</MenuItem>
            <MenuItem value={365}>1 Year</MenuItem>
          </Select>
        </FormControl>
      </Box>

      {/* Summary Statistics */}
      <Grid container spacing={3} sx={{ mb: 3 }}>
        <Grid item xs={12} sm={6} md={3}>
          <Card>
            <CardContent>
              <Box display="flex" alignItems="center">
                <Assessment color="primary" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Total Detections
                  </Typography>
                  <Typography variant="h4">
                    {detectionStats.totalDetections.toLocaleString()}
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
                <PhotoCamera color="secondary" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Unique Species
                  </Typography>
                  <Typography variant="h4">
                    {detectionStats.uniqueSpecies}
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
                <TrendingUp color="success" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Avg Confidence
                  </Typography>
                  <Typography variant="h4">
                    {detectionStats.averageConfidence.toFixed(1)}%
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
                <Timeline color="info" sx={{ mr: 2, fontSize: 40 }} />
                <Box>
                  <Typography color="textSecondary" gutterBottom>
                    Verification Rate
                  </Typography>
                  <Typography variant="h4">
                    {detectionStats.verificationRate.toFixed(1)}%
                  </Typography>
                </Box>
              </Box>
            </CardContent>
          </Card>
        </Grid>
      </Grid>

      <Grid container spacing={3}>
        {/* Species Distribution */}
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Species Distribution ({timeRange} days)
              </Typography>
              {analyticsData.speciesDistribution.length > 0 ? (
                <ResponsiveContainer width="100%" height={300}>
                  <PieChart>
                    <Pie
                      data={analyticsData.speciesDistribution}
                      cx="50%"
                      cy="50%"
                      labelLine={false}
                      label={({ name, percent }) => `${name} ${(percent * 100).toFixed(0)}%`}
                      outerRadius={100}
                      fill="#8884d8"
                      dataKey="value"
                    >
                      {analyticsData.speciesDistribution.map((entry, index) => (
                        <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
                      ))}
                    </Pie>
                    <Tooltip content={<CustomTooltip />} />
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
                24-Hour Activity Pattern
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <AreaChart data={analyticsData.activityPatterns}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="hour" />
                  <YAxis />
                  <Tooltip content={<CustomTooltip />} />
                  <Area 
                    type="monotone" 
                    dataKey="detections" 
                    stroke="#8884d8" 
                    fill="#8884d8" 
                    fillOpacity={0.6}
                  />
                </AreaChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Detection Trends */}
        <Grid item xs={12}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Detection Trends Over Time
              </Typography>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={analyticsData.trends}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="date" />
                  <YAxis />
                  <Tooltip content={<CustomTooltip />} />
                  <Line 
                    type="monotone" 
                    dataKey="detections" 
                    stroke="#8884d8" 
                    strokeWidth={2}
                    name="Daily Detections"
                  />
                  <Line 
                    type="monotone" 
                    dataKey="species" 
                    stroke="#82ca9d" 
                    strokeWidth={2}
                    name="Species Count"
                  />
                </LineChart>
              </ResponsiveContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Top Species List */}
        <Grid item xs={12} md={8}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Top Detected Species
              </Typography>
              <TableContainer>
                <Table size="small">
                  <TableHead>
                    <TableRow>
                      <TableCell>Rank</TableCell>
                      <TableCell>Species</TableCell>
                      <TableCell align="right">Detections</TableCell>
                      <TableCell align="right">Avg Confidence</TableCell>
                      <TableCell align="right">Trend</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    {analyticsData.topSpecies.map((species) => (
                      <TableRow key={species.name}>
                        <TableCell>{species.rank}</TableCell>
                        <TableCell>
                          <Box display="flex" alignItems="center">
                            <Avatar sx={{ width: 32, height: 32, mr: 1, bgcolor: COLORS[species.rank % COLORS.length] }}>
                              {species.name.charAt(0)}
                            </Avatar>
                            {species.name}
                          </Box>
                        </TableCell>
                        <TableCell align="right">{species.value}</TableCell>
                        <TableCell align="right">{(species.confidence * 100).toFixed(1)}%</TableCell>
                        <TableCell align="right">
                          <Box display="flex" alignItems="center" justifyContent="flex-end">
                            {species.trend === 'up' ? (
                              <TrendingUp color="success" fontSize="small" />
                            ) : (
                              <TrendingDown color="error" fontSize="small" />
                            )}
                            <Typography 
                              variant="body2" 
                              color={species.trend === 'up' ? 'success.main' : 'error.main'}
                              sx={{ ml: 0.5 }}
                            >
                              {species.change}%
                            </Typography>
                          </Box>
                        </TableCell>
                      </TableRow>
                    ))}
                  </TableBody>
                </Table>
              </TableContainer>
            </CardContent>
          </Card>
        </Grid>

        {/* Quick Insights */}
        <Grid item xs={12} md={4}>
          <Card>
            <CardContent>
              <Typography variant="h6" gutterBottom>
                Key Insights
              </Typography>
              <List>
                <ListItem>
                  <ListItemAvatar>
                    <Avatar sx={{ bgcolor: 'primary.main' }}>
                      <Schedule />
                    </Avatar>
                  </ListItemAvatar>
                  <ListItemText
                    primary="Peak Activity"
                    secondary="Most wildlife activity occurs during dawn (5-7 AM) and dusk (6-8 PM) hours"
                  />
                </ListItem>
                
                <ListItem>
                  <ListItemAvatar>
                    <Avatar sx={{ bgcolor: 'secondary.main' }}>
                      <TrendingUp />
                    </Avatar>
                  </ListItemAvatar>
                  <ListItemText
                    primary="Species Diversity"
                    secondary={`${detectionStats.uniqueSpecies} different species detected with ${detectionStats.averageConfidence.toFixed(0)}% average confidence`}
                  />
                </ListItem>
                
                <ListItem>
                  <ListItemAvatar>
                    <Avatar sx={{ bgcolor: 'success.main' }}>
                      <Assessment />
                    </Avatar>
                  </ListItemAvatar>
                  <ListItemText
                    primary="Verification Progress"
                    secondary={`${detectionStats.verificationRate.toFixed(0)}% of detections have been manually verified`}
                  />
                </ListItem>
              </List>
            </CardContent>
          </Card>
        </Grid>
      </Grid>
    </Box>
  );
}

export default SpeciesAnalytics;