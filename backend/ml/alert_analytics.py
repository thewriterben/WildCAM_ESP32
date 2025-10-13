"""
Alert Analytics and Learning System
Tracks alert performance, user feedback, and continuous improvement
"""

import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple
from collections import defaultdict, deque
import logging

logger = logging.getLogger(__name__)


class AlertAnalytics:
    """
    Alert performance tracking and continuous learning
    """
    
    def __init__(self):
        # Performance metrics
        self.total_alerts = 0
        self.true_positives = 0
        self.false_positives = 0
        self.user_feedback_count = 0
        
        # Historical data for trends
        self.alert_history = deque(maxlen=10000)
        self.feedback_history = deque(maxlen=5000)
        
        # Species-specific metrics
        self.species_metrics = defaultdict(lambda: {
            'alerts': 0,
            'true_positives': 0,
            'false_positives': 0,
            'avg_confidence': 0.0,
            'user_feedbacks': 0
        })
        
        # Time-based patterns
        self.hourly_patterns = defaultdict(int)
        self.daily_patterns = defaultdict(int)
        
        # Alert response times
        self.response_times = deque(maxlen=1000)
        
    def track_alert(
        self,
        alert_data: Dict,
        processing_time_ms: float,
        delivery_status: Dict
    ):
        """
        Track new alert for analytics
        
        Args:
            alert_data: Alert information
            processing_time_ms: Processing time in milliseconds
            delivery_status: Notification delivery results
        """
        self.total_alerts += 1
        
        # Store in history
        self.alert_history.append({
            'timestamp': datetime.utcnow(),
            'species': alert_data.get('species'),
            'confidence': alert_data.get('confidence'),
            'alert_level': alert_data.get('alert_level'),
            'camera_id': alert_data.get('camera_id'),
            'processing_time_ms': processing_time_ms,
            'delivered': any(v.get('success', False) for v in delivery_status.values()),
            'channels_used': list(delivery_status.keys())
        })
        
        # Update species metrics
        species = alert_data.get('species', 'unknown')
        metrics = self.species_metrics[species]
        metrics['alerts'] += 1
        
        # Update average confidence (running average)
        confidence = alert_data.get('confidence', 0)
        metrics['avg_confidence'] = (
            (metrics['avg_confidence'] * (metrics['alerts'] - 1) + confidence) /
            metrics['alerts']
        )
        
        # Update temporal patterns
        now = datetime.utcnow()
        self.hourly_patterns[now.hour] += 1
        self.daily_patterns[now.strftime('%A')] += 1
        
        # Track response time
        self.response_times.append(processing_time_ms)
    
    def record_user_feedback(
        self,
        alert_id: str,
        is_accurate: bool,
        species_correction: Optional[str] = None,
        notes: Optional[str] = None
    ):
        """
        Record user feedback on alert accuracy
        
        Args:
            alert_id: Alert identifier
            is_accurate: Whether alert was accurate
            species_correction: Corrected species if misidentified
            notes: Additional feedback notes
        """
        self.user_feedback_count += 1
        
        feedback = {
            'timestamp': datetime.utcnow(),
            'alert_id': alert_id,
            'is_accurate': is_accurate,
            'species_correction': species_correction,
            'notes': notes
        }
        
        self.feedback_history.append(feedback)
        
        # Update true/false positive counts
        if is_accurate:
            self.true_positives += 1
        else:
            self.false_positives += 1
        
        # Find original alert in history
        for alert in reversed(self.alert_history):
            if alert.get('alert_id') == alert_id:
                species = alert.get('species', 'unknown')
                metrics = self.species_metrics[species]
                
                if is_accurate:
                    metrics['true_positives'] += 1
                else:
                    metrics['false_positives'] += 1
                
                metrics['user_feedbacks'] += 1
                break
    
    def get_accuracy_metrics(self, days: int = 7) -> Dict:
        """
        Calculate accuracy metrics for specified time period
        
        Args:
            days: Number of days to analyze
            
        Returns:
            Dictionary with accuracy metrics
        """
        cutoff = datetime.utcnow() - timedelta(days=days)
        
        # Filter recent feedback
        recent_feedback = [
            f for f in self.feedback_history
            if f['timestamp'] > cutoff
        ]
        
        if not recent_feedback:
            return {
                'accuracy': 0.0,
                'precision': 0.0,
                'false_positive_rate': 0.0,
                'feedback_count': 0,
                'period_days': days
            }
        
        accurate_count = sum(1 for f in recent_feedback if f['is_accurate'])
        total_feedback = len(recent_feedback)
        
        # Calculate metrics
        accuracy = accurate_count / total_feedback if total_feedback > 0 else 0.0
        
        # False positive rate
        false_positive_count = total_feedback - accurate_count
        fp_rate = false_positive_count / total_feedback if total_feedback > 0 else 0.0
        
        return {
            'accuracy': round(accuracy, 3),
            'precision': round(accuracy, 3),  # Simplified precision
            'false_positive_rate': round(fp_rate, 3),
            'true_positives': accurate_count,
            'false_positives': false_positive_count,
            'feedback_count': total_feedback,
            'period_days': days
        }
    
    def get_species_performance(self, species: Optional[str] = None) -> Dict:
        """
        Get performance metrics by species
        
        Args:
            species: Specific species to query, or None for all
            
        Returns:
            Species performance metrics
        """
        if species:
            metrics = self.species_metrics.get(species, {})
            if not metrics:
                return {'error': f'No data for species: {species}'}
            
            total = metrics['true_positives'] + metrics['false_positives']
            accuracy = (metrics['true_positives'] / total) if total > 0 else 0.0
            
            return {
                'species': species,
                'total_alerts': metrics['alerts'],
                'accuracy': round(accuracy, 3),
                'avg_confidence': round(metrics['avg_confidence'], 3),
                'true_positives': metrics['true_positives'],
                'false_positives': metrics['false_positives'],
                'user_feedbacks': metrics['user_feedbacks']
            }
        
        # Return all species
        results = []
        for sp, metrics in self.species_metrics.items():
            total = metrics['true_positives'] + metrics['false_positives']
            accuracy = (metrics['true_positives'] / total) if total > 0 else 0.0
            
            results.append({
                'species': sp,
                'total_alerts': metrics['alerts'],
                'accuracy': round(accuracy, 3),
                'avg_confidence': round(metrics['avg_confidence'], 3),
                'true_positives': metrics['true_positives'],
                'false_positives': metrics['false_positives']
            })
        
        # Sort by alert count
        results.sort(key=lambda x: x['total_alerts'], reverse=True)
        return {'species_performance': results}
    
    def get_temporal_patterns(self) -> Dict:
        """
        Get temporal activity patterns
        
        Returns:
            Hourly and daily activity patterns
        """
        # Get total alerts for percentage calculation
        total_alerts = sum(self.hourly_patterns.values())
        
        hourly_percentages = {
            hour: (count / total_alerts * 100) if total_alerts > 0 else 0
            for hour, count in self.hourly_patterns.items()
        }
        
        return {
            'hourly_patterns': dict(self.hourly_patterns),
            'hourly_percentages': {
                hour: round(pct, 1) for hour, pct in hourly_percentages.items()
            },
            'daily_patterns': dict(self.daily_patterns),
            'peak_hour': max(self.hourly_patterns.items(), key=lambda x: x[1])[0]
                if self.hourly_patterns else None
        }
    
    def get_performance_metrics(self) -> Dict:
        """
        Get overall system performance metrics
        
        Returns:
            System performance statistics
        """
        # Calculate average response time
        avg_response_time = (
            sum(self.response_times) / len(self.response_times)
            if self.response_times else 0
        )
        
        # Calculate max response time
        max_response_time = max(self.response_times) if self.response_times else 0
        
        # Calculate overall accuracy from feedback
        total_feedback = self.true_positives + self.false_positives
        overall_accuracy = (
            self.true_positives / total_feedback
            if total_feedback > 0 else 0.0
        )
        
        # Calculate false positive rate
        fp_rate = (
            self.false_positives / total_feedback
            if total_feedback > 0 else 0.0
        )
        
        # Estimate uptime (simplified - based on recent alert frequency)
        uptime_estimate = 99.9  # Placeholder - would need actual monitoring
        
        return {
            'total_alerts': self.total_alerts,
            'true_positives': self.true_positives,
            'false_positives': self.false_positives,
            'overall_accuracy': round(overall_accuracy, 3),
            'false_positive_rate': round(fp_rate, 3),
            'false_positive_reduction': round((1 - fp_rate) * 100, 1),
            'avg_processing_time_ms': round(avg_response_time, 2),
            'max_processing_time_ms': round(max_response_time, 2),
            'uptime_percentage': uptime_estimate,
            'user_feedback_count': self.user_feedback_count,
            'feedback_rate': round(
                (self.user_feedback_count / self.total_alerts * 100)
                if self.total_alerts > 0 else 0, 1
            )
        }
    
    def detect_anomalies(self, window_hours: int = 24) -> List[Dict]:
        """
        Detect anomalous alert patterns
        
        Args:
            window_hours: Time window for analysis
            
        Returns:
            List of detected anomalies
        """
        cutoff = datetime.utcnow() - timedelta(hours=window_hours)
        recent_alerts = [
            alert for alert in self.alert_history
            if alert['timestamp'] > cutoff
        ]
        
        if len(recent_alerts) < 10:
            return []
        
        anomalies = []
        
        # Check for unusual spike in alerts
        alerts_per_hour = len(recent_alerts) / window_hours
        historical_avg = self.total_alerts / max(1, (
            (datetime.utcnow() - self.alert_history[0]['timestamp']).total_seconds() / 3600
        )) if self.alert_history else 0
        
        if alerts_per_hour > historical_avg * 3:
            anomalies.append({
                'type': 'alert_spike',
                'severity': 'warning',
                'description': f'Alert rate ({alerts_per_hour:.1f}/hr) is 3x normal ({historical_avg:.1f}/hr)',
                'timestamp': datetime.utcnow().isoformat()
            })
        
        # Check for unusual species
        recent_species = defaultdict(int)
        for alert in recent_alerts:
            recent_species[alert['species']] += 1
        
        for species, count in recent_species.items():
            historical_count = self.species_metrics[species]['alerts']
            if count > historical_count * 0.5 and historical_count > 0:
                anomalies.append({
                    'type': 'species_anomaly',
                    'severity': 'info',
                    'description': f'Unusual activity for {species}: {count} alerts in {window_hours}h',
                    'species': species,
                    'timestamp': datetime.utcnow().isoformat()
                })
        
        # Check for low confidence trend
        recent_confidences = [alert['confidence'] for alert in recent_alerts]
        avg_recent_confidence = np.mean(recent_confidences)
        
        if avg_recent_confidence < 0.6:
            anomalies.append({
                'type': 'low_confidence',
                'severity': 'warning',
                'description': f'Recent alerts have low avg confidence: {avg_recent_confidence:.2f}',
                'timestamp': datetime.utcnow().isoformat()
            })
        
        return anomalies
    
    def recommend_threshold_adjustments(self) -> Dict:
        """
        Recommend threshold adjustments based on performance
        
        Returns:
            Recommended threshold adjustments
        """
        recommendations = {
            'adjustments': [],
            'reasoning': []
        }
        
        # Check overall false positive rate
        total_feedback = self.true_positives + self.false_positives
        if total_feedback > 50:
            fp_rate = self.false_positives / total_feedback
            
            if fp_rate > 0.3:
                recommendations['adjustments'].append({
                    'parameter': 'confidence_threshold',
                    'current': 0.7,
                    'recommended': 0.75,
                    'reason': f'High false positive rate: {fp_rate:.1%}'
                })
            elif fp_rate < 0.05:
                recommendations['adjustments'].append({
                    'parameter': 'confidence_threshold',
                    'current': 0.7,
                    'recommended': 0.65,
                    'reason': f'Very low false positive rate: {fp_rate:.1%}, can be more sensitive'
                })
        
        # Check species-specific performance
        for species, metrics in self.species_metrics.items():
            total = metrics['true_positives'] + metrics['false_positives']
            if total > 10:
                accuracy = metrics['true_positives'] / total
                
                if accuracy < 0.6:
                    recommendations['adjustments'].append({
                        'parameter': f'{species}_threshold',
                        'current': 0.7,
                        'recommended': 0.8,
                        'reason': f'Low accuracy for {species}: {accuracy:.1%}'
                    })
        
        # Check response time
        if self.response_times:
            avg_time = np.mean(self.response_times)
            if avg_time > 2000:  # > 2 seconds
                recommendations['adjustments'].append({
                    'parameter': 'processing_optimization',
                    'issue': 'slow_processing',
                    'avg_time_ms': round(avg_time, 2),
                    'recommendation': 'Consider optimizing ML inference or reducing image resolution'
                })
        
        return recommendations
    
    def generate_report(self, days: int = 7) -> Dict:
        """
        Generate comprehensive analytics report
        
        Args:
            days: Number of days to include in report
            
        Returns:
            Complete analytics report
        """
        return {
            'report_period_days': days,
            'generated_at': datetime.utcnow().isoformat(),
            'accuracy_metrics': self.get_accuracy_metrics(days),
            'performance_metrics': self.get_performance_metrics(),
            'temporal_patterns': self.get_temporal_patterns(),
            'top_species': self.get_species_performance(),
            'anomalies': self.detect_anomalies(window_hours=24),
            'recommendations': self.recommend_threshold_adjustments()
        }
