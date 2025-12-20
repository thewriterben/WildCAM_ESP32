/**
 * Data Analytics Component
 * Advanced wildlife data analytics with activity patterns, species frequency, and time-of-day charts
 */

import React, { useState, useEffect, useMemo } from 'react';
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
  CircularProgress,
  Paper,
  Tabs,
  Tab,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  Chip,
  ToggleButton,
  ToggleButtonGroup,
} from '@mui/material';
import {
  Schedule,
  Pets,
  BarChart as BarChartIcon,
  ShowChart,
  TrendingUp,
  AccessTime,
} from '@mui/icons-material';
import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  RadarChart,
  PolarGrid,
  PolarAngleAxis,
  PolarRadiusAxis,
  Radar,
  Legend,
  AreaChart,
  Area,
  ComposedChart,
  Line,
  Cell,
} from 'recharts';

import apiService from '../services/api';

// Color palette for species
const SPECIES_COLORS = [
  '#2e7d32', '#1976d2', '#d32f2f', '#ed6c02', '#9c27b0',
  '#00838f', '#6d4c41', '#546e7a', '#f50057', '#00c853',
];

// Time period labels
const TIME_PERIODS = {
  dawn: { start: 5, end: 7, label: 'Dawn', color: '#ffd54f' },
  morning: { start: 7, end: 12, label: 'Morning', color: '#81d4fa' },
  afternoon: { start: 12, end: 17, label: 'Afternoon', color: '#fff176' },
  evening: { start: 17, end: 20, label: 'Evening', color: '#ffab91' },
  night: { start: 20, end: 5, label: 'Night', color: '#90a4ae' },
};

function DataAnalytics() {
  const [loading, setLoading] = useState(true);
  const [timeRange, setTimeRange] = useState(30);
  const [selectedTab, setSelectedTab] = useState(0);
  const [selectedSpecies, setSelectedSpecies] = useState('all');
  const [chartType, setChartType] = useState('radar');
  
  const [analyticsData, setAnalyticsData] = useState({
    speciesFrequency: [],
    hourlyActivity: [],
    activityBySpecies: {},
    dailyTrends: [],
    timePeriodSummary: [],
  });

  useEffect(() => {
    loadAnalyticsData();
  }, [timeRange]);

  const loadAnalyticsData = async () => {
    try {
      setLoading(true);
      
      // Fetch comprehensive data analytics from the new endpoint
      const analyticsRes = await apiService.getDataAnalytics(timeRange);

      // Process species frequency data
      const speciesFrequency = (analyticsRes.species_frequency || [])
        .filter(item => item.species)
        .map((item, index) => ({
          species: item.species,
          count: item.count,
          avgConfidence: (item.avg_confidence * 100).toFixed(1),
          color: SPECIES_COLORS[index % SPECIES_COLORS.length],
        }))
        .sort((a, b) => b.count - a.count);

      // Process hourly activity data (ensure 24 hours)
      const hourlyData = Array.from({ length: 24 }, (_, hour) => {
        const activity = (analyticsRes.hourly_activity || []).find(a => a.hour === hour);
        return {
          hour,
          hourLabel: `${hour.toString().padStart(2, '0')}:00`,
          detections: activity?.count || 0,
          period: getTimePeriod(hour),
        };
      });

      // Calculate time period summary
      const timePeriodSummary = calculateTimePeriodSummary(hourlyData);

      // Process daily trends
      const dailyTrends = (analyticsRes.daily_activity || []).map(item => ({
        date: item.date,
        detections: item.count,
      }));

      // Build activity by species from API data
      const activityBySpecies = {};
      const speciesHourlyActivity = analyticsRes.species_hourly_activity || {};
      
      Object.entries(speciesHourlyActivity).forEach(([speciesName, hourlyActivity]) => {
        activityBySpecies[speciesName] = Array.from({ length: 24 }, (_, hour) => {
          const activity = hourlyActivity.find(a => a.hour === hour);
          return {
            hour,
            hourLabel: `${hour.toString().padStart(2, '0')}:00`,
            detections: activity?.count || 0,
            period: getTimePeriod(hour),
          };
        });
      });

      setAnalyticsData({
        speciesFrequency,
        hourlyActivity: hourlyData,
        activityBySpecies,
        dailyTrends,
        timePeriodSummary,
      });

    } catch (error) {
      console.error('Failed to load analytics data:', error);
    } finally {
      setLoading(false);
    }
  };

  const getTimePeriod = (hour) => {
    if (hour >= 5 && hour < 7) return 'dawn';
    if (hour >= 7 && hour < 12) return 'morning';
    if (hour >= 12 && hour < 17) return 'afternoon';
    if (hour >= 17 && hour < 20) return 'evening';
    return 'night';
  };

  const calculateTimePeriodSummary = (hourlyData) => {
    const summary = Object.entries(TIME_PERIODS).map(([key, period]) => {
      let total = 0;
      hourlyData.forEach(h => {
        if (key === 'night') {
          if (h.hour >= 20 || h.hour < 5) total += h.detections;
        } else if (h.hour >= period.start && h.hour < period.end) {
          total += h.detections;
        }
      });
      return {
        period: period.label,
        detections: total,
        color: period.color,
      };
    });
    return summary;
  };

  // Get current activity data based on species selection
  const currentActivityData = useMemo(() => {
    if (selectedSpecies === 'all') {
      return analyticsData.hourlyActivity;
    }
    return analyticsData.activityBySpecies[selectedSpecies] || analyticsData.hourlyActivity;
  }, [selectedSpecies, analyticsData]);

  // Prepare radar chart data (24-hour polar chart)
  const radarData = useMemo(() => {
    return currentActivityData.map(item => ({
      ...item,
      fullMark: Math.max(...currentActivityData.map(d => d.detections)) * 1.2 || 100,
    }));
  }, [currentActivityData]);

  // Calculate activity insights - memoized to avoid redundant calculations
  const activityInsights = useMemo(() => {
    if (!currentActivityData || currentActivityData.length === 0) {
      return {
        peakHour: { hourLabel: 'N/A', detections: 0 },
        quietestHour: { hourLabel: 'N/A', detections: 0 },
        averagePerHour: 0,
      };
    }
    
    const peakHour = currentActivityData.reduce(
      (max, h) => (h.detections > max.detections ? h : max),
      currentActivityData[0]
    );
    
    const quietestHour = currentActivityData.reduce(
      (min, h) => (h.detections < min.detections ? h : min),
      currentActivityData[0]
    );
    
    const totalDetections = currentActivityData.reduce((sum, h) => sum + h.detections, 0);
    const averagePerHour = totalDetections / currentActivityData.length;
    
    return {
      peakHour,
      quietestHour,
      averagePerHour,
    };
  }, [currentActivityData]);

  const CustomTooltip = ({ active, payload, label }) => {
    if (active && payload && payload.length) {
      return (
        <Paper sx={{ p: 1.5, bgcolor: 'background.paper', border: 1, borderColor: 'divider' }}>
          <Typography variant="subtitle2">{label}</Typography>
          {payload.map((entry, index) => (
            <Typography key={index} style={{ color: entry.color }} variant="body2">
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
      <Box sx={{ mb: 3, display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: 2 }}>
        <Typography variant="h4" component="h1">
          Data Analytics
        </Typography>
        
        <Box sx={{ display: 'flex', gap: 2 }}>
          <FormControl variant="outlined" size="small" sx={{ minWidth: 150 }}>
            <InputLabel>Species Filter</InputLabel>
            <Select
              value={selectedSpecies}
              label="Species Filter"
              onChange={(e) => setSelectedSpecies(e.target.value)}
            >
              <MenuItem value="all">All Species</MenuItem>
              {analyticsData.speciesFrequency.map((species) => (
                <MenuItem key={species.species} value={species.species}>
                  {species.species}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          
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
      </Box>

      {/* Tab Navigation */}
      <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 3 }}>
        <Tabs value={selectedTab} onChange={(e, newValue) => setSelectedTab(newValue)}>
          <Tab icon={<Schedule />} label="Activity Patterns" iconPosition="start" />
          <Tab icon={<Pets />} label="Species Frequency" iconPosition="start" />
          <Tab icon={<AccessTime />} label="Time-of-Day Analysis" iconPosition="start" />
        </Tabs>
      </Box>

      {/* Tab Content */}
      {selectedTab === 0 && (
        <Grid container spacing={3}>
          {/* Activity Pattern Polar Chart */}
          <Grid item xs={12} lg={8}>
            <Card>
              <CardContent>
                <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
                  <Typography variant="h6">
                    24-Hour Activity Pattern
                    {selectedSpecies !== 'all' && (
                      <Chip 
                        label={selectedSpecies} 
                        size="small" 
                        sx={{ ml: 1 }}
                        color="primary"
                      />
                    )}
                  </Typography>
                  <ToggleButtonGroup
                    value={chartType}
                    exclusive
                    onChange={(e, newType) => newType && setChartType(newType)}
                    size="small"
                  >
                    <ToggleButton value="radar">
                      <ShowChart fontSize="small" />
                    </ToggleButton>
                    <ToggleButton value="bar">
                      <BarChartIcon fontSize="small" />
                    </ToggleButton>
                  </ToggleButtonGroup>
                </Box>
                
                <ResponsiveContainer width="100%" height={400}>
                  {chartType === 'radar' ? (
                    <RadarChart data={radarData}>
                      <PolarGrid gridType="polygon" />
                      <PolarAngleAxis 
                        dataKey="hourLabel" 
                        tick={{ fontSize: 11 }}
                      />
                      <PolarRadiusAxis 
                        angle={90} 
                        domain={[0, 'dataMax']}
                        tick={{ fontSize: 10 }}
                      />
                      <Radar
                        name="Detections"
                        dataKey="detections"
                        stroke="#2e7d32"
                        fill="#2e7d32"
                        fillOpacity={0.5}
                      />
                      <Legend />
                      <Tooltip content={<CustomTooltip />} />
                    </RadarChart>
                  ) : (
                    <BarChart data={currentActivityData}>
                      <CartesianGrid strokeDasharray="3 3" />
                      <XAxis 
                        dataKey="hourLabel" 
                        tick={{ fontSize: 10 }}
                        interval={1}
                      />
                      <YAxis />
                      <Tooltip content={<CustomTooltip />} />
                      <Bar dataKey="detections" name="Detections">
                        {currentActivityData.map((entry, index) => (
                          <Cell 
                            key={`cell-${index}`} 
                            fill={TIME_PERIODS[entry.period]?.color || '#2e7d32'}
                          />
                        ))}
                      </Bar>
                    </BarChart>
                  )}
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </Grid>

          {/* Time Period Summary */}
          <Grid item xs={12} lg={4}>
            <Card sx={{ height: '100%' }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Activity by Time Period
                </Typography>
                <Box sx={{ mt: 2 }}>
                  {analyticsData.timePeriodSummary.map((period) => (
                    <Box 
                      key={period.period}
                      sx={{ 
                        display: 'flex', 
                        justifyContent: 'space-between', 
                        alignItems: 'center',
                        p: 1.5,
                        mb: 1,
                        borderRadius: 1,
                        bgcolor: period.color,
                        color: 'black',
                      }}
                    >
                      <Typography variant="body1" fontWeight="medium">
                        {period.period}
                      </Typography>
                      <Chip 
                        label={`${period.detections} detections`}
                        size="small"
                        sx={{ bgcolor: 'rgba(255,255,255,0.8)' }}
                      />
                    </Box>
                  ))}
                </Box>
                
                <Box sx={{ mt: 3 }}>
                  <Typography variant="subtitle2" color="textSecondary" gutterBottom>
                    Peak Activity Periods
                  </Typography>
                  <Typography variant="body2">
                    Based on {timeRange}-day data analysis, wildlife is most active during:
                  </Typography>
                  <Box sx={{ mt: 1 }}>
                    {analyticsData.timePeriodSummary
                      .sort((a, b) => b.detections - a.detections)
                      .slice(0, 2)
                      .map((period, index) => (
                        <Chip
                          key={period.period}
                          label={period.period}
                          size="small"
                          sx={{ mr: 1, mb: 1 }}
                          color={index === 0 ? 'primary' : 'default'}
                        />
                      ))
                    }
                  </Box>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {selectedTab === 1 && (
        <Grid container spacing={3}>
          {/* Species Frequency Bar Chart */}
          <Grid item xs={12} lg={8}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Species Frequency Distribution ({timeRange} Days)
                </Typography>
                <ResponsiveContainer width="100%" height={400}>
                  <BarChart 
                    data={analyticsData.speciesFrequency}
                    layout="vertical"
                    margin={{ top: 5, right: 30, left: 100, bottom: 5 }}
                  >
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis type="number" />
                    <YAxis 
                      type="category" 
                      dataKey="species" 
                      tick={{ fontSize: 12 }}
                      width={90}
                    />
                    <Tooltip content={<CustomTooltip />} />
                    <Bar dataKey="count" name="Detections">
                      {analyticsData.speciesFrequency.map((entry, index) => (
                        <Cell key={`cell-${index}`} fill={entry.color} />
                      ))}
                    </Bar>
                  </BarChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </Grid>

          {/* Species Statistics Table */}
          <Grid item xs={12} lg={4}>
            <Card sx={{ height: '100%' }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Species Statistics
                </Typography>
                <TableContainer>
                  <Table size="small">
                    <TableHead>
                      <TableRow>
                        <TableCell>Species</TableCell>
                        <TableCell align="right">Count</TableCell>
                        <TableCell align="right">Confidence</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {analyticsData.speciesFrequency.slice(0, 10).map((species, index) => (
                        <TableRow key={species.species}>
                          <TableCell>
                            <Box sx={{ display: 'flex', alignItems: 'center' }}>
                              <Box
                                sx={{
                                  width: 12,
                                  height: 12,
                                  borderRadius: '50%',
                                  bgcolor: species.color,
                                  mr: 1,
                                }}
                              />
                              {species.species}
                            </Box>
                          </TableCell>
                          <TableCell align="right">{species.count}</TableCell>
                          <TableCell align="right">{species.avgConfidence}%</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
                
                <Box sx={{ mt: 2, pt: 2, borderTop: 1, borderColor: 'divider' }}>
                  <Typography variant="body2" color="textSecondary">
                    Total Species: {analyticsData.speciesFrequency.length}
                  </Typography>
                  <Typography variant="body2" color="textSecondary">
                    Total Detections: {analyticsData.speciesFrequency.reduce((sum, s) => sum + s.count, 0)}
                  </Typography>
                </Box>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}

      {selectedTab === 2 && (
        <Grid container spacing={3}>
          {/* Time-of-Day Heatmap Style Chart */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Time-of-Day Activity Analysis
                </Typography>
                <ResponsiveContainer width="100%" height={350}>
                  <ComposedChart data={currentActivityData}>
                    <CartesianGrid strokeDasharray="3 3" />
                    <XAxis 
                      dataKey="hourLabel"
                      tick={{ fontSize: 11 }}
                    />
                    <YAxis />
                    <Tooltip content={<CustomTooltip />} />
                    <Legend />
                    <Area
                      type="monotone"
                      dataKey="detections"
                      name="Activity Level"
                      fill="#8884d8"
                      stroke="#8884d8"
                      fillOpacity={0.3}
                    />
                    <Line
                      type="monotone"
                      dataKey="detections"
                      name="Detections"
                      stroke="#2e7d32"
                      strokeWidth={2}
                      dot={{ r: 4 }}
                    />
                  </ComposedChart>
                </ResponsiveContainer>
              </CardContent>
            </Card>
          </Grid>

          {/* Hour-by-Hour Breakdown */}
          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Hourly Breakdown (00:00 - 11:00)
                </Typography>
                <TableContainer>
                  <Table size="small">
                    <TableHead>
                      <TableRow>
                        <TableCell>Hour</TableCell>
                        <TableCell align="center">Period</TableCell>
                        <TableCell align="right">Detections</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {currentActivityData.slice(0, 12).map((hour) => (
                        <TableRow key={hour.hour}>
                          <TableCell>{hour.hourLabel}</TableCell>
                          <TableCell align="center">
                            <Chip
                              label={TIME_PERIODS[hour.period]?.label || hour.period}
                              size="small"
                              sx={{ 
                                bgcolor: TIME_PERIODS[hour.period]?.color,
                                color: 'black',
                                fontSize: '0.7rem',
                              }}
                            />
                          </TableCell>
                          <TableCell align="right">{hour.detections}</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
              </CardContent>
            </Card>
          </Grid>

          <Grid item xs={12} md={6}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Hourly Breakdown (12:00 - 23:00)
                </Typography>
                <TableContainer>
                  <Table size="small">
                    <TableHead>
                      <TableRow>
                        <TableCell>Hour</TableCell>
                        <TableCell align="center">Period</TableCell>
                        <TableCell align="right">Detections</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {currentActivityData.slice(12, 24).map((hour) => (
                        <TableRow key={hour.hour}>
                          <TableCell>{hour.hourLabel}</TableCell>
                          <TableCell align="center">
                            <Chip
                              label={TIME_PERIODS[hour.period]?.label || hour.period}
                              size="small"
                              sx={{ 
                                bgcolor: TIME_PERIODS[hour.period]?.color,
                                color: 'black',
                                fontSize: '0.7rem',
                              }}
                            />
                          </TableCell>
                          <TableCell align="right">{hour.detections}</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </TableContainer>
              </CardContent>
            </Card>
          </Grid>

          {/* Activity Insights */}
          <Grid item xs={12}>
            <Card>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Activity Insights
                </Typography>
                <Grid container spacing={2}>
                  <Grid item xs={12} sm={4}>
                    <Paper sx={{ p: 2, bgcolor: 'background.default' }}>
                      <Typography variant="subtitle2" color="textSecondary">
                        Peak Hour
                      </Typography>
                      <Typography variant="h4">
                        {activityInsights.peakHour?.hourLabel || 'N/A'}
                      </Typography>
                      <Typography variant="body2" color="textSecondary">
                        {activityInsights.peakHour?.detections || 0} detections
                      </Typography>
                    </Paper>
                  </Grid>
                  <Grid item xs={12} sm={4}>
                    <Paper sx={{ p: 2, bgcolor: 'background.default' }}>
                      <Typography variant="subtitle2" color="textSecondary">
                        Quietest Hour
                      </Typography>
                      <Typography variant="h4">
                        {activityInsights.quietestHour?.hourLabel || 'N/A'}
                      </Typography>
                      <Typography variant="body2" color="textSecondary">
                        {activityInsights.quietestHour?.detections || 0} detections
                      </Typography>
                    </Paper>
                  </Grid>
                  <Grid item xs={12} sm={4}>
                    <Paper sx={{ p: 2, bgcolor: 'background.default' }}>
                      <Typography variant="subtitle2" color="textSecondary">
                        Average per Hour
                      </Typography>
                      <Typography variant="h4">
                        {activityInsights.averagePerHour.toFixed(1)}
                      </Typography>
                      <Typography variant="body2" color="textSecondary">
                        detections/hour
                      </Typography>
                    </Paper>
                  </Grid>
                </Grid>
              </CardContent>
            </Card>
          </Grid>
        </Grid>
      )}
    </Box>
  );
}

export default DataAnalytics;
