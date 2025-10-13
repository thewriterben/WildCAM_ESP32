# 3D Asset Pipeline for AR Wildlife Models

## Overview

This document describes the complete pipeline for creating, optimizing, and deploying 3D wildlife models for the AR mobile interface.

## Asset Requirements

### Model Specifications

| Requirement | Specification |
|------------|---------------|
| Format | GLTF 2.0 / GLB (binary) |
| Maximum File Size | 50MB per species (all LODs) |
| Polygon Budget | 100K triangles (LOD0) |
| Texture Resolution | 2048x2048 (LOD0) |
| Animation Format | Skeletal, 30 FPS |
| PBR Materials | Required |
| Compression | Draco geometry, KTX2 textures |

### LOD (Level of Detail) Requirements

| LOD Level | Use Case | Triangle Count | Distance | File Size Target |
|-----------|----------|----------------|----------|------------------|
| LOD0 | Close inspection | 50K-100K | 0-5m | 15-25MB |
| LOD1 | Normal viewing | 20K-50K | 5-20m | 8-15MB |
| LOD2 | Mid distance | 5K-20K | 20-50m | 3-8MB |
| LOD3 | Far distance | 1K-5K | 50m+ | 1-3MB |

## Creation Workflow

### 1. Reference Gathering

**Sources:**
- Field photography
- Museum specimens
- Scientific illustrations
- Video footage
- Anatomical diagrams

**Requirements:**
- High-resolution photos from multiple angles
- Scale references (measurements)
- Behavioral video clips
- Audio recordings of calls

### 2. Photogrammetry (Recommended Method)

**Software:**
- RealityCapture (Commercial)
- Meshroom (Open Source)
- Agisoft Metashape (Commercial)

**Process:**
```bash
# Example using Meshroom
1. Import 100-200 photos of subject
2. Run photo alignment
3. Build dense point cloud
4. Generate mesh
5. Create texture maps
6. Export as OBJ/FBX
```

**Best Practices:**
- Capture in overcast lighting (even illumination)
- Use tripod or turntable for consistent angles
- Overlap photos by 60-80%
- Include scale reference in shots
- Shoot RAW format for best quality

### 3. Manual 3D Modeling (Alternative Method)

**Software:**
- Blender (Open Source)
- ZBrush (Commercial)
- Maya/3ds Max (Commercial)

**Process:**
```
1. Base mesh creation
   - Start with primitive shapes
   - Build anatomically correct base
   - Focus on major muscle groups

2. Sculpting (ZBrush/Blender)
   - Add fine detail
   - Create fur/feather patterns
   - Sculpt facial features

3. Retopology
   - Create low-poly version
   - Maintain silhouette
   - Optimize edge loops for animation

4. UV Unwrapping
   - Minimize seams
   - Maximize texture space usage
   - Align to anatomical features

5. Texturing
   - Base color (albedo)
   - Normal maps
   - Roughness
   - Metallic (minimal for organics)
```

## Optimization Pipeline

### 1. Geometry Optimization

**Blender Script:**
```python
import bpy

def optimize_mesh(obj, target_tris):
    """Reduce mesh to target triangle count"""
    # Select object
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    
    # Add decimate modifier
    mod = obj.modifiers.new("Decimate", 'DECIMATE')
    mod.decimate_type = 'COLLAPSE'
    
    # Calculate ratio
    current_tris = len(obj.data.polygons)
    mod.ratio = target_tris / current_tris
    
    # Apply modifier
    bpy.ops.object.modifier_apply(modifier="Decimate")
    
    print(f"Reduced from {current_tris} to {len(obj.data.polygons)} triangles")

# Usage
obj = bpy.context.active_object
optimize_mesh(obj, 50000)  # Target 50K triangles
```

**Manual Optimization:**
- Remove hidden geometry
- Merge coplanar faces
- Remove doubles (merge vertices)
- Delete interior faces
- Simplify symmetrical features

### 2. LOD Generation

**Automated LOD Creation:**
```python
import bpy

def create_lod_levels(source_obj, lod_counts):
    """Create multiple LOD levels"""
    lod_objects = []
    
    for i, tri_count in enumerate(lod_counts):
        # Duplicate object
        new_obj = source_obj.copy()
        new_obj.data = source_obj.data.copy()
        new_obj.name = f"{source_obj.name}_LOD{i}"
        bpy.context.collection.objects.link(new_obj)
        
        # Optimize
        optimize_mesh(new_obj, tri_count)
        lod_objects.append(new_obj)
    
    return lod_objects

# Generate LODs
lod_counts = [75000, 30000, 10000, 3000]  # LOD0-3
lod_models = create_lod_levels(bpy.context.active_object, lod_counts)
```

### 3. Texture Optimization

**Resolution Targets:**
```
LOD0: 2048x2048 (Base Color, Normal)
LOD1: 1024x1024 (Base Color, Normal)
LOD2: 512x512 (Base Color), 1024x1024 (Normal)
LOD3: 512x512 (Base Color only)
```

**Compression Pipeline:**

```bash
# Install tools
npm install -g gltf-pipeline
pip install Pillow

# Optimize textures (Python script)
from PIL import Image

def compress_texture(input_path, output_path, size, quality=85):
    """Compress and resize texture"""
    img = Image.open(input_path)
    img = img.resize((size, size), Image.LANCZOS)
    img.save(output_path, 'JPEG', quality=quality, optimize=True)

# Usage
compress_texture('deer_albedo.png', 'deer_albedo_1k.jpg', 1024, 85)
compress_texture('deer_normal.png', 'deer_normal_1k.jpg', 1024, 90)

# Convert to KTX2 for mobile
toktx --t2 --genmipmap --bcmp deer_albedo.ktx2 deer_albedo_1k.jpg
```

### 4. GLTF Export

**Blender Export Settings:**
```python
import bpy

def export_gltf(obj, output_path, lod_level):
    """Export as GLTF with optimization"""
    
    # Select object
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    
    # Export settings
    bpy.ops.export_scene.gltf(
        filepath=output_path,
        export_format='GLB',
        export_selected=True,
        export_draco_mesh_compression_enable=True,
        export_draco_mesh_compression_level=10,
        export_texture_dir='',
        export_jpeg_quality=85,
        export_image_format='JPEG',
        export_yup=True
    )

# Export LODs
for i, lod_obj in enumerate(lod_models):
    export_gltf(lod_obj, f'deer_lod{i}.glb', i)
```

## Animation Pipeline

### 1. Rigging

**Bone Structure:**
```
Root
├── Spine_01
│   ├── Spine_02
│   │   ├── Spine_03
│   │   │   ├── Neck_01
│   │   │   │   ├── Neck_02
│   │   │   │   │   └── Head
│   │   │   │   │       ├── Jaw
│   │   │   │   │       ├── Ear_L
│   │   │   │   │       └── Ear_R
│   │   │   └── Shoulder_L/R
│   │   │       └── Elbow_L/R
│   │   │           └── Wrist_L/R
│   │   │               └── Hoof_L/R
│   │   └── Pelvis
│   │       └── Hip_L/R
│   │           └── Knee_L/R
│   │               └── Ankle_L/R
│   │                   └── Hoof_L/R
│   └── Tail_01
│       ├── Tail_02
│       └── Tail_03
```

**Blender Auto-Rigify:**
```python
import bpy

def create_rig(armature_name):
    """Create basic rig for quadruped"""
    bpy.ops.object.armature_add()
    rig = bpy.context.active_object
    rig.name = armature_name
    
    # Switch to edit mode and add bones
    bpy.ops.object.mode_set(mode='EDIT')
    
    # Add bones programmatically
    # (simplified - use Rigify addon for production)
    
    return rig

# Create rig
deer_rig = create_rig("Deer_Rig")
```

### 2. Weight Painting

**Automatic Weights:**
```python
import bpy

def auto_weight(mesh_obj, armature_obj):
    """Automatically assign bone weights"""
    # Select mesh and armature
    bpy.context.view_layer.objects.active = mesh_obj
    mesh_obj.select_set(True)
    armature_obj.select_set(True)
    
    # Parent with automatic weights
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')

# Usage
auto_weight(deer_mesh, deer_rig)
```

### 3. Animation Creation

**Required Animations:**

| Animation | Duration | Loop | Priority |
|-----------|----------|------|----------|
| Idle | 3-5s | Yes | High |
| Walk | 2-3s | Yes | High |
| Run | 1-2s | Yes | High |
| Feed | 3-5s | Yes | Medium |
| Drink | 3-5s | No | Medium |
| Alert | 1-2s | No | High |
| Groom | 3-5s | Yes | Low |
| Sleep | 5-10s | Yes | Low |

**Animation Export:**
```python
import bpy

def export_animation(armature, anim_name, start_frame, end_frame):
    """Export single animation"""
    bpy.context.scene.frame_start = start_frame
    bpy.context.scene.frame_end = end_frame
    
    # Select armature
    bpy.ops.object.select_all(action='DESELECT')
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    
    # Export
    bpy.ops.export_scene.gltf(
        filepath=f'{anim_name}.glb',
        export_animations=True,
        export_frame_range=True,
        export_nla_strips=False
    )

# Export animations
export_animation(deer_rig, 'deer_walk', 1, 60)
export_animation(deer_rig, 'deer_run', 61, 90)
```

## Texture Pipeline

### 1. PBR Material Setup

**Required Maps:**
- Base Color (Albedo)
- Normal Map
- Roughness
- Metallic (minimal for organic materials)
- Ambient Occlusion (baked)

**Texture Creation (Substance Painter):**
```
1. Import base mesh
2. Auto-unwrap if needed
3. Create base material layers:
   - Fur/feather base
   - Color variations
   - Dirt/wear
   - Subsurface details

4. Export texture sets:
   - 2048x2048 for LOD0
   - 1024x1024 for LOD1
   - 512x512 for LOD2+

5. Export presets:
   - GLTF PBR Metallic Roughness
   - Separate alpha for cutouts
```

### 2. Seasonal Variations

**Texture Variants:**
```
deer_summer.ktx2      # Reddish-brown coat
deer_winter.ktx2      # Grayish-brown coat
deer_spring.ktx2      # Transitional coat
deer_fall.ktx2        # Transitional coat
```

**Blending Script:**
```python
from PIL import Image

def blend_seasonal_textures(base, overlay, blend_factor):
    """Blend two textures for seasonal transition"""
    base_img = Image.open(base)
    overlay_img = Image.open(overlay)
    
    # Blend images
    blended = Image.blend(base_img, overlay_img, blend_factor)
    return blended

# Create transition textures
spring_tex = blend_seasonal_textures('deer_winter.jpg', 'deer_summer.jpg', 0.5)
spring_tex.save('deer_spring.jpg')
```

## Quality Assurance

### 1. Validation Checklist

- [ ] Polygon count within budget
- [ ] All textures power-of-2 dimensions
- [ ] No flipped normals
- [ ] Clean UV layout (no overlaps)
- [ ] Animations loop smoothly
- [ ] Materials use PBR workflow
- [ ] File size under target
- [ ] GLTF validation passes
- [ ] No unused vertices/materials
- [ ] Proper scale (real-world units)

### 2. Testing

**Desktop Testing:**
```bash
# View in Blender
blender deer_lod0.glb

# View in glTF Viewer
npx gltf-viewer deer_lod0.glb

# Validate
gltf_validator deer_lod0.glb
```

**Mobile Testing:**
```bash
# Deploy to test device
adb push deer_lod0.glb /sdcard/wildlife_models/

# Run test app
adb shell am start -n com.wildlife.monitor/.ARTestActivity
```

### 3. Performance Profiling

**Metrics to Check:**
- Load time: <2 seconds target
- Memory usage: <50MB per model
- Frame rate impact: <5ms render time
- Texture streaming: <500ms

## Deployment

### 1. Asset Organization

```
models/
├── mammals/
│   ├── white_tailed_deer/
│   │   ├── deer_lod0.glb (25MB)
│   │   ├── deer_lod1.glb (12MB)
│   │   ├── deer_lod2.glb (5MB)
│   │   ├── deer_lod3.glb (2MB)
│   │   ├── animations/
│   │   │   ├── walk.bin
│   │   │   ├── run.bin
│   │   │   └── feed.bin
│   │   └── metadata.json
│   ├── black_bear/
│   └── ...
├── birds/
└── reptiles/
```

### 2. Metadata File

**metadata.json:**
```json
{
  "modelId": "model_white_tailed_deer",
  "speciesId": "white_tailed_deer",
  "version": "1.0.0",
  "created": "2024-10-01",
  "author": "Wildlife 3D Team",
  "lodLevels": [
    {
      "level": 0,
      "file": "deer_lod0.glb",
      "triangles": 75000,
      "fileSize": 26214400,
      "maxDistance": 5.0
    },
    {
      "level": 1,
      "file": "deer_lod1.glb",
      "triangles": 30000,
      "fileSize": 12582912,
      "maxDistance": 20.0
    }
  ],
  "animations": [
    {
      "name": "walk",
      "file": "animations/walk.bin",
      "duration": 2.5,
      "fps": 30,
      "loopable": true
    }
  ],
  "textures": [
    {
      "name": "summer",
      "season": "SUMMER",
      "files": {
        "baseColor": "textures/deer_summer_bc.ktx2",
        "normal": "textures/deer_normal.ktx2"
      }
    }
  ],
  "boundingBox": {
    "min": [-0.8, 0.0, -0.3],
    "max": [0.8, 1.8, 0.3]
  },
  "realWorldScale": {
    "length": 1.8,
    "height": 1.0,
    "weight": 68.0
  }
}
```

### 3. CDN Upload

```bash
# Upload to CDN
aws s3 cp models/ s3://wildlife-cdn/models/ \
  --recursive \
  --acl public-read \
  --cache-control "max-age=31536000"

# Invalidate cache
aws cloudfront create-invalidation \
  --distribution-id E123456 \
  --paths "/models/*"
```

### 4. Database Registration

```sql
-- Register model in database
INSERT INTO ar_models (
  species_id,
  model_id,
  version,
  lod0_url,
  lod1_url,
  lod2_url,
  lod3_url,
  thumbnail_url,
  total_size,
  created_at
) VALUES (
  'white_tailed_deer',
  'model_white_tailed_deer',
  '1.0.0',
  'https://cdn.wildlife-monitor.com/models/mammals/white_tailed_deer/deer_lod0.glb',
  'https://cdn.wildlife-monitor.com/models/mammals/white_tailed_deer/deer_lod1.glb',
  'https://cdn.wildlife-monitor.com/models/mammals/white_tailed_deer/deer_lod2.glb',
  'https://cdn.wildlife-monitor.com/models/mammals/white_tailed_deer/deer_lod3.glb',
  'https://cdn.wildlife-monitor.com/thumbnails/white_tailed_deer.jpg',
  44040192,
  NOW()
);
```

## Tools & Resources

### Recommended Software

| Tool | Purpose | Cost | Link |
|------|---------|------|------|
| Blender | 3D modeling, rigging, animation | Free | blender.org |
| Substance Painter | Texture creation | Paid | adobe.com |
| RealityCapture | Photogrammetry | Paid | capturingreality.com |
| Meshroom | Photogrammetry | Free | alicevision.org |
| gltf-pipeline | GLTF optimization | Free | github.com/CesiumGS |

### Learning Resources

- Blender fundamentals: blender.org/support/tutorials
- PBR texturing: substance3d.adobe.com/tutorials
- Animal anatomy: anatomy4sculptors.com
- GLTF specification: khronos.org/gltf

### Reference Databases

- Morphosource (3D scans): morphosource.org
- Sketchfab (models): sketchfab.com
- iNaturalist (photos): inaturalist.org
- Xeno-canto (sounds): xeno-canto.org

## Batch Processing

### Automation Script

```bash
#!/bin/bash
# batch_process_models.sh

SPECIES_LIST="white_tailed_deer black_bear raccoon coyote"

for SPECIES in $SPECIES_LIST; do
  echo "Processing $SPECIES..."
  
  # Run Blender script
  blender -b ${SPECIES}_source.blend -P optimize_model.py
  
  # Export LODs
  blender -b ${SPECIES}_optimized.blend -P export_lods.py
  
  # Compress textures
  python compress_textures.py $SPECIES
  
  # Validate
  gltf_validator ${SPECIES}_lod0.glb
  
  # Upload to CDN
  aws s3 cp ${SPECIES}_*.glb s3://wildlife-cdn/models/${SPECIES}/
  
  echo "✓ $SPECIES complete"
done
```

## Troubleshooting

### Common Issues

**Problem**: Model appears black in AR
**Solution**: Check material is PBR, has albedo texture, normals facing correct direction

**Problem**: Animations don't play
**Solution**: Verify bone naming, check animation export settings, ensure root motion

**Problem**: Textures blurry
**Solution**: Increase resolution, check mipmap generation, verify texture format

**Problem**: Model loads slowly
**Solution**: Reduce polygon count, compress textures, enable Draco compression

**Problem**: File size too large
**Solution**: Aggressive LOD reduction, texture compression, remove unused data

## Version Control

Use Git LFS for model files:

```bash
# Track model files
git lfs track "*.glb"
git lfs track "*.ktx2"
git lfs track "*.bin"

# Commit
git add .gitattributes
git add models/
git commit -m "Add deer 3D model with LODs"
```

## Conclusion

Following this pipeline ensures consistent, high-quality 3D assets optimized for mobile AR. Regular updates and quality checks maintain the library's usability and performance.
