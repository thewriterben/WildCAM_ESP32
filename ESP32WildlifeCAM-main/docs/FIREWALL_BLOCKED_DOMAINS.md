# 🔒 ESP32 Wildlife CAM - Firewall Blocked Domains

## Overview
Documentation of domains blocked by firewall that may affect ESP32 Wildlife CAM development, with workarounds and offline alternatives.

## 🚫 Known Blocked Domains

### Development Resources
| Domain | Purpose | Impact | Workaround |
|--------|---------|--------|------------|
| `github.com` | Source code repositories | High | Git over SSH, offline clones |
| `arduino.cc` | Arduino libraries | Medium | Local library cache |
| `platformio.org` | PlatformIO packages | High | Offline package mirror |
| `espressif.com` | ESP32 documentation | Medium | Cached documentation |

### Package Repositories
- `pypi.org` - Python packages
- `npmjs.com` - JavaScript packages
- `registry.platformio.org` - Platform packages
- `downloads.arduino.cc` - Arduino downloads

### Documentation Sites
- `docs.espressif.com` - ESP32 official docs
- `randomnerdtutorials.com` - Tutorial resources
- `stackoverflow.com` - Development help

## 🛠️ Offline Workarounds

### Complete Offline Setup
```bash
# Setup offline development environment
./scripts/setup_offline_environment.sh --complete --cache_all
```

### Manual Package Caching
```bash
# Cache essential packages
./scripts/cache_dependencies.sh --essential_only
```

## 📞 Emergency Contacts
- **IT Support**: Contact system administrator
- **Network Team**: Request domain whitelist
- **Alternative Access**: VPN or mobile hotspot

---

**Document Version**: 1.0  
**Last Updated**: 2025-09-01