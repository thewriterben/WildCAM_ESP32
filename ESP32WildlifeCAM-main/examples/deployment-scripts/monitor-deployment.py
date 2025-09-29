#!/usr/bin/env python3
"""
ESP32 Wildlife Camera Performance Monitoring Script

This script monitors and reports on wildlife camera deployment performance,
including system health, data quality, network status, and conservation impact.

Usage:
    python3 monitor-deployment.py --deployment-id camera_001 --report-type daily
    python3 monitor-deployment.py --dashboard --port 8080
    python3 monitor-deployment.py --alert-check
"""

import argparse
import json
import time
import sys
from datetime import datetime, timedelta
from pathlib import Path
import sqlite3
import requests
import matplotlib.pyplot as plt
import pandas as pd
from typing import Dict, List, Optional, Tuple

class WildlifeCameraMonitor:
    """Wildlife camera deployment monitoring and performance analysis"""
    
    def __init__(self, db_path: str = "monitoring.db"):
        self.db_path = Path(db_path)
        self.init_database()
        
        # Performance thresholds
        self.thresholds = {
            'battery_warning': 30.0,        # Battery warning at 30%
            'battery_critical': 15.0,       # Battery critical at 15%
            'temperature_warning': 45.0,    # Temperature warning at 45°C
            'temperature_critical': 60.0,   # Temperature critical at 60°C
            'disk_warning': 80.0,           # Disk warning at 80% full
            'disk_critical': 95.0,          # Disk critical at 95% full
            'network_latency_warning': 5.0, # Network latency warning at 5s
            'detection_rate_warning': 0.1,  # Detection rate warning below 10%
        }
    
    def init_database(self):
        """Initialize monitoring database"""
        with sqlite3.connect(self.db_path) as conn:
            conn.executescript("""
                CREATE TABLE IF NOT EXISTS deployments (
                    id TEXT PRIMARY KEY,
                    name TEXT,
                    location_lat REAL,
                    location_lng REAL,
                    board_type TEXT,
                    scenario TEXT,
                    deployment_date TEXT,
                    status TEXT
                );
                
                CREATE TABLE IF NOT EXISTS system_metrics (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    deployment_id TEXT,
                    timestamp TEXT,
                    battery_voltage REAL,
                    battery_percentage REAL,
                    solar_voltage REAL,
                    temperature REAL,
                    humidity REAL,
                    free_memory_kb INTEGER,
                    disk_usage_percentage REAL,
                    cpu_usage_percentage REAL,
                    network_signal_strength REAL,
                    FOREIGN KEY (deployment_id) REFERENCES deployments (id)
                );
                
                CREATE TABLE IF NOT EXISTS detections (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    deployment_id TEXT,
                    timestamp TEXT,
                    species TEXT,
                    confidence REAL,
                    behavior TEXT,
                    image_path TEXT,
                    ai_processing_time_ms INTEGER,
                    validated BOOLEAN,
                    FOREIGN KEY (deployment_id) REFERENCES deployments (id)
                );
                
                CREATE TABLE IF NOT EXISTS alerts (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    deployment_id TEXT,
                    timestamp TEXT,
                    alert_type TEXT,
                    severity TEXT,
                    message TEXT,
                    resolved BOOLEAN DEFAULT FALSE,
                    FOREIGN KEY (deployment_id) REFERENCES deployments (id)
                );
                
                CREATE TABLE IF NOT EXISTS network_stats (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    deployment_id TEXT,
                    timestamp TEXT,
                    protocol TEXT,
                    bytes_sent INTEGER,
                    bytes_received INTEGER,
                    packets_sent INTEGER,
                    packets_received INTEGER,
                    packet_loss_rate REAL,
                    latency_ms REAL,
                    FOREIGN KEY (deployment_id) REFERENCES deployments (id)
                );
            """)
    
    def register_deployment(self, deployment_id: str, config: Dict) -> bool:
        """Register a new deployment for monitoring"""
        try:
            with sqlite3.connect(self.db_path) as conn:
                conn.execute("""
                    INSERT OR REPLACE INTO deployments 
                    (id, name, location_lat, location_lng, board_type, scenario, deployment_date, status)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                """, (
                    deployment_id,
                    config.get('name', f'Camera {deployment_id}'),
                    config.get('latitude', 0.0),
                    config.get('longitude', 0.0),
                    config.get('board_type', 'unknown'),
                    config.get('scenario', 'unknown'),
                    datetime.now().isoformat(),
                    'active'
                ))
            
            print(f"✅ Registered deployment: {deployment_id}")
            return True
            
        except Exception as e:
            print(f"❌ Failed to register deployment: {e}")
            return False
    
    def collect_system_metrics(self, deployment_id: str) -> Dict:
        """Collect current system metrics from deployment"""
        try:
            # In a real implementation, this would query the actual device
            # For demo purposes, we'll simulate some realistic metrics
            import random
            
            metrics = {
                'timestamp': datetime.now().isoformat(),
                'battery_voltage': round(3.2 + random.random() * 0.8, 2),
                'battery_percentage': round(20 + random.random() * 60, 1),
                'solar_voltage': round(random.random() * 6, 2),
                'temperature': round(15 + random.random() * 25, 1),
                'humidity': round(40 + random.random() * 40, 1),
                'free_memory_kb': int(1000 + random.random() * 2000),
                'disk_usage_percentage': round(20 + random.random() * 50, 1),
                'cpu_usage_percentage': round(random.random() * 30, 1),
                'network_signal_strength': round(-40 - random.random() * 40, 1)
            }
            
            # Store metrics in database
            with sqlite3.connect(self.db_path) as conn:
                conn.execute("""
                    INSERT INTO system_metrics 
                    (deployment_id, timestamp, battery_voltage, battery_percentage, 
                     solar_voltage, temperature, humidity, free_memory_kb, 
                     disk_usage_percentage, cpu_usage_percentage, network_signal_strength)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """, (
                    deployment_id, metrics['timestamp'], metrics['battery_voltage'],
                    metrics['battery_percentage'], metrics['solar_voltage'],
                    metrics['temperature'], metrics['humidity'], metrics['free_memory_kb'],
                    metrics['disk_usage_percentage'], metrics['cpu_usage_percentage'],
                    metrics['network_signal_strength']
                ))
            
            return metrics
            
        except Exception as e:
            print(f"❌ Failed to collect metrics: {e}")
            return {}
    
    def check_alerts(self, deployment_id: str, metrics: Dict) -> List[Dict]:
        """Check for alert conditions and generate alerts"""
        alerts = []
        
        # Battery alerts
        if metrics.get('battery_percentage', 100) < self.thresholds['battery_critical']:
            alerts.append({
                'type': 'battery_critical',
                'severity': 'critical',
                'message': f"Battery critically low: {metrics['battery_percentage']:.1f}%"
            })
        elif metrics.get('battery_percentage', 100) < self.thresholds['battery_warning']:
            alerts.append({
                'type': 'battery_warning',
                'severity': 'warning',
                'message': f"Battery low: {metrics['battery_percentage']:.1f}%"
            })
        
        # Temperature alerts
        if metrics.get('temperature', 0) > self.thresholds['temperature_critical']:
            alerts.append({
                'type': 'temperature_critical',
                'severity': 'critical',
                'message': f"Temperature critically high: {metrics['temperature']:.1f}°C"
            })
        elif metrics.get('temperature', 0) > self.thresholds['temperature_warning']:
            alerts.append({
                'type': 'temperature_warning',
                'severity': 'warning',
                'message': f"Temperature high: {metrics['temperature']:.1f}°C"
            })
        
        # Disk space alerts
        if metrics.get('disk_usage_percentage', 0) > self.thresholds['disk_critical']:
            alerts.append({
                'type': 'disk_critical',
                'severity': 'critical',
                'message': f"Disk space critically low: {metrics['disk_usage_percentage']:.1f}% used"
            })
        elif metrics.get('disk_usage_percentage', 0) > self.thresholds['disk_warning']:
            alerts.append({
                'type': 'disk_warning',
                'severity': 'warning',
                'message': f"Disk space low: {metrics['disk_usage_percentage']:.1f}% used"
            })
        
        # Network signal alerts
        if metrics.get('network_signal_strength', 0) < -90:
            alerts.append({
                'type': 'network_weak',
                'severity': 'warning',
                'message': f"Weak network signal: {metrics['network_signal_strength']:.1f} dBm"
            })
        
        # Store alerts in database
        for alert in alerts:
            with sqlite3.connect(self.db_path) as conn:
                conn.execute("""
                    INSERT INTO alerts (deployment_id, timestamp, alert_type, severity, message)
                    VALUES (?, ?, ?, ?, ?)
                """, (
                    deployment_id, datetime.now().isoformat(),
                    alert['type'], alert['severity'], alert['message']
                ))
        
        return alerts
    
    def generate_performance_report(self, deployment_id: str, period: str = 'daily') -> Dict:
        """Generate performance report for a deployment"""
        try:
            # Calculate time range
            if period == 'daily':
                start_time = datetime.now() - timedelta(days=1)
            elif period == 'weekly':
                start_time = datetime.now() - timedelta(weeks=1)
            elif period == 'monthly':
                start_time = datetime.now() - timedelta(days=30)
            else:
                start_time = datetime.now() - timedelta(days=1)
            
            start_time_str = start_time.isoformat()
            
            with sqlite3.connect(self.db_path) as conn:
                # System health metrics
                cursor = conn.execute("""
                    SELECT AVG(battery_percentage), MIN(battery_percentage), MAX(battery_percentage),
                           AVG(temperature), MIN(temperature), MAX(temperature),
                           AVG(disk_usage_percentage), AVG(cpu_usage_percentage)
                    FROM system_metrics 
                    WHERE deployment_id = ? AND timestamp > ?
                """, (deployment_id, start_time_str))
                
                system_stats = cursor.fetchone()
                
                # Detection statistics
                cursor = conn.execute("""
                    SELECT COUNT(*) as total_detections,
                           COUNT(DISTINCT species) as unique_species,
                           AVG(confidence) as avg_confidence,
                           AVG(ai_processing_time_ms) as avg_processing_time
                    FROM detections 
                    WHERE deployment_id = ? AND timestamp > ?
                """, (deployment_id, start_time_str))
                
                detection_stats = cursor.fetchone()
                
                # Alert statistics
                cursor = conn.execute("""
                    SELECT COUNT(*) as total_alerts,
                           SUM(CASE WHEN severity = 'critical' THEN 1 ELSE 0 END) as critical_alerts,
                           SUM(CASE WHEN severity = 'warning' THEN 1 ELSE 0 END) as warning_alerts
                    FROM alerts 
                    WHERE deployment_id = ? AND timestamp > ?
                """, (deployment_id, start_time_str))
                
                alert_stats = cursor.fetchone()
                
                # Network statistics
                cursor = conn.execute("""
                    SELECT AVG(latency_ms) as avg_latency,
                           AVG(packet_loss_rate) as avg_packet_loss,
                           SUM(bytes_sent) as total_bytes_sent,
                           SUM(bytes_received) as total_bytes_received
                    FROM network_stats 
                    WHERE deployment_id = ? AND timestamp > ?
                """, (deployment_id, start_time_str))
                
                network_stats = cursor.fetchone()
            
            # Compile report
            report = {
                'deployment_id': deployment_id,
                'period': period,
                'report_timestamp': datetime.now().isoformat(),
                'system_health': {
                    'battery_avg': system_stats[0] if system_stats[0] else 0,
                    'battery_min': system_stats[1] if system_stats[1] else 0,
                    'battery_max': system_stats[2] if system_stats[2] else 0,
                    'temperature_avg': system_stats[3] if system_stats[3] else 0,
                    'temperature_min': system_stats[4] if system_stats[4] else 0,
                    'temperature_max': system_stats[5] if system_stats[5] else 0,
                    'disk_usage_avg': system_stats[6] if system_stats[6] else 0,
                    'cpu_usage_avg': system_stats[7] if system_stats[7] else 0
                },
                'detection_performance': {
                    'total_detections': detection_stats[0] if detection_stats[0] else 0,
                    'unique_species': detection_stats[1] if detection_stats[1] else 0,
                    'avg_confidence': detection_stats[2] if detection_stats[2] else 0,
                    'avg_processing_time_ms': detection_stats[3] if detection_stats[3] else 0
                },
                'alert_summary': {
                    'total_alerts': alert_stats[0] if alert_stats[0] else 0,
                    'critical_alerts': alert_stats[1] if alert_stats[1] else 0,
                    'warning_alerts': alert_stats[2] if alert_stats[2] else 0
                },
                'network_performance': {
                    'avg_latency_ms': network_stats[0] if network_stats[0] else 0,
                    'avg_packet_loss': network_stats[1] if network_stats[1] else 0,
                    'total_bytes_sent': network_stats[2] if network_stats[2] else 0,
                    'total_bytes_received': network_stats[3] if network_stats[3] else 0
                }
            }
            
            # Calculate health score
            report['overall_health_score'] = self.calculate_health_score(report)
            
            return report
            
        except Exception as e:
            print(f"❌ Failed to generate report: {e}")
            return {}
    
    def calculate_health_score(self, report: Dict) -> float:
        """Calculate overall deployment health score (0-100)"""
        score = 100.0
        
        # Battery health impact
        battery_avg = report['system_health']['battery_avg']
        if battery_avg < 20:
            score -= 30
        elif battery_avg < 40:
            score -= 15
        elif battery_avg < 60:
            score -= 5
        
        # Temperature impact
        temp_avg = report['system_health']['temperature_avg']
        if temp_avg > 50:
            score -= 20
        elif temp_avg > 40:
            score -= 10
        
        # Alert impact
        critical_alerts = report['alert_summary']['critical_alerts']
        warning_alerts = report['alert_summary']['warning_alerts']
        score -= critical_alerts * 15
        score -= warning_alerts * 5
        
        # Detection performance impact
        detections = report['detection_performance']['total_detections']
        if detections == 0:
            score -= 20
        elif detections < 5:  # Less than 5 detections per day
            score -= 10
        
        # Network performance impact
        avg_latency = report['network_performance']['avg_latency_ms']
        if avg_latency > 10000:  # > 10 seconds
            score -= 15
        elif avg_latency > 5000:  # > 5 seconds
            score -= 8
        
        return max(0.0, min(100.0, score))
    
    def generate_visualizations(self, deployment_id: str, output_dir: str = "reports"):
        """Generate performance visualization charts"""
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
        
        try:
            # Query recent data
            with sqlite3.connect(self.db_path) as conn:
                # Battery level over time
                df_battery = pd.read_sql_query("""
                    SELECT timestamp, battery_percentage, battery_voltage
                    FROM system_metrics 
                    WHERE deployment_id = ? 
                    ORDER BY timestamp DESC LIMIT 100
                """, conn, params=(deployment_id,))
                
                if not df_battery.empty:
                    df_battery['timestamp'] = pd.to_datetime(df_battery['timestamp'])
                    
                    plt.figure(figsize=(12, 8))
                    
                    # Battery percentage subplot
                    plt.subplot(2, 2, 1)
                    plt.plot(df_battery['timestamp'], df_battery['battery_percentage'], 'b-', linewidth=2)
                    plt.title('Battery Level Over Time')
                    plt.xlabel('Time')
                    plt.ylabel('Battery (%)')
                    plt.grid(True, alpha=0.3)
                    plt.xticks(rotation=45)
                    
                    # Temperature subplot
                    df_temp = pd.read_sql_query("""
                        SELECT timestamp, temperature
                        FROM system_metrics 
                        WHERE deployment_id = ? 
                        ORDER BY timestamp DESC LIMIT 100
                    """, conn, params=(deployment_id,))
                    
                    if not df_temp.empty:
                        df_temp['timestamp'] = pd.to_datetime(df_temp['timestamp'])
                        plt.subplot(2, 2, 2)
                        plt.plot(df_temp['timestamp'], df_temp['temperature'], 'r-', linewidth=2)
                        plt.title('Temperature Over Time')
                        plt.xlabel('Time')
                        plt.ylabel('Temperature (°C)')
                        plt.grid(True, alpha=0.3)
                        plt.xticks(rotation=45)
                    
                    # Detection count by species
                    df_detections = pd.read_sql_query("""
                        SELECT species, COUNT(*) as count
                        FROM detections 
                        WHERE deployment_id = ? 
                        GROUP BY species
                        ORDER BY count DESC LIMIT 10
                    """, conn, params=(deployment_id,))
                    
                    if not df_detections.empty:
                        plt.subplot(2, 2, 3)
                        plt.bar(df_detections['species'], df_detections['count'])
                        plt.title('Species Detection Count')
                        plt.xlabel('Species')
                        plt.ylabel('Detections')
                        plt.xticks(rotation=45)
                    
                    # Alert timeline
                    df_alerts = pd.read_sql_query("""
                        SELECT timestamp, alert_type, severity
                        FROM alerts 
                        WHERE deployment_id = ? 
                        ORDER BY timestamp DESC LIMIT 50
                    """, conn, params=(deployment_id,))
                    
                    if not df_alerts.empty:
                        df_alerts['timestamp'] = pd.to_datetime(df_alerts['timestamp'])
                        plt.subplot(2, 2, 4)
                        
                        # Create alert timeline
                        critical_alerts = df_alerts[df_alerts['severity'] == 'critical']
                        warning_alerts = df_alerts[df_alerts['severity'] == 'warning']
                        
                        if not critical_alerts.empty:
                            plt.scatter(critical_alerts['timestamp'], [1]*len(critical_alerts), 
                                      c='red', s=50, label='Critical', alpha=0.7)
                        if not warning_alerts.empty:
                            plt.scatter(warning_alerts['timestamp'], [0.5]*len(warning_alerts), 
                                      c='orange', s=30, label='Warning', alpha=0.7)
                        
                        plt.title('Alert Timeline')
                        plt.xlabel('Time')
                        plt.ylabel('Alert Severity')
                        plt.legend()
                        plt.xticks(rotation=45)
                    
                    plt.tight_layout()
                    chart_path = output_path / f"{deployment_id}_performance_charts.png"
                    plt.savefig(chart_path, dpi=300, bbox_inches='tight')
                    plt.close()
                    
                    print(f"📊 Performance charts saved: {chart_path}")
                    return str(chart_path)
            
        except Exception as e:
            print(f"❌ Failed to generate visualizations: {e}")
            return None
    
    def run_monitoring_cycle(self, deployment_id: str) -> bool:
        """Run a complete monitoring cycle for a deployment"""
        print(f"🔍 Running monitoring cycle for {deployment_id}")
        
        try:
            # Collect system metrics
            metrics = self.collect_system_metrics(deployment_id)
            if not metrics:
                print("❌ Failed to collect metrics")
                return False
            
            print(f"📊 System Status:")
            print(f"  Battery: {metrics.get('battery_percentage', 0):.1f}%")
            print(f"  Temperature: {metrics.get('temperature', 0):.1f}°C")
            print(f"  Disk Usage: {metrics.get('disk_usage_percentage', 0):.1f}%")
            print(f"  Signal: {metrics.get('network_signal_strength', 0):.1f} dBm")
            
            # Check for alerts
            alerts = self.check_alerts(deployment_id, metrics)
            if alerts:
                print(f"⚠️  {len(alerts)} alert(s) detected:")
                for alert in alerts:
                    severity_icon = "🚨" if alert['severity'] == 'critical' else "⚠️"
                    print(f"  {severity_icon} {alert['message']}")
            else:
                print("✅ No alerts detected")
            
            return True
            
        except Exception as e:
            print(f"❌ Monitoring cycle failed: {e}")
            return False
    
    def start_continuous_monitoring(self, deployment_ids: List[str], interval_minutes: int = 5):
        """Start continuous monitoring for multiple deployments"""
        print(f"🔄 Starting continuous monitoring for {len(deployment_ids)} deployments")
        print(f"   Monitoring interval: {interval_minutes} minutes")
        
        try:
            while True:
                for deployment_id in deployment_ids:
                    self.run_monitoring_cycle(deployment_id)
                    print()
                
                print(f"💤 Sleeping for {interval_minutes} minutes...")
                time.sleep(interval_minutes * 60)
                
        except KeyboardInterrupt:
            print("\n🛑 Monitoring stopped by user")
        except Exception as e:
            print(f"\n❌ Monitoring error: {e}")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='ESP32 Wildlife Camera Performance Monitor',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--deployment-id', type=str,
                       help='Deployment ID to monitor')
    
    parser.add_argument('--register', action='store_true',
                       help='Register a new deployment')
    
    parser.add_argument('--monitor', action='store_true',
                       help='Run single monitoring cycle')
    
    parser.add_argument('--continuous', action='store_true',
                       help='Start continuous monitoring')
    
    parser.add_argument('--report', choices=['daily', 'weekly', 'monthly'],
                       help='Generate performance report')
    
    parser.add_argument('--charts', action='store_true',
                       help='Generate performance charts')
    
    parser.add_argument('--alert-check', action='store_true',
                       help='Check for alerts only')
    
    parser.add_argument('--interval', type=int, default=5,
                       help='Monitoring interval in minutes (default: 5)')
    
    parser.add_argument('--config', type=str,
                       help='Configuration file for deployment registration')
    
    args = parser.parse_args()
    
    # Create monitor instance
    monitor = WildlifeCameraMonitor()
    
    # Handle deployment registration
    if args.register:
        if not args.deployment_id:
            print("❌ Deployment ID required for registration")
            return 1
        
        config = {}
        if args.config:
            try:
                with open(args.config, 'r') as f:
                    config = json.load(f)
            except Exception as e:
                print(f"❌ Failed to load config: {e}")
                return 1
        
        success = monitor.register_deployment(args.deployment_id, config)
        return 0 if success else 1
    
    # All other operations require deployment ID
    if not args.deployment_id:
        print("❌ Deployment ID required")
        return 1
    
    # Handle single monitoring cycle
    if args.monitor:
        success = monitor.run_monitoring_cycle(args.deployment_id)
        return 0 if success else 1
    
    # Handle continuous monitoring
    if args.continuous:
        monitor.start_continuous_monitoring([args.deployment_id], args.interval)
        return 0
    
    # Handle report generation
    if args.report:
        report = monitor.generate_performance_report(args.deployment_id, args.report)
        if report:
            print(f"📊 {args.report.title()} Performance Report")
            print("=" * 50)
            print(f"Deployment: {report['deployment_id']}")
            print(f"Period: {report['period']}")
            print(f"Health Score: {report['overall_health_score']:.1f}/100")
            print()
            
            print("System Health:")
            print(f"  Battery: {report['system_health']['battery_avg']:.1f}% avg")
            print(f"  Temperature: {report['system_health']['temperature_avg']:.1f}°C avg")
            print(f"  Disk Usage: {report['system_health']['disk_usage_avg']:.1f}%")
            print()
            
            print("Detection Performance:")
            print(f"  Total Detections: {report['detection_performance']['total_detections']}")
            print(f"  Unique Species: {report['detection_performance']['unique_species']}")
            print(f"  Avg Confidence: {report['detection_performance']['avg_confidence']:.2f}")
            print()
            
            print("Alerts:")
            print(f"  Critical: {report['alert_summary']['critical_alerts']}")
            print(f"  Warning: {report['alert_summary']['warning_alerts']}")
            
            return 0
        else:
            return 1
    
    # Handle chart generation
    if args.charts:
        chart_path = monitor.generate_visualizations(args.deployment_id)
        return 0 if chart_path else 1
    
    # Handle alert checking
    if args.alert_check:
        metrics = monitor.collect_system_metrics(args.deployment_id)
        alerts = monitor.check_alerts(args.deployment_id, metrics)
        
        if alerts:
            print(f"⚠️  {len(alerts)} alert(s) found:")
            for alert in alerts:
                print(f"  {alert['severity'].upper()}: {alert['message']}")
            return 1
        else:
            print("✅ No alerts detected")
            return 0
    
    # Default: show help
    parser.print_help()
    return 0

if __name__ == '__main__':
    sys.exit(main())