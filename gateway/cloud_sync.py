"""
Cloud Synchronization Service for WildCAM Gateway
Handles batched uploads to cloud API with retry logic and offline-first operation
"""

import logging
import json
import time
import requests
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional
from threading import Thread, Event, Lock
from queue import Queue, Empty

from config import CloudSyncConfig
from database import GatewayDatabase


logger = logging.getLogger(__name__)


class CloudSyncService:
    """
    Cloud synchronization service for gateway data
    Implements offline-first operation with retry logic
    """
    
    def __init__(self, config: CloudSyncConfig, database: GatewayDatabase):
        """
        Initialize cloud sync service
        
        Args:
            config: Cloud sync configuration
            database: Database instance
        """
        self.config = config
        self.db = database
        
        # Threading
        self.running = False
        self.stop_event = Event()
        self.sync_thread = None
        self.retry_thread = None
        
        # Queue for immediate sync requests
        self.sync_queue = Queue()
        
        # Statistics
        self.syncs_attempted = 0
        self.syncs_succeeded = 0
        self.syncs_failed = 0
        self.last_sync_time = None
        self.lock = Lock()
        
        # Retry configuration
        self.max_retry_attempts = config.retry_attempts
        self.base_retry_delay = config.retry_delay
        self.max_retry_delay = 3600  # 1 hour max backoff
    
    def start(self):
        """Start cloud sync service"""
        if not self.config.enabled:
            logger.info("Cloud sync is disabled in configuration")
            return
        
        if not self.config.cloud_api_url:
            logger.warning("Cloud API URL not configured. Cloud sync disabled.")
            return
        
        if self.running:
            logger.warning("Cloud sync service already running")
            return
        
        self.running = True
        self.stop_event.clear()
        
        # Start sync thread
        self.sync_thread = Thread(target=self._sync_loop, daemon=True)
        self.sync_thread.start()
        
        # Start retry thread
        self.retry_thread = Thread(target=self._retry_loop, daemon=True)
        self.retry_thread.start()
        
        logger.info("Cloud sync service started")
    
    def stop(self):
        """Stop cloud sync service"""
        if not self.running:
            return
        
        self.running = False
        self.stop_event.set()
        
        if self.sync_thread:
            self.sync_thread.join(timeout=10)
        
        if self.retry_thread:
            self.retry_thread.join(timeout=10)
        
        logger.info("Cloud sync service stopped")
    
    def _sync_loop(self):
        """Main sync loop - runs periodically"""
        logger.info("Sync thread started")
        
        while self.running and not self.stop_event.is_set():
            try:
                # Sync detections
                self._sync_detections()
                
                # Sync telemetry (optional - can be disabled for bandwidth)
                # self._sync_telemetry()
                
                # Update last sync time
                with self.lock:
                    self.last_sync_time = datetime.now()
                
                # Wait for next sync interval
                self.stop_event.wait(timeout=self.config.sync_interval)
                
            except Exception as e:
                logger.error(f"Error in sync loop: {e}")
                time.sleep(60)  # Wait before retrying on error
        
        logger.info("Sync thread stopped")
    
    def _retry_loop(self):
        """Retry loop - processes failed sync items"""
        logger.info("Retry thread started")
        
        while self.running and not self.stop_event.is_set():
            try:
                # Process retry queue
                self._process_retry_queue()
                
                # Wait before next retry check
                self.stop_event.wait(timeout=60)  # Check every minute
                
            except Exception as e:
                logger.error(f"Error in retry loop: {e}")
                time.sleep(60)
        
        logger.info("Retry thread stopped")
    
    def _sync_detections(self):
        """Sync wildlife detections to cloud"""
        try:
            # Get unsynced detections
            detections = self.db.get_detections(
                limit=self.config.batch_size,
                unsynced_only=True
            )
            
            if not detections:
                logger.debug("No detections to sync")
                return
            
            logger.info(f"Syncing {len(detections)} detections to cloud")
            
            # Prepare batch data
            batch_data = {
                'gateway_id': self._get_gateway_id(),
                'timestamp': datetime.now().isoformat(),
                'data_type': 'detections',
                'count': len(detections),
                'detections': detections
            }
            
            # Send to cloud
            success = self._send_to_cloud('/api/gateway/detections', batch_data)
            
            if success:
                # Mark detections as synced
                for detection in detections:
                    self.db.mark_detection_synced(detection['id'])
                
                with self.lock:
                    self.syncs_succeeded += 1
                
                logger.info(f"Successfully synced {len(detections)} detections")
            else:
                # Add to retry queue
                for detection in detections:
                    self._add_to_retry_queue('detection', detection['id'])
                
                with self.lock:
                    self.syncs_failed += 1
                
                logger.warning(f"Failed to sync {len(detections)} detections")
                
        except Exception as e:
            logger.error(f"Error syncing detections: {e}")
            with self.lock:
                self.syncs_failed += 1
    
    def _sync_telemetry(self):
        """Sync telemetry data to cloud (optional)"""
        try:
            # Get recent telemetry (last hour)
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    SELECT * FROM telemetry
                    WHERE timestamp >= datetime('now', '-1 hour')
                    ORDER BY timestamp DESC
                    LIMIT ?
                """, (self.config.batch_size,))
                
                telemetry = [dict(row) for row in cursor.fetchall()]
            
            if not telemetry:
                logger.debug("No telemetry to sync")
                return
            
            logger.info(f"Syncing {len(telemetry)} telemetry records to cloud")
            
            # Prepare batch data
            batch_data = {
                'gateway_id': self._get_gateway_id(),
                'timestamp': datetime.now().isoformat(),
                'data_type': 'telemetry',
                'count': len(telemetry),
                'telemetry': telemetry
            }
            
            # Send to cloud
            success = self._send_to_cloud('/api/gateway/telemetry', batch_data)
            
            if success:
                with self.lock:
                    self.syncs_succeeded += 1
                logger.info(f"Successfully synced {len(telemetry)} telemetry records")
            else:
                with self.lock:
                    self.syncs_failed += 1
                logger.warning(f"Failed to sync {len(telemetry)} telemetry records")
                
        except Exception as e:
            logger.error(f"Error syncing telemetry: {e}")
    
    def _send_to_cloud(self, endpoint: str, data: Dict[str, Any]) -> bool:
        """
        Send data to cloud API
        
        Args:
            endpoint: API endpoint path
            data: Data to send
            
        Returns:
            True if successful
        """
        if not self.config.cloud_api_url:
            logger.error("Cloud API URL not configured")
            return False
        
        try:
            url = f"{self.config.cloud_api_url.rstrip('/')}{endpoint}"
            
            headers = {
                'Content-Type': 'application/json'
            }
            
            # Add API key if configured
            if self.config.cloud_api_key:
                headers['X-API-Key'] = self.config.cloud_api_key
            
            with self.lock:
                self.syncs_attempted += 1
            
            # Send request with timeout
            response = requests.post(
                url,
                json=data,
                headers=headers,
                timeout=30
            )
            
            if response.status_code in [200, 201]:
                logger.debug(f"Successfully sent data to {endpoint}")
                return True
            else:
                logger.error(f"Cloud API returned status {response.status_code}: {response.text}")
                return False
                
        except requests.exceptions.Timeout:
            logger.error(f"Timeout sending data to {endpoint}")
            return False
        except requests.exceptions.ConnectionError as e:
            logger.error(f"Connection error sending data to {endpoint}: {e}")
            return False
        except Exception as e:
            logger.error(f"Error sending data to cloud: {e}")
            return False
    
    def _add_to_retry_queue(self, data_type: str, data_id: int):
        """
        Add failed sync item to retry queue
        
        Args:
            data_type: Type of data (e.g., 'detection', 'telemetry')
            data_id: ID of the data record
        """
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                # Check if already in queue
                cursor.execute("""
                    SELECT id FROM sync_queue
                    WHERE data_type = ? AND data_id = ? AND status = 'pending'
                """, (data_type, data_id))
                
                if cursor.fetchone():
                    logger.debug(f"{data_type} {data_id} already in retry queue")
                    return
                
                # Add to queue
                cursor.execute("""
                    INSERT INTO sync_queue (data_type, data_id, status)
                    VALUES (?, ?, 'pending')
                """, (data_type, data_id))
                
                conn.commit()
                logger.debug(f"Added {data_type} {data_id} to retry queue")
                
        except Exception as e:
            logger.error(f"Error adding to retry queue: {e}")
    
    def _process_retry_queue(self):
        """Process items in retry queue"""
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                # Get pending items that are ready for retry
                cursor.execute("""
                    SELECT * FROM sync_queue
                    WHERE status = 'pending'
                    AND retry_count < ?
                    AND (last_attempt IS NULL OR 
                         datetime(last_attempt, '+' || ? || ' seconds') <= datetime('now'))
                    ORDER BY created_at ASC
                    LIMIT ?
                """, (self.max_retry_attempts, self.base_retry_delay, 10))
                
                retry_items = [dict(row) for row in cursor.fetchall()]
            
            if not retry_items:
                return
            
            logger.info(f"Processing {len(retry_items)} items from retry queue")
            
            for item in retry_items:
                self._retry_sync_item(item)
                
        except Exception as e:
            logger.error(f"Error processing retry queue: {e}")
    
    def _retry_sync_item(self, item: Dict[str, Any]):
        """
        Retry syncing a single item
        
        Args:
            item: Retry queue item
        """
        try:
            data_type = item['data_type']
            data_id = item['data_id']
            retry_count = item['retry_count']
            
            # Calculate exponential backoff delay
            delay = min(
                self.base_retry_delay * (2 ** retry_count),
                self.max_retry_delay
            )
            
            logger.info(f"Retrying {data_type} {data_id} (attempt {retry_count + 1})")
            
            # Get the data based on type
            success = False
            
            if data_type == 'detection':
                detection = self._get_detection_by_id(data_id)
                if detection:
                    batch_data = {
                        'gateway_id': self._get_gateway_id(),
                        'timestamp': datetime.now().isoformat(),
                        'data_type': 'detections',
                        'count': 1,
                        'detections': [detection]
                    }
                    success = self._send_to_cloud('/api/gateway/detections', batch_data)
                    
                    if success:
                        self.db.mark_detection_synced(data_id)
            
            # Update retry queue
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                if success:
                    # Mark as completed
                    cursor.execute("""
                        UPDATE sync_queue
                        SET status = 'completed', last_attempt = CURRENT_TIMESTAMP
                        WHERE id = ?
                    """, (item['id'],))
                    logger.info(f"Successfully synced {data_type} {data_id} on retry")
                else:
                    # Increment retry count
                    new_retry_count = retry_count + 1
                    
                    if new_retry_count >= self.max_retry_attempts:
                        # Max retries reached, mark as failed
                        cursor.execute("""
                            UPDATE sync_queue
                            SET status = 'failed', retry_count = ?, 
                                last_attempt = CURRENT_TIMESTAMP,
                                error_message = 'Max retry attempts reached'
                            WHERE id = ?
                        """, (new_retry_count, item['id']))
                        logger.error(f"Max retries reached for {data_type} {data_id}")
                    else:
                        # Update retry count
                        cursor.execute("""
                            UPDATE sync_queue
                            SET retry_count = ?, last_attempt = CURRENT_TIMESTAMP
                            WHERE id = ?
                        """, (new_retry_count, item['id']))
                        logger.warning(f"Retry {new_retry_count} failed for {data_type} {data_id}")
                
                conn.commit()
                
        except Exception as e:
            logger.error(f"Error retrying sync item: {e}")
    
    def _get_detection_by_id(self, detection_id: int) -> Optional[Dict[str, Any]]:
        """Get detection by ID"""
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("SELECT * FROM detections WHERE id = ?", (detection_id,))
                row = cursor.fetchone()
                return dict(row) if row else None
        except Exception as e:
            logger.error(f"Error getting detection {detection_id}: {e}")
            return None
    
    def _get_gateway_id(self) -> str:
        """Get gateway ID from config or generate one"""
        # This should be retrieved from the gateway config
        # For now, return a placeholder
        return "gateway_001"
    
    def sync_now(self):
        """Trigger immediate sync (non-blocking)"""
        if not self.running:
            logger.warning("Cloud sync service not running")
            return
        
        logger.info("Triggering immediate sync")
        # Simply wake up the sync thread
        self.stop_event.set()
        self.stop_event.clear()
    
    def get_stats(self) -> Dict[str, Any]:
        """
        Get sync statistics
        
        Returns:
            Dictionary containing sync statistics
        """
        with self.lock:
            stats = {
                'enabled': self.config.enabled,
                'running': self.running,
                'syncs_attempted': self.syncs_attempted,
                'syncs_succeeded': self.syncs_succeeded,
                'syncs_failed': self.syncs_failed,
                'last_sync_time': self.last_sync_time.isoformat() if self.last_sync_time else None,
                'success_rate': (self.syncs_succeeded / self.syncs_attempted * 100) 
                               if self.syncs_attempted > 0 else 0
            }
        
        # Get retry queue stats
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                cursor.execute("""
                    SELECT COUNT(*) as count FROM sync_queue WHERE status = 'pending'
                """)
                stats['retry_queue_pending'] = cursor.fetchone()['count']
                
                cursor.execute("""
                    SELECT COUNT(*) as count FROM sync_queue WHERE status = 'failed'
                """)
                stats['retry_queue_failed'] = cursor.fetchone()['count']
                
        except Exception as e:
            logger.error(f"Error getting retry queue stats: {e}")
            stats['retry_queue_pending'] = 0
            stats['retry_queue_failed'] = 0
        
        return stats
    
    def get_retry_queue_status(self) -> List[Dict[str, Any]]:
        """
        Get current retry queue status
        
        Returns:
            List of items in retry queue
        """
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    SELECT * FROM sync_queue
                    WHERE status IN ('pending', 'failed')
                    ORDER BY created_at DESC
                    LIMIT 100
                """)
                return [dict(row) for row in cursor.fetchall()]
        except Exception as e:
            logger.error(f"Error getting retry queue status: {e}")
            return []
    
    def clear_failed_items(self):
        """Clear failed items from retry queue"""
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("DELETE FROM sync_queue WHERE status = 'failed'")
                deleted = cursor.rowcount
                conn.commit()
                logger.info(f"Cleared {deleted} failed items from retry queue")
                return deleted
        except Exception as e:
            logger.error(f"Error clearing failed items: {e}")
            return 0
    
    def reset_pending_items(self):
        """Reset pending items for retry"""
        try:
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("""
                    UPDATE sync_queue
                    SET retry_count = 0, last_attempt = NULL
                    WHERE status = 'pending'
                """)
                updated = cursor.rowcount
                conn.commit()
                logger.info(f"Reset {updated} pending items in retry queue")
                return updated
        except Exception as e:
            logger.error(f"Error resetting pending items: {e}")
            return 0
