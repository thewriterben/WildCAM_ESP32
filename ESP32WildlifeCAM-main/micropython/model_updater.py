"""
Model Updater for Wildlife AI System

Handles automatic model updates, version management, and
cloud synchronization for the ESP32 Wildlife Camera AI models.

Author: ESP32WildlifeCAM Project
Version: 1.0.0
"""

import json
import time
import hashlib
import gc
from typing import Dict, List, Optional, Tuple

class ModelUpdater:
    """
    AI Model Update and Management System
    
    Handles downloading, validating, and deploying new AI models
    for the wildlife camera system.
    """
    
    def __init__(self):
        self.models_directory = "/models"
        self.backup_directory = "/models/backup"
        self.temp_directory = "/models/temp"
        self.version_file = "/models/versions.json"
        
        self.current_versions = {}
        self.available_updates = {}
        self.update_in_progress = False
        
        # Configuration
        self.auto_update_enabled = True
        self.wifi_required = True
        self.battery_threshold = 3.6  # Minimum battery for updates
        
        # Update sources
        self.update_sources = {
            'primary': 'https://models.wildlifecam.org/updates/',
            'backup': 'https://backup.wildlifecam.org/models/',
            'edge_impulse': 'https://studio.edgeimpulse.com/models/'
        }
    
    def init(self) -> bool:
        """Initialize the model updater."""
        try:
            print("Initializing Model Updater...")
            
            # Create directories if they don't exist
            self._ensure_directories()
            
            # Load current version information
            self.current_versions = self._load_version_info()
            
            print("Model Updater initialized successfully")
            return True
            
        except Exception as e:
            print(f"Error initializing Model Updater: {e}")
            return False
    
    def check_for_updates(self, force_check: bool = False) -> Dict[str, Any]:
        """
        Check for available model updates.
        
        Args:
            force_check: Force check even if recently checked
            
        Returns:
            Dictionary with update information
        """
        try:
            print("Checking for model updates...")
            
            # Check if we should skip (battery, wifi, etc.)
            if not self._can_perform_update() and not force_check:
                return {'updates_available': False, 'reason': 'Conditions not met'}
            
            # Get available versions from server
            available_versions = self._fetch_available_versions()
            
            # Compare with current versions
            updates = self._compare_versions(available_versions)
            
            self.available_updates = updates
            
            result = {
                'updates_available': len(updates) > 0,
                'available_updates': updates,
                'current_versions': self.current_versions,
                'last_check': time.time()
            }
            
            print(f"Update check completed. {len(updates)} updates available.")
            return result
            
        except Exception as e:
            print(f"Error checking for updates: {e}")
            return {'updates_available': False, 'error': str(e)}
    
    def download_update(self, model_name: str) -> bool:
        """
        Download a specific model update.
        
        Args:
            model_name: Name of the model to update
            
        Returns:
            True if download successful
        """
        if self.update_in_progress:
            print("Update already in progress")
            return False
        
        try:
            self.update_in_progress = True
            print(f"Downloading update for {model_name}...")
            
            # Get update info
            if model_name not in self.available_updates:
                print(f"No update available for {model_name}")
                return False
            
            update_info = self.available_updates[model_name]
            
            # Download model file
            success = self._download_model_file(model_name, update_info)
            
            if success:
                print(f"Download completed for {model_name}")
            else:
                print(f"Download failed for {model_name}")
            
            return success
            
        except Exception as e:
            print(f"Error downloading update: {e}")
            return False
        finally:
            self.update_in_progress = False
    
    def install_update(self, model_name: str) -> bool:
        """
        Install a downloaded model update.
        
        Args:
            model_name: Name of the model to install
            
        Returns:
            True if installation successful
        """
        try:
            print(f"Installing update for {model_name}...")
            
            # Validate downloaded file
            if not self._validate_downloaded_model(model_name):
                print(f"Validation failed for {model_name}")
                return False
            
            # Backup current model
            if not self._backup_current_model(model_name):
                print(f"Backup failed for {model_name}")
                return False
            
            # Install new model
            if not self._install_new_model(model_name):
                print(f"Installation failed for {model_name}")
                # Restore backup
                self._restore_backup(model_name)
                return False
            
            # Update version info
            self._update_version_info(model_name)
            
            print(f"Update installed successfully for {model_name}")
            return True
            
        except Exception as e:
            print(f"Error installing update: {e}")
            return False
    
    def rollback_update(self, model_name: str) -> bool:
        """
        Rollback to previous model version.
        
        Args:
            model_name: Name of the model to rollback
            
        Returns:
            True if rollback successful
        """
        try:
            print(f"Rolling back {model_name}...")
            
            # Restore from backup
            if self._restore_backup(model_name):
                # Update version info
                self._rollback_version_info(model_name)
                print(f"Rollback successful for {model_name}")
                return True
            else:
                print(f"Rollback failed for {model_name}")
                return False
                
        except Exception as e:
            print(f"Error during rollback: {e}")
            return False
    
    def auto_update_models(self) -> Dict[str, bool]:
        """
        Automatically check and install available updates.
        
        Returns:
            Dictionary with update results for each model
        """
        if not self.auto_update_enabled:
            return {}
        
        try:
            print("Starting automatic model update...")
            
            # Check for updates
            update_check = self.check_for_updates()
            if not update_check.get('updates_available', False):
                return {}
            
            results = {}
            
            # Download and install each update
            for model_name in self.available_updates:
                print(f"Auto-updating {model_name}...")
                
                # Download
                download_success = self.download_update(model_name)
                if not download_success:
                    results[model_name] = False
                    continue
                
                # Install
                install_success = self.install_update(model_name)
                results[model_name] = install_success
                
                # Clean up temporary files
                self._cleanup_temp_files(model_name)
                
                # Garbage collect between updates
                gc.collect()
            
            print("Automatic update completed")
            return results
            
        except Exception as e:
            print(f"Error during auto-update: {e}")
            return {}
    
    def get_model_info(self, model_name: str = None) -> Dict[str, Any]:
        """
        Get information about models.
        
        Args:
            model_name: Specific model name, or None for all models
            
        Returns:
            Dictionary with model information
        """
        if model_name:
            return self.current_versions.get(model_name, {})
        else:
            return self.current_versions.copy()
    
    def validate_all_models(self) -> Dict[str, bool]:
        """
        Validate integrity of all installed models.
        
        Returns:
            Dictionary with validation results for each model
        """
        results = {}
        
        for model_name in self.current_versions:
            try:
                results[model_name] = self._validate_model_file(model_name)
            except Exception as e:
                print(f"Error validating {model_name}: {e}")
                results[model_name] = False
        
        return results
    
    def cleanup_old_files(self) -> None:
        """Clean up old backup and temporary files."""
        try:
            print("Cleaning up old files...")
            
            # Clean temporary directory
            self._cleanup_directory(self.temp_directory)
            
            # Clean old backups (keep only latest)
            self._cleanup_old_backups()
            
            print("Cleanup completed")
            
        except Exception as e:
            print(f"Error during cleanup: {e}")
    
    def configure(self, **kwargs) -> None:
        """Configure model updater settings."""
        for key, value in kwargs.items():
            if key == 'auto_update_enabled':
                self.auto_update_enabled = value
            elif key == 'wifi_required':
                self.wifi_required = value
            elif key == 'battery_threshold':
                self.battery_threshold = value
            elif key == 'models_directory':
                self.models_directory = value
            
            print(f"Model updater config updated: {key} = {value}")
    
    def _ensure_directories(self) -> None:
        """Ensure required directories exist."""
        import os
        
        directories = [
            self.models_directory,
            self.backup_directory,
            self.temp_directory
        ]
        
        for directory in directories:
            try:
                os.makedirs(directory)
            except OSError:
                pass  # Directory already exists
    
    def _load_version_info(self) -> Dict[str, Any]:
        """Load current version information."""
        try:
            with open(self.version_file, 'r') as f:
                return json.load(f)
        except:
            # Create default version file
            default_versions = {
                'species_classifier': {
                    'version': '1.0.0',
                    'checksum': '',
                    'size': 0,
                    'installed_date': time.time()
                },
                'behavior_analyzer': {
                    'version': '1.0.0',
                    'checksum': '',
                    'size': 0,
                    'installed_date': time.time()
                },
                'motion_detector': {
                    'version': '1.0.0',
                    'checksum': '',
                    'size': 0,
                    'installed_date': time.time()
                }
            }
            self._save_version_info(default_versions)
            return default_versions
    
    def _save_version_info(self, versions: Dict[str, Any]) -> None:
        """Save version information to file."""
        try:
            with open(self.version_file, 'w') as f:
                json.dump(versions, f)
        except Exception as e:
            print(f"Error saving version info: {e}")
    
    def _can_perform_update(self) -> bool:
        """Check if update conditions are met."""
        # Check WiFi connection (simulated)
        if self.wifi_required:
            # This would check actual WiFi status
            wifi_connected = True  # Placeholder
            if not wifi_connected:
                return False
        
        # Check battery level (simulated)
        # This would get actual battery voltage
        battery_voltage = 3.8  # Placeholder
        if battery_voltage < self.battery_threshold:
            return False
        
        # Check if update already in progress
        if self.update_in_progress:
            return False
        
        return True
    
    def _fetch_available_versions(self) -> Dict[str, Any]:
        """Fetch available versions from server."""
        # This would make actual HTTP requests to update servers
        # For simulation, return some fake update data
        
        return {
            'species_classifier': {
                'version': '1.1.0',
                'checksum': 'abc123def456',
                'size': 524288,
                'url': 'https://models.wildlifecam.org/species_v1.1.0.tflite',
                'release_date': time.time() - 86400,
                'description': 'Improved accuracy for small mammals'
            },
            'behavior_analyzer': {
                'version': '1.0.1',
                'checksum': 'def456ghi789',
                'size': 307200,
                'url': 'https://models.wildlifecam.org/behavior_v1.0.1.tflite',
                'release_date': time.time() - 43200,
                'description': 'Bug fixes for feeding behavior detection'
            }
        }
    
    def _compare_versions(self, available: Dict[str, Any]) -> Dict[str, Any]:
        """Compare available versions with current versions."""
        updates = {}
        
        for model_name, available_info in available.items():
            current_info = self.current_versions.get(model_name, {})
            current_version = current_info.get('version', '0.0.0')
            available_version = available_info.get('version', '0.0.0')
            
            # Simple version comparison (should use proper semver)
            if available_version > current_version:
                updates[model_name] = available_info
        
        return updates
    
    def _download_model_file(self, model_name: str, update_info: Dict[str, Any]) -> bool:
        """Download model file from server."""
        try:
            # This would implement actual HTTP download
            # For simulation, just create a placeholder file
            
            temp_path = f"{self.temp_directory}/{model_name}_new.tflite"
            
            # Simulate download delay
            time.sleep(2)
            
            # Create placeholder file
            with open(temp_path, 'wb') as f:
                f.write(b'dummy_model_data' * 1000)  # Fake model data
            
            print(f"Downloaded {model_name} to {temp_path}")
            return True
            
        except Exception as e:
            print(f"Download error: {e}")
            return False
    
    def _validate_downloaded_model(self, model_name: str) -> bool:
        """Validate downloaded model file."""
        try:
            temp_path = f"{self.temp_directory}/{model_name}_new.tflite"
            
            # Check if file exists
            try:
                with open(temp_path, 'rb') as f:
                    data = f.read()
            except:
                return False
            
            # Validate file size and checksum (simplified)
            if len(data) < 1000:  # Minimum size check
                return False
            
            # Calculate checksum
            checksum = hashlib.md5(data).hexdigest()
            expected_checksum = self.available_updates[model_name].get('checksum', '')
            
            # For simulation, always validate successfully
            return True
            
        except Exception as e:
            print(f"Validation error: {e}")
            return False
    
    def _backup_current_model(self, model_name: str) -> bool:
        """Backup current model before update."""
        try:
            current_path = f"{self.models_directory}/{model_name}.tflite"
            backup_path = f"{self.backup_directory}/{model_name}_backup.tflite"
            
            # Copy current model to backup (simplified)
            try:
                with open(current_path, 'rb') as src:
                    data = src.read()
                with open(backup_path, 'wb') as dst:
                    dst.write(data)
                return True
            except:
                return False
            
        except Exception as e:
            print(f"Backup error: {e}")
            return False
    
    def _install_new_model(self, model_name: str) -> bool:
        """Install new model file."""
        try:
            temp_path = f"{self.temp_directory}/{model_name}_new.tflite"
            install_path = f"{self.models_directory}/{model_name}.tflite"
            
            # Move temp file to final location
            try:
                with open(temp_path, 'rb') as src:
                    data = src.read()
                with open(install_path, 'wb') as dst:
                    dst.write(data)
                
                # Remove temp file
                import os
                os.remove(temp_path)
                
                return True
            except:
                return False
            
        except Exception as e:
            print(f"Installation error: {e}")
            return False
    
    def _update_version_info(self, model_name: str) -> None:
        """Update version information after successful install."""
        if model_name in self.available_updates:
            update_info = self.available_updates[model_name]
            self.current_versions[model_name] = {
                'version': update_info['version'],
                'checksum': update_info['checksum'],
                'size': update_info['size'],
                'installed_date': time.time(),
                'previous_version': self.current_versions.get(model_name, {}).get('version', 'unknown')
            }
            self._save_version_info(self.current_versions)
    
    def _cleanup_temp_files(self, model_name: str = None) -> None:
        """Clean up temporary files."""
        import os
        
        try:
            if model_name:
                temp_file = f"{self.temp_directory}/{model_name}_new.tflite"
                try:
                    os.remove(temp_file)
                except:
                    pass
            else:
                # Clean all temp files
                try:
                    files = os.listdir(self.temp_directory)
                    for file in files:
                        os.remove(f"{self.temp_directory}/{file}")
                except:
                    pass
        except Exception as e:
            print(f"Cleanup error: {e}")


# Global model updater instance
model_updater = ModelUpdater()

# Convenience functions
def init_updater() -> bool:
    """Initialize the model updater."""
    return model_updater.init()

def check_updates(force: bool = False) -> Dict[str, Any]:
    """Check for available model updates."""
    return model_updater.check_for_updates(force)

def update_model(model_name: str) -> bool:
    """Download and install a model update."""
    if model_updater.download_update(model_name):
        return model_updater.install_update(model_name)
    return False

def auto_update() -> Dict[str, bool]:
    """Automatically update all models."""
    return model_updater.auto_update_models()

def get_versions() -> Dict[str, Any]:
    """Get current model versions."""
    return model_updater.get_model_info()

def configure_updater(**kwargs) -> None:
    """Configure the model updater."""
    model_updater.configure(**kwargs)

# Example usage
if __name__ == "__main__":
    # Initialize updater
    if init_updater():
        print("Model updater ready!")
        
        # Check for updates
        updates = check_updates()
        print(f"Updates available: {updates}")
        
        # Show current versions
        versions = get_versions()
        print(f"Current versions: {versions}")
    else:
        print("Failed to initialize model updater")