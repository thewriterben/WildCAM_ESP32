"""
Cloud API Client for edge gateways

Handles communication with WildCAM cloud backend services
"""

import asyncio
import aiohttp
import logging
from typing import Optional, Dict, Any, List
from datetime import datetime
import json

logger = logging.getLogger(__name__)


class CloudAPIClient:
    """Async client for WildCAM cloud API"""
    
    def __init__(self, base_url: str, api_key: str, timeout: int = 30):
        """
        Initialize cloud API client
        
        Args:
            base_url: Cloud API base URL (e.g., https://api.wildcam.example.com)
            api_key: API key for authentication
            timeout: Request timeout in seconds
        """
        self.base_url = base_url.rstrip('/')
        self.api_key = api_key
        self.timeout = aiohttp.ClientTimeout(total=timeout)
        self.session: Optional[aiohttp.ClientSession] = None
        
    async def __aenter__(self):
        """Async context manager entry"""
        self.session = aiohttp.ClientSession(
            headers={
                'Authorization': f'Bearer {self.api_key}',
                'Content-Type': 'application/json'
            },
            timeout=self.timeout
        )
        return self
        
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        """Async context manager exit"""
        if self.session:
            await self.session.close()
            
    async def upload_detection(self, detection: Dict[str, Any]) -> bool:
        """
        Upload wildlife detection to cloud
        
        Args:
            detection: Detection data dictionary
            
        Returns:
            True if successful, False otherwise
        """
        try:
            async with self.session.post(
                f'{self.base_url}/api/detections',
                json=detection
            ) as response:
                if response.status == 200:
                    logger.info(f"Detection uploaded successfully: {detection.get('species')}")
                    return True
                else:
                    logger.error(f"Detection upload failed: {response.status}")
                    return False
        except Exception as e:
            logger.error(f"Detection upload error: {e}")
            return False
            
    async def upload_image(self, image_data: bytes, metadata: Dict[str, Any]) -> Optional[str]:
        """
        Upload image with metadata to cloud storage
        
        Args:
            image_data: Raw image bytes
            metadata: Image metadata
            
        Returns:
            Image URL if successful, None otherwise
        """
        try:
            data = aiohttp.FormData()
            data.add_field('image', image_data, 
                          filename=f"{metadata.get('node_id')}_{metadata.get('timestamp')}.jpg",
                          content_type='image/jpeg')
            data.add_field('metadata', json.dumps(metadata))
            
            async with self.session.post(
                f'{self.base_url}/api/images',
                data=data
            ) as response:
                if response.status == 200:
                    result = await response.json()
                    image_url = result.get('url')
                    logger.info(f"Image uploaded: {image_url}")
                    return image_url
                else:
                    logger.error(f"Image upload failed: {response.status}")
                    return None
        except Exception as e:
            logger.error(f"Image upload error: {e}")
            return None
            
    async def upload_telemetry(self, telemetry: List[Dict[str, Any]]) -> bool:
        """
        Upload batch of telemetry data
        
        Args:
            telemetry: List of telemetry records
            
        Returns:
            True if successful, False otherwise
        """
        try:
            async with self.session.post(
                f'{self.base_url}/api/telemetry/batch',
                json={'records': telemetry}
            ) as response:
                if response.status == 200:
                    logger.info(f"Telemetry batch uploaded: {len(telemetry)} records")
                    return True
                else:
                    logger.error(f"Telemetry upload failed: {response.status}")
                    return False
        except Exception as e:
            logger.error(f"Telemetry upload error: {e}")
            return False
            
    async def get_node_config(self, node_id: int) -> Optional[Dict[str, Any]]:
        """
        Fetch configuration for a specific node
        
        Args:
            node_id: ESP32 node ID
            
        Returns:
            Configuration dictionary if successful, None otherwise
        """
        try:
            async with self.session.get(
                f'{self.base_url}/api/nodes/{node_id}/config'
            ) as response:
                if response.status == 200:
                    config = await response.json()
                    logger.info(f"Config fetched for node {node_id}")
                    return config
                else:
                    logger.warning(f"Config fetch failed for node {node_id}: {response.status}")
                    return None
        except Exception as e:
            logger.error(f"Config fetch error: {e}")
            return None
            
    async def health_check(self) -> bool:
        """
        Check cloud API health
        
        Returns:
            True if cloud is reachable, False otherwise
        """
        try:
            async with self.session.get(f'{self.base_url}/health') as response:
                return response.status == 200
        except Exception:
            return False
