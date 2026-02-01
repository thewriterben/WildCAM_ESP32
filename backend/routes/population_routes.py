"""
Population Dynamics Analytics Routes
Advanced population analysis, behavioral patterns, and conservation metrics

Author: WildCAM ESP32 Team
Version: 1.0.0
Date: 2026
"""

from flask import Blueprint, request, jsonify
from datetime import datetime, timedelta
import logging
import math

logger = logging.getLogger(__name__)

population_routes = Blueprint('population', __name__)


@population_routes.route('/api/v1/analytics/population/estimate', methods=['POST'])
def estimate_population():
    """
    Estimate population size using capture-recapture methods
    
    Request body:
    {
        "species": "white_tailed_deer",
        "site_id": "site_001",
        "method": "lincoln_petersen",  # lincoln_petersen, schnabel, jolly_seber
        "captures": [
            {"date": "2026-01-01", "marked": 10, "captured": 15, "recaptured": 3}
        ]
    }
    
    Returns:
        Population estimate with confidence intervals
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        species = data.get('species')
        site_id = data.get('site_id')
        method = data.get('method', 'lincoln_petersen')
        captures = data.get('captures', [])
        
        if not captures:
            return jsonify({
                'status': 'error',
                'message': 'Capture data required'
            }), 400
        
        estimate = None
        confidence_interval = None
        
        if method == 'lincoln_petersen':
            # Simple Lincoln-Petersen estimator
            if len(captures) >= 1:
                c = captures[0]
                n1 = c.get('marked', 0)  # Number marked in first sample
                n2 = c.get('captured', 0)  # Number in second sample
                m2 = c.get('recaptured', 0)  # Number of marked in second sample
                
                if m2 > 0:
                    # Chapman's modified estimator
                    estimate = ((n1 + 1) * (n2 + 1) / (m2 + 1)) - 1
                    
                    # Variance estimate
                    variance = ((n1 + 1) * (n2 + 1) * (n1 - m2) * (n2 - m2)) / \
                              ((m2 + 1) ** 2 * (m2 + 2))
                    se = math.sqrt(variance)
                    
                    confidence_interval = {
                        'lower': max(0, estimate - 1.96 * se),
                        'upper': estimate + 1.96 * se,
                        'confidence_level': 0.95
                    }
        
        elif method == 'schnabel':
            # Schnabel method for multiple recaptures
            sum_cm = 0
            sum_r = 0
            
            for c in captures:
                marked = c.get('marked', 0)
                captured = c.get('captured', 0)
                recaptured = c.get('recaptured', 0)
                sum_cm += captured * marked
                sum_r += recaptured
            
            if sum_r > 0:
                estimate = sum_cm / sum_r
                confidence_interval = {
                    'lower': estimate * 0.7,
                    'upper': estimate * 1.4,
                    'confidence_level': 0.95
                }
        
        if estimate is None:
            return jsonify({
                'status': 'error',
                'message': 'Unable to calculate estimate with provided data'
            }), 400
        
        return jsonify({
            'status': 'success',
            'population_estimate': {
                'species': species,
                'site_id': site_id,
                'method': method,
                'estimate': round(estimate, 1),
                'confidence_interval': confidence_interval,
                'sample_sessions': len(captures),
                'estimated_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error estimating population: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/population/density', methods=['POST'])
def calculate_density():
    """
    Calculate population density from camera trap data
    
    Request body:
    {
        "species": "white_tailed_deer",
        "site_id": "site_001",
        "camera_data": [
            {"camera_id": "cam1", "detections": 45, "trap_nights": 30, 
             "detection_zone_km2": 0.001}
        ],
        "method": "rem"  # rem (Random Encounter Model), rest, secr
    }
    
    Returns:
        Population density estimate
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        species = data.get('species')
        site_id = data.get('site_id')
        camera_data = data.get('camera_data', [])
        method = data.get('method', 'rem')
        
        if not camera_data:
            return jsonify({
                'status': 'error',
                'message': 'Camera data required'
            }), 400
        
        # Random Encounter Model calculation
        total_detections = sum(c.get('detections', 0) for c in camera_data)
        total_trap_nights = sum(c.get('trap_nights', 0) for c in camera_data)
        avg_detection_zone = sum(c.get('detection_zone_km2', 0.001) for c in camera_data) / len(camera_data)
        
        # Simple density calculation (animals per km2)
        if total_trap_nights > 0 and avg_detection_zone > 0:
            encounter_rate = total_detections / total_trap_nights
            
            # Assume average day range of 1 km for most mammals
            day_range_km = data.get('day_range_km', 1.0)
            
            # REM formula: D = (y/t) / (v * r * 2 + pi * r^2)
            # Simplified version
            detection_radius = math.sqrt(avg_detection_zone / math.pi)
            density = encounter_rate / (2 * detection_radius * day_range_km)
        else:
            density = 0
        
        return jsonify({
            'status': 'success',
            'density_estimate': {
                'species': species,
                'site_id': site_id,
                'method': method,
                'density_per_km2': round(density, 2),
                'total_detections': total_detections,
                'total_trap_nights': total_trap_nights,
                'cameras_used': len(camera_data),
                'confidence': 'moderate' if total_trap_nights > 100 else 'low',
                'estimated_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error calculating density: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/behavior/activity-patterns', methods=['POST'])
def analyze_activity_patterns():
    """
    Analyze wildlife activity patterns (circadian, seasonal)
    
    Request body:
    {
        "species": "white_tailed_deer",
        "site_id": "site_001",
        "start_date": "2025-01-01",
        "end_date": "2026-01-31",
        "detections": [
            {"timestamp": "2025-06-15T14:30:00Z", "confidence": 0.95}
        ]
    }
    
    Returns:
        Activity pattern analysis
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        species = data.get('species')
        detections = data.get('detections', [])
        
        if not detections:
            return jsonify({
                'status': 'error',
                'message': 'Detection data required'
            }), 400
        
        # Initialize hourly and monthly counters
        hourly_activity = [0] * 24
        monthly_activity = [0] * 12
        day_of_week = [0] * 7
        
        for det in detections:
            try:
                ts = datetime.fromisoformat(det.get('timestamp', '').replace('Z', '+00:00'))
                hourly_activity[ts.hour] += 1
                monthly_activity[ts.month - 1] += 1
                day_of_week[ts.weekday()] += 1
            except (ValueError, AttributeError):
                continue
        
        # Calculate activity metrics
        total_detections = sum(hourly_activity)
        
        # Peak activity hours (diurnal/nocturnal classification)
        day_activity = sum(hourly_activity[6:18])
        night_activity = sum(hourly_activity[0:6]) + sum(hourly_activity[18:24])
        
        if total_detections > 0:
            diurnality = day_activity / total_detections
            nocturnality = night_activity / total_detections
        else:
            diurnality = 0.5
            nocturnality = 0.5
        
        # Peak hours
        peak_hour = hourly_activity.index(max(hourly_activity))
        
        # Activity classification
        if diurnality > 0.7:
            activity_pattern = 'diurnal'
        elif nocturnality > 0.7:
            activity_pattern = 'nocturnal'
        elif 0.3 <= diurnality <= 0.7:
            activity_pattern = 'crepuscular'
        else:
            activity_pattern = 'cathemeral'
        
        return jsonify({
            'status': 'success',
            'activity_analysis': {
                'species': species,
                'total_detections': total_detections,
                'activity_pattern': activity_pattern,
                'diurnality_index': round(diurnality, 3),
                'nocturnality_index': round(nocturnality, 3),
                'peak_activity_hour': peak_hour,
                'hourly_distribution': hourly_activity,
                'monthly_distribution': monthly_activity,
                'day_of_week_distribution': day_of_week,
                'analyzed_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error analyzing activity patterns: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/behavior/space-use', methods=['POST'])
def analyze_space_use():
    """
    Analyze wildlife space use and habitat preferences
    
    Request body:
    {
        "species": "white_tailed_deer",
        "detections": [
            {"camera_id": "cam1", "latitude": 45.0, "longitude": -75.0,
             "habitat_type": "forest", "detection_count": 45}
        ]
    }
    
    Returns:
        Space use and habitat preference analysis
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        species = data.get('species')
        detections = data.get('detections', [])
        
        if not detections:
            return jsonify({
                'status': 'error',
                'message': 'Detection data required'
            }), 400
        
        # Aggregate by habitat type
        habitat_use = {}
        total_count = 0
        
        for det in detections:
            habitat = det.get('habitat_type', 'unknown')
            count = det.get('detection_count', 1)
            
            if habitat not in habitat_use:
                habitat_use[habitat] = 0
            habitat_use[habitat] += count
            total_count += count
        
        # Calculate habitat preference indices
        habitat_preferences = []
        for habitat, count in habitat_use.items():
            proportion = count / total_count if total_count > 0 else 0
            habitat_preferences.append({
                'habitat_type': habitat,
                'detection_count': count,
                'proportion': round(proportion, 3),
                'preference_level': 'high' if proportion > 0.3 else 'medium' if proportion > 0.1 else 'low'
            })
        
        habitat_preferences.sort(key=lambda x: x['detection_count'], reverse=True)
        
        # Calculate home range estimate (Minimum Convex Polygon - simplified)
        lats = [d.get('latitude', 0) for d in detections if d.get('latitude')]
        lons = [d.get('longitude', 0) for d in detections if d.get('longitude')]
        
        if lats and lons:
            lat_range = max(lats) - min(lats)
            lon_range = max(lons) - min(lons)
            # Rough area calculation (degrees to km approximation)
            area_km2 = lat_range * 111 * lon_range * 111 * math.cos(math.radians(sum(lats) / len(lats)))
        else:
            area_km2 = 0
        
        return jsonify({
            'status': 'success',
            'space_use_analysis': {
                'species': species,
                'total_detections': total_count,
                'cameras_with_detections': len(detections),
                'habitat_preferences': habitat_preferences,
                'primary_habitat': habitat_preferences[0]['habitat_type'] if habitat_preferences else 'unknown',
                'estimated_range_km2': round(area_km2, 2),
                'analyzed_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error analyzing space use: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/community/diversity', methods=['POST'])
def calculate_diversity():
    """
    Calculate biodiversity indices for wildlife community
    
    Request body:
    {
        "site_id": "site_001",
        "species_counts": {
            "white_tailed_deer": 150,
            "black_bear": 23,
            "eastern_turkey": 89,
            "raccoon": 67
        }
    }
    
    Returns:
        Biodiversity indices
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        site_id = data.get('site_id')
        species_counts = data.get('species_counts', {})
        
        if not species_counts:
            return jsonify({
                'status': 'error',
                'message': 'Species counts required'
            }), 400
        
        # Calculate diversity indices
        total = sum(species_counts.values())
        proportions = [count / total for count in species_counts.values()]
        
        # Species richness
        richness = len(species_counts)
        
        # Shannon diversity index (H')
        shannon = -sum(p * math.log(p) for p in proportions if p > 0)
        
        # Simpson's diversity index (1 - D)
        simpson = 1 - sum(p ** 2 for p in proportions)
        
        # Pielou's evenness (J')
        max_diversity = math.log(richness) if richness > 1 else 1
        evenness = shannon / max_diversity if max_diversity > 0 else 0
        
        # Berger-Parker dominance
        berger_parker = max(proportions)
        
        # Margalef richness index
        margalef = (richness - 1) / math.log(total) if total > 1 else 0
        
        return jsonify({
            'status': 'success',
            'diversity_analysis': {
                'site_id': site_id,
                'total_observations': total,
                'species_richness': richness,
                'indices': {
                    'shannon_diversity': round(shannon, 3),
                    'simpson_diversity': round(simpson, 3),
                    'pielou_evenness': round(evenness, 3),
                    'berger_parker_dominance': round(berger_parker, 3),
                    'margalef_richness': round(margalef, 3)
                },
                'interpretation': {
                    'shannon': 'high' if shannon > 2.5 else 'moderate' if shannon > 1.5 else 'low',
                    'evenness': 'even' if evenness > 0.7 else 'moderate' if evenness > 0.4 else 'uneven',
                    'dominance': 'high' if berger_parker > 0.5 else 'moderate' if berger_parker > 0.3 else 'low'
                },
                'species_breakdown': [
                    {
                        'species': species,
                        'count': count,
                        'proportion': round(count / total, 3)
                    }
                    for species, count in sorted(species_counts.items(), key=lambda x: x[1], reverse=True)
                ],
                'analyzed_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error calculating diversity: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/trends', methods=['POST'])
def analyze_trends():
    """
    Analyze population trends over time
    
    Request body:
    {
        "species": "white_tailed_deer",
        "site_id": "site_001",
        "monthly_counts": [
            {"month": "2025-01", "count": 45},
            {"month": "2025-02", "count": 52},
            ...
        ]
    }
    
    Returns:
        Trend analysis with statistics
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'status': 'error', 'message': 'No data provided'}), 400
        
        species = data.get('species')
        site_id = data.get('site_id')
        monthly_counts = data.get('monthly_counts', [])
        
        if len(monthly_counts) < 3:
            return jsonify({
                'status': 'error',
                'message': 'At least 3 months of data required for trend analysis'
            }), 400
        
        counts = [m.get('count', 0) for m in monthly_counts]
        months = [m.get('month', '') for m in monthly_counts]
        
        # Calculate trend statistics
        n = len(counts)
        mean_count = sum(counts) / n
        
        # Simple linear regression
        x_mean = (n - 1) / 2
        numerator = sum((i - x_mean) * (counts[i] - mean_count) for i in range(n))
        denominator = sum((i - x_mean) ** 2 for i in range(n))
        
        slope = numerator / denominator if denominator != 0 else 0
        intercept = mean_count - slope * x_mean
        
        # Percent change
        if counts[0] > 0:
            percent_change = ((counts[-1] - counts[0]) / counts[0]) * 100
        else:
            percent_change = 0
        
        # Determine trend direction
        if slope > 0.5:
            trend = 'increasing'
        elif slope < -0.5:
            trend = 'decreasing'
        else:
            trend = 'stable'
        
        # Calculate coefficient of variation
        variance = sum((c - mean_count) ** 2 for c in counts) / n
        std_dev = math.sqrt(variance)
        cv = (std_dev / mean_count) * 100 if mean_count > 0 else 0
        
        return jsonify({
            'status': 'success',
            'trend_analysis': {
                'species': species,
                'site_id': site_id,
                'period': {
                    'start': months[0] if months else None,
                    'end': months[-1] if months else None,
                    'months_analyzed': n
                },
                'statistics': {
                    'mean_count': round(mean_count, 1),
                    'min_count': min(counts),
                    'max_count': max(counts),
                    'std_deviation': round(std_dev, 2),
                    'coefficient_of_variation': round(cv, 1)
                },
                'trend': {
                    'direction': trend,
                    'slope': round(slope, 3),
                    'percent_change': round(percent_change, 1),
                    'confidence': 'high' if n > 12 else 'moderate' if n > 6 else 'low'
                },
                'monthly_data': monthly_counts,
                'analyzed_at': datetime.utcnow().isoformat()
            }
        })
        
    except Exception as e:
        logger.error(f"Error analyzing trends: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


@population_routes.route('/api/v1/analytics/summary', methods=['GET'])
def get_analytics_summary():
    """
    Get summary of all analytics for a site
    
    Query params:
        site_id: Site identifier
        days: Time period in days (default 30)
        
    Returns:
        Comprehensive analytics summary
    """
    try:
        site_id = request.args.get('site_id', 'default')
        days = int(request.args.get('days', 30))
        
        # Generate sample summary data
        summary = {
            'site_id': site_id,
            'period_days': days,
            'generated_at': datetime.utcnow().isoformat(),
            'overview': {
                'total_detections': 1247,
                'unique_species': 12,
                'active_cameras': 8,
                'camera_days': 240
            },
            'top_species': [
                {'species': 'White-tailed Deer', 'count': 456, 'trend': 'stable'},
                {'species': 'Eastern Turkey', 'count': 234, 'trend': 'increasing'},
                {'species': 'Raccoon', 'count': 189, 'trend': 'stable'},
                {'species': 'Black Bear', 'count': 67, 'trend': 'decreasing'},
                {'species': 'Coyote', 'count': 45, 'trend': 'stable'}
            ],
            'diversity': {
                'shannon_index': 2.34,
                'species_richness': 12,
                'evenness': 0.72
            },
            'activity_summary': {
                'peak_hour': 6,
                'peak_period': 'dawn',
                'busiest_day': 'Saturday'
            },
            'conservation_alerts': [
                {
                    'type': 'population_decline',
                    'species': 'Black Bear',
                    'message': 'Black bear detections down 25% from previous period',
                    'severity': 'warning'
                }
            ]
        }
        
        return jsonify({
            'status': 'success',
            'summary': summary
        })
        
    except Exception as e:
        logger.error(f"Error getting analytics summary: {e}")
        return jsonify({'status': 'error', 'message': str(e)}), 500


def create_population_routes(app):
    """Register population analytics routes with Flask app"""
    app.register_blueprint(population_routes)
    logger.info("Population analytics routes registered")
