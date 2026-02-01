/**
 * WildCAM Mobile - Species Detail Screen
 * Detailed view of wildlife detection with species information
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  ScrollView,
  StyleSheet,
  TouchableOpacity,
  Image,
  Alert,
  ActivityIndicator,
  Dimensions,
  Share,
} from 'react-native';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const { width } = Dimensions.get('window');

const SpeciesDetailScreen = ({ route, navigation }) => {
  const { detectionId } = route.params || {};
  
  const [loading, setLoading] = useState(true);
  const [detection, setDetection] = useState(null);
  const [isVerifying, setIsVerifying] = useState(false);

  useEffect(() => {
    if (detectionId) {
      loadDetection();
    }
  }, [detectionId]);

  const loadDetection = async () => {
    setLoading(true);
    try {
      const response = await APIService.getDetection(detectionId);
      setDetection(response);
    } catch (error) {
      console.error('Failed to load detection:', error);
      Alert.alert('Error', 'Failed to load detection details');
    } finally {
      setLoading(false);
    }
  };

  const verifyDetection = async (isCorrect, correctedSpecies = null) => {
    setIsVerifying(true);
    try {
      await APIService.verifyDetection(detectionId, {
        verified: true,
        is_correct: isCorrect,
        corrected_species: correctedSpecies,
        verified_at: new Date().toISOString(),
      });
      
      setDetection(prev => ({
        ...prev,
        verified: true,
        is_correct: isCorrect,
        corrected_species: correctedSpecies,
      }));
      
      Alert.alert('Success', 'Detection verified successfully');
    } catch (error) {
      console.error('Failed to verify detection:', error);
      Alert.alert('Error', 'Failed to verify detection');
    } finally {
      setIsVerifying(false);
    }
  };

  const shareDetection = async () => {
    try {
      const message = `Wildlife Detection: ${detection.species}\n` +
        `Confidence: ${Math.round(detection.confidence * 100)}%\n` +
        `Location: ${detection.latitude?.toFixed(4)}, ${detection.longitude?.toFixed(4)}\n` +
        `Date: ${new Date(detection.timestamp).toLocaleString()}\n\n` +
        `Detected by WildCAM ESP32`;

      await Share.share({
        message,
        title: `Wildlife Detection: ${detection.species}`,
      });
    } catch (error) {
      console.error('Failed to share:', error);
    }
  };

  const getConfidenceColor = (confidence) => {
    if (confidence >= 0.9) return '#4CAF50';
    if (confidence >= 0.7) return '#8BC34A';
    if (confidence >= 0.5) return '#FF9800';
    return '#F44336';
  };

  if (loading) {
    return (
      <View style={styles.loadingContainer}>
        <ActivityIndicator size="large" color="#4CAF50" />
        <Text style={styles.loadingText}>Loading detection...</Text>
      </View>
    );
  }

  if (!detection) {
    return (
      <View style={styles.errorContainer}>
        <Icon name="error-outline" size={64} color="#F44336" />
        <Text style={styles.errorText}>Detection not found</Text>
        <TouchableOpacity
          style={styles.retryButton}
          onPress={() => navigation.goBack()}
        >
          <Text style={styles.retryButtonText}>Go Back</Text>
        </TouchableOpacity>
      </View>
    );
  }

  const confidence = detection.confidence || 0;
  const confidenceColor = getConfidenceColor(confidence);

  return (
    <ScrollView style={styles.container}>
      {/* Detection Image */}
      <View style={styles.imageContainer}>
        {detection.image_url ? (
          <Image
            source={{ uri: detection.image_url }}
            style={styles.detectionImage}
            resizeMode="contain"
          />
        ) : (
          <View style={[styles.detectionImage, styles.placeholderImage]}>
            <Icon name="image" size={64} color="#666" />
            <Text style={styles.placeholderText}>No image available</Text>
          </View>
        )}
        
        {/* Verification Badge */}
        {detection.verified && (
          <View style={styles.verifiedBadge}>
            <Icon name="check-circle" size={16} color="#fff" />
            <Text style={styles.verifiedText}>Verified</Text>
          </View>
        )}
      </View>

      {/* Species Information */}
      <View style={styles.section}>
        <View style={styles.speciesHeader}>
          <View style={styles.speciesInfo}>
            <Text style={styles.speciesName}>
              {detection.species || 'Unknown Species'}
            </Text>
            {detection.scientific_name && (
              <Text style={styles.scientificName}>
                {detection.scientific_name}
              </Text>
            )}
          </View>
          <View style={styles.confidenceContainer}>
            <Text style={[styles.confidenceValue, { color: confidenceColor }]}>
              {Math.round(confidence * 100)}%
            </Text>
            <Text style={styles.confidenceLabel}>Confidence</Text>
          </View>
        </View>

        {/* Confidence Bar */}
        <View style={styles.confidenceBar}>
          <View
            style={[
              styles.confidenceFill,
              {
                width: `${confidence * 100}%`,
                backgroundColor: confidenceColor,
              },
            ]}
          />
        </View>
      </View>

      {/* Detection Details */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Detection Details</Text>
        
        <DetailRow
          icon="schedule"
          label="Date & Time"
          value={new Date(detection.timestamp).toLocaleString()}
        />
        <DetailRow
          icon="videocam"
          label="Camera"
          value={detection.device_name || detection.device_id || 'Unknown'}
        />
        {(detection.latitude && detection.longitude) && (
          <DetailRow
            icon="location-on"
            label="Location"
            value={`${detection.latitude.toFixed(4)}, ${detection.longitude.toFixed(4)}`}
          />
        )}
        {detection.temperature && (
          <DetailRow
            icon="thermostat"
            label="Temperature"
            value={`${detection.temperature}°C`}
          />
        )}
        {detection.humidity && (
          <DetailRow
            icon="water-drop"
            label="Humidity"
            value={`${detection.humidity}%`}
          />
        )}
      </View>

      {/* AI Analysis */}
      {detection.analysis && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>AI Analysis</Text>
          
          {detection.analysis.bounding_box && (
            <DetailRow
              icon="crop-free"
              label="Bounding Box"
              value={`${detection.analysis.bounding_box.width}x${detection.analysis.bounding_box.height}`}
            />
          )}
          {detection.analysis.processing_time && (
            <DetailRow
              icon="timer"
              label="Processing Time"
              value={`${detection.analysis.processing_time}ms`}
            />
          )}
          {detection.analysis.model_version && (
            <DetailRow
              icon="memory"
              label="Model"
              value={detection.analysis.model_version}
            />
          )}

          {/* Alternative Classifications */}
          {detection.analysis.alternatives && detection.analysis.alternatives.length > 0 && (
            <View style={styles.alternativesSection}>
              <Text style={styles.alternativesTitle}>Other Possibilities</Text>
              {detection.analysis.alternatives.map((alt, index) => (
                <View key={index} style={styles.alternativeItem}>
                  <Text style={styles.alternativeSpecies}>{alt.species}</Text>
                  <Text style={styles.alternativeConfidence}>
                    {Math.round(alt.confidence * 100)}%
                  </Text>
                </View>
              ))}
            </View>
          )}
        </View>
      )}

      {/* Species Facts (if available) */}
      {detection.species_info && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>About {detection.species}</Text>
          <Text style={styles.speciesDescription}>
            {detection.species_info.description}
          </Text>
          
          {detection.species_info.conservation_status && (
            <View style={styles.conservationBadge}>
              <Icon name="eco" size={16} color="#fff" />
              <Text style={styles.conservationText}>
                Conservation Status: {detection.species_info.conservation_status}
              </Text>
            </View>
          )}
        </View>
      )}

      {/* Verification Section */}
      {!detection.verified && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Verify Detection</Text>
          <Text style={styles.verifyDescription}>
            Help improve our AI by verifying this detection
          </Text>
          
          <View style={styles.verifyButtons}>
            <TouchableOpacity
              style={[styles.verifyButton, styles.verifyCorrect]}
              onPress={() => verifyDetection(true)}
              disabled={isVerifying}
            >
              {isVerifying ? (
                <ActivityIndicator size="small" color="#fff" />
              ) : (
                <>
                  <Icon name="check" size={24} color="#fff" />
                  <Text style={styles.verifyButtonText}>Correct</Text>
                </>
              )}
            </TouchableOpacity>
            
            <TouchableOpacity
              style={[styles.verifyButton, styles.verifyIncorrect]}
              onPress={() => {
                Alert.prompt(
                  'Correct Species',
                  'What is the correct species?',
                  (correctedSpecies) => {
                    if (correctedSpecies) {
                      verifyDetection(false, correctedSpecies);
                    }
                  }
                );
              }}
              disabled={isVerifying}
            >
              <Icon name="close" size={24} color="#fff" />
              <Text style={styles.verifyButtonText}>Incorrect</Text>
            </TouchableOpacity>
          </View>
        </View>
      )}

      {/* Actions */}
      <View style={styles.section}>
        <TouchableOpacity style={styles.actionButton} onPress={shareDetection}>
          <Icon name="share" size={24} color="#fff" />
          <Text style={styles.actionButtonText}>Share Detection</Text>
        </TouchableOpacity>
      </View>

      <View style={styles.bottomPadding} />
    </ScrollView>
  );
};

const DetailRow = ({ icon, label, value }) => (
  <View style={styles.detailRow}>
    <Icon name={icon} size={20} color="#4CAF50" style={styles.detailIcon} />
    <Text style={styles.detailLabel}>{label}</Text>
    <Text style={styles.detailValue}>{value}</Text>
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
  errorContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#121212',
    padding: 32,
  },
  errorText: {
    color: '#fff',
    fontSize: 18,
    marginTop: 16,
  },
  retryButton: {
    marginTop: 24,
    backgroundColor: '#4CAF50',
    paddingHorizontal: 24,
    paddingVertical: 12,
    borderRadius: 8,
  },
  retryButtonText: {
    color: '#fff',
    fontWeight: 'bold',
  },
  imageContainer: {
    position: 'relative',
  },
  detectionImage: {
    width: width,
    height: width * 0.75,
    backgroundColor: '#1e1e1e',
  },
  placeholderImage: {
    justifyContent: 'center',
    alignItems: 'center',
  },
  placeholderText: {
    color: '#666',
    marginTop: 12,
  },
  verifiedBadge: {
    position: 'absolute',
    top: 16,
    right: 16,
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#4CAF50',
    borderRadius: 16,
    paddingHorizontal: 12,
    paddingVertical: 6,
  },
  verifiedText: {
    color: '#fff',
    fontSize: 12,
    fontWeight: 'bold',
    marginLeft: 4,
  },
  section: {
    padding: 16,
    borderTopWidth: 1,
    borderTopColor: '#333',
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 16,
    textTransform: 'uppercase',
    letterSpacing: 1,
  },
  speciesHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'flex-start',
  },
  speciesInfo: {
    flex: 1,
  },
  speciesName: {
    color: '#fff',
    fontSize: 24,
    fontWeight: 'bold',
  },
  scientificName: {
    color: '#888',
    fontSize: 14,
    fontStyle: 'italic',
    marginTop: 4,
  },
  confidenceContainer: {
    alignItems: 'center',
    marginLeft: 16,
  },
  confidenceValue: {
    fontSize: 32,
    fontWeight: 'bold',
  },
  confidenceLabel: {
    color: '#888',
    fontSize: 12,
    marginTop: 4,
  },
  confidenceBar: {
    height: 8,
    backgroundColor: '#333',
    borderRadius: 4,
    marginTop: 16,
    overflow: 'hidden',
  },
  confidenceFill: {
    height: '100%',
    borderRadius: 4,
  },
  detailRow: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  detailIcon: {
    marginRight: 12,
  },
  detailLabel: {
    flex: 1,
    color: '#888',
    fontSize: 14,
  },
  detailValue: {
    color: '#fff',
    fontSize: 14,
  },
  alternativesSection: {
    marginTop: 16,
    padding: 12,
    backgroundColor: '#1e1e1e',
    borderRadius: 8,
  },
  alternativesTitle: {
    color: '#888',
    fontSize: 12,
    marginBottom: 8,
    textTransform: 'uppercase',
  },
  alternativeItem: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    paddingVertical: 8,
  },
  alternativeSpecies: {
    color: '#fff',
    fontSize: 14,
  },
  alternativeConfidence: {
    color: '#888',
    fontSize: 14,
  },
  speciesDescription: {
    color: '#ccc',
    fontSize: 14,
    lineHeight: 22,
  },
  conservationBadge: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#FF9800',
    borderRadius: 8,
    padding: 12,
    marginTop: 16,
  },
  conservationText: {
    color: '#fff',
    fontSize: 14,
    marginLeft: 8,
    fontWeight: '500',
  },
  verifyDescription: {
    color: '#888',
    fontSize: 14,
    marginBottom: 16,
  },
  verifyButtons: {
    flexDirection: 'row',
    gap: 12,
  },
  verifyButton: {
    flex: 1,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 12,
    padding: 16,
  },
  verifyCorrect: {
    backgroundColor: '#4CAF50',
  },
  verifyIncorrect: {
    backgroundColor: '#F44336',
  },
  verifyButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: 'bold',
    marginLeft: 8,
  },
  actionButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#2196F3',
    borderRadius: 12,
    padding: 16,
  },
  actionButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: 'bold',
    marginLeft: 8,
  },
  bottomPadding: {
    height: 32,
  },
});

export default SpeciesDetailScreen;
