"""
LoRa Metadata Transmitter
Sends compact detection metadata via LoRa
Compatible with WildCAM ESP32 LoRa mesh protocol
"""

import logging
import time
from typing import Optional, Dict, List
from queue import Queue, Empty
from datetime import datetime
import threading

logger = logging.getLogger(__name__)


class LoRaTransmitter:
    """
    Transmits detection metadata via LoRa radio
    Uses compact binary encoding for efficient transmission
    """
    
    def __init__(self, config: Dict):
        """
        Initialize LoRa transmitter
        
        Args:
            config: Configuration dictionary from YAML
        """
        self.config = config.get('lora', {})
        
        # Configuration
        self.enabled = self.config.get('enabled', True)
        self.frequency = self.config.get('frequency', 915.0)
        self.spreading_factor = self.config.get('spreading_factor', 10)
        self.bandwidth = self.config.get('bandwidth', 125)
        self.tx_power = self.config.get('tx_power', 17)
        self.max_retries = self.config.get('max_retries', 3)
        self.retry_delay = self.config.get('retry_delay', 1.0)
        
        # Queue for outgoing messages
        self.queue = Queue(maxsize=self.config.get('max_queue_size', 100))
        
        # LoRa radio
        self.radio = None
        
        # Transmission thread
        self.running = False
        self.tx_thread = None
        
        # Statistics
        self.stats = {
            'total_sent': 0,
            'total_failed': 0,
            'total_retries': 0,
            'queue_overflows': 0,
        }
        
        if self.enabled:
            self._initialize_radio()
    
    def _initialize_radio(self) -> bool:
        """
        Initialize LoRa radio hardware
        
        Returns:
            True if successful, False otherwise
        """
        try:
            # Import LoRa library (SX127x)
            try:
                from LoRa import LoRa, BOARD
            except ImportError:
                logger.warning("LoRa library not available")
                logger.info("Install with: pip install pyLoRa")
                logger.info("For development, LoRa transmission will be simulated")
                return False
            
            # Setup board pins
            BOARD.setup()
            
            # Create LoRa instance
            self.radio = LoRa(
                verbose=False,
                do_calibration=True,
                calibration_freq=self.frequency
            )
            
            # Configure radio
            self.radio.set_mode(0)  # Sleep mode
            self.radio.set_freq(self.frequency)
            self.radio.set_spreading_factor(self.spreading_factor)
            self.radio.set_bw(self.bandwidth * 1000)  # Convert to Hz
            self.radio.set_pa_config(pa_select=1, max_power=21, output_power=self.tx_power)
            
            logger.info(f"LoRa radio initialized: {self.frequency} MHz, SF{self.spreading_factor}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize LoRa radio: {e}")
            return False
    
    def start(self):
        """Start transmission thread"""
        if not self.enabled:
            logger.info("LoRa transmitter disabled")
            return
        
        if self.running:
            logger.warning("LoRa transmitter already running")
            return
        
        self.running = True
        
        # Start transmission thread
        self.tx_thread = threading.Thread(target=self._transmission_loop)
        self.tx_thread.daemon = True
        self.tx_thread.start()
        
        logger.info("LoRa transmitter started")
    
    def stop(self):
        """Stop transmission thread"""
        if not self.running:
            return
        
        self.running = False
        
        if self.tx_thread:
            self.tx_thread.join(timeout=5.0)
        
        # Cleanup radio
        if self.radio:
            try:
                from LoRa import BOARD
                BOARD.teardown()
            except Exception as e:
                logger.error(f"Error cleaning up LoRa radio: {e}")
        
        logger.info("LoRa transmitter stopped")
    
    def send_detection(self, detection) -> bool:
        """
        Queue detection for LoRa transmission
        
        Args:
            detection: IMX500Detection object
            
        Returns:
            True if queued successfully, False if queue full
        """
        if not self.enabled:
            return False
        
        try:
            # Encode detection to compact payload
            payload = detection.to_lora_payload()
            
            # Add to queue
            self.queue.put_nowait({
                'payload': payload,
                'timestamp': datetime.now(),
                'detection': detection,
            })
            
            logger.debug(f"Detection queued for LoRa transmission: {detection.species}")
            return True
            
        except Exception as e:
            logger.warning(f"Failed to queue detection: {e}")
            self.stats['queue_overflows'] += 1
            return False
    
    def _transmission_loop(self):
        """Main transmission loop (runs in separate thread)"""
        while self.running:
            try:
                # Get next message from queue (with timeout)
                try:
                    message = self.queue.get(timeout=1.0)
                except Empty:
                    continue
                
                # Transmit with retries
                success = self._transmit_with_retry(message['payload'])
                
                if success:
                    self.stats['total_sent'] += 1
                    logger.debug(f"LoRa transmission successful")
                else:
                    self.stats['total_failed'] += 1
                    logger.warning(f"LoRa transmission failed after retries")
                
                self.queue.task_done()
                
            except Exception as e:
                logger.error(f"Transmission loop error: {e}")
    
    def _transmit_with_retry(self, payload: bytes) -> bool:
        """
        Transmit payload with retry logic
        
        Args:
            payload: Binary payload to transmit
            
        Returns:
            True if successful, False if all retries failed
        """
        for attempt in range(self.max_retries):
            try:
                if self._transmit(payload):
                    if attempt > 0:
                        self.stats['total_retries'] += attempt
                    return True
                
                # Wait before retry
                if attempt < self.max_retries - 1:
                    time.sleep(self.retry_delay)
                    
            except Exception as e:
                logger.error(f"Transmission attempt {attempt + 1} failed: {e}")
        
        return False
    
    def _transmit(self, payload: bytes) -> bool:
        """
        Transmit single payload
        
        Args:
            payload: Binary payload
            
        Returns:
            True if successful, False otherwise
        """
        if not self.radio:
            # Simulation mode
            logger.debug(f"[SIMULATED] LoRa TX: {len(payload)} bytes")
            return True
        
        try:
            # Send via LoRa radio
            self.radio.set_mode(1)  # Standby mode
            
            # Write payload
            self.radio.write_payload(list(payload))
            
            # Transmit
            self.radio.set_mode(3)  # TX mode
            
            # Wait for transmission to complete
            # This is simplified - real implementation should use interrupts
            time.sleep(0.1)
            
            return True
            
        except Exception as e:
            logger.error(f"LoRa transmission error: {e}")
            return False
    
    def send_telemetry(self, telemetry: Dict) -> bool:
        """
        Send telemetry data via LoRa
        
        Args:
            telemetry: Telemetry dictionary
            
        Returns:
            True if queued successfully, False otherwise
        """
        # For now, just log - could implement custom telemetry encoding
        logger.debug(f"Telemetry: {telemetry}")
        return True
    
    def get_stats(self) -> Dict:
        """
        Get transmitter statistics
        
        Returns:
            Dictionary with transmission stats
        """
        stats = self.stats.copy()
        stats['enabled'] = self.enabled
        stats['queue_size'] = self.queue.qsize()
        stats['queue_max'] = self.queue.maxsize
        
        # Calculate success rate
        total_attempts = stats['total_sent'] + stats['total_failed']
        if total_attempts > 0:
            stats['success_rate'] = stats['total_sent'] / total_attempts
        else:
            stats['success_rate'] = 0.0
        
        return stats
