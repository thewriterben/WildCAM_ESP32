/**
 * WildCAM Mobile - Analytics Screen
 * Wildlife detection analytics and statistics visualization
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect, useCallback } from 'react';
import {
  View,
  Text,
  ScrollView,
  RefreshControl,
  StyleSheet,
  TouchableOpacity,
  ActivityIndicator,
  Dimensions,
} from 'react-native';
import { LineChart, BarChart, PieChart } from 'react-native-chart-kit';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const { width } = Dimensions.get('window');
const CHART_WIDTH = width - 32;

const AnalyticsScreen = ({ navigation }) => {
  const [refreshing, setRefreshing] = useState(false);
  const [loading, setLoading] = useState(true);
  const [timeRange, setTimeRange] = useState('7d'); // 24h, 7d, 30d, 90d
  
  // Analytics data
  const [speciesData, setSpeciesData] = useState({});
  const [activityPatterns, setActivityPatterns] = useState({});
  const [detectionTrend, setDetectionTrend] = useState([]);
  const [summary, setSummary] = useState({});

  useEffect(() => {
    loadAnalytics();
  }, [timeRange]);

  const loadAnalytics = async () => {
    setLoading(true);
    try {
      const days = getDaysFromRange(timeRange);
      
      const [speciesResponse, activityResponse] = await Promise.all([
        APIService.getSpeciesAnalytics({ days }),
        APIService.getActivityPatterns({ days }),
      ]);

      setSpeciesData(speciesResponse.species_counts || {});
      setActivityPatterns(activityResponse || {});
      
      // Calculate summary stats
      const totalDetections = Object.values(speciesResponse.species_counts || {}).reduce(
        (sum, count) => sum + count,
        0
      );
      const uniqueSpecies = Object.keys(speciesResponse.species_counts || {}).length;
      const mostActive = Object.entries(speciesResponse.species_counts || {}).sort(
        (a, b) => b[1] - a[1]
      )[0];

      setSummary({
        totalDetections,
        uniqueSpecies,
        mostActiveSpecies: mostActive ? mostActive[0] : 'N/A',
        mostActiveCount: mostActive ? mostActive[1] : 0,
      });

      // Generate trend data
      generateTrendData(days);
    } catch (error) {
      console.error('Failed to load analytics:', error);
    } finally {
      setLoading(false);
    }
  };

  const getDaysFromRange = (range) => {
    switch (range) {
      case '24h': return 1;
      case '7d': return 7;
      case '30d': return 30;
      case '90d': return 90;
      default: return 7;
    }
  };

  const generateTrendData = (days) => {
    // Generate sample trend data (in production, this would come from API)
    const data = [];
    for (let i = days; i >= 0; i--) {
      const date = new Date();
      date.setDate(date.getDate() - i);
      data.push({
        date: date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' }),
        count: Math.floor(Math.random() * 20) + 5,
      });
    }
    setDetectionTrend(data);
  };

  const onRefresh = useCallback(async () => {
    setRefreshing(true);
    await loadAnalytics();
    setRefreshing(false);
  }, [timeRange]);

  const getChartConfig = () => ({
    backgroundColor: '#1e1e1e',
    backgroundGradientFrom: '#1e1e1e',
    backgroundGradientTo: '#1e1e1e',
    decimalPlaces: 0,
    color: (opacity = 1) => `rgba(76, 175, 80, ${opacity})`,
    labelColor: (opacity = 1) => `rgba(255, 255, 255, ${opacity})`,
    style: {
      borderRadius: 16,
    },
    propsForDots: {
      r: '4',
      strokeWidth: '2',
      stroke: '#4CAF50',
    },
  });

  const getSpeciesPieData = () => {
    const colors = ['#4CAF50', '#2196F3', '#FF9800', '#9C27B0', '#F44336', '#00BCD4'];
    return Object.entries(speciesData)
      .slice(0, 6)
      .map(([species, count], index) => ({
        name: species,
        population: count,
        color: colors[index % colors.length],
        legendFontColor: '#fff',
        legendFontSize: 12,
      }));
  };

  const getActivityChartData = () => {
    const hourlyData = activityPatterns.hourly_activity || {};
    const hours = Array.from({ length: 24 }, (_, i) => i);
    
    return {
      labels: hours.filter((_, i) => i % 4 === 0).map(h => `${h}h`),
      datasets: [
        {
          data: hours.map(h => hourlyData[h] || 0),
        },
      ],
    };
  };

  const getTrendChartData = () => {
    const labels = detectionTrend
      .filter((_, i) => i % Math.ceil(detectionTrend.length / 7) === 0)
      .map(d => d.date);
    
    return {
      labels,
      datasets: [
        {
          data: detectionTrend.map(d => d.count),
          color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
          strokeWidth: 2,
        },
      ],
    };
  };

  if (loading) {
    return (
      <View style={styles.loadingContainer}>
        <ActivityIndicator size="large" color="#4CAF50" />
        <Text style={styles.loadingText}>Loading analytics...</Text>
      </View>
    );
  }

  return (
    <ScrollView
      style={styles.container}
      refreshControl={
        <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
      }
    >
      {/* Time Range Selector */}
      <View style={styles.rangeSelector}>
        {['24h', '7d', '30d', '90d'].map((range) => (
          <TouchableOpacity
            key={range}
            style={[
              styles.rangeButton,
              timeRange === range && styles.rangeButtonActive,
            ]}
            onPress={() => setTimeRange(range)}
          >
            <Text
              style={[
                styles.rangeButtonText,
                timeRange === range && styles.rangeButtonTextActive,
              ]}
            >
              {range}
            </Text>
          </TouchableOpacity>
        ))}
      </View>

      {/* Summary Cards */}
      <View style={styles.summaryContainer}>
        <SummaryCard
          icon="photo-camera"
          label="Total Detections"
          value={summary.totalDetections}
          color="#4CAF50"
        />
        <SummaryCard
          icon="pets"
          label="Species Found"
          value={summary.uniqueSpecies}
          color="#2196F3"
        />
      </View>

      <View style={styles.summaryContainer}>
        <View style={[styles.summaryCard, { flex: 1 }]}>
          <Icon name="star" size={24} color="#FF9800" />
          <Text style={styles.summaryLabel}>Most Active Species</Text>
          <Text style={[styles.summaryValue, { color: '#FF9800' }]}>
            {summary.mostActiveSpecies}
          </Text>
          <Text style={styles.summarySubtext}>
            {summary.mostActiveCount} detections
          </Text>
        </View>
      </View>

      {/* Detection Trend Chart */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Detection Trend</Text>
        {detectionTrend.length > 0 && (
          <LineChart
            data={getTrendChartData()}
            width={CHART_WIDTH}
            height={200}
            chartConfig={getChartConfig()}
            bezier
            style={styles.chart}
          />
        )}
      </View>

      {/* Species Distribution */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Species Distribution</Text>
        {Object.keys(speciesData).length > 0 ? (
          <PieChart
            data={getSpeciesPieData()}
            width={CHART_WIDTH}
            height={200}
            chartConfig={getChartConfig()}
            accessor="population"
            backgroundColor="transparent"
            paddingLeft="15"
            style={styles.chart}
          />
        ) : (
          <Text style={styles.noDataText}>No species data available</Text>
        )}
      </View>

      {/* Hourly Activity */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Hourly Activity Pattern</Text>
        {Object.keys(activityPatterns).length > 0 ? (
          <BarChart
            data={getActivityChartData()}
            width={CHART_WIDTH}
            height={200}
            chartConfig={getChartConfig()}
            style={styles.chart}
            showValuesOnTopOfBars
          />
        ) : (
          <Text style={styles.noDataText}>No activity data available</Text>
        )}
      </View>

      {/* Species Breakdown */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Species Breakdown</Text>
        {Object.entries(speciesData)
          .sort((a, b) => b[1] - a[1])
          .map(([species, count]) => (
            <View key={species} style={styles.speciesRow}>
              <View style={styles.speciesInfo}>
                <Icon name="pets" size={20} color="#4CAF50" />
                <Text style={styles.speciesName}>{species}</Text>
              </View>
              <View style={styles.speciesStats}>
                <Text style={styles.speciesCount}>{count}</Text>
                <View style={styles.speciesBar}>
                  <View
                    style={[
                      styles.speciesBarFill,
                      {
                        width: `${(count / summary.totalDetections) * 100}%`,
                      },
                    ]}
                  />
                </View>
              </View>
            </View>
          ))}
      </View>

      {/* Export Options */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Export Data</Text>
        <View style={styles.exportButtons}>
          <TouchableOpacity style={styles.exportButton}>
            <Icon name="file-download" size={20} color="#fff" />
            <Text style={styles.exportButtonText}>CSV</Text>
          </TouchableOpacity>
          <TouchableOpacity style={styles.exportButton}>
            <Icon name="code" size={20} color="#fff" />
            <Text style={styles.exportButtonText}>JSON</Text>
          </TouchableOpacity>
          <TouchableOpacity style={styles.exportButton}>
            <Icon name="picture-as-pdf" size={20} color="#fff" />
            <Text style={styles.exportButtonText}>PDF Report</Text>
          </TouchableOpacity>
        </View>
      </View>

      <View style={styles.bottomPadding} />
    </ScrollView>
  );
};

const SummaryCard = ({ icon, label, value, color }) => (
  <View style={styles.summaryCard}>
    <Icon name={icon} size={24} color={color} />
    <Text style={styles.summaryLabel}>{label}</Text>
    <Text style={[styles.summaryValue, { color }]}>{value}</Text>
  </View>
);

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  loadingContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#121212',
  },
  loadingText: {
    color: '#888',
    marginTop: 12,
  },
  rangeSelector: {
    flexDirection: 'row',
    justifyContent: 'center',
    padding: 16,
    gap: 8,
  },
  rangeButton: {
    paddingHorizontal: 20,
    paddingVertical: 8,
    backgroundColor: '#1e1e1e',
    borderRadius: 20,
    borderWidth: 1,
    borderColor: '#333',
  },
  rangeButtonActive: {
    backgroundColor: '#4CAF50',
    borderColor: '#4CAF50',
  },
  rangeButtonText: {
    color: '#888',
    fontSize: 14,
  },
  rangeButtonTextActive: {
    color: '#fff',
    fontWeight: 'bold',
  },
  summaryContainer: {
    flexDirection: 'row',
    paddingHorizontal: 16,
    marginBottom: 8,
    gap: 8,
  },
  summaryCard: {
    flex: 1,
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
    alignItems: 'center',
  },
  summaryLabel: {
    color: '#888',
    fontSize: 12,
    marginTop: 8,
  },
  summaryValue: {
    fontSize: 24,
    fontWeight: 'bold',
    marginTop: 4,
  },
  summarySubtext: {
    color: '#666',
    fontSize: 11,
    marginTop: 2,
  },
  section: {
    padding: 16,
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 16,
    textTransform: 'uppercase',
    letterSpacing: 1,
  },
  chart: {
    borderRadius: 12,
    marginVertical: 8,
  },
  noDataText: {
    color: '#666',
    textAlign: 'center',
    padding: 32,
  },
  speciesRow: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  speciesInfo: {
    flexDirection: 'row',
    alignItems: 'center',
    flex: 1,
  },
  speciesName: {
    color: '#fff',
    fontSize: 14,
    marginLeft: 12,
  },
  speciesStats: {
    alignItems: 'flex-end',
  },
  speciesCount: {
    color: '#4CAF50',
    fontSize: 16,
    fontWeight: 'bold',
  },
  speciesBar: {
    width: 80,
    height: 4,
    backgroundColor: '#333',
    borderRadius: 2,
    marginTop: 4,
    overflow: 'hidden',
  },
  speciesBarFill: {
    height: '100%',
    backgroundColor: '#4CAF50',
    borderRadius: 2,
  },
  exportButtons: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  exportButton: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    paddingHorizontal: 20,
    paddingVertical: 12,
    borderRadius: 8,
    gap: 8,
  },
  exportButtonText: {
    color: '#fff',
    fontSize: 14,
  },
  bottomPadding: {
    height: 32,
  },
});

export default AnalyticsScreen;
