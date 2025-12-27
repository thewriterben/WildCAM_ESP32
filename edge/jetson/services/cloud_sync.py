"""
Cloud Sync Service - Async cloud synchronization with retry queue

Handles uploading detections, telemetry, images, and videos to cloud storage
with offline operation support and exponential backoff retry logic.
"""

import asyncio
import aiosqlite
from typing import Dict, Any, List, Optional
from dataclasses import dataclass, asdict
from datetime import datetime, timedelta
from pathlib import Path
import logging
import json

from edge.shared.protocols.cloud_api import CloudAPIClient
from edge.shared.utils.db_helpers import DatabaseManager

logger = logging.getLogger(__name__)


@dataclass
class SyncQueueItem:
    """Item in sync queue"""
    id: Optional[int] = None
    item_type: str = "detection"  # detection, telemetry, image, video
    payload: Dict[str, Any] = None
    created_at: datetime = None
    retry_count: int = 0
    next_retry: datetime = None
    status: str = "pending"  # pending, syncing, synced, failed
    
    def __post_init__(self):
        if self.created_at is None:
            self.created_at = datetime.utcnow()
        if self.next_retry is None:
            self.next_retry = datetime.utcnow()


class CloudSyncService:
    """
    Cloud synchronization service with offline queue and retry logic
    """
    
    def __init__(
        self,
        db_path: str,
        cloud_api_url: str,
        cloud_api_key: str,
        batch_size: int = 10,
        sync_interval: int = 60,
        max_retry_count: int = 5,
        retry_base_delay: int = 60
    ):
        """
        Initialize cloud sync service
        
        Args:
            db_path: Path to sync queue database
            cloud_api_url: Cloud API base URL
            cloud_api_key: Cloud API authentication key
            batch_size: Number of items to sync per batch
            sync_interval: Sync interval in seconds
            max_retry_count: Maximum retry attempts
            retry_base_delay: Base delay for exponential backoff (seconds)
        """
        self.db = DatabaseManager(db_path)
        self.cloud_api_url = cloud_api_url
        self.cloud_api_key = cloud_api_key
        self.batch_size = batch_size
        self.sync_interval = sync_interval
        self.max_retry_count = max_retry_count
        self.retry_base_delay = retry_base_delay
        
        # Statistics
        self.stats = {
            'synced': 0,
            'failed': 0,
            'bytes_uploaded': 0,
            'last_sync': None
        }
        
        # Control
        self.running = False
        self.sync_task: Optional[asyncio.Task] = None
        
    async def initialize(self):
        """Initialize database schema"""
        schema = """
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            item_type TEXT NOT NULL,
            payload TEXT NOT NULL,
            created_at TEXT NOT NULL,
            retry_count INTEGER DEFAULT 0,
            next_retry TEXT NOT NULL,
            status TEXT DEFAULT 'pending'
        """
        await self.db.create_table("sync_queue", schema)
        logger.info("Cloud sync service initialized")
        
    async def queue_item(self, item_type: str, payload: Dict[str, Any]) -> int:
        """
        Add item to sync queue
        
        Args:
            item_type: Type of item (detection, telemetry, image, video)
            payload: Item data
            
        Returns:
            Queue item ID
        """
        item = SyncQueueItem(
            item_type=item_type,
            payload=payload
        )
        
        query = """
            INSERT INTO sync_queue (item_type, payload, created_at, retry_count, next_retry, status)
            VALUES (?, ?, ?, ?, ?, ?)
        """
        params = (
            item.item_type,
            json.dumps(item.payload),
            item.created_at.isoformat(),
            item.retry_count,
            item.next_retry.isoformat(),
            item.status
        )
        
        await self.db.execute(query, params)
        
        # Get the ID of inserted item
        result = await self.db.fetchone("SELECT last_insert_rowid() as id")
        item_id = result['id']
        
        logger.debug(f"Queued {item_type} item {item_id}")
        return item_id
        
    async def get_pending_items(self) -> List[Dict[str, Any]]:
        """
        Get pending items ready for sync
        
        Returns:
            List of pending items
        """
        query = """
            SELECT * FROM sync_queue
            WHERE status = 'pending'
            AND next_retry <= ?
            ORDER BY created_at ASC
            LIMIT ?
        """
        items = await self.db.fetchall(query, (datetime.utcnow().isoformat(), self.batch_size))
        
        # Parse JSON payloads
        for item in items:
            item['payload'] = json.loads(item['payload'])
            item['created_at'] = datetime.fromisoformat(item['created_at'])
            item['next_retry'] = datetime.fromisoformat(item['next_retry'])
            
        return items
        
    async def sync_batch(self):
        """Sync a batch of pending items"""
        items = await self.get_pending_items()
        
        if not items:
            return
            
        logger.info(f"Syncing batch of {len(items)} items")
        
        async with CloudAPIClient(self.cloud_api_url, self.cloud_api_key) as api:
            # Check cloud connectivity
            if not await api.health_check():
                logger.warning("Cloud API unreachable, skipping sync")
                return
                
            for item in items:
                try:
                    # Mark as syncing
                    await self._update_item_status(item['id'], 'syncing')
                    
                    # Sync based on type
                    success = False
                    if item['item_type'] == 'detection':
                        success = await api.upload_detection(item['payload'])
                    elif item['item_type'] == 'telemetry':
                        success = await api.upload_telemetry([item['payload']])
                    elif item['item_type'] == 'image':
                        # Image data should be in payload['data'] as bytes
                        image_data = item['payload'].pop('data', b'')
                        success = await api.upload_image(image_data, item['payload']) is not None
                    elif item['item_type'] == 'video':
                        # Similar to image
                        logger.warning("Video sync not yet implemented")
                        success = False
                        
                    if success:
                        await self._update_item_status(item['id'], 'synced')
                        self.stats['synced'] += 1
                        self.stats['bytes_uploaded'] += len(json.dumps(item['payload']))
                    else:
                        await self._handle_sync_failure(item)
                        
                except Exception as e:
                    logger.error(f"Sync error for item {item['id']}: {e}")
                    await self._handle_sync_failure(item)
                    
        self.stats['last_sync'] = datetime.utcnow()
        
    async def _update_item_status(self, item_id: int, status: str):
        """Update item status"""
        query = "UPDATE sync_queue SET status = ? WHERE id = ?"
        await self.db.execute(query, (status, item_id))
        
    async def _handle_sync_failure(self, item: Dict[str, Any]):
        """Handle sync failure with exponential backoff"""
        retry_count = item['retry_count'] + 1
        
        if retry_count >= self.max_retry_count:
            # Max retries exceeded, mark as failed
            await self._update_item_status(item['id'], 'failed')
            self.stats['failed'] += 1
            logger.error(f"Item {item['id']} failed after {retry_count} retries")
        else:
            # Schedule retry with exponential backoff
            delay = self.retry_base_delay * (2 ** retry_count)
            next_retry = datetime.utcnow() + timedelta(seconds=delay)
            
            query = """
                UPDATE sync_queue
                SET retry_count = ?, next_retry = ?, status = 'pending'
                WHERE id = ?
            """
            await self.db.execute(query, (retry_count, next_retry.isoformat(), item['id']))
            logger.warning(f"Item {item['id']} retry scheduled for {next_retry}")
            
    async def cleanup_old_items(self, days: int = 7):
        """
        Remove old synced items from queue
        
        Args:
            days: Remove synced items older than this many days
        """
        cutoff = datetime.utcnow() - timedelta(days=days)
        query = "DELETE FROM sync_queue WHERE status = 'synced' AND created_at < ?"
        await self.db.execute(query, (cutoff.isoformat(),))
        logger.info(f"Cleaned up synced items older than {days} days")
        
    async def start(self):
        """Start sync service"""
        if self.running:
            logger.warning("Cloud sync service already running")
            return
            
        self.running = True
        logger.info("Starting cloud sync service")
        
        # Start sync loop
        self.sync_task = asyncio.create_task(self._sync_loop())
        
    async def stop(self):
        """Stop sync service"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping cloud sync service")
        
        if self.sync_task:
            self.sync_task.cancel()
            try:
                await self.sync_task
            except asyncio.CancelledError:
                pass
                
    async def _sync_loop(self):
        """Main sync loop"""
        while self.running:
            try:
                await self.sync_batch()
            except Exception as e:
                logger.error(f"Sync loop error: {e}")
                
            # Wait for next sync interval
            await asyncio.sleep(self.sync_interval)
            
    def get_statistics(self) -> Dict[str, Any]:
        """Get sync statistics"""
        return {
            **self.stats,
            'last_sync': self.stats['last_sync'].isoformat() if self.stats['last_sync'] else None
        }
