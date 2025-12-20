#!/usr/bin/env python3
"""
Wildlife Training Data Preparation for WildCAM_ESP32
Organizes downloaded images into train/val/test splits and applies preprocessing

Author: Manus AI
Date: October 29, 2025
"""

import os
import json
import shutil
import argparse
from pathlib import Path
from PIL import Image
import random
from tqdm import tqdm
from collections import defaultdict

class DataPreparator:
    def __init__(self, input_dir, output_dir, train_split=0.7, val_split=0.15, test_split=0.15):
        self.input_dir = Path(input_dir)
        self.output_dir = Path(output_dir)
        self.train_split = train_split
        self.val_split = val_split
        self.test_split = test_split
        
        assert abs(train_split + val_split + test_split - 1.0) < 0.01, \
            "Splits must sum to 1.0"
        
        # Create output directories
        for split in ['train', 'val', 'test']:
            (self.output_dir / split).mkdir(parents=True, exist_ok=True)
    
    def validate_image(self, img_path):
        """Check if image is valid and readable"""
        try:
            with Image.open(img_path) as img:
                img.verify()
            return True
        except Exception:
            return False
    
    def get_image_stats(self, img_path):
        """Get image dimensions and format"""
        try:
            with Image.open(img_path) as img:
                return {
                    'width': img.width,
                    'height': img.height,
                    'format': img.format,
                    'mode': img.mode
                }
        except Exception:
            return None
    
    def scan_dataset(self):
        """Scan input directory and count images per species"""
        print("Scanning dataset...")
        
        species_counts = defaultdict(int)
        species_dirs = {}
        
        # Find all species directories
        for dataset_dir in self.input_dir.iterdir():
            if not dataset_dir.is_dir():
                continue
            
            for species_dir in dataset_dir.iterdir():
                if not species_dir.is_dir():
                    continue
                
                species_name = species_dir.name
                image_files = list(species_dir.glob('*.jpg')) + \
                             list(species_dir.glob('*.jpeg')) + \
                             list(species_dir.glob('*.png'))
                
                if image_files:
                    species_counts[species_name] += len(image_files)
                    if species_name not in species_dirs:
                        species_dirs[species_name] = []
                    species_dirs[species_name].append(species_dir)
        
        print(f"\nFound {len(species_counts)} species:")
        for species, count in sorted(species_counts.items()):
            print(f"  {species}: {count} images")
        
        return species_dirs, species_counts
    
    def create_splits(self, species_dirs):
        """Create train/val/test splits for each species"""
        print("\nCreating train/val/test splits...")
        
        split_summary = {
            'train': defaultdict(int),
            'val': defaultdict(int),
            'test': defaultdict(int)
        }
        
        for species_name, dirs in species_dirs.items():
            print(f"\nProcessing: {species_name}")
            
            # Collect all images for this species
            all_images = []
            for species_dir in dirs:
                images = list(species_dir.glob('*.jpg')) + \
                        list(species_dir.glob('*.jpeg')) + \
                        list(species_dir.glob('*.png'))
                all_images.extend(images)
            
            # Validate images
            valid_images = []
            print(f"  Validating {len(all_images)} images...")
            for img_path in tqdm(all_images, desc="  Validation"):
                if self.validate_image(img_path):
                    valid_images.append(img_path)
            
            print(f"  Valid images: {len(valid_images)}/{len(all_images)}")
            
            if len(valid_images) < 10:
                print(f"  ⚠ Skipping {species_name}: too few valid images")
                continue
            
            # Shuffle and split
            random.shuffle(valid_images)
            
            train_end = int(len(valid_images) * self.train_split)
            val_end = train_end + int(len(valid_images) * self.val_split)
            
            splits = {
                'train': valid_images[:train_end],
                'val': valid_images[train_end:val_end],
                'test': valid_images[val_end:]
            }
            
            # Copy images to split directories
            for split_name, images in splits.items():
                split_dir = self.output_dir / split_name / species_name
                split_dir.mkdir(parents=True, exist_ok=True)
                
                print(f"  Copying {len(images)} images to {split_name}...")
                for img_path in tqdm(images, desc=f"  {split_name}"):
                    dest_path = split_dir / img_path.name
                    shutil.copy2(img_path, dest_path)
                    split_summary[split_name][species_name] += 1
        
        return split_summary
    
    def generate_dataset_report(self, split_summary):
        """Generate a comprehensive dataset report"""
        report_path = self.output_dir / 'dataset_report.txt'
        
        with open(report_path, 'w') as f:
            f.write("="*80 + "\n")
            f.write("WildCAM_ESP32 Training Dataset Report\n")
            f.write("="*80 + "\n\n")
            
            f.write(f"Output Directory: {self.output_dir}\n")
            f.write(f"Train Split: {self.train_split*100:.0f}%\n")
            f.write(f"Validation Split: {self.val_split*100:.0f}%\n")
            f.write(f"Test Split: {self.test_split*100:.0f}%\n\n")
            
            for split_name in ['train', 'val', 'test']:
                f.write(f"\n{split_name.upper()} SET:\n")
                f.write("-"*80 + "\n")
                
                total = 0
                for species, count in sorted(split_summary[split_name].items()):
                    f.write(f"  {species:30s} {count:6d} images\n")
                    total += count
                
                f.write("-"*80 + "\n")
                f.write(f"  {'TOTAL':30s} {total:6d} images\n")
            
            # Overall summary
            grand_total = sum(
                sum(counts.values()) 
                for counts in split_summary.values()
            )
            
            f.write("\n" + "="*80 + "\n")
            f.write(f"GRAND TOTAL: {grand_total} images\n")
            f.write("="*80 + "\n")
        
        print(f"\nDataset report saved to: {report_path}")
        
        # Also print to console
        with open(report_path, 'r') as f:
            print("\n" + f.read())
    
    def generate_class_mapping(self, split_summary):
        """Generate class index mapping for training"""
        species_list = sorted(set(
            species 
            for split_counts in split_summary.values() 
            for species in split_counts.keys()
        ))
        
        class_mapping = {
            species: idx 
            for idx, species in enumerate(species_list)
        }
        
        mapping_path = self.output_dir / 'class_mapping.json'
        with open(mapping_path, 'w') as f:
            json.dump(class_mapping, f, indent=2)
        
        print(f"\nClass mapping saved to: {mapping_path}")
        print("\nClass indices:")
        for species, idx in class_mapping.items():
            print(f"  {idx}: {species}")
        
        return class_mapping
    
    def analyze_image_properties(self):
        """Analyze image dimensions and formats"""
        print("\nAnalyzing image properties...")
        
        widths = []
        heights = []
        formats = defaultdict(int)
        modes = defaultdict(int)
        
        for split in ['train', 'val', 'test']:
            split_dir = self.output_dir / split
            for species_dir in split_dir.iterdir():
                if not species_dir.is_dir():
                    continue
                
                for img_path in species_dir.glob('*'):
                    if img_path.suffix.lower() in ['.jpg', '.jpeg', '.png']:
                        stats = self.get_image_stats(img_path)
                        if stats:
                            widths.append(stats['width'])
                            heights.append(stats['height'])
                            formats[stats['format']] += 1
                            modes[stats['mode']] += 1
        
        if widths and heights:
            print(f"\nImage Statistics:")
            print(f"  Width:  min={min(widths)}, max={max(widths)}, avg={sum(widths)//len(widths)}")
            print(f"  Height: min={min(heights)}, max={max(heights)}, avg={sum(heights)//len(heights)}")
            print(f"\nFormats:")
            for fmt, count in formats.items():
                print(f"  {fmt}: {count}")
            print(f"\nColor Modes:")
            for mode, count in modes.items():
                print(f"  {mode}: {count}")
    
    def run(self):
        """Execute the complete data preparation pipeline"""
        print("="*80)
        print("WildCAM_ESP32 Data Preparation Pipeline")
        print("="*80)
        
        # Step 1: Scan dataset
        species_dirs, species_counts = self.scan_dataset()
        
        if not species_dirs:
            print("\n❌ No species directories found!")
            print(f"Make sure your data is in: {self.input_dir}")
            return False
        
        # Step 2: Create splits
        split_summary = self.create_splits(species_dirs)
        
        # Step 3: Generate reports
        self.generate_dataset_report(split_summary)
        class_mapping = self.generate_class_mapping(split_summary)
        
        # Step 4: Analyze images
        self.analyze_image_properties()
        
        print("\n" + "="*80)
        print("✓ Data preparation complete!")
        print(f"✓ Dataset ready for training at: {self.output_dir}")
        print("="*80)
        
        return True

def main():
    parser = argparse.ArgumentParser(
        description='Prepare wildlife dataset for WildCAM_ESP32 training'
    )
    parser.add_argument(
        '--input-dir',
        required=True,
        help='Input directory containing downloaded images'
    )
    parser.add_argument(
        '--output-dir',
        required=True,
        help='Output directory for organized training data'
    )
    parser.add_argument(
        '--train-split',
        type=float,
        default=0.7,
        help='Training set proportion (default: 0.7)'
    )
    parser.add_argument(
        '--val-split',
        type=float,
        default=0.15,
        help='Validation set proportion (default: 0.15)'
    )
    parser.add_argument(
        '--test-split',
        type=float,
        default=0.15,
        help='Test set proportion (default: 0.15)'
    )
    parser.add_argument(
        '--seed',
        type=int,
        default=42,
        help='Random seed for reproducibility (default: 42)'
    )
    
    args = parser.parse_args()
    
    # Set random seed
    random.seed(args.seed)
    
    # Create preparator and run
    preparator = DataPreparator(
        args.input_dir,
        args.output_dir,
        args.train_split,
        args.val_split,
        args.test_split
    )
    
    success = preparator.run()
    
    if success:
        print("\nNext steps:")
        print("  1. Review the dataset_report.txt")
        print("  2. Check the class_mapping.json")
        print("  3. Run the training script: python train_model.py")
    
    return 0 if success else 1

if __name__ == '__main__':
    exit(main())

