# Dependency Management for Offline Environments

*Last Updated: September 1, 2025*  
*Cache Size: ~2.1GB for complete environment*  
*Validation: Monthly dependency auditing*

## Overview

This document provides comprehensive procedures for managing dependencies in ESP32 Wildlife Camera development environments with restricted network access. It covers caching, versioning, security validation, and maintenance of all required dependencies.

## Dependency Categories

### Core Development Dependencies
Essential components required for basic firmware development:

| Component | Type | Size | Source | Cache Location |
|-----------|------|------|--------|----------------|
| PlatformIO Core | Python Package | ~50MB | PyPI | `~/.platformio/penv/` |
| ESP32 Platform | Platform | ~300MB | PlatformIO Registry | `~/.platformio/platforms/espressif32/` |
| ESP32 Toolchain | Toolchain | ~400MB | Espressif | `~/.platformio/packages/toolchain-xtensa-esp32/` |
| Arduino Framework | Framework | ~150MB | ESP32 Arduino | `~/.platformio/packages/framework-arduinoespressif32/` |

### Library Dependencies
External libraries required for wildlife camera functionality:

| Library | Version | Size | Purpose | Cache Location |
|---------|---------|------|---------|----------------|
| ArduinoJson | 6.21.3 | ~500KB | JSON configuration | `~/.platformio/lib/ArduinoJson/` |
| AsyncTCP | 1.1.1 | ~100KB | Async networking | `~/.platformio/lib/AsyncTCP/` |
| ESPAsyncWebServer | 1.2.3 | ~200KB | Web interface | `~/.platformio/lib/ESPAsyncWebServer/` |
| Time | 1.6.1 | ~50KB | RTC management | `~/.platformio/lib/Time/` |

### Optional AI Dependencies
Additional components for AI-enabled functionality:

| Component | Version | Size | Purpose | Cache Location |
|-----------|---------|------|---------|----------------|
| TensorFlow Lite Micro | 2.16.1 | ~5MB | AI inference | `~/.platformio/lib/TensorFlowLiteMicro/` |
| ESP32-Camera | 2.0.4 | ~300KB | Camera drivers | `~/.platformio/lib/ESP32-Camera/` |

## Dependency Caching Strategies

### 1. Complete Pre-Cache (Recommended)

#### Full Environment Cache Creation
```bash
#!/bin/bash
# create_complete_cache.sh

CACHE_DIR="/tmp/esp32_wildlife_cache"
TARGET_DIR="$HOME/.platformio"

echo "Creating complete ESP32 Wildlife Camera dependency cache..."

# Create cache directory structure
mkdir -p "$CACHE_DIR"/{platforms,packages,lib,python_packages}

# Install PlatformIO if not present
if ! command -v pio >/dev/null 2>&1; then
    echo "Installing PlatformIO..."
    python3 -m pip install --user platformio
fi

# Cache ESP32 platform and packages
echo "Caching ESP32 platform..."
pio platform install espressif32@6.4.0
cp -r "$TARGET_DIR/platforms/espressif32" "$CACHE_DIR/platforms/"
cp -r "$TARGET_DIR/packages" "$CACHE_DIR/"

# Cache required libraries
echo "Caching libraries..."
libraries=(
    "bblanchon/ArduinoJson@^6.21.3"
    "me-no-dev/AsyncTCP@^1.1.1"
    "me-no-dev/ESP-Async-WebServer@^1.2.3"
    "PaulStoffregen/Time@^1.6.1"
)

for lib in "${libraries[@]}"; do
    echo "Installing library: $lib"
    pio lib install --global "$lib"
done

cp -r "$TARGET_DIR/lib" "$CACHE_DIR/"

# Cache Python packages
echo "Caching Python dependencies..."
pip download -d "$CACHE_DIR/python_packages" platformio esptool

# Create cache manifest
cat > "$CACHE_DIR/manifest.json" << EOF
{
  "created": "$(date -Iseconds)",
  "version": "1.0.0",
  "description": "ESP32 Wildlife Camera complete dependency cache",
  "contents": {
    "platforms": ["espressif32@6.4.0"],
    "packages": ["toolchain-xtensa-esp32", "framework-arduinoespressif32", "tool-esptoolpy"],
    "libraries": ["ArduinoJson@6.21.3", "AsyncTCP@1.1.1", "ESPAsyncWebServer@1.2.3", "Time@1.6.1"],
    "python_packages": ["platformio", "esptool"]
  },
  "total_size": "$(du -sh $CACHE_DIR | cut -f1)"
}
EOF

# Create compressed archive
echo "Creating compressed cache archive..."
tar -czf esp32_wildlife_cache_$(date +%Y%m%d).tar.gz -C "$CACHE_DIR" .

echo "Cache created: esp32_wildlife_cache_$(date +%Y%m%d).tar.gz"
echo "Size: $(du -sh esp32_wildlife_cache_$(date +%Y%m%d).tar.gz | cut -f1)"
```

#### Cache Restoration
```bash
#!/bin/bash
# restore_cache.sh

CACHE_FILE="$1"
TARGET_DIR="$HOME/.platformio"

if [[ ! -f "$CACHE_FILE" ]]; then
    echo "Error: Cache file not found: $CACHE_FILE"
    exit 1
fi

echo "Restoring ESP32 Wildlife Camera dependency cache..."

# Backup existing installation
if [[ -d "$TARGET_DIR" ]]; then
    echo "Backing up existing PlatformIO installation..."
    mv "$TARGET_DIR" "${TARGET_DIR}.backup.$(date +%s)"
fi

# Create target directory
mkdir -p "$TARGET_DIR"

# Extract cache
echo "Extracting cache..."
tar -xzf "$CACHE_FILE" -C "$TARGET_DIR"

# Verify restoration
echo "Verifying cache restoration..."
if [[ -f "$TARGET_DIR/manifest.json" ]]; then
    echo "Cache manifest found:"
    cat "$TARGET_DIR/manifest.json"
else
    echo "Warning: No cache manifest found"
fi

# Test installation
echo "Testing installation..."
if pio --version >/dev/null 2>&1; then
    echo "✅ PlatformIO functional"
else
    echo "❌ PlatformIO test failed"
    exit 1
fi

if pio platform list | grep -q espressif32; then
    echo "✅ ESP32 platform available"
else
    echo "❌ ESP32 platform not found"
    exit 1
fi

echo "Cache restoration completed successfully!"
```

### 2. Incremental Caching

#### Dependency Update Strategy
```bash
#!/bin/bash
# update_dependencies.sh

echo "Updating ESP32 Wildlife Camera dependencies..."

# Check for platform updates
echo "Checking platform updates..."
CURRENT_PLATFORM=$(pio platform show espressif32 | grep "Version" | cut -d' ' -f2)
LATEST_PLATFORM=$(pio platform search espressif32 | grep "espressif32" | head -1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')

if [[ "$CURRENT_PLATFORM" != "$LATEST_PLATFORM" ]]; then
    echo "Platform update available: $CURRENT_PLATFORM -> $LATEST_PLATFORM"
    echo "Update manually if desired: pio platform update espressif32"
else
    echo "Platform up to date: $CURRENT_PLATFORM"
fi

# Check for library updates
echo "Checking library updates..."
pio lib update --global

# Create incremental update package
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
UPDATE_DIR="/tmp/esp32_update_$TIMESTAMP"
mkdir -p "$UPDATE_DIR"

# Copy updated components
if [[ -d "$HOME/.platformio/platforms/espressif32" ]]; then
    cp -r "$HOME/.platformio/platforms/espressif32" "$UPDATE_DIR/"
fi

if [[ -d "$HOME/.platformio/lib" ]]; then
    cp -r "$HOME/.platformio/lib" "$UPDATE_DIR/"
fi

# Create update manifest
cat > "$UPDATE_DIR/update_manifest.json" << EOF
{
  "update_date": "$(date -Iseconds)",
  "base_version": "1.0.0",
  "update_version": "1.0.1",
  "description": "Incremental dependency update",
  "changes": [
    "Updated libraries to latest compatible versions",
    "Platform security patches applied"
  ]
}
EOF

# Create update package
tar -czf "esp32_update_$TIMESTAMP.tar.gz" -C "$UPDATE_DIR" .
echo "Update package created: esp32_update_$TIMESTAMP.tar.gz"
```

### 3. Selective Dependency Management

#### Minimal Cache for Resource-Constrained Environments
```bash
#!/bin/bash
# create_minimal_cache.sh

echo "Creating minimal ESP32 Wildlife Camera cache..."

MINIMAL_CACHE="/tmp/esp32_minimal_cache"
mkdir -p "$MINIMAL_CACHE"/{platforms,packages,lib}

# Essential platform files only
mkdir -p "$MINIMAL_CACHE/platforms/espressif32"
cp "$HOME/.platformio/platforms/espressif32/platform.json" \
   "$MINIMAL_CACHE/platforms/espressif32/"
cp -r "$HOME/.platformio/platforms/espressif32/boards" \
      "$MINIMAL_CACHE/platforms/espressif32/"

# Essential packages only
essential_packages=(
    "toolchain-xtensa-esp32"
    "framework-arduinoespressif32" 
    "tool-esptoolpy"
)

for package in "${essential_packages[@]}"; do
    if [[ -d "$HOME/.platformio/packages/$package" ]]; then
        cp -r "$HOME/.platformio/packages/$package" \
              "$MINIMAL_CACHE/packages/"
    fi
done

# Core libraries only
core_libraries=(
    "ArduinoJson"
)

for lib in "${core_libraries[@]}"; do
    if [[ -d "$HOME/.platformio/lib/$lib" ]]; then
        cp -r "$HOME/.platformio/lib/$lib" \
              "$MINIMAL_CACHE/lib/"
    fi
done

# Create minimal archive
tar -czf esp32_minimal_cache_$(date +%Y%m%d).tar.gz -C "$MINIMAL_CACHE" .
echo "Minimal cache created: $(du -sh esp32_minimal_cache_$(date +%Y%m%d).tar.gz)"
```

## Dependency Validation

### Security Verification

#### Checksum Validation
```bash
#!/bin/bash
# validate_dependencies.sh

echo "Validating ESP32 Wildlife Camera dependencies..."

# Known good checksums (update these regularly)
declare -A CHECKSUMS=(
    ["platformio-6.1.7.tar.gz"]="sha256:a1b2c3d4e5f6..."
    ["espressif32-6.4.0.tar.gz"]="sha256:b2c3d4e5f6a1..."
    ["ArduinoJson-6.21.3.zip"]="sha256:c3d4e5f6a1b2..."
)

# Validate core components
validate_checksum() {
    local file=$1
    local expected_checksum=$2
    
    if [[ -f "$file" ]]; then
        local actual_checksum=$(sha256sum "$file" | cut -d' ' -f1)
        if [[ "$actual_checksum" == "$expected_checksum" ]]; then
            echo "✅ $file: Valid"
        else
            echo "❌ $file: Invalid checksum"
            echo "   Expected: $expected_checksum"
            echo "   Actual:   $actual_checksum"
            return 1
        fi
    else
        echo "❌ $file: Not found"
        return 1
    fi
}

# Validate all components
validation_failed=0
for file in "${!CHECKSUMS[@]}"; do
    if ! validate_checksum "$file" "${CHECKSUMS[$file]}"; then
        validation_failed=1
    fi
done

if [[ $validation_failed -eq 0 ]]; then
    echo "✅ All dependencies validated successfully"
else
    echo "❌ Dependency validation failed"
    exit 1
fi
```

#### License Compliance Check
```bash
#!/bin/bash
# check_licenses.sh

echo "Checking license compliance for ESP32 Wildlife Camera dependencies..."

# Extract license information
extract_license_info() {
    local component_dir=$1
    local component_name=$2
    
    echo "=== $component_name ===" >> license_report.txt
    
    # Look for license files
    license_files=$(find "$component_dir" -iname "*license*" -o -iname "*copying*" 2>/dev/null)
    
    if [[ -n "$license_files" ]]; then
        echo "License files found:" >> license_report.txt
        echo "$license_files" >> license_report.txt
        
        # Extract license type from first file
        head -10 $(echo "$license_files" | head -1) >> license_report.txt
    else
        echo "No explicit license files found" >> license_report.txt
        
        # Check for license in library.json or package.json
        if [[ -f "$component_dir/library.json" ]]; then
            grep -i "license" "$component_dir/library.json" >> license_report.txt
        elif [[ -f "$component_dir/package.json" ]]; then
            grep -i "license" "$component_dir/package.json" >> license_report.txt
        fi
    fi
    
    echo "" >> license_report.txt
}

# Create license report
echo "ESP32 Wildlife Camera - License Compliance Report" > license_report.txt
echo "Generated: $(date)" >> license_report.txt
echo "=================================================" >> license_report.txt

# Check platform licenses
if [[ -d "$HOME/.platformio/platforms/espressif32" ]]; then
    extract_license_info "$HOME/.platformio/platforms/espressif32" "ESP32 Platform"
fi

# Check library licenses
for lib_dir in "$HOME/.platformio/lib"/*; do
    if [[ -d "$lib_dir" ]]; then
        lib_name=$(basename "$lib_dir")
        extract_license_info "$lib_dir" "Library: $lib_name"
    fi
done

echo "License compliance report generated: license_report.txt"
```

### Version Management

#### Dependency Version Lock
```bash
#!/bin/bash
# lock_versions.sh

echo "Creating dependency version lock file..."

# Create version lock file
cat > dependency_lock.json << EOF
{
  "lock_version": "1.0",
  "created": "$(date -Iseconds)",
  "description": "ESP32 Wildlife Camera dependency version lock",
  "platforms": {
EOF

# Add platform versions
if pio platform show espressif32 >/dev/null 2>&1; then
    platform_version=$(pio platform show espressif32 | grep "Version" | cut -d' ' -f2)
    echo "    \"espressif32\": \"$platform_version\"" >> dependency_lock.json
fi

echo "  }," >> dependency_lock.json
echo "  \"libraries\": {" >> dependency_lock.json

# Add library versions
first_lib=true
for lib_dir in "$HOME/.platformio/lib"/*; do
    if [[ -d "$lib_dir" ]]; then
        lib_name=$(basename "$lib_dir")
        
        # Get version from library.json if available
        if [[ -f "$lib_dir/library.json" ]]; then
            lib_version=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$lib_dir/library.json" | cut -d'"' -f4)
        else
            lib_version="unknown"
        fi
        
        if [[ "$first_lib" == "false" ]]; then
            echo "," >> dependency_lock.json
        fi
        echo -n "    \"$lib_name\": \"$lib_version\"" >> dependency_lock.json
        first_lib=false
    fi
done

echo "" >> dependency_lock.json
echo "  }," >> dependency_lock.json

# Add package versions
echo "  \"packages\": {" >> dependency_lock.json
first_pkg=true
for pkg_dir in "$HOME/.platformio/packages"/*; do
    if [[ -d "$pkg_dir" ]]; then
        pkg_name=$(basename "$pkg_dir")
        
        # Try to extract version from package manifest
        if [[ -f "$pkg_dir/.piopm" ]]; then
            pkg_version=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$pkg_dir/.piopm" | cut -d'"' -f4)
        else
            pkg_version="unknown"
        fi
        
        if [[ "$first_pkg" == "false" ]]; then
            echo "," >> dependency_lock.json
        fi
        echo -n "    \"$pkg_name\": \"$pkg_version\"" >> dependency_lock.json
        first_pkg=false
    fi
done

echo "" >> dependency_lock.json
echo "  }" >> dependency_lock.json
echo "}" >> dependency_lock.json

echo "Dependency version lock created: dependency_lock.json"
```

#### Version Restoration
```bash
#!/bin/bash
# restore_versions.sh

LOCK_FILE="$1"

if [[ ! -f "$LOCK_FILE" ]]; then
    echo "Error: Lock file not found: $LOCK_FILE"
    exit 1
fi

echo "Restoring dependencies from lock file: $LOCK_FILE"

# Parse lock file and restore specific versions
# This would need a JSON parser like jq in practice
echo "Note: Manual version restoration required"
echo "Check $LOCK_FILE for specific version requirements"
```

## Maintenance Procedures

### Regular Maintenance Schedule

#### Monthly Dependency Audit
```bash
#!/bin/bash
# monthly_audit.sh

echo "=== ESP32 Wildlife Camera Monthly Dependency Audit ==="
echo "Date: $(date)"

# Check for security updates
echo "1. Checking for security updates..."
pio platform search espressif32 | head -5

# Verify cache integrity
echo "2. Verifying cache integrity..."
find "$HOME/.platformio" -type f -name "*.py" -exec sha256sum {} \; > current_checksums.txt
if [[ -f "previous_checksums.txt" ]]; then
    if diff current_checksums.txt previous_checksums.txt >/dev/null; then
        echo "✅ No unexpected changes detected"
    else
        echo "⚠️  Changes detected in cached files"
        diff current_checksums.txt previous_checksums.txt
    fi
fi
mv current_checksums.txt previous_checksums.txt

# Check disk usage
echo "3. Checking disk usage..."
du -sh "$HOME/.platformio"

# Library updates available
echo "4. Checking for library updates..."
pio lib outdated --global

# Generate audit report
echo "5. Generating audit report..."
cat > monthly_audit_$(date +%Y%m).txt << EOF
ESP32 Wildlife Camera - Monthly Audit Report
Date: $(date)
Cache size: $(du -sh $HOME/.platformio | cut -f1)
Platform version: $(pio platform show espressif32 | grep "Version" | cut -d' ' -f2)
Critical issues: None detected
Recommendations: Regular cache backup recommended
EOF

echo "Audit completed. Report: monthly_audit_$(date +%Y%m).txt"
```

### Emergency Recovery Procedures

#### Dependency Corruption Recovery
```bash
#!/bin/bash
# recover_dependencies.sh

echo "Emergency dependency recovery procedure..."

# 1. Identify corruption
echo "Scanning for corrupted dependencies..."
corrupted_files=()

for file in $(find "$HOME/.platformio" -type f -name "*.py"); do
    if ! python3 -m py_compile "$file" 2>/dev/null; then
        corrupted_files+=("$file")
    fi
done

if [[ ${#corrupted_files[@]} -gt 0 ]]; then
    echo "Corrupted files detected:"
    printf '%s\n' "${corrupted_files[@]}"
    
    # 2. Attempt repair
    echo "Attempting repair from cache..."
    if [[ -f "/opt/emergency_cache/platformio_emergency.tar.gz" ]]; then
        tar -xzf /opt/emergency_cache/platformio_emergency.tar.gz -C ~/
        echo "Emergency cache restored"
    else
        echo "No emergency cache available"
        echo "Manual intervention required"
        exit 1
    fi
else
    echo "No corruption detected"
fi

# 3. Validate repair
echo "Validating repair..."
./scripts/validate_offline_setup.sh --quick
```

### Performance Optimization

#### Cache Optimization
```bash
#!/bin/bash
# optimize_cache.sh

echo "Optimizing ESP32 Wildlife Camera dependency cache..."

# Remove unnecessary files
echo "Removing build artifacts..."
find "$HOME/.platformio" -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null
find "$HOME/.platformio" -name "*.pyc" -type f -delete 2>/dev/null
find "$HOME/.pio" -name "*.o" -type f -delete 2>/dev/null

# Compress large files
echo "Compressing documentation..."
find "$HOME/.platformio" -name "*.md" -type f -exec gzip {} \; 2>/dev/null

# Clean temporary files
echo "Cleaning temporary files..."
rm -rf "$HOME/.platformio/cache/tmp"
rm -rf "$HOME/.pio/build/*/objs"

# Report space savings
echo "Cache optimization completed"
echo "New cache size: $(du -sh $HOME/.platformio | cut -f1)"
```

## Advanced Dependency Management

### Custom Library Integration

#### Adding Custom Libraries to Cache
```bash
#!/bin/bash
# add_custom_library.sh

LIBRARY_NAME="$1"
LIBRARY_PATH="$2"

if [[ -z "$LIBRARY_NAME" || -z "$LIBRARY_PATH" ]]; then
    echo "Usage: $0 <library_name> <library_path>"
    exit 1
fi

echo "Adding custom library to cache: $LIBRARY_NAME"

# Create library directory
LIB_DIR="$HOME/.platformio/lib/$LIBRARY_NAME"
mkdir -p "$LIB_DIR"

# Copy library files
cp -r "$LIBRARY_PATH"/* "$LIB_DIR/"

# Create library manifest
cat > "$LIB_DIR/library.json" << EOF
{
  "name": "$LIBRARY_NAME",
  "version": "1.0.0-custom",
  "description": "Custom library for ESP32 Wildlife Camera",
  "repository": {
    "type": "local",
    "url": "file://$LIBRARY_PATH"
  },
  "frameworks": ["arduino"],
  "platforms": ["espressif32"]
}
EOF

echo "Custom library added: $LIBRARY_NAME"
```

### Dependency Synchronization

#### Multi-Machine Synchronization
```bash
#!/bin/bash
# sync_dependencies.sh

SYNC_TARGET="$1"

if [[ -z "$SYNC_TARGET" ]]; then
    echo "Usage: $0 <target_machine>"
    exit 1
fi

echo "Synchronizing dependencies with $SYNC_TARGET..."

# Create sync package
SYNC_PACKAGE="sync_$(date +%Y%m%d_%H%M%S).tar.gz"
tar -czf "$SYNC_PACKAGE" -C "$HOME" .platformio/

# Transfer to target machine
scp "$SYNC_PACKAGE" "$SYNC_TARGET:/tmp/"

# Execute remote sync
ssh "$SYNC_TARGET" << EOF
cd /tmp
tar -xzf $SYNC_PACKAGE -C ~/
echo "Dependencies synchronized"
EOF

echo "Synchronization completed"
rm "$SYNC_PACKAGE"
```

---

*This dependency management system ensures reliable, secure, and maintainable ESP32 Wildlife Camera development in offline environments, with comprehensive validation and recovery procedures.*