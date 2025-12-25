"""
Diagnostics and Health Monitoring for WildCAM Gateway
Provides system health checks, network monitoring, and diagnostic reports
"""

import logging
import os
import time
import platform
import json
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional
from pathlib import Path

try:
    import psutil
    PSUTIL_AVAILABLE = True
except ImportError:
    PSUTIL_AVAILABLE = False
    logging.warning("psutil not available. Install with: pip install psutil")

from database import GatewayDatabase
from config import GatewayConfig


logger = logging.getLogger(__name__)


class GatewayDiagnostics:
    """
    Diagnostics and health monitoring for gateway
    """
    
    def __init__(self, config: GatewayConfig, database: GatewayDatabase):
        """
        Initialize diagnostics module
        
        Args:
            config: Gateway configuration
            database: Database instance
        """
        self.config = config
        self.db = database
        self.start_time = datetime.now()
        self.lora_gateway = None
        self.mqtt_bridge = None
        
        # Health check thresholds
        self.cpu_threshold = 90.0  # percentage
        self.memory_threshold = 90.0  # percentage
        self.disk_threshold = 90.0  # percentage
        self.temperature_threshold = 80.0  # Celsius
    
    def set_lora_gateway(self, lora_gateway):
        """Set LoRa gateway reference for monitoring"""
        self.lora_gateway = lora_gateway
    
    def set_mqtt_bridge(self, mqtt_bridge):
        """Set MQTT bridge reference for monitoring"""
        self.mqtt_bridge = mqtt_bridge
    
    def get_system_health(self) -> Dict[str, Any]:
        """
        Get comprehensive system health information
        
        Returns:
            Dictionary containing system health metrics
        """
        health = {
            'timestamp': datetime.now().isoformat(),
            'status': 'healthy',
            'checks': {},
            'warnings': [],
            'errors': []
        }
        
        # CPU check
        cpu_check = self._check_cpu()
        health['checks']['cpu'] = cpu_check
        if cpu_check['status'] == 'warning':
            health['warnings'].append(cpu_check['message'])
            health['status'] = 'degraded'
        elif cpu_check['status'] == 'critical':
            health['errors'].append(cpu_check['message'])
            health['status'] = 'unhealthy'
        
        # Memory check
        memory_check = self._check_memory()
        health['checks']['memory'] = memory_check
        if memory_check['status'] == 'warning':
            health['warnings'].append(memory_check['message'])
            health['status'] = 'degraded'
        elif memory_check['status'] == 'critical':
            health['errors'].append(memory_check['message'])
            health['status'] = 'unhealthy'
        
        # Disk check
        disk_check = self._check_disk()
        health['checks']['disk'] = disk_check
        if disk_check['status'] == 'warning':
            health['warnings'].append(disk_check['message'])
            health['status'] = 'degraded'
        elif disk_check['status'] == 'critical':
            health['errors'].append(disk_check['message'])
            health['status'] = 'unhealthy'
        
        # Temperature check
        temp_check = self._check_temperature()
        health['checks']['temperature'] = temp_check
        if temp_check['status'] == 'warning':
            health['warnings'].append(temp_check['message'])
            health['status'] = 'degraded'
        elif temp_check['status'] == 'critical':
            health['errors'].append(temp_check['message'])
            health['status'] = 'unhealthy'
        
        # Database check
        db_check = self._check_database()
        health['checks']['database'] = db_check
        if db_check['status'] != 'healthy':
            health['errors'].append(db_check['message'])
            health['status'] = 'unhealthy'
        
        return health
    
    def _check_cpu(self) -> Dict[str, Any]:
        """Check CPU usage"""
        if not PSUTIL_AVAILABLE:
            return {
                'status': 'unknown',
                'message': 'psutil not available',
                'usage_percent': None
            }
        
        try:
            cpu_percent = psutil.cpu_percent(interval=1)
            cpu_count = psutil.cpu_count()
            
            status = 'healthy'
            message = f'CPU usage: {cpu_percent}%'
            
            if cpu_percent >= self.cpu_threshold:
                status = 'critical'
                message = f'CPU usage critical: {cpu_percent}% (threshold: {self.cpu_threshold}%)'
            elif cpu_percent >= self.cpu_threshold * 0.8:
                status = 'warning'
                message = f'CPU usage high: {cpu_percent}%'
            
            return {
                'status': status,
                'message': message,
                'usage_percent': cpu_percent,
                'cpu_count': cpu_count,
                'threshold': self.cpu_threshold
            }
        except Exception as e:
            logger.error(f"Error checking CPU: {e}")
            return {
                'status': 'error',
                'message': f'Error checking CPU: {e}',
                'usage_percent': None
            }
    
    def _check_memory(self) -> Dict[str, Any]:
        """Check memory usage"""
        if not PSUTIL_AVAILABLE:
            return {
                'status': 'unknown',
                'message': 'psutil not available',
                'usage_percent': None
            }
        
        try:
            memory = psutil.virtual_memory()
            
            status = 'healthy'
            message = f'Memory usage: {memory.percent}%'
            
            if memory.percent >= self.memory_threshold:
                status = 'critical'
                message = f'Memory usage critical: {memory.percent}% (threshold: {self.memory_threshold}%)'
            elif memory.percent >= self.memory_threshold * 0.8:
                status = 'warning'
                message = f'Memory usage high: {memory.percent}%'
            
            return {
                'status': status,
                'message': message,
                'usage_percent': memory.percent,
                'total_mb': memory.total / (1024 * 1024),
                'available_mb': memory.available / (1024 * 1024),
                'used_mb': memory.used / (1024 * 1024),
                'threshold': self.memory_threshold
            }
        except Exception as e:
            logger.error(f"Error checking memory: {e}")
            return {
                'status': 'error',
                'message': f'Error checking memory: {e}',
                'usage_percent': None
            }
    
    def _check_disk(self) -> Dict[str, Any]:
        """Check disk usage"""
        if not PSUTIL_AVAILABLE:
            return {
                'status': 'unknown',
                'message': 'psutil not available',
                'usage_percent': None
            }
        
        try:
            disk = psutil.disk_usage('/')
            
            status = 'healthy'
            message = f'Disk usage: {disk.percent}%'
            
            if disk.percent >= self.disk_threshold:
                status = 'critical'
                message = f'Disk usage critical: {disk.percent}% (threshold: {self.disk_threshold}%)'
            elif disk.percent >= self.disk_threshold * 0.8:
                status = 'warning'
                message = f'Disk usage high: {disk.percent}%'
            
            return {
                'status': status,
                'message': message,
                'usage_percent': disk.percent,
                'total_gb': disk.total / (1024 ** 3),
                'free_gb': disk.free / (1024 ** 3),
                'used_gb': disk.used / (1024 ** 3),
                'threshold': self.disk_threshold
            }
        except Exception as e:
            logger.error(f"Error checking disk: {e}")
            return {
                'status': 'error',
                'message': f'Error checking disk: {e}',
                'usage_percent': None
            }
    
    def _check_temperature(self) -> Dict[str, Any]:
        """Check system temperature (Raspberry Pi specific)"""
        try:
            # Try to read Raspberry Pi temperature
            temp_file = Path('/sys/class/thermal/thermal_zone0/temp')
            if temp_file.exists():
                temp_raw = int(temp_file.read_text().strip())
                temp_celsius = temp_raw / 1000.0
                
                status = 'healthy'
                message = f'Temperature: {temp_celsius:.1f}°C'
                
                if temp_celsius >= self.temperature_threshold:
                    status = 'critical'
                    message = f'Temperature critical: {temp_celsius:.1f}°C (threshold: {self.temperature_threshold}°C)'
                elif temp_celsius >= self.temperature_threshold * 0.9:
                    status = 'warning'
                    message = f'Temperature high: {temp_celsius:.1f}°C'
                
                return {
                    'status': status,
                    'message': message,
                    'temperature_celsius': temp_celsius,
                    'threshold': self.temperature_threshold
                }
        except Exception as e:
            logger.debug(f"Could not read temperature: {e}")
        
        # Fallback if temperature reading not available
        return {
            'status': 'unknown',
            'message': 'Temperature monitoring not available',
            'temperature_celsius': None
        }
    
    def _check_database(self) -> Dict[str, Any]:
        """Check database health"""
        try:
            # Test database connection
            stats = self.db.get_stats()
            
            # Check database size
            db_size_mb = stats.get('db_size_mb', 0)
            max_size_mb = 1000  # 1GB threshold
            
            status = 'healthy'
            message = f'Database operational (size: {db_size_mb:.2f} MB)'
            
            if db_size_mb >= max_size_mb:
                status = 'warning'
                message = f'Database size large: {db_size_mb:.2f} MB'
            
            return {
                'status': status,
                'message': message,
                'size_mb': db_size_mb,
                'stats': stats
            }
        except Exception as e:
            logger.error(f"Error checking database: {e}")
            return {
                'status': 'error',
                'message': f'Database error: {e}',
                'size_mb': None
            }
    
    def get_mesh_health(self) -> Dict[str, Any]:
        """
        Get mesh network health status
        
        Returns:
            Dictionary containing mesh network metrics
        """
        try:
            # Get node statistics
            all_nodes = self.db.get_all_nodes()
            active_nodes = self.db.get_all_nodes(active_only=True)
            
            # Calculate network metrics
            total_nodes = len(all_nodes)
            active_count = len(active_nodes)
            inactive_count = total_nodes - active_count
            
            # Get recent packet statistics
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                # Packets in last hour
                cursor.execute("""
                    SELECT COUNT(*) as count, AVG(rssi) as avg_rssi, AVG(snr) as avg_snr
                    FROM packets
                    WHERE timestamp >= datetime('now', '-1 hour')
                """)
                packet_stats = cursor.fetchone()
                
                # Node activity in last hour
                cursor.execute("""
                    SELECT COUNT(DISTINCT source_id) as active_senders
                    FROM packets
                    WHERE timestamp >= datetime('now', '-1 hour')
                """)
                active_senders = cursor.fetchone()['active_senders']
            
            # Determine network health status
            status = 'healthy'
            warnings = []
            
            if active_count == 0:
                status = 'critical'
                warnings.append('No active nodes detected')
            elif active_count < total_nodes * 0.5:
                status = 'degraded'
                warnings.append(f'{inactive_count} nodes inactive')
            
            if packet_stats['count'] == 0:
                status = 'degraded'
                warnings.append('No packets received in last hour')
            
            return {
                'status': status,
                'timestamp': datetime.now().isoformat(),
                'total_nodes': total_nodes,
                'active_nodes': active_count,
                'inactive_nodes': inactive_count,
                'active_senders_1h': active_senders,
                'packets_1h': packet_stats['count'],
                'average_rssi': packet_stats['avg_rssi'] if packet_stats['avg_rssi'] else 0,
                'average_snr': packet_stats['avg_snr'] if packet_stats['avg_snr'] else 0,
                'warnings': warnings
            }
        except Exception as e:
            logger.error(f"Error getting mesh health: {e}")
            return {
                'status': 'error',
                'message': f'Error: {e}',
                'timestamp': datetime.now().isoformat()
            }
    
    def get_lora_status(self) -> Dict[str, Any]:
        """
        Get LoRa radio status
        
        Returns:
            Dictionary containing LoRa radio status
        """
        if not self.lora_gateway:
            return {
                'status': 'unknown',
                'message': 'LoRa gateway not initialized',
                'available': False
            }
        
        try:
            stats = self.lora_gateway.get_stats()
            
            status = 'healthy'
            warnings = []
            
            if not stats.get('running', False):
                status = 'offline'
                warnings.append('LoRa gateway not running')
            
            error_rate = (stats.get('packets_errors', 0) / 
                         max(stats.get('packets_received', 1), 1) * 100)
            
            if error_rate > 10:
                status = 'degraded'
                warnings.append(f'High error rate: {error_rate:.1f}%')
            
            return {
                'status': status,
                'available': True,
                'running': stats.get('running', False),
                'uptime_seconds': stats.get('uptime_seconds', 0),
                'packets_received': stats.get('packets_received', 0),
                'packets_processed': stats.get('packets_processed', 0),
                'packets_errors': stats.get('packets_errors', 0),
                'error_rate_percent': error_rate,
                'success_rate_percent': stats.get('success_rate', 0),
                'warnings': warnings,
                'timestamp': datetime.now().isoformat()
            }
        except Exception as e:
            logger.error(f"Error getting LoRa status: {e}")
            return {
                'status': 'error',
                'message': f'Error: {e}',
                'available': False,
                'timestamp': datetime.now().isoformat()
            }
    
    def get_uptime(self) -> Dict[str, Any]:
        """
        Get gateway uptime information
        
        Returns:
            Dictionary containing uptime metrics
        """
        uptime = datetime.now() - self.start_time
        
        # Get system uptime if available
        system_uptime = None
        if PSUTIL_AVAILABLE:
            try:
                boot_time = datetime.fromtimestamp(psutil.boot_time())
                system_uptime = datetime.now() - boot_time
            except Exception as e:
                logger.debug(f"Could not get system uptime: {e}")
        
        return {
            'gateway_uptime_seconds': int(uptime.total_seconds()),
            'gateway_uptime_formatted': str(uptime).split('.')[0],
            'gateway_start_time': self.start_time.isoformat(),
            'system_uptime_seconds': int(system_uptime.total_seconds()) if system_uptime else None,
            'system_uptime_formatted': str(system_uptime).split('.')[0] if system_uptime else None,
            'timestamp': datetime.now().isoformat()
        }
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get comprehensive gateway statistics
        
        Returns:
            Dictionary containing various statistics
        """
        stats = {
            'timestamp': datetime.now().isoformat(),
            'gateway': {
                'id': self.config.gateway_id,
                'name': self.config.gateway_name,
                'location': self.config.location
            },
            'database': self.db.get_stats(),
            'system': self._get_system_info(),
            'uptime': self.get_uptime()
        }
        
        # Add LoRa stats if available
        if self.lora_gateway:
            stats['lora'] = self.lora_gateway.get_stats()
        
        # Add MQTT stats if available
        if self.mqtt_bridge:
            stats['mqtt'] = self.mqtt_bridge.get_stats()
        
        return stats
    
    def _get_system_info(self) -> Dict[str, Any]:
        """Get system information"""
        info = {
            'platform': platform.system(),
            'platform_release': platform.release(),
            'platform_version': platform.version(),
            'architecture': platform.machine(),
            'hostname': platform.node(),
            'python_version': platform.python_version()
        }
        
        if PSUTIL_AVAILABLE:
            try:
                info['cpu_count'] = psutil.cpu_count()
                info['memory_total_mb'] = psutil.virtual_memory().total / (1024 * 1024)
            except Exception as e:
                logger.debug(f"Could not get extended system info: {e}")
        
        return info
    
    def generate_diagnostic_report(self, format: str = 'json') -> str:
        """
        Generate comprehensive diagnostic report
        
        Args:
            format: Output format ('json' or 'text')
            
        Returns:
            Diagnostic report as string
        """
        report = {
            'report_generated': datetime.now().isoformat(),
            'gateway_info': {
                'id': self.config.gateway_id,
                'name': self.config.gateway_name,
                'location': self.config.location
            },
            'system_health': self.get_system_health(),
            'mesh_health': self.get_mesh_health(),
            'lora_status': self.get_lora_status(),
            'statistics': self.get_statistics(),
            'uptime': self.get_uptime()
        }
        
        if format == 'json':
            return json.dumps(report, indent=2)
        elif format == 'text':
            return self._format_report_text(report)
        else:
            raise ValueError(f"Unsupported format: {format}")
    
    def _format_report_text(self, report: Dict[str, Any]) -> str:
        """Format diagnostic report as text"""
        lines = [
            "=" * 70,
            "WildCAM Gateway Diagnostic Report",
            "=" * 70,
            f"Generated: {report['report_generated']}",
            "",
            "GATEWAY INFORMATION:",
            f"  ID: {report['gateway_info']['id']}",
            f"  Name: {report['gateway_info']['name']}",
            f"  Location: {report['gateway_info']['location'] or 'Not set'}",
            "",
            "SYSTEM HEALTH:",
            f"  Overall Status: {report['system_health']['status'].upper()}",
        ]
        
        for check_name, check_data in report['system_health']['checks'].items():
            lines.append(f"  {check_name.capitalize()}: {check_data['message']}")
        
        if report['system_health']['warnings']:
            lines.append("\n  Warnings:")
            for warning in report['system_health']['warnings']:
                lines.append(f"    - {warning}")
        
        if report['system_health']['errors']:
            lines.append("\n  Errors:")
            for error in report['system_health']['errors']:
                lines.append(f"    - {error}")
        
        lines.extend([
            "",
            "MESH NETWORK HEALTH:",
            f"  Status: {report['mesh_health']['status'].upper()}",
            f"  Total Nodes: {report['mesh_health']['total_nodes']}",
            f"  Active Nodes: {report['mesh_health']['active_nodes']}",
            f"  Packets (1h): {report['mesh_health']['packets_1h']}",
            f"  Average RSSI: {report['mesh_health']['average_rssi']:.1f} dBm",
            "",
            "LORA RADIO STATUS:",
            f"  Status: {report['lora_status']['status'].upper()}",
            f"  Available: {report['lora_status']['available']}",
        ])
        
        if report['lora_status']['available']:
            lines.extend([
                f"  Packets Received: {report['lora_status'].get('packets_received', 0)}",
                f"  Packets Processed: {report['lora_status'].get('packets_processed', 0)}",
                f"  Error Rate: {report['lora_status'].get('error_rate_percent', 0):.1f}%",
            ])
        
        uptime = report['uptime']
        lines.extend([
            "",
            "UPTIME:",
            f"  Gateway: {uptime['gateway_uptime_formatted']}",
            f"  System: {uptime['system_uptime_formatted'] or 'Unknown'}",
            "",
            "=" * 70
        ])
        
        return "\n".join(lines)
