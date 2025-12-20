#!/usr/bin/env python3
"""
Idaho Camera Traps Dataset Downloader using gsutil
Downloads from Google Cloud Storage (reliable and fast)

Author: Manus AI
Date: October 29, 2025
"""

import os
import sys
import json
import subprocess
import requests
from pathlib import Path
from tqdm import tqdm
import zipfile
import random

# Dataset configuration
METADATA_URL = 'https://storage.googleapis.com/public-datasets-lila/idaho-camera-traps/idaho-camera-traps.json.zip'
GCS_BASE = 'gs://public-datasets-lila/idaho-camera-traps/public/'
HTTP_BASE = 'https://storage.googleapis.com/public-datasets-lila/idaho-camera-traps/public/'

# Target species
TARGET_SPECIES = [
    'deer', 'elk', 'moose', 'bear', 'coyote', 'wolf',
    'mountain lion', 'bobcat', 'fox', 'raccoon',
    'squirrel', 'rabbit', 'skunk', 'opossum', 'porcupine'
]

def check_gsutil():
    """Check if gsutil is installed"""
    try:
        result = subprocess.run(['gsutil', 'version'], capture_output=True, text=True)
        return result.returncode == 0
    except FileNotFoundError:
        return False

def install_gsutil_instructions():
    """Print instructions for installing gsutil"""
    print("\n" + "="*60)
    print("gsutil is not installed")
    print("="*60)
    print("\ngsutil is part of Google Cloud SDK.")
    print("\nInstallation options:")
    print("\n1. Quick install (recommended):")
    print("   curl https://sdk.cloud.google.com | bash")
    print("   exec -l $SHELL")
    print("\n2. Manual install:")
    print("   Visit: https://cloud.google.com/storage/docs/gsutil_install")
    print("\n3. Use pip (simpler but slower):")
    print("   pip install gsutil")
    print("\n" + "="*60)
    
    choice = input("\nInstall gsutil via pip now? (y/n): ").lower()
    if choice == 'y':
        print("Installing gsutil...")
        try:
            subprocess.run([sys.executable, '-m', 'pip', 'install', 'gsutil'], check=True)
            print("✓ gsutil installed")
            return True
        except subprocess.CalledProcessError:
            print("ERROR: Installation failed")
            return False
    return False

def download_metadata(output_dir):
    """Download and extract metadata"""
    print("\n" + "="*60)
    print("Downloading Idaho Camera Traps Metadata")
    print("="*60)
    
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    zip_path = output_dir / 'metadata.zip'
    json_path = output_dir / 'idaho-camera-traps.json'
    
    # Download metadata zip
    print(f"\nDownloading from: {METADATA_URL}")
    try:
        response = requests.get(METADATA_URL, stream=True)
        response.raise_for_status()
        
        total_size = int(response.headers.get('content-length', 0))
        
        with open(zip_path, 'wb') as f, tqdm(
            desc="Downloading metadata",
            total=total_size,
            unit='iB',
            unit_scale=True,
        ) as pbar:
            for chunk in response.iter_content(chunk_size=8192):
                size = f.write(chunk)
                pbar.update(size)
        
        print("✓ Metadata downloaded")
    except Exception as e:
        print(f"ERROR: Failed to download metadata: {e}")
        return None
    
    # Extract JSON
    print("Extracting metadata...")
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(output_dir)
        
        # Find the JSON file
        json_files = list(output_dir.glob('*.json'))
        if json_files:
            json_path = json_files[0]
            print(f"✓ Metadata extracted: {json_path}")
        else:
            print("ERROR: No JSON file found in zip")
            return None
    except Exception as e:
        print(f"ERROR: Failed to extract metadata: {e}")
        return None
    
    # Load metadata
    print("Loading metadata...")
    try:
        with open(json_path, 'r') as f:
            metadata = json.load(f)
        
        print(f"✓ Loaded {len(metadata.get('images', []))} images")
        return metadata
    except Exception as e:
        print(f"ERROR: Failed to load metadata: {e}")
        return None

def filter_species(metadata, max_images_per_species=1500):
    """Filter metadata for target species"""
    print("\n" + "="*60)
    print("Filtering for Target Species")
    print("="*60)
    
    # Create category mapping
    categories = {cat['id']: cat['name'].lower() 
                 for cat in metadata.get('categories', [])}
    
    # Find matching species
    species_matches = {}
    for cat_id, cat_name in categories.items():
        for target in TARGET_SPECIES:
            if target in cat_name or cat_name in target:
                species_matches[cat_id] = cat_name
                print(f"  Found: {cat_name} (ID: {cat_id})")
    
    if not species_matches:
        print("ERROR: No matching species found!")
        return None
    
    # Group images by species
    print("\nGrouping images by species...")
    species_images = {cat_id: [] for cat_id in species_matches.keys()}
    
    for img in tqdm(metadata.get('images', []), desc="Processing images"):
        # Get annotations for this image
        img_annotations = [ann for ann in metadata.get('annotations', []) 
                         if ann.get('image_id') == img.get('id')]
        
        for ann in img_annotations:
            cat_id = ann.get('category_id')
            if cat_id in species_matches:
                species_images[cat_id].append(img)
                break
    
    # Limit and shuffle
    print("\nLimiting to max images per species...")
    for cat_id in species_images:
        if len(species_images[cat_id]) > max_images_per_species:
            random.shuffle(species_images[cat_id])
            species_images[cat_id] = species_images[cat_id][:max_images_per_species]
    
    # Print summary
    print("\n" + "="*60)
    print("Filtered Dataset Summary")
    print("="*60)
    total = 0
    for cat_id, images in species_images.items():
        count = len(images)
        print(f"  {species_matches[cat_id]:20s} {count:6d} images")
        total += count
    print("-"*60)
    print(f"  {'TOTAL':20s} {total:6d} images")
    print("="*60)
    
    return species_images, species_matches

def download_images_http(species_images, species_matches, output_dir):
    """Download images using HTTP (fallback method)"""
    print("\n" + "="*60)
    print("Downloading Images (HTTP method)")
    print("="*60)
    
    output_dir = Path(output_dir)
    
    downloaded = 0
    failed = 0
    
    for cat_id, images in species_images.items():
        species_name = species_matches[cat_id].replace(' ', '_')
        species_dir = output_dir / species_name
        species_dir.mkdir(parents=True, exist_ok=True)
        
        print(f"\nDownloading {species_name}...")
        
        for img in tqdm(images, desc=f"  {species_name}"):
            file_name = img.get('file_name', '')
            if not file_name:
                continue
            
            dest_path = species_dir / Path(file_name).name
            
            if dest_path.exists():
                downloaded += 1
                continue
            
            img_url = HTTP_BASE + file_name
            
            try:
                response = requests.get(img_url, timeout=30)
                response.raise_for_status()
                
                with open(dest_path, 'wb') as f:
                    f.write(response.content)
                
                downloaded += 1
            except Exception as e:
                failed += 1
                if failed < 10:
                    print(f"    Error: {file_name}: {e}")
    
    print("\n" + "="*60)
    print("Download Complete")
    print("="*60)
    print(f"  ✓ Downloaded: {downloaded}")
    print(f"  ✗ Failed: {failed}")
    print("="*60)

def download_images_gsutil(species_images, species_matches, output_dir):
    """Download images using gsutil (faster, batch method)"""
    print("\n" + "="*60)
    print("Downloading Images (gsutil method)")
    print("="*60)
    
    output_dir = Path(output_dir)
    
    # Create a file list for gsutil
    file_list_path = output_dir / 'download_list.txt'
    
    print("Creating download list...")
    with open(file_list_path, 'w') as f:
        for cat_id, images in species_images.items():
            for img in images:
                file_name = img.get('file_name', '')
                if file_name:
                    f.write(f"{GCS_BASE}{file_name}\n")
    
    total_files = sum(len(images) for images in species_images.values())
    print(f"✓ Created list with {total_files} files")
    
    # Download using gsutil
    print("\nDownloading files with gsutil...")
    print("(This may take a while...)")
    
    temp_dir = output_dir / 'temp_download'
    temp_dir.mkdir(parents=True, exist_ok=True)
    
    try:
        subprocess.run([
            'gsutil', '-m', 'cp',
            '-I', str(temp_dir)
        ], stdin=open(file_list_path), check=True)
        
        print("✓ Download complete")
    except subprocess.CalledProcessError as e:
        print(f"ERROR: gsutil download failed: {e}")
        print("Falling back to HTTP download...")
        return False
    
    # Organize downloaded files
    print("\nOrganizing files by species...")
    for cat_id, images in species_images.items():
        species_name = species_matches[cat_id].replace(' ', '_')
        species_dir = output_dir / species_name
        species_dir.mkdir(parents=True, exist_ok=True)
        
        for img in images:
            file_name = img.get('file_name', '')
            if file_name:
                src = temp_dir / Path(file_name).name
                dst = species_dir / Path(file_name).name
                if src.exists():
                    src.rename(dst)
    
    print("✓ Files organized")
    return True

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Download Idaho Camera Traps dataset')
    parser.add_argument('--output-dir', default='./wildlife_data/idaho', help='Output directory')
    parser.add_argument('--max-images', type=int, default=1500, help='Max images per species')
    parser.add_argument('--method', choices=['gsutil', 'http', 'auto'], default='auto', 
                       help='Download method')
    
    args = parser.parse_args()
    
    print("="*60)
    print("Idaho Camera Traps Dataset Downloader")
    print("="*60)
    
    # Check gsutil
    has_gsutil = check_gsutil()
    
    if args.method == 'gsutil' and not has_gsutil:
        print("\ngsutil not found but required for --method gsutil")
        if not install_gsutil_instructions():
            sys.exit(1)
        has_gsutil = check_gsutil()
    
    if args.method == 'auto':
        if has_gsutil:
            print("✓ gsutil found - will use fast batch download")
            args.method = 'gsutil'
        else:
            print("⚠ gsutil not found - will use slower HTTP download")
            args.method = 'http'
    
    # Download metadata
    metadata = download_metadata(args.output_dir)
    if not metadata:
        sys.exit(1)
    
    # Filter species
    result = filter_species(metadata, args.max_images)
    if not result:
        sys.exit(1)
    
    species_images, species_matches = result
    
    # Download images
    if args.method == 'gsutil':
        success = download_images_gsutil(species_images, species_matches, args.output_dir)
        if not success:
            download_images_http(species_images, species_matches, args.output_dir)
    else:
        download_images_http(species_images, species_matches, args.output_dir)
    
    print("\n" + "="*60)
    print("All Done!")
    print("="*60)
    print(f"Dataset location: {args.output_dir}")
    print("\nNext step:")
    print(f"  python prepare_training_data.py --input-dir {args.output_dir} --output-dir ./training_data")
    print("="*60)

if __name__ == '__main__':
    main()

