#!/usr/bin/env python3
"""
3D Reconstruction Image Aggregator for WildCAM ESP32

This script aggregates images captured from multiple synchronized cameras
and prepares them for 3D reconstruction using external tools like Meshroom or COLMAP.

Features:
- Collects images from multiple camera nodes
- Parses camera position and orientation metadata
- Validates image synchronization
- Exports camera calibration data for reconstruction
- Generates scripts for popular 3D reconstruction tools

Usage:
    python3 3d_reconstruction_aggregator.py --session <session_id> --input <input_dir> --output <output_dir>
    
Dependencies:
    pip install pillow numpy
    
For 3D reconstruction:
    - Meshroom: https://alicevision.org/#meshroom
    - COLMAP: https://colmap.github.io/
"""

import argparse
import json
import os
import sys
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Any, Tuple

try:
    from PIL import Image
    import PIL.ExifTags as ExifTags
except ImportError:
    print("Warning: PIL (Pillow) not installed. Install with: pip install pillow")
    Image = None
    ExifTags = None

try:
    import numpy as np
except ImportError:
    print("Warning: numpy not installed. Install with: pip install numpy")
    np = None


class CameraMetadata:
    """Represents metadata for a single camera capture"""
    
    def __init__(self, metadata_file: Path):
        self.metadata_file = metadata_file
        with open(metadata_file, 'r') as f:
            self.data = json.load(f)
        
        self.session_id = self.data.get('session_id', '')
        self.node_id = self.data.get('node_id', 0)
        self.timestamp = self.data.get('timestamp', 0)
        self.image_filename = self.data.get('image_filename', '')
        
        # Camera position (x, y, z in meters or other units)
        pos = self.data.get('camera_position', {})
        self.position = (
            float(pos.get('x', 0)),
            float(pos.get('y', 0)),
            float(pos.get('z', 0))
        )
        
        # Camera orientation (pitch, yaw, roll in degrees)
        orient = self.data.get('camera_orientation', {})
        self.orientation = (
            float(orient.get('pitch', 0)),
            float(orient.get('yaw', 0)),
            float(orient.get('roll', 0))
        )
        
        # Image properties
        props = self.data.get('image_properties', {})
        self.width = props.get('width', 0)
        self.height = props.get('height', 0)
        self.format = props.get('format', 0)
        self.size_bytes = props.get('size_bytes', 0)
    
    def __repr__(self):
        return f"Camera(node={self.node_id}, pos={self.position}, orient={self.orientation})"


class ReconstructionAggregator:
    """Aggregates multi-camera images for 3D reconstruction"""
    
    def __init__(self, input_dir: Path, output_dir: Path, session_id: str = None):
        self.input_dir = Path(input_dir)
        self.output_dir = Path(output_dir)
        self.session_id = session_id
        self.cameras: List[CameraMetadata] = []
        
        # Create output directory structure
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.images_dir = self.output_dir / "images"
        self.images_dir.mkdir(exist_ok=True)
        self.metadata_dir = self.output_dir / "metadata"
        self.metadata_dir.mkdir(exist_ok=True)
    
    def find_capture_sessions(self) -> List[str]:
        """Find all capture sessions in the input directory"""
        sessions = set()
        for metadata_file in self.input_dir.rglob("*_3d_meta.json"):
            with open(metadata_file, 'r') as f:
                data = json.load(f)
                session_id = data.get('session_id', '')
                if session_id:
                    sessions.add(session_id)
        return sorted(list(sessions))
    
    def load_session_images(self, session_id: str = None) -> int:
        """Load all images for a specific session"""
        target_session = session_id or self.session_id
        
        if not target_session:
            print("Error: No session ID specified")
            return 0
        
        print(f"Loading images for session: {target_session}")
        
        # Find all metadata files for this session
        metadata_files = []
        for metadata_file in self.input_dir.rglob("*_3d_meta.json"):
            with open(metadata_file, 'r') as f:
                data = json.load(f)
                if data.get('session_id', '') == target_session:
                    metadata_files.append(metadata_file)
        
        print(f"Found {len(metadata_files)} camera captures for session {target_session}")
        
        # Load metadata for each camera
        for metadata_file in metadata_files:
            try:
                camera = CameraMetadata(metadata_file)
                self.cameras.append(camera)
                print(f"  Loaded: {camera}")
            except Exception as e:
                print(f"  Error loading {metadata_file}: {e}")
        
        return len(self.cameras)
    
    def validate_synchronization(self, max_time_diff_ms: int = 1000) -> bool:
        """Validate that images were captured within acceptable time window"""
        if len(self.cameras) < 2:
            print("Warning: Need at least 2 cameras for synchronization check")
            return True
        
        timestamps = [cam.timestamp for cam in self.cameras]
        min_time = min(timestamps)
        max_time = max(timestamps)
        time_diff = max_time - min_time
        
        print(f"Synchronization check: {time_diff}ms time difference")
        
        if time_diff > max_time_diff_ms:
            print(f"Warning: Time difference ({time_diff}ms) exceeds threshold ({max_time_diff_ms}ms)")
            return False
        
        print("Synchronization: OK")
        return True
    
    def copy_images_to_output(self) -> int:
        """Copy all session images to output directory with standardized names"""
        copied = 0
        
        for i, camera in enumerate(self.cameras):
            # Find the image file
            image_path = self.input_dir / camera.image_filename
            if not image_path.exists():
                # Try relative path from metadata file location
                metadata_dir = camera.metadata_file.parent
                image_path = metadata_dir / Path(camera.image_filename).name
            
            if not image_path.exists():
                print(f"Warning: Image not found: {camera.image_filename}")
                continue
            
            # Copy to output with standardized name
            output_name = f"cam_{camera.node_id:03d}_{i:04d}.jpg"
            output_path = self.images_dir / output_name
            
            try:
                import shutil
                shutil.copy2(image_path, output_path)
                copied += 1
                print(f"  Copied: {camera.image_filename} -> {output_name}")
            except Exception as e:
                print(f"  Error copying {image_path}: {e}")
        
        return copied
    
    def export_camera_calibration(self) -> str:
        """Export camera positions and orientations for reconstruction"""
        calibration_file = self.metadata_dir / "camera_calibration.json"
        
        calibration_data = {
            "session_id": self.session_id,
            "timestamp": datetime.now().isoformat(),
            "num_cameras": len(self.cameras),
            "cameras": []
        }
        
        for i, camera in enumerate(self.cameras):
            cam_data = {
                "camera_id": i,
                "node_id": camera.node_id,
                "image_filename": f"cam_{camera.node_id:03d}_{i:04d}.jpg",
                "position": {
                    "x": camera.position[0],
                    "y": camera.position[1],
                    "z": camera.position[2],
                    "units": "meters"
                },
                "orientation": {
                    "pitch": camera.orientation[0],
                    "yaw": camera.orientation[1],
                    "roll": camera.orientation[2],
                    "units": "degrees"
                },
                "resolution": {
                    "width": camera.width,
                    "height": camera.height
                }
            }
            calibration_data["cameras"].append(cam_data)
        
        with open(calibration_file, 'w') as f:
            json.dump(calibration_data, f, indent=2)
        
        print(f"Exported camera calibration: {calibration_file}")
        return str(calibration_file)
    
    def generate_meshroom_script(self) -> str:
        """Generate a script to run Meshroom reconstruction"""
        script_file = self.output_dir / "run_meshroom.sh"
        
        script_content = f"""#!/bin/bash
# Meshroom 3D Reconstruction Script
# Generated by WildCAM ESP32 3D Reconstruction Aggregator
# Session: {self.session_id}
# Cameras: {len(self.cameras)}

IMAGES_DIR="{self.images_dir.absolute()}"
OUTPUT_DIR="{self.output_dir.absolute()}/meshroom_output"

echo "Starting Meshroom 3D reconstruction..."
echo "Input images: $IMAGES_DIR"
echo "Output directory: $OUTPUT_DIR"

# Check if Meshroom is installed
if ! command -v meshroom_batch &> /dev/null; then
    echo "Error: Meshroom not found. Please install from https://alicevision.org/#meshroom"
    exit 1
fi

# Run Meshroom batch processing
meshroom_batch \\
    --input "$IMAGES_DIR" \\
    --output "$OUTPUT_DIR" \\
    --save "$OUTPUT_DIR/reconstruction.mg"

echo "Meshroom reconstruction complete!"
echo "Results saved to: $OUTPUT_DIR"
"""
        
        with open(script_file, 'w') as f:
            f.write(script_content)
        
        # Make script executable
        os.chmod(script_file, 0o755)
        
        print(f"Generated Meshroom script: {script_file}")
        return str(script_file)
    
    def generate_colmap_script(self) -> str:
        """Generate a script to run COLMAP reconstruction"""
        script_file = self.output_dir / "run_colmap.sh"
        
        script_content = f"""#!/bin/bash
# COLMAP 3D Reconstruction Script
# Generated by WildCAM ESP32 3D Reconstruction Aggregator
# Session: {self.session_id}
# Cameras: {len(self.cameras)}

IMAGES_DIR="{self.images_dir.absolute()}"
WORKSPACE_DIR="{self.output_dir.absolute()}/colmap_workspace"
DATABASE_PATH="$WORKSPACE_DIR/database.db"

echo "Starting COLMAP 3D reconstruction..."
echo "Input images: $IMAGES_DIR"
echo "Workspace: $WORKSPACE_DIR"

# Check if COLMAP is installed
if ! command -v colmap &> /dev/null; then
    echo "Error: COLMAP not found. Please install from https://colmap.github.io/"
    exit 1
fi

# Create workspace
mkdir -p "$WORKSPACE_DIR"

# Feature extraction
echo "Step 1: Feature extraction..."
colmap feature_extractor \\
    --database_path "$DATABASE_PATH" \\
    --image_path "$IMAGES_DIR"

# Feature matching
echo "Step 2: Feature matching..."
colmap exhaustive_matcher \\
    --database_path "$DATABASE_PATH"

# Sparse reconstruction
echo "Step 3: Sparse reconstruction..."
mkdir -p "$WORKSPACE_DIR/sparse"
colmap mapper \\
    --database_path "$DATABASE_PATH" \\
    --image_path "$IMAGES_DIR" \\
    --output_path "$WORKSPACE_DIR/sparse"

# Dense reconstruction (optional, computationally intensive)
echo "Step 4: Dense reconstruction..."
mkdir -p "$WORKSPACE_DIR/dense"
colmap image_undistorter \\
    --image_path "$IMAGES_DIR" \\
    --input_path "$WORKSPACE_DIR/sparse/0" \\
    --output_path "$WORKSPACE_DIR/dense" \\
    --output_type COLMAP

colmap patch_match_stereo \\
    --workspace_path "$WORKSPACE_DIR/dense" \\
    --workspace_format COLMAP \\
    --PatchMatchStereo.geom_consistency true

colmap stereo_fusion \\
    --workspace_path "$WORKSPACE_DIR/dense" \\
    --workspace_format COLMAP \\
    --input_type geometric \\
    --output_path "$WORKSPACE_DIR/dense/fused.ply"

echo "COLMAP reconstruction complete!"
echo "Sparse model: $WORKSPACE_DIR/sparse/0"
echo "Dense model: $WORKSPACE_DIR/dense/fused.ply"
"""
        
        with open(script_file, 'w') as f:
            f.write(script_content)
        
        # Make script executable
        os.chmod(script_file, 0o755)
        
        print(f"Generated COLMAP script: {script_file}")
        return str(script_file)
    
    def generate_readme(self) -> str:
        """Generate README with instructions"""
        readme_file = self.output_dir / "README.md"
        
        readme_content = f"""# 3D Reconstruction Data
## Session: {self.session_id}

### Overview
This directory contains aggregated images from {len(self.cameras)} synchronized cameras
for 3D reconstruction of wildlife subjects.

**Captured:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

### Directory Structure
```
{self.output_dir.name}/
├── images/                   # Synchronized camera images
├── metadata/                 # Camera calibration data
│   └── camera_calibration.json
├── run_meshroom.sh          # Meshroom reconstruction script
├── run_colmap.sh            # COLMAP reconstruction script
└── README.md                # This file
```

### Camera Setup
"""
        
        for i, camera in enumerate(self.cameras):
            readme_content += f"""
**Camera {i+1}** (Node {camera.node_id})
- Position: ({camera.position[0]:.2f}, {camera.position[1]:.2f}, {camera.position[2]:.2f})
- Orientation: Pitch={camera.orientation[0]:.1f}°, Yaw={camera.orientation[1]:.1f}°, Roll={camera.orientation[2]:.1f}°
- Resolution: {camera.width}x{camera.height}
"""
        
        readme_content += """
### Reconstruction Workflow

#### Option 1: Meshroom (User-friendly GUI)
1. Install Meshroom from https://alicevision.org/#meshroom
2. Run the provided script:
   ```bash
   ./run_meshroom.sh
   ```
3. Results will be in `meshroom_output/`

#### Option 2: COLMAP (Advanced, command-line)
1. Install COLMAP from https://colmap.github.io/
2. Run the provided script:
   ```bash
   ./run_colmap.sh
   ```
3. Results will be in `colmap_workspace/`

#### Option 3: Manual Processing
1. Open your preferred photogrammetry software
2. Import images from `images/`
3. Use camera calibration data from `metadata/camera_calibration.json`
4. Process according to software documentation

### Tips for Best Results
- Ensure cameras had adequate overlap in their fields of view
- Verify synchronization was within acceptable limits
- More cameras = better reconstruction quality
- Avoid moving subjects for best results
- Good lighting improves feature detection

### Camera Calibration Data
See `metadata/camera_calibration.json` for detailed camera positions,
orientations, and intrinsic parameters.

---
Generated by WildCAM ESP32 3D Reconstruction Aggregator
"""
        
        with open(readme_file, 'w') as f:
            f.write(readme_content)
        
        print(f"Generated README: {readme_file}")
        return str(readme_file)
    
    def process(self) -> bool:
        """Execute full aggregation pipeline"""
        print("="*60)
        print("WildCAM ESP32 - 3D Reconstruction Aggregator")
        print("="*60)
        
        # Load images for session
        if self.load_session_images() == 0:
            print("Error: No images found for session")
            return False
        
        # Validate synchronization
        self.validate_synchronization()
        
        # Copy images
        print("\nCopying images...")
        copied = self.copy_images_to_output()
        print(f"Copied {copied} images")
        
        # Export camera data
        print("\nExporting camera calibration...")
        self.export_camera_calibration()
        
        # Generate reconstruction scripts
        print("\nGenerating reconstruction scripts...")
        self.generate_meshroom_script()
        self.generate_colmap_script()
        
        # Generate README
        print("\nGenerating documentation...")
        self.generate_readme()
        
        print("\n" + "="*60)
        print(f"Aggregation complete!")
        print(f"Output directory: {self.output_dir.absolute()}")
        print("="*60)
        
        return True


def main():
    parser = argparse.ArgumentParser(
        description="Aggregate multi-camera images for 3D reconstruction",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Process a specific session
  python3 3d_reconstruction_aggregator.py --session session_001 --input /sd/3d_captures --output ./output
  
  # List available sessions
  python3 3d_reconstruction_aggregator.py --list --input /sd/3d_captures
        """
    )
    
    parser.add_argument('--input', '-i', 
                        required=True,
                        help='Input directory containing captured images and metadata')
    
    parser.add_argument('--output', '-o',
                        help='Output directory for aggregated data (default: ./3d_reconstruction_output)')
    
    parser.add_argument('--session', '-s',
                        help='Session ID to process')
    
    parser.add_argument('--list', '-l',
                        action='store_true',
                        help='List available capture sessions')
    
    args = parser.parse_args()
    
    input_dir = Path(args.input)
    if not input_dir.exists():
        print(f"Error: Input directory not found: {input_dir}")
        return 1
    
    # List sessions mode
    if args.list:
        aggregator = ReconstructionAggregator(input_dir, Path("."), None)
        sessions = aggregator.find_capture_sessions()
        print(f"Found {len(sessions)} capture sessions:")
        for session in sessions:
            print(f"  - {session}")
        return 0
    
    # Process mode
    if not args.session:
        print("Error: --session is required (use --list to see available sessions)")
        return 1
    
    output_dir = Path(args.output) if args.output else Path("./3d_reconstruction_output")
    
    aggregator = ReconstructionAggregator(input_dir, output_dir, args.session)
    
    if aggregator.process():
        return 0
    else:
        return 1


if __name__ == "__main__":
    sys.exit(main())
