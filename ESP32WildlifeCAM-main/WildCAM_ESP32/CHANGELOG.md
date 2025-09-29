# WildCAM_ESP32 Changelog

## v2.5.0-Essential (2025-09-03)

### Migration from ESP32WildlifeCAM
- **CREATED**: New streamlined repository containing only essential files
- **REDUCED**: Source files reduced from 396 to 39 (90% reduction)
- **SIMPLIFIED**: Removed experimental features, extensive documentation, and non-essential components

### Core Features Retained
- ✅ **Motion Detection** - PIR sensors with intelligent algorithms
- ✅ **AI Wildlife Classification** - On-device species identification using TensorFlow Lite
- ✅ **Power Management** - Solar charging with optimized battery life
- ✅ **Image Capture** - High-quality wildlife photography
- ✅ **Data Storage** - Local SD card storage and wireless transmission
- ✅ **Multi-Board Support** - ESP32-CAM, ESP32-S3-CAM, TTGO T-Camera

### Essential Components Included
- Core source code (src/)
- Header files (include/)
- Board configurations (hardware/)
- PlatformIO configuration (platformio.ini)
- Main application (main.cpp)
- System configuration (config.h, wifi_config.h)
- Essential documentation (README.md, LICENSE)

### Removed Components
- Extensive documentation files (guides, roadmaps, status reports)
- 3D models and STL files
- Mobile app components
- Examples directory
- Test/validation scripts
- Assembly guides and cost analysis
- Experimental features (blockchain, federated learning, etc.)
- Multiple board variants (keeping only essential ones)

### Directory Structure
```
WildCAM_ESP32/
├── src/                    # Core source code (380K)
│   ├── ai_detection/      # Wildlife AI classification
│   ├── camera/            # Camera management
│   ├── core/              # System management
│   ├── hardware/          # Board detection
│   ├── power/             # Power management
│   ├── sensors/           # Environmental sensors
│   └── utils/             # Utility functions
├── include/               # Header files (120K)
├── hardware/              # Board definitions (4K)
├── main.cpp              # Main application entry (8K)
├── config.h              # System configuration (4K)
├── wifi_config.h         # WiFi configuration (8K)
├── platformio.ini        # Build configuration (4K)
├── partitions.csv        # Memory partitions (4K)
├── README.md             # Project documentation (4K)
└── LICENSE               # License file (36K)
```

### Total Repository Size
- **Before**: ~6MB+ with 396 source files
- **After**: ~580KB with 39 source files (90%+ reduction)

### Build Configuration
- Simplified to 3 essential board configurations
- Removed experimental and unused dependencies
- Streamlined for production deployment

### Next Steps
This streamlined version focuses on stable, production-ready functionality. For experimental features and extended development, refer to the original ESP32WildlifeCAM repository.