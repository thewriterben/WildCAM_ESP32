/**
 * WildCAM Mobile - Detections Screen
 * Wildlife detection gallery and management
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect, useCallback } from 'react';
import {
  View,
  Text,
  FlatList,
  RefreshControl,
  StyleSheet,
  TouchableOpacity,
  Image,
  ActivityIndicator,
  Dimensions,
  Modal,
} from 'react-native';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const { width } = Dimensions.get('window');
const COLUMN_COUNT = 2;
const ITEM_WIDTH = (width - 48) / COLUMN_COUNT;

const DetectionsScreen = ({ navigation }) => {
  const [refreshing, setRefreshing] = useState(false);
  const [detections, setDetections] = useState([]);
  const [loading, setLoading] = useState(true);
  const [page, setPage] = useState(1);
  const [hasMore, setHasMore] = useState(true);
  const [filter, setFilter] = useState('all'); // all, verified, unverified
  const [speciesFilter, setSpeciesFilter] = useState(null);
  const [showFilterModal, setShowFilterModal] = useState(false);
  const [species, setSpecies] = useState([]);

  useEffect(() => {
    loadDetections();
    loadSpecies();
  }, []);

  useEffect(() => {
    setPage(1);
    setDetections([]);
    loadDetections(1);
  }, [filter, speciesFilter]);

  const loadDetections = async (pageNum = page) => {
    if (pageNum === 1) setLoading(true);
    
    try {
      const params = {
        limit: 20,
        page: pageNum,
      };

      if (filter === 'verified') {
        params.verified = true;
      } else if (filter === 'unverified') {
        params.verified = false;
      }

      if (speciesFilter) {
        params.species = speciesFilter;
      }

      const response = await APIService.getDetections(params);
      const newDetections = response.data || [];

      if (pageNum === 1) {
        setDetections(newDetections);
      } else {
        setDetections((prev) => [...prev, ...newDetections]);
      }

      setHasMore(newDetections.length === 20);
      setPage(pageNum);
    } catch (error) {
      console.error('Failed to load detections:', error);
    } finally {
      setLoading(false);
    }
  };

  const loadSpecies = async () => {
    try {
      const response = await APIService.getSpeciesAnalytics({ days: 30 });
      const speciesList = Object.keys(response.species_counts || {});
      setSpecies(speciesList);
    } catch (error) {
      console.error('Failed to load species:', error);
    }
  };

  const onRefresh = useCallback(async () => {
    setRefreshing(true);
    setPage(1);
    await loadDetections(1);
    setRefreshing(false);
  }, [filter, speciesFilter]);

  const loadMore = () => {
    if (!loading && hasMore) {
      loadDetections(page + 1);
    }
  };

  const renderDetectionItem = ({ item, index }) => (
    <TouchableOpacity
      style={styles.detectionItem}
      onPress={() => navigation.navigate('SpeciesDetail', { detectionId: item.id })}
    >
      {item.image_url ? (
        <Image source={{ uri: item.image_url }} style={styles.detectionImage} />
      ) : (
        <View style={[styles.detectionImage, styles.placeholderImage]}>
          <Icon name="image" size={40} color="#666" />
        </View>
      )}
      <View style={styles.detectionOverlay}>
        <Text style={styles.detectionSpecies} numberOfLines={1}>
          {item.species || 'Unknown'}
        </Text>
        <View style={styles.detectionMeta}>
          <Text style={styles.detectionConfidence}>
            {Math.round((item.confidence || 0) * 100)}%
          </Text>
          {item.verified && (
            <Icon name="check-circle" size={14} color="#4CAF50" />
          )}
        </View>
      </View>
    </TouchableOpacity>
  );

  const renderHeader = () => (
    <View style={styles.header}>
      {/* Filter Chips */}
      <View style={styles.filterRow}>
        <TouchableOpacity
          style={[styles.filterChip, filter === 'all' && styles.filterChipActive]}
          onPress={() => setFilter('all')}
        >
          <Text style={[styles.filterText, filter === 'all' && styles.filterTextActive]}>
            All
          </Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={[styles.filterChip, filter === 'verified' && styles.filterChipActive]}
          onPress={() => setFilter('verified')}
        >
          <Icon name="check-circle" size={16} color={filter === 'verified' ? '#fff' : '#888'} />
          <Text style={[styles.filterText, filter === 'verified' && styles.filterTextActive]}>
            Verified
          </Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={[styles.filterChip, filter === 'unverified' && styles.filterChipActive]}
          onPress={() => setFilter('unverified')}
        >
          <Icon name="help" size={16} color={filter === 'unverified' ? '#fff' : '#888'} />
          <Text style={[styles.filterText, filter === 'unverified' && styles.filterTextActive]}>
            Unverified
          </Text>
        </TouchableOpacity>
      </View>

      {/* Species Filter */}
      <TouchableOpacity
        style={styles.speciesFilterButton}
        onPress={() => setShowFilterModal(true)}
      >
        <Icon name="filter-list" size={20} color="#4CAF50" />
        <Text style={styles.speciesFilterText}>
          {speciesFilter || 'All Species'}
        </Text>
        <Icon name="arrow-drop-down" size={20} color="#888" />
      </TouchableOpacity>

      {/* Stats */}
      <Text style={styles.resultsText}>
        {detections.length} detections found
      </Text>
    </View>
  );

  const renderFooter = () => {
    if (!hasMore) return null;
    return (
      <View style={styles.footer}>
        <ActivityIndicator size="small" color="#4CAF50" />
      </View>
    );
  };

  const renderEmpty = () => (
    <View style={styles.emptyContainer}>
      <Icon name="photo-camera" size={64} color="#666" />
      <Text style={styles.emptyTitle}>No Detections Found</Text>
      <Text style={styles.emptySubtitle}>
        Try adjusting your filters or wait for new wildlife detections
      </Text>
    </View>
  );

  return (
    <View style={styles.container}>
      {loading && page === 1 ? (
        <View style={styles.loadingContainer}>
          <ActivityIndicator size="large" color="#4CAF50" />
          <Text style={styles.loadingText}>Loading detections...</Text>
        </View>
      ) : (
        <FlatList
          data={detections}
          keyExtractor={(item) => item.id.toString()}
          renderItem={renderDetectionItem}
          numColumns={COLUMN_COUNT}
          contentContainerStyle={styles.list}
          ListHeaderComponent={renderHeader}
          ListFooterComponent={renderFooter}
          ListEmptyComponent={renderEmpty}
          refreshControl={
            <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
          }
          onEndReached={loadMore}
          onEndReachedThreshold={0.5}
        />
      )}

      {/* Species Filter Modal */}
      <Modal
        visible={showFilterModal}
        transparent
        animationType="slide"
        onRequestClose={() => setShowFilterModal(false)}
      >
        <View style={styles.modalContainer}>
          <View style={styles.modalContent}>
            <View style={styles.modalHeader}>
              <Text style={styles.modalTitle}>Filter by Species</Text>
              <TouchableOpacity onPress={() => setShowFilterModal(false)}>
                <Icon name="close" size={24} color="#fff" />
              </TouchableOpacity>
            </View>
            <TouchableOpacity
              style={[
                styles.speciesOption,
                !speciesFilter && styles.speciesOptionActive,
              ]}
              onPress={() => {
                setSpeciesFilter(null);
                setShowFilterModal(false);
              }}
            >
              <Text
                style={[
                  styles.speciesOptionText,
                  !speciesFilter && styles.speciesOptionTextActive,
                ]}
              >
                All Species
              </Text>
            </TouchableOpacity>
            {species.map((s) => (
              <TouchableOpacity
                key={s}
                style={[
                  styles.speciesOption,
                  speciesFilter === s && styles.speciesOptionActive,
                ]}
                onPress={() => {
                  setSpeciesFilter(s);
                  setShowFilterModal(false);
                }}
              >
                <Text
                  style={[
                    styles.speciesOptionText,
                    speciesFilter === s && styles.speciesOptionTextActive,
                  ]}
                >
                  {s}
                </Text>
              </TouchableOpacity>
            ))}
          </View>
        </View>
      </Modal>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  loadingContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  loadingText: {
    color: '#888',
    marginTop: 12,
  },
  list: {
    padding: 16,
  },
  header: {
    marginBottom: 16,
  },
  filterRow: {
    flexDirection: 'row',
    marginBottom: 12,
  },
  filterChip: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 20,
    paddingHorizontal: 16,
    paddingVertical: 8,
    marginRight: 8,
    borderWidth: 1,
    borderColor: '#333',
  },
  filterChipActive: {
    backgroundColor: '#4CAF50',
    borderColor: '#4CAF50',
  },
  filterText: {
    color: '#888',
    fontSize: 14,
    marginLeft: 4,
  },
  filterTextActive: {
    color: '#fff',
    fontWeight: 'bold',
  },
  speciesFilterButton: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 8,
    padding: 12,
    marginBottom: 12,
  },
  speciesFilterText: {
    flex: 1,
    color: '#fff',
    fontSize: 14,
    marginLeft: 8,
  },
  resultsText: {
    color: '#888',
    fontSize: 12,
  },
  detectionItem: {
    width: ITEM_WIDTH,
    margin: 4,
    borderRadius: 12,
    overflow: 'hidden',
    backgroundColor: '#1e1e1e',
  },
  detectionImage: {
    width: '100%',
    height: ITEM_WIDTH,
  },
  placeholderImage: {
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#333',
  },
  detectionOverlay: {
    padding: 8,
  },
  detectionSpecies: {
    color: '#fff',
    fontSize: 14,
    fontWeight: '500',
  },
  detectionMeta: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    marginTop: 4,
  },
  detectionConfidence: {
    color: '#888',
    fontSize: 12,
  },
  footer: {
    padding: 16,
    alignItems: 'center',
  },
  emptyContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 32,
    marginTop: 64,
  },
  emptyTitle: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
    marginTop: 16,
  },
  emptySubtitle: {
    color: '#888',
    fontSize: 14,
    textAlign: 'center',
    marginTop: 8,
  },
  modalContainer: {
    flex: 1,
    backgroundColor: 'rgba(0, 0, 0, 0.8)',
    justifyContent: 'flex-end',
  },
  modalContent: {
    backgroundColor: '#1e1e1e',
    borderTopLeftRadius: 20,
    borderTopRightRadius: 20,
    padding: 20,
    maxHeight: '70%',
  },
  modalHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 16,
  },
  modalTitle: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
  },
  speciesOption: {
    padding: 16,
    borderRadius: 8,
    marginBottom: 8,
    backgroundColor: '#333',
  },
  speciesOptionActive: {
    backgroundColor: '#4CAF50',
  },
  speciesOptionText: {
    color: '#fff',
    fontSize: 16,
  },
  speciesOptionTextActive: {
    fontWeight: 'bold',
  },
});

export default DetectionsScreen;
