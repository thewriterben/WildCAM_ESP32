#!/usr/bin/env python3
"""
Dataset Manager for Wildlife Classification

Handles automated dataset collection, labeling, and management for 
TensorFlow Lite wildlife classification models.
"""

import os
import json
import hashlib
import logging
from typing import Dict, List, Tuple, Optional, Union
from datetime import datetime
import shutil
from pathlib import Path

import numpy as np
from PIL import Image, ImageEnhance
import requests
from sklearn.model_selection import train_test_split

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class DatasetManager:
    """
    Manages wildlife dataset collection, preprocessing, and organization
    for training TensorFlow Lite models optimized for ESP32 deployment.
    """
    
    def __init__(self, base_dir: str, species_config: str = None):
        """
        Initialize dataset manager
        
        Args:
            base_dir: Base directory for dataset storage
            species_config: Path to species configuration file
        """
        self.base_dir = Path(base_dir)
        self.base_dir.mkdir(parents=True, exist_ok=True)
        
        # Dataset structure
        self.raw_dir = self.base_dir / "raw"
        self.processed_dir = self.base_dir / "processed" 
        self.train_dir = self.processed_dir / "train"
        self.val_dir = self.processed_dir / "validation"
        self.test_dir = self.processed_dir / "test"
        
        # Create directory structure
        for dir_path in [self.raw_dir, self.processed_dir, self.train_dir, 
                        self.val_dir, self.test_dir]:
            dir_path.mkdir(parents=True, exist_ok=True)
        
        # Load species configuration
        self.species_config = self._load_species_config(species_config)
        self.species_mapping = {idx: name for idx, name in enumerate(self.species_config.keys())}
        
        # Dataset metadata
        self.metadata = {
            "created_date": datetime.now().isoformat(),
            "total_samples": 0,
            "species_count": len(self.species_config),
            "train_samples": 0,
            "val_samples": 0,
            "test_samples": 0,
            "data_sources": [],
            "preprocessing_config": {}
        }
        
        logger.info(f"Dataset manager initialized with {len(self.species_config)} species")
    
    def _load_species_config(self, config_path: str) -> Dict:
        """Load species configuration from file or use defaults"""
        if config_path and os.path.exists(config_path):
            with open(config_path, 'r') as f:
                return json.load(f)
        
        # Default species configuration for North American wildlife
        return {
            "white_tailed_deer": {"id": 0, "scientific_name": "Odocoileus virginianus", "priority": "high"},
            "black_bear": {"id": 1, "scientific_name": "Ursus americanus", "priority": "high"},
            "red_fox": {"id": 2, "scientific_name": "Vulpes vulpes", "priority": "medium"},
            "gray_wolf": {"id": 3, "scientific_name": "Canis lupus", "priority": "high"},
            "mountain_lion": {"id": 4, "scientific_name": "Puma concolor", "priority": "high"},
            "elk": {"id": 5, "scientific_name": "Cervus canadensis", "priority": "medium"},
            "moose": {"id": 6, "scientific_name": "Alces alces", "priority": "medium"},
            "wild_turkey": {"id": 7, "scientific_name": "Meleagris gallopavo", "priority": "low"},
            "bald_eagle": {"id": 8, "scientific_name": "Haliaeetus leucocephalus", "priority": "high"},
            "red_tailed_hawk": {"id": 9, "scientific_name": "Buteo jamaicensis", "priority": "medium"},
            "great_blue_heron": {"id": 10, "scientific_name": "Ardea herodias", "priority": "low"},
            "canada_goose": {"id": 11, "scientific_name": "Branta canadensis", "priority": "low"},
            "mallard_duck": {"id": 12, "scientific_name": "Anas platyrhynchos", "priority": "low"},
            "raccoon": {"id": 13, "scientific_name": "Procyon lotor", "priority": "medium"},
            "opossum": {"id": 14, "scientific_name": "Didelphis virginiana", "priority": "low"},
            "squirrel": {"id": 15, "scientific_name": "Sciurus carolinensis", "priority": "low"},
            "chipmunk": {"id": 16, "scientific_name": "Tamias striatus", "priority": "low"},
            "rabbit": {"id": 17, "scientific_name": "Sylvilagus floridanus", "priority": "low"},
            "skunk": {"id": 18, "scientific_name": "Mephitis mephitis", "priority": "low"},
            "porcupine": {"id": 19, "scientific_name": "Erethizon dorsatum", "priority": "low"},
            "human": {"id": 49, "scientific_name": "Homo sapiens", "priority": "high"}
        }
    
    def collect_from_inaturalist(self, species_list: List[str], 
                                samples_per_species: int = 1000,
                                quality_grade: str = "research") -> Dict:
        """
        Collect dataset from iNaturalist API
        
        Args:
            species_list: List of species to collect
            samples_per_species: Number of samples per species
            quality_grade: Quality grade filter ('research', 'needs_id', 'casual')
            
        Returns:
            Collection statistics
        """
        logger.info(f"Collecting data from iNaturalist for {len(species_list)} species")
        
        stats = {"collected": 0, "failed": 0, "species_stats": {}}
        
        for species_name in species_list:
            if species_name not in self.species_config:
                logger.warning(f"Species {species_name} not in configuration, skipping")
                continue
            
            species_id = self.species_config[species_name]["id"]
            scientific_name = self.species_config[species_name]["scientific_name"]
            
            logger.info(f"Collecting {species_name} ({scientific_name})")
            
            # Create species directory
            species_dir = self.raw_dir / species_name
            species_dir.mkdir(exist_ok=True)
            
            collected_count = self._collect_species_from_inaturalist(
                species_name, scientific_name, samples_per_species, 
                quality_grade, species_dir
            )
            
            stats["collected"] += collected_count
            stats["species_stats"][species_name] = collected_count
            
            logger.info(f"Collected {collected_count} samples for {species_name}")
        
        self._update_metadata("data_sources", "iNaturalist")
        logger.info(f"Collection completed: {stats['collected']} total samples")
        
        return stats
    
    def _collect_species_from_inaturalist(self, species_name: str, scientific_name: str,
                                        max_samples: int, quality_grade: str,
                                        output_dir: Path) -> int:
        """Collect samples for a specific species from iNaturalist"""
        
        # iNaturalist API endpoint
        base_url = "https://api.inaturalist.org/v1/observations"
        
        collected = 0
        page = 1
        per_page = 200
        
        while collected < max_samples:
            params = {
                "taxon_name": scientific_name,
                "quality_grade": quality_grade,
                "photos": "true",
                "geoprivacy": "open",
                "page": page,
                "per_page": min(per_page, max_samples - collected)
            }
            
            try:
                response = requests.get(base_url, params=params, timeout=30)
                response.raise_for_status()
                data = response.json()
                
                if not data.get("results"):
                    break
                
                for observation in data["results"]:
                    if collected >= max_samples:
                        break
                    
                    # Download observation photos
                    for photo in observation.get("photos", []):
                        if collected >= max_samples:
                            break
                        
                        photo_url = photo.get("url", "").replace("square", "medium")
                        if photo_url:
                            success = self._download_image(
                                photo_url, 
                                output_dir / f"{species_name}_{observation['id']}_{photo['id']}.jpg",
                                observation
                            )
                            if success:
                                collected += 1
                
                page += 1
                
            except requests.RequestException as e:
                logger.error(f"Error collecting from iNaturalist: {e}")
                break
        
        return collected
    
    def _download_image(self, url: str, filepath: Path, metadata: Dict = None) -> bool:
        """Download image from URL with metadata"""
        try:
            response = requests.get(url, timeout=30, stream=True)
            response.raise_for_status()
            
            # Save image
            with open(filepath, 'wb') as f:
                shutil.copyfileobj(response.raw, f)
            
            # Save metadata
            if metadata:
                metadata_file = filepath.with_suffix('.json')
                with open(metadata_file, 'w') as f:
                    json.dump(metadata, f, indent=2)
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to download {url}: {e}")
            return False
    
    def preprocess_dataset(self, image_size: Tuple[int, int] = (224, 224),
                          augmentation: bool = True) -> Dict:
        """
        Preprocess raw dataset for training
        
        Args:
            image_size: Target image size (width, height)
            augmentation: Whether to apply data augmentation
            
        Returns:
            Preprocessing statistics
        """
        logger.info(f"Preprocessing dataset to {image_size} with augmentation={augmentation}")
        
        stats = {"processed": 0, "failed": 0, "species_stats": {}}
        
        # Process each species directory
        for species_dir in self.raw_dir.iterdir():
            if not species_dir.is_dir():
                continue
            
            species_name = species_dir.name
            if species_name not in self.species_config:
                continue
            
            logger.info(f"Processing {species_name}")
            
            # Create processed directories for this species
            for split_dir in [self.train_dir, self.val_dir, self.test_dir]:
                (split_dir / species_name).mkdir(exist_ok=True)
            
            # Get all images for this species
            image_files = list(species_dir.glob("*.jpg")) + list(species_dir.glob("*.png"))
            
            if not image_files:
                logger.warning(f"No images found for {species_name}")
                continue
            
            # Split dataset
            train_files, temp_files = train_test_split(image_files, test_size=0.3, random_state=42)
            val_files, test_files = train_test_split(temp_files, test_size=0.5, random_state=42)
            
            # Process each split
            splits = [
                (train_files, self.train_dir, "train"),
                (val_files, self.val_dir, "val"), 
                (test_files, self.test_dir, "test")
            ]
            
            species_processed = 0
            for files, output_dir, split_name in splits:
                for img_file in files:
                    try:
                        processed_path = output_dir / species_name / img_file.name
                        if self._preprocess_image(img_file, processed_path, image_size):
                            species_processed += 1
                            
                            # Apply augmentation to training data
                            if augmentation and split_name == "train":
                                self._apply_augmentation(processed_path, image_size)
                                
                    except Exception as e:
                        logger.error(f"Failed to process {img_file}: {e}")
                        stats["failed"] += 1
            
            stats["processed"] += species_processed
            stats["species_stats"][species_name] = species_processed
            
            logger.info(f"Processed {species_processed} images for {species_name}")
        
        self._update_dataset_splits()
        logger.info(f"Preprocessing completed: {stats['processed']} images processed")
        
        return stats
    
    def _preprocess_image(self, input_path: Path, output_path: Path, 
                         target_size: Tuple[int, int]) -> bool:
        """Preprocess a single image"""
        try:
            with Image.open(input_path) as img:
                # Convert to RGB if needed
                if img.mode != 'RGB':
                    img = img.convert('RGB')
                
                # Resize maintaining aspect ratio
                img.thumbnail(target_size, Image.Resampling.LANCZOS)
                
                # Create new image with target size (pad if needed)
                new_img = Image.new('RGB', target_size, (0, 0, 0))
                paste_x = (target_size[0] - img.width) // 2
                paste_y = (target_size[1] - img.height) // 2
                new_img.paste(img, (paste_x, paste_y))
                
                # Save processed image
                new_img.save(output_path, 'JPEG', quality=95)
                
                return True
                
        except Exception as e:
            logger.error(f"Error processing image {input_path}: {e}")
            return False
    
    def _apply_augmentation(self, image_path: Path, target_size: Tuple[int, int]):
        """Apply data augmentation to increase dataset diversity"""
        try:
            with Image.open(image_path) as img:
                base_name = image_path.stem
                output_dir = image_path.parent
                
                # Rotation augmentation
                for angle in [90, 180, 270]:
                    rotated = img.rotate(angle, expand=True)
                    rotated.thumbnail(target_size, Image.Resampling.LANCZOS)
                    
                    # Pad to target size
                    new_img = Image.new('RGB', target_size, (0, 0, 0))
                    paste_x = (target_size[0] - rotated.width) // 2
                    paste_y = (target_size[1] - rotated.height) // 2
                    new_img.paste(rotated, (paste_x, paste_y))
                    
                    aug_path = output_dir / f"{base_name}_rot{angle}.jpg"
                    new_img.save(aug_path, 'JPEG', quality=95)
                
                # Brightness augmentation
                enhancer = ImageEnhance.Brightness(img)
                for factor in [0.7, 1.3]:
                    enhanced = enhancer.enhance(factor)
                    aug_path = output_dir / f"{base_name}_bright{factor}.jpg"
                    enhanced.save(aug_path, 'JPEG', quality=95)
                
        except Exception as e:
            logger.error(f"Error applying augmentation to {image_path}: {e}")
    
    def _update_dataset_splits(self):
        """Update metadata with dataset split information"""
        splits = {"train": 0, "val": 0, "test": 0}
        
        for split_name, split_dir in [("train", self.train_dir), 
                                     ("val", self.val_dir), 
                                     ("test", self.test_dir)]:
            count = 0
            for species_dir in split_dir.iterdir():
                if species_dir.is_dir():
                    count += len(list(species_dir.glob("*.jpg")))
            splits[split_name] = count
        
        self.metadata.update({
            "train_samples": splits["train"],
            "val_samples": splits["val"], 
            "test_samples": splits["test"],
            "total_samples": sum(splits.values())
        })
        
        # Save updated metadata
        with open(self.base_dir / "dataset_metadata.json", 'w') as f:
            json.dump(self.metadata, f, indent=2)
    
    def _update_metadata(self, key: str, value: Union[str, int, Dict]):
        """Update metadata with new information"""
        if key == "data_sources":
            if "data_sources" not in self.metadata:
                self.metadata["data_sources"] = []
            if value not in self.metadata["data_sources"]:
                self.metadata["data_sources"].append(value)
        else:
            self.metadata[key] = value
    
    def get_dataset_statistics(self) -> Dict:
        """Get comprehensive dataset statistics"""
        stats = self.metadata.copy()
        
        # Add per-species statistics
        species_stats = {}
        for species_name in self.species_config.keys():
            species_stats[species_name] = {}
            for split_name, split_dir in [("train", self.train_dir),
                                         ("val", self.val_dir),
                                         ("test", self.test_dir)]:
                species_dir = split_dir / species_name
                if species_dir.exists():
                    count = len(list(species_dir.glob("*.jpg")))
                    species_stats[species_name][split_name] = count
        
        stats["species_statistics"] = species_stats
        return stats
    
    def export_tensorflow_dataset(self, output_dir: str) -> bool:
        """
        Export dataset in TensorFlow-compatible format
        
        Args:
            output_dir: Output directory for TensorFlow dataset
            
        Returns:
            True if export successful
        """
        try:
            output_path = Path(output_dir)
            output_path.mkdir(parents=True, exist_ok=True)
            
            # Copy processed dataset
            for split_name, split_dir in [("train", self.train_dir),
                                         ("validation", self.val_dir),
                                         ("test", self.test_dir)]:
                split_output = output_path / split_name
                if split_dir.exists():
                    shutil.copytree(split_dir, split_output, dirs_exist_ok=True)
            
            # Create labels file
            labels = [name for name in sorted(self.species_config.keys())]
            with open(output_path / "labels.txt", 'w') as f:
                f.write('\n'.join(labels))
            
            # Copy metadata
            shutil.copy2(self.base_dir / "dataset_metadata.json", 
                        output_path / "dataset_metadata.json")
            
            logger.info(f"Dataset exported to {output_dir}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to export dataset: {e}")
            return False


if __name__ == "__main__":
    # Example usage
    manager = DatasetManager("/data/wildlife_dataset")
    
    # Collect data from iNaturalist
    species_to_collect = ["white_tailed_deer", "black_bear", "red_fox"]
    collection_stats = manager.collect_from_inaturalist(species_to_collect, samples_per_species=500)
    
    # Preprocess dataset
    preprocessing_stats = manager.preprocess_dataset(image_size=(224, 224), augmentation=True)
    
    # Export for TensorFlow training
    manager.export_tensorflow_dataset("/data/tensorflow_dataset")
    
    # Print statistics
    stats = manager.get_dataset_statistics()
    print("Dataset Statistics:")
    print(json.dumps(stats, indent=2))