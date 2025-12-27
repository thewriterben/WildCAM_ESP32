"""
Cloud Synchronization Service
Handles robust data sync with WildCAM cloud API including offline queue and retry logic
"""

import requests
import json
import time
import logging
from typing import Dict, List, Optional, Any
from pathlib import Path
from dataclasses import dataclass, asdict
from enum import Enum
import queue
import threading
from datetime import datetime, timedelta
import sqlite3
import base64

logger = logging.getLogger(__name__)


class SyncStatus(Enum):
    """Sync item status"""
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    COMPLETED = "completed"
    FAILED = "failed"
    RETRY = "retry"


@dataclass
class SyncItem:
    """Item to be synchronized"""
    id: str
    type: str  # 'detection', 'image', 'telemetry'
    data: Dict
    priority: int = 5  # 1-10, higher = more important
    created_at: float = None
    attempts: int = 0
    status: SyncStatus = SyncStatus.PENDING
    last_error: Optional[str] = None
    
    def __post_init__(self):
        if self.created_at is None:
            self.created_at = time.time()


class CloudSyncService:
    """
    Manages synchronization with WildCAM cloud backend
    Provides offline queue, retry logic, and batch operations
    """
    
    def __init__(
        self,
        api_url: str,
        api_key: str,
        sync_interval: int = 300,
        max_queue_size: int = 1000,
        max_retries: int = 5,
        batch_size: int = 50,
        offline_db_path: str = "sync_queue.db"
    ):
        """
        Initialize cloud sync service
        
        Args:
            api_url: Base URL of WildCAM cloud API
            api_key: API authentication key
            sync_interval: Sync interval in seconds
            max_queue_size: Maximum items in memory queue
            max_retries: Maximum retry attempts per item
            batch_size: Number of items to sync in one batch
            offline_db_path: Path to offline queue database
        """
        self.api_url = api_url.rstrip('/')
        self.api_key = api_key
        self.sync_interval = sync_interval
        self.max_queue_size = max_queue_size
        self.max_retries = max_retries
        self.batch_size = batch_size
        self.offline_db_path = Path(offline_db_path)
        
        # In-memory queue
        self.sync_queue = queue.PriorityQueue(maxsize=max_queue_size)
        
        # Threading
        self.running = False
        self.sync_thread = None
        
        # Statistics
        self.total_synced = 0
        self.total_failed = 0
        self.last_sync_time = 0
        self.cloud_available = False
        
        # Initialize offline database
        self._init_offline_db()
        
        # HTTP session
        self.session = requests.Session()
        self.session.headers.update({
            'Authorization': f'Bearer {self.api_key}',
            'Content-Type': 'application/json'
        })
    
    def _init_offline_db(self):
        """Initialize SQLite database for offline queue"""
        self.offline_db_path.parent.mkdir(parents=True, exist_ok=True)
        
        conn = sqlite3.connect(str(self.offline_db_path))
        cursor = conn.cursor()
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS sync_queue (
                id TEXT PRIMARY KEY,
                type TEXT NOT NULL,
                data TEXT NOT NULL,
                priority INTEGER NOT NULL,
                created_at REAL NOT NULL,
                attempts INTEGER DEFAULT 0,
                status TEXT DEFAULT 'pending',
                last_error TEXT,
                updated_at REAL
            )
        ''')
        
        cursor.execute('''
            CREATE INDEX IF NOT EXISTS idx_status_priority
            ON sync_queue(status, priority DESC, created_at)
        ''')
        
        conn.commit()
        conn.close()
        
        logger.info(f"Initialized offline queue database: {self.offline_db_path}")
    
    def start(self):
        """Start sync service"""
        if self.running:
            logger.warning("Sync service already running")
            return
        
        logger.info("Starting cloud sync service")
        self.running = True
        
        # Load pending items from offline database
        self._load_from_offline_db()
        
        # Start sync thread
        self.sync_thread = threading.Thread(target=self._sync_loop, daemon=True)
        self.sync_thread.start()
        
        logger.info("Cloud sync service started")
    
    def stop(self):
        """Stop sync service"""
        logger.info("Stopping cloud sync service")
        self.running = False
        
        if self.sync_thread:
            self.sync_thread.join(timeout=10.0)
        
        # Save pending items to offline database
        self._save_to_offline_db()
        
        logger.info("Cloud sync service stopped")
    
    def add_detection(
        self,
        camera_name: str,
        timestamp: float,
        detections: List[Dict],
        image_path: Optional[str] = None,
        priority: int = 7
    ):
        """
        Add detection result to sync queue
        
        Args:
            camera_name: Camera identifier
            timestamp: Detection timestamp
            detections: List of detection objects
            image_path: Optional path to image file
            priority: Sync priority (1-10)
        """
        item_id = f"det_{camera_name}_{int(timestamp * 1000)}"
        
        data = {
            'camera_name': camera_name,
            'timestamp': timestamp,
            'detections': detections,
            'image_path': image_path
        }
        
        item = SyncItem(
            id=item_id,
            type='detection',
            data=data,
            priority=priority
        )
        
        self._add_to_queue(item)
    
    def add_telemetry(
        self,
        device_id: str,
        metrics: Dict,
        priority: int = 3
    ):
        """
        Add telemetry data to sync queue
        
        Args:
            device_id: Device/camera identifier
            metrics: Telemetry metrics
            priority: Sync priority (1-10)
        """
        item_id = f"telem_{device_id}_{int(time.time() * 1000)}"
        
        data = {
            'device_id': device_id,
            'timestamp': time.time(),
            'metrics': metrics
        }
        
        item = SyncItem(
            id=item_id,
            type='telemetry',
            data=data,
            priority=priority
        )
        
        self._add_to_queue(item)
    
    def add_esp32_data(
        self,
        node_id: str,
        data: Dict,
        priority: int = 5
    ):
        """
        Add ESP32 field node data to sync queue
        
        Args:
            node_id: ESP32 node identifier
            data: Node data payload
            priority: Sync priority (1-10)
        """
        item_id = f"esp32_{node_id}_{int(time.time() * 1000)}"
        
        sync_data = {
            'node_id': node_id,
            'timestamp': time.time(),
            'payload': data
        }
        
        item = SyncItem(
            id=item_id,
            type='esp32_data',
            data=sync_data,
            priority=priority
        )
        
        self._add_to_queue(item)
    
    def _add_to_queue(self, item: SyncItem):
        """Add item to sync queue"""
        try:
            # Priority queue uses (priority, item) tuples
            # Negate priority so higher values are processed first
            self.sync_queue.put_nowait((-item.priority, item))
            logger.debug(f"Added to sync queue: {item.id} (priority {item.priority})")
        except queue.Full:
            logger.warning("Sync queue full, saving to offline database")
            self._save_item_to_db(item)
    
    def _sync_loop(self):
        """Main sync loop running in separate thread"""
        logger.info("Sync loop started")
        
        while self.running:
            try:
                # Check cloud availability periodically
                if time.time() - self.last_sync_time >= self.sync_interval:
                    self.cloud_available = self._check_cloud_connection()
                    self.last_sync_time = time.time()
                
                if not self.cloud_available:
                    logger.warning("Cloud unavailable, queueing items offline")
                    time.sleep(self.sync_interval)
                    continue
                
                # Process batch of items
                batch = self._get_batch()
                
                if len(batch) == 0:
                    time.sleep(1.0)
                    continue
                
                # Sync batch
                self._sync_batch(batch)
                
            except Exception as e:
                logger.error(f"Error in sync loop: {e}")
                time.sleep(5.0)
        
        logger.info("Sync loop stopped")
    
    def _get_batch(self) -> List[SyncItem]:
        """Get batch of items from queue"""
        batch = []
        
        while len(batch) < self.batch_size:
            try:
                _, item = self.sync_queue.get_nowait()
                batch.append(item)
            except queue.Empty:
                break
        
        return batch
    
    def _sync_batch(self, batch: List[SyncItem]):
        """
        Synchronize batch of items
        
        Args:
            batch: List of sync items
        """
        logger.info(f"Syncing batch of {len(batch)} items")
        
        for item in batch:
            try:
                # Update status
                item.status = SyncStatus.IN_PROGRESS
                item.attempts += 1
                
                # Sync based on type
                if item.type == 'detection':
                    success = self._sync_detection(item)
                elif item.type == 'telemetry':
                    success = self._sync_telemetry(item)
                elif item.type == 'esp32_data':
                    success = self._sync_esp32_data(item)
                else:
                    logger.error(f"Unknown sync type: {item.type}")
                    success = False
                
                if success:
                    item.status = SyncStatus.COMPLETED
                    self.total_synced += 1
                    logger.info(f"Successfully synced: {item.id}")
                else:
                    # Retry or fail
                    if item.attempts >= self.max_retries:
                        item.status = SyncStatus.FAILED
                        self.total_failed += 1
                        logger.error(f"Max retries exceeded: {item.id}")
                        self._save_item_to_db(item)  # Save failed item
                    else:
                        item.status = SyncStatus.RETRY
                        logger.warning(f"Retry scheduled: {item.id} (attempt {item.attempts})")
                        # Re-add to queue (backoff handled by next sync interval)
                        self._add_to_queue(item)
                
            except Exception as e:
                logger.error(f"Error syncing item {item.id}: {e}")
                item.last_error = str(e)
                item.status = SyncStatus.RETRY
                self._add_to_queue(item)
    
    def _sync_detection(self, item: SyncItem) -> bool:
        """
        Sync detection data to cloud
        
        Args:
            item: Detection sync item
            
        Returns:
            True if successful
        """
        endpoint = f"{self.api_url}/api/detections"
        
        payload = {
            'camera_name': item.data['camera_name'],
            'timestamp': item.data['timestamp'],
            'detections': item.data['detections']
        }
        
        # Upload image if provided
        if item.data.get('image_path'):
            image_url = self._upload_image(item.data['image_path'])
            if image_url:
                payload['image_url'] = image_url
        
        try:
            response = self.session.post(endpoint, json=payload, timeout=30)
            response.raise_for_status()
            return True
        except requests.exceptions.RequestException as e:
            logger.error(f"Failed to sync detection: {e}")
            item.last_error = str(e)
            return False
    
    def _sync_telemetry(self, item: SyncItem) -> bool:
        """
        Sync telemetry data to cloud
        
        Args:
            item: Telemetry sync item
            
        Returns:
            True if successful
        """
        endpoint = f"{self.api_url}/api/telemetry"
        
        payload = {
            'device_id': item.data['device_id'],
            'timestamp': item.data['timestamp'],
            'metrics': item.data['metrics']
        }
        
        try:
            response = self.session.post(endpoint, json=payload, timeout=30)
            response.raise_for_status()
            return True
        except requests.exceptions.RequestException as e:
            logger.error(f"Failed to sync telemetry: {e}")
            item.last_error = str(e)
            return False
    
    def _sync_esp32_data(self, item: SyncItem) -> bool:
        """
        Sync ESP32 field node data to cloud
        
        Args:
            item: ESP32 data sync item
            
        Returns:
            True if successful
        """
        endpoint = f"{self.api_url}/api/field_nodes/{item.data['node_id']}/data"
        
        payload = {
            'timestamp': item.data['timestamp'],
            'data': item.data['payload']
        }
        
        try:
            response = self.session.post(endpoint, json=payload, timeout=30)
            response.raise_for_status()
            return True
        except requests.exceptions.RequestException as e:
            logger.error(f"Failed to sync ESP32 data: {e}")
            item.last_error = str(e)
            return False
    
    def _upload_image(self, image_path: str) -> Optional[str]:
        """
        Upload image to cloud storage
        
        Args:
            image_path: Path to image file
            
        Returns:
            Image URL if successful, None otherwise
        """
        endpoint = f"{self.api_url}/api/images/upload"
        
        try:
            with open(image_path, 'rb') as f:
                files = {'image': f}
                response = self.session.post(endpoint, files=files, timeout=60)
                response.raise_for_status()
                
                result = response.json()
                return result.get('url')
        except Exception as e:
            logger.error(f"Failed to upload image {image_path}: {e}")
            return None
    
    def _check_cloud_connection(self) -> bool:
        """
        Check if cloud API is reachable
        
        Returns:
            True if cloud is available
        """
        endpoint = f"{self.api_url}/api/health"
        
        try:
            response = self.session.get(endpoint, timeout=5)
            return response.status_code == 200
        except requests.exceptions.RequestException:
            return False
    
    def _save_item_to_db(self, item: SyncItem):
        """Save sync item to offline database"""
        conn = sqlite3.connect(str(self.offline_db_path))
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT OR REPLACE INTO sync_queue
            (id, type, data, priority, created_at, attempts, status, last_error, updated_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            item.id,
            item.type,
            json.dumps(item.data),
            item.priority,
            item.created_at,
            item.attempts,
            item.status.value,
            item.last_error,
            time.time()
        ))
        
        conn.commit()
        conn.close()
    
    def _load_from_offline_db(self):
        """Load pending items from offline database"""
        conn = sqlite3.connect(str(self.offline_db_path))
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT id, type, data, priority, created_at, attempts, status, last_error
            FROM sync_queue
            WHERE status IN ('pending', 'retry', 'in_progress')
            ORDER BY priority DESC, created_at
            LIMIT ?
        ''', (self.max_queue_size,))
        
        rows = cursor.fetchall()
        conn.close()
        
        for row in rows:
            item = SyncItem(
                id=row[0],
                type=row[1],
                data=json.loads(row[2]),
                priority=row[3],
                created_at=row[4],
                attempts=row[5],
                status=SyncStatus(row[6]),
                last_error=row[7]
            )
            self._add_to_queue(item)
        
        logger.info(f"Loaded {len(rows)} items from offline database")
    
    def _save_to_offline_db(self):
        """Save pending items to offline database"""
        items = []
        
        # Get all items from queue
        while True:
            try:
                _, item = self.sync_queue.get_nowait()
                items.append(item)
            except queue.Empty:
                break
        
        for item in items:
            self._save_item_to_db(item)
        
        logger.info(f"Saved {len(items)} items to offline database")
    
    def get_stats(self) -> Dict:
        """Get sync service statistics"""
        return {
            'running': self.running,
            'cloud_available': self.cloud_available,
            'queue_size': self.sync_queue.qsize(),
            'total_synced': self.total_synced,
            'total_failed': self.total_failed,
            'last_sync_time': self.last_sync_time
        }
    
    def __del__(self):
        """Cleanup on deletion"""
        self.stop()
