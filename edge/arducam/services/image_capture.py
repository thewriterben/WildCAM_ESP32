"""
Image Capture Service
Triggers high-resolution captures on wildlife detections
"""

import logging
import os
from datetime import datetime, timedelta
from pathlib import Path
from typing import Optional, Dict, Set
import threading
from collections import defaultdict

logger = logging.getLogger(__name__)


class ImageCaptureService:
    """
    Manages high-resolution image capture on detection
    Includes species filtering, cooldown, and storage management
    """
    
    def __init__(self, config: Dict):
        """
        Initialize image capture service
        
        Args:
            config: Configuration dictionary from YAML
        """
        self.config = config.get('capture', {})
        
        # Configuration
        self.enabled = self.config.get('enabled', True)
        self.trigger_species = set(self.config.get('trigger_species', []))
        self.capture_threshold = self.config.get('capture_threshold', 0.6)
        self.save_path = self.config.get('save_path', '/data/captures')
        self.max_storage_gb = self.config.get('max_storage_gb', 32)
        self.cooldown_seconds = self.config.get('cooldown_seconds', 30)
        self.embed_metadata = self.config.get('embed_metadata', True)
        self.format = self.config.get('format', 'JPEG')
        self.quality = self.config.get('quality', 95)
        
        # State
        self.last_capture_time: Dict[str, datetime] = defaultdict(lambda: datetime.min)
        self.total_captures = 0
        self.storage_lock = threading.Lock()
        
        # Ensure save path exists
        Path(self.save_path).mkdir(parents=True, exist_ok=True)
        
        logger.info(f"ImageCaptureService initialized (enabled={self.enabled})")
    
    def should_capture(self, species: str, confidence: float) -> bool:
        """
        Determine if image should be captured for this detection
        
        Args:
            species: Detected species name
            confidence: Detection confidence
            
        Returns:
            True if should capture, False otherwise
        """
        if not self.enabled:
            return False
        
        # Check confidence threshold
        if confidence < self.capture_threshold:
            return False
        
        # Check if species is in trigger list (empty list = capture all)
        if self.trigger_species and species not in self.trigger_species:
            return False
        
        # Check cooldown
        last_capture = self.last_capture_time.get(species, datetime.min)
        time_since_last = datetime.now() - last_capture
        
        if time_since_last.total_seconds() < self.cooldown_seconds:
            logger.debug(f"Cooldown active for {species} ({time_since_last.total_seconds():.1f}s)")
            return False
        
        # Check storage space
        if not self._check_storage_available():
            logger.warning("Storage limit reached, skipping capture")
            return False
        
        return True
    
    def capture_image(self, detector, detection) -> Optional[str]:
        """
        Capture high-resolution image
        
        Args:
            detector: IMX500WildlifeDetector instance
            detection: IMX500Detection object
            
        Returns:
            Path to saved image, or None if capture failed
        """
        try:
            # Generate filename
            timestamp_str = detection.timestamp.strftime('%Y%m%d_%H%M%S')
            filename = f"{detection.species}_{timestamp_str}_{detection.confidence:.2f}.jpg"
            filepath = os.path.join(self.save_path, filename)
            
            # Capture high-res image
            logger.info(f"Capturing high-res image for {detection.species}")
            image_bytes = detector.capture_high_res(save_path=filepath)
            
            if image_bytes:
                # Embed metadata if enabled
                if self.embed_metadata:
                    self._embed_metadata(filepath, detection)
                
                # Update state
                self.last_capture_time[detection.species] = datetime.now()
                self.total_captures += 1
                
                logger.info(f"Image captured: {filepath}")
                return filepath
            else:
                logger.error("Failed to capture image")
                return None
                
        except Exception as e:
            logger.error(f"Image capture error: {e}")
            return None
    
    def _embed_metadata(self, filepath: str, detection):
        """
        Embed detection metadata into image EXIF
        
        Args:
            filepath: Path to image file
            detection: Detection object
        """
        try:
            from PIL import Image
            from PIL.ExifTags import TAGS
            import piexif
            
            # Load image
            img = Image.open(filepath)
            
            # Create EXIF data
            exif_dict = {
                "0th": {},
                "Exif": {},
                "GPS": {},
                "1st": {},
                "thumbnail": None
            }
            
            # Add custom fields
            exif_dict["0th"][piexif.ImageIFD.ImageDescription] = (
                f"WildCAM Detection: {detection.species} "
                f"(confidence: {detection.confidence:.2f})"
            )
            
            exif_dict["0th"][piexif.ImageIFD.Artist] = "WildCAM Arducam Pivistation"
            exif_dict["0th"][piexif.ImageIFD.Software] = "IMX500 Wildlife Detector v1.0"
            
            # Convert to bytes
            exif_bytes = piexif.dump(exif_dict)
            
            # Save with EXIF
            img.save(filepath, "JPEG", quality=self.quality, exif=exif_bytes)
            
            logger.debug(f"Metadata embedded in {filepath}")
            
        except ImportError:
            logger.warning("piexif not installed, skipping metadata embedding")
        except Exception as e:
            logger.error(f"Failed to embed metadata: {e}")
    
    def _check_storage_available(self) -> bool:
        """
        Check if storage is available
        
        Returns:
            True if storage available, False if limit reached
        """
        try:
            # Get directory size
            total_size = sum(
                f.stat().st_size
                for f in Path(self.save_path).rglob('*')
                if f.is_file()
            )
            
            size_gb = total_size / (1024 ** 3)
            
            if size_gb >= self.max_storage_gb:
                logger.warning(f"Storage limit reached: {size_gb:.2f} GB / {self.max_storage_gb} GB")
                
                # Trigger cleanup
                self._cleanup_old_images()
                
                # Check again
                total_size = sum(
                    f.stat().st_size
                    for f in Path(self.save_path).rglob('*')
                    if f.is_file()
                )
                size_gb = total_size / (1024 ** 3)
                
                return size_gb < self.max_storage_gb
            
            return True
            
        except Exception as e:
            logger.error(f"Error checking storage: {e}")
            return True  # Assume available on error
    
    def _cleanup_old_images(self):
        """Remove oldest images to free up space"""
        try:
            with self.storage_lock:
                # Get all image files sorted by modification time
                files = sorted(
                    Path(self.save_path).rglob('*.jpg'),
                    key=lambda f: f.stat().st_mtime
                )
                
                # Remove oldest 20%
                num_to_remove = max(1, len(files) // 5)
                
                for file in files[:num_to_remove]:
                    try:
                        file.unlink()
                        logger.info(f"Removed old image: {file.name}")
                    except Exception as e:
                        logger.error(f"Failed to remove {file}: {e}")
                
                logger.info(f"Cleaned up {num_to_remove} old images")
                
        except Exception as e:
            logger.error(f"Cleanup error: {e}")
    
    def get_stats(self) -> Dict:
        """
        Get capture statistics
        
        Returns:
            Dictionary with capture stats
        """
        try:
            # Count files and calculate total size
            files = list(Path(self.save_path).rglob('*.jpg'))
            total_size = sum(f.stat().st_size for f in files)
            
            return {
                'enabled': self.enabled,
                'total_captures': self.total_captures,
                'total_files': len(files),
                'storage_used_gb': total_size / (1024 ** 3),
                'storage_limit_gb': self.max_storage_gb,
                'storage_percent': (total_size / (self.max_storage_gb * 1024 ** 3)) * 100,
                'trigger_species': list(self.trigger_species),
            }
        except Exception as e:
            logger.error(f"Error getting stats: {e}")
            return {
                'enabled': self.enabled,
                'error': str(e)
            }
