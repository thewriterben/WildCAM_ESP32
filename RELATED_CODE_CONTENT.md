# 🔗 WildCAM ESP32 - Related Code Content Guide

**A comprehensive cross-reference guide for navigating the WildCAM ESP32 codebase**

This document helps developers understand how different components across the firmware, backend, frontend, and documentation relate to each other. Use this as a navigation guide to find related code when working on specific features.

---

## 📋 Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Component Relationships](#component-relationships)
3. [Feature-Based Code Mapping](#feature-based-code-mapping)
4. [API Integration Points](#api-integration-points)
5. [Documentation Cross-References](#documentation-cross-references)
6. [Testing & Validation](#testing--validation)
7. [Quick Reference Tables](#quick-reference-tables)

---

## 🏗️ Architecture Overview

### Multi-Tier System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Frontend Layer                           │
│  React Dashboard + Mobile App + Collaboration UI             │
└──────────────┬──────────────────────────────────────────────┘
               │ REST API / WebSocket / GraphQL
┌──────────────▼──────────────────────────────────────────────┐
│                     Backend Layer                            │
│  Flask API + JWT Auth + RBAC + Collaboration Services        │
└──────────────┬──────────────────────────────────────────────┘
               │ MQTT / WebSocket / HTTP
┌──────────────▼──────────────────────────────────────────────┐
│                     Firmware Layer                           │
│  ESP32 + AI Detection + Camera + Security + Networking       │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔗 Component Relationships

### 1. Authentication & Authorization

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Backend Auth** | `backend/auth.py` | JWT token management, user authentication |
| **RBAC System** | `backend/rbac.py` | Role-based access control, permissions |
| **Frontend Auth** | `frontend/dashboard/src/hooks/useAuth.js` | Authentication hooks and context |
| **RBAC Hook** | `frontend/dashboard/src/hooks/useRBAC.js` | Permission checking in React components |
| **API Integration** | `backend/app.py` (lines 83-86, 593-635) | RBAC service initialization and endpoints |

#### Flow:
```
User Login → backend/auth.py → JWT Token → backend/rbac.py (check permissions) 
           → frontend/hooks/useRBAC.js → UI Components (conditional rendering)
```

#### Documentation:
- [COLLABORATION_IMPLEMENTATION_COMPLETE.md](COLLABORATION_IMPLEMENTATION_COMPLETE.md) - Full RBAC documentation
- [backend/rbac.py](backend/rbac.py) - 20 permissions across 6 roles
- [examples/test_rbac.py](examples/test_rbac.py) - Comprehensive tests

---

### 2. Real-Time Collaboration Features

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Collaboration Service** | `backend/collaboration.py` | WebSocket event handling, session management |
| **WebRTC Signaling** | `backend/webrtc_signaling.py` | Video/audio call signaling |
| **User Presence** | `frontend/dashboard/src/components/collaboration/UserPresence.js` | Active user display |
| **Team Chat** | `frontend/dashboard/src/components/collaboration/TeamChat.js` | Real-time messaging |
| **Task Manager** | `frontend/dashboard/src/components/collaboration/TaskManager.js` | Team task coordination |
| **Collaboration Panel** | `frontend/dashboard/src/components/CollaborationPanel.js` | Main integration component |
| **WebSocket Service** | `frontend/dashboard/src/services/WebSocketService.js` | Client-side WebSocket handling |
| **Database Models** | `backend/models.py` | UserSession, ChatMessage, Task, Annotation |

#### Flow:
```
Frontend WebSocket → backend/collaboration.py → Database (models.py) 
                  → Emit to all clients → Update UI Components
```

#### Documentation:
- [COLLABORATION_IMPLEMENTATION_SUMMARY.md](COLLABORATION_IMPLEMENTATION_SUMMARY.md) - Features overview
- [COLLABORATION_README.md](COLLABORATION_README.md) - Setup guide
- [PHASE_3_IMPLEMENTATION_COMPLETE.md](PHASE_3_IMPLEMENTATION_COMPLETE.md) - WebRTC details

---

### 3. AI Detection System

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Firmware AI Core** | `firmware/ml_models/yolo_tiny/yolo_tiny_detector.h/cpp` | Edge AI detection engine |
| **Wildlife Detector** | `firmware/ml_models/wildlife_detector.h/cpp` | Species-specific detection |
| **AI Processing Task** | `firmware/main.cpp` (lines 82-144) | Real-time detection loop |
| **Camera Manager** | `firmware/src/camera/camera_manager.h/cpp` | Image capture and buffer management |
| **Backend ML** | `backend/ml/species_classifier.py` | Server-side ML processing |
| **API Endpoint** | `backend/app.py` (detection endpoints) | Detection data ingestion |
| **Frontend Display** | `frontend/dashboard/src/components/LiveDetections.js` | Real-time detection UI |
| **Mobile App** | `frontend/mobile/src/screens/DetectionsScreen.js` | Mobile detection view |

#### Flow:
```
PIR Trigger → Camera Capture (camera_manager.cpp) → YOLO Detection (yolo_tiny_detector.cpp)
           → MQTT/HTTP → Backend API → Database → WebSocket → Frontend Display
```

#### Documentation:
- [firmware/CRITICAL_FIXES_README.md](firmware/CRITICAL_FIXES_README.md) - Camera implementation details
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md) - Complete fix documentation
- [ML_INTEGRATION_README.md](ML_INTEGRATION_README.md) - ML pipeline overview

---

### 4. Security & Encryption

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Security Manager** | `firmware/security/security_manager.h/cpp` | Hardware security orchestration |
| **LoRa Encryption** | `firmware/networking/lora_encryption.h/cpp` | AES-256-CBC for LoRa messages |
| **Quantum-Safe Crypto** | `firmware/security/quantum_safe_crypto.h/cpp` | Post-quantum encryption |
| **Hash-Based Signatures** | `firmware/security/hash_based_signatures.h/cpp` | SPHINCS+ signatures |
| **Backend Security** | `backend/auth.py` | JWT with secure token blacklist |
| **API Security** | `backend/app.py` (JWT decorators) | Endpoint protection |
| **Encryption Tests** | `firmware/test/test_encryption.cpp` | Security validation tests |

#### Flow:
```
Data Generation → AES Encryption (security_manager.cpp) → LoRa Transmission
               → Backend Reception → JWT Verification → Database Storage
```

#### Documentation:
- [QUANTUM_SAFE_SECURITY.md](QUANTUM_SAFE_SECURITY.md) - Quantum-safe cryptography
- [QUANTUM_SAFE_IMPLEMENTATION_SUMMARY.md](QUANTUM_SAFE_IMPLEMENTATION_SUMMARY.md) - Implementation details
- [ENTERPRISE_SECURITY.md](ENTERPRISE_SECURITY.md) - Enterprise security features
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md#priority-4) - Encryption implementation

---

### 5. Power Management System

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **MPPT Controller** | `firmware/power/mppt_controller.h/cpp` | Solar charging optimization |
| **Power Manager** | `firmware/power/power_manager.h/cpp` | Battery and power orchestration |
| **Power Task** | `firmware/main.cpp` (lines 166-192) | Power monitoring loop |
| **Environmental Sensors** | `firmware/drivers/environmental_suite.h/cpp` | Weather-aware power management |
| **Battery Monitoring** | `firmware/hal/battery_monitor.h/cpp` | Battery health tracking |
| **Backend Analytics** | `backend/api/analytics.py` | Power consumption analytics |
| **Dashboard Display** | `frontend/dashboard/src/components/PowerMonitoring.js` | Power metrics UI |

#### Flow:
```
Solar Panel → MPPT Controller → Battery Manager → Power Budget Calculation
           → Deep Sleep Decisions → Backend Logging → Analytics Dashboard
```

#### Documentation:
- [HARDWARE_REQUIREMENTS.md](HARDWARE_REQUIREMENTS.md#power-system) - Power system specs
- [firmware/power/README.md](firmware/power/README.md) - Power management guide

---

### 6. Networking & Communication

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Mesh Config** | `firmware/networking/mesh_config.h` | ESP-MESH network configuration |
| **LoRa Communication** | `firmware/networking/lora_manager.h/cpp` | Long-range wireless |
| **Satellite Integration** | `firmware/networking/satellite_integration.h/cpp` | Iridium/Swarm/RockBLOCK support |
| **WiFi Manager** | `firmware/networking/wifi_manager.h/cpp` | WiFi connectivity |
| **Global Network** | `firmware/src/production/enterprise/cloud/global_conservation_network.h/cpp` | International collaboration |
| **Backend API Client** | `firmware/networking/api_client.h/cpp` | HTTP/MQTT client |
| **WebSocket Handler** | `backend/app.py` (WebSocket events) | Real-time server communication |
| **Frontend WebSocket** | `frontend/dashboard/src/services/WebSocketService.js` | Client WebSocket manager |

#### Flow:
```
ESP32 Firmware → WiFi/LoRa/Satellite → Backend API/WebSocket 
              → Redis/Database → Frontend via WebSocket → UI Update
```

#### Documentation:
- [ESP32WildlifeCAM-main/docs/SATELLITE_COMMUNICATION.md](ESP32WildlifeCAM-main/docs/SATELLITE_COMMUNICATION.md) - Satellite setup
- [ESP32WildlifeCAM-main/docs/GLOBAL_CONSERVATION_NETWORK.md](ESP32WildlifeCAM-main/docs/GLOBAL_CONSERVATION_NETWORK.md) - Global network
- [GLOBAL_NETWORK_IMPLEMENTATION_SUMMARY.md](GLOBAL_NETWORK_IMPLEMENTATION_SUMMARY.md) - Network implementation

---

### 7. Data Storage & Analytics

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Database Models** | `backend/models.py` | SQLAlchemy ORM models |
| **Migrations** | `backend/migrations/` | Database schema versions |
| **InfluxDB Integration** | `backend/services/influxdb_service.py` | Time-series data storage |
| **Analytics API** | `backend/api/analytics.py` | Analytics endpoints |
| **Research Platform** | `backend/research_platform.py` | Research data management |
| **Frontend Analytics** | `frontend/dashboard/src/components/SpeciesAnalytics.js` | Data visualization |
| **Field Notes** | `backend/models.py` (FieldNote model) | Field observation storage |

#### Flow:
```
Firmware Detection → MQTT/HTTP → Backend API → PostgreSQL/InfluxDB 
                  → Analytics Processing → REST API → Frontend Charts
```

#### Documentation:
- [SCIENTIFIC_RESEARCH_PLATFORM_COMPLETE.md](SCIENTIFIC_RESEARCH_PLATFORM_COMPLETE.md) - Research features
- [COMPREHENSIVE_FEATURES_SUMMARY.md](COMPREHENSIVE_FEATURES_SUMMARY.md) - All features overview

---

### 8. Testing & Quality Assurance

#### Related Files:
| Component | Location | Purpose |
|-----------|----------|---------|
| **Firmware Tests** | `firmware/test/test_*.cpp` | Unity-based unit tests |
| **Camera Tests** | `firmware/test/test_camera_manager.cpp` | Camera functionality tests |
| **Encryption Tests** | `firmware/test/test_encryption.cpp` | Security validation |
| **Memory Tests** | `firmware/test/test_memory_management.cpp` | Memory leak detection |
| **Backend Tests** | `backend/examples/test_rbac.py` | RBAC system tests |
| **Integration Tests** | `examples/test_integration.py` | End-to-end tests |
| **Field Test Scripts** | `ESP32WildlifeCAM-main/scripts/field_test_automation.py` | Automated field testing |

#### Documentation:
- [firmware/CRITICAL_FIXES_README.md](firmware/CRITICAL_FIXES_README.md#priority-5) - Testing infrastructure
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md#priority-5) - Test coverage details
- [ESP32WildlifeCAM-main/docs/software/README.md](ESP32WildlifeCAM-main/docs/software/README.md#testing) - Testing guide

---

## 🎯 Feature-Based Code Mapping

### Feature: Wildlife Detection Pipeline

**End-to-End Code Flow:**

1. **Trigger** → `firmware/drivers/pir_sensor.cpp` - Motion detection
2. **Capture** → `firmware/src/camera/camera_manager.cpp` - Image acquisition
3. **Detection** → `firmware/ml_models/yolo_tiny/yolo_tiny_detector.cpp` - AI inference
4. **Encryption** → `firmware/security/security_manager.cpp` - Data protection
5. **Transmission** → `firmware/networking/lora_manager.cpp` - Send to backend
6. **Reception** → `backend/app.py` (detection endpoint) - Receive data
7. **Storage** → `backend/models.py` (WildlifeDetection) - Database save
8. **Broadcasting** → `backend/app.py` (WebSocket emit) - Real-time updates
9. **Display** → `frontend/dashboard/src/components/LiveDetections.js` - User view

**Related Documentation:**
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md#priority-1) - Camera capture implementation
- [ML_INTEGRATION_README.md](ML_INTEGRATION_README.md) - AI detection guide

---

### Feature: User Collaboration

**End-to-End Code Flow:**

1. **Authentication** → `backend/auth.py` - User login
2. **RBAC Check** → `backend/rbac.py` - Permission validation
3. **Session Creation** → `backend/models.py` (UserSession) - Track active session
4. **WebSocket Connect** → `backend/collaboration.py` - Establish real-time connection
5. **Presence Update** → `frontend/dashboard/src/components/collaboration/UserPresence.js` - Show online users
6. **Message Send** → `frontend/dashboard/src/components/collaboration/TeamChat.js` - User types message
7. **Message Broadcast** → `backend/collaboration.py` (handle_chat_message) - Server relay
8. **Message Store** → `backend/models.py` (ChatMessage) - Database save
9. **Message Display** → All connected clients receive and display

**Related Documentation:**
- [COLLABORATION_IMPLEMENTATION_COMPLETE.md](COLLABORATION_IMPLEMENTATION_COMPLETE.md) - Full collaboration guide
- [backend/rbac.py](backend/rbac.py) - RBAC implementation with inline docs

---

### Feature: Satellite Communication

**End-to-End Code Flow:**

1. **Detection** → `firmware/ml_models/wildlife_detector.cpp` - Wildlife identified
2. **Message Prep** → `firmware/networking/satellite_integration.cpp` - Format alert
3. **Transmission** → `firmware/networking/satellite_integration.cpp` (sendMessage) - Via Iridium/Swarm
4. **Reception** → Cloud endpoint (configured in satellite provider)
5. **Webhook** → `backend/routes/satellite_webhook.py` - Receive satellite message
6. **Processing** → `backend/app.py` - Parse and store
7. **Alert** → `backend/routes/alerts.py` - Generate alert
8. **Notification** → WebSocket broadcast to all users
9. **Display** → `frontend/dashboard/src/components/AlertPanel.js` - Show alert

**Related Documentation:**
- [ESP32WildlifeCAM-main/docs/SATELLITE_QUICK_START.md](ESP32WildlifeCAM-main/docs/SATELLITE_QUICK_START.md) - Setup guide
- [ESP32WildlifeCAM-main/docs/SATELLITE_COMMUNICATION.md](ESP32WildlifeCAM-main/docs/SATELLITE_COMMUNICATION.md) - Full documentation
- [README.md](README.md#satellite-communication) - Overview

---

### Feature: Quantum-Safe Security

**End-to-End Code Flow:**

1. **Initialization** → `firmware/security/security_manager.cpp` - Security setup
2. **Key Generation** → `firmware/security/quantum_safe_crypto.cpp` - Hybrid keys
3. **Encryption** → `firmware/security/quantum_safe_crypto.cpp` (encrypt) - Protect data
4. **Signature** → `firmware/security/hash_based_signatures.cpp` - Sign message
5. **Transmission** → `firmware/networking/lora_encryption.cpp` - Encrypted LoRa
6. **Reception** → `backend/services/security_service.py` - Receive encrypted data
7. **Verification** → `backend/services/security_service.py` - Verify signature
8. **Decryption** → `backend/services/security_service.py` - Decrypt payload
9. **Processing** → Standard backend processing

**Related Documentation:**
- [QUANTUM_SAFE_SECURITY.md](QUANTUM_SAFE_SECURITY.md) - Complete quantum-safe guide
- [QUANTUM_SAFE_QUICKSTART.md](QUANTUM_SAFE_QUICKSTART.md) - Quick start
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md#priority-4) - Encryption details

---

## 🔌 API Integration Points

### Firmware → Backend Communication

#### MQTT Topics:
```cpp
// Defined in: firmware/networking/mqtt_client.h
wildlife/detection       → backend/app.py (detection handler)
wildlife/telemetry      → backend/api/telemetry.py
wildlife/alerts         → backend/routes/alerts.py
wildlife/health         → backend/api/health.py
```

#### HTTP Endpoints (Firmware as Client):
```cpp
// Defined in: firmware/networking/api_client.cpp
POST /api/detections    → backend/app.py (upload_detection)
POST /api/images        → backend/app.py (upload_image)
GET  /api/config        → backend/app.py (get_device_config)
POST /api/telemetry     → backend/api/telemetry.py
```

---

### Frontend → Backend Communication

#### REST API Endpoints:
```javascript
// Defined in: frontend/dashboard/src/services/api.js
// Backend handlers in: backend/app.py

// Authentication
POST   /api/auth/login           → auth.py (login)
POST   /api/auth/logout          → auth.py (logout)
POST   /api/auth/refresh         → auth.py (refresh_token)

// RBAC
GET    /api/rbac/permissions     → app.py (get_user_permissions)
POST   /api/rbac/check           → app.py (check_permission)
GET    /api/rbac/roles           → app.py (get_available_roles)

// Detections
GET    /api/detections           → app.py (get_detections)
GET    /api/detections/:id       → app.py (get_detection_detail)
POST   /api/detections/:id/annotate → app.py (add_annotation)

// Collaboration
GET    /api/collaboration/users/active → app.py (get_active_users_api)
POST   /api/collaboration/sessions     → collaboration.py (create_session)
GET    /api/collaboration/chat/:channel → collaboration.py (get_chat_history)

// Analytics
GET    /api/analytics/species    → backend/api/analytics.py
GET    /api/analytics/timeline   → backend/api/analytics.py
GET    /api/analytics/heatmap    → backend/api/analytics.py
```

#### WebSocket Events:
```javascript
// Defined in: frontend/dashboard/src/services/WebSocketService.js
// Backend handlers in: backend/app.py, backend/collaboration.py

// User Presence
emit('user_presence')           → collaboration.py (handle_presence_update)
on('presence_update')           ← collaboration.py (broadcast presence)

// Chat
emit('chat_message')            → collaboration.py (handle_chat_message)
on('new_message')               ← collaboration.py (broadcast message)

// Channels
emit('join_channel')            → app.py (handle_join_channel)
emit('leave_channel')           → app.py (handle_leave_channel)

// Real-time Data
on('new_detection')             ← app.py (broadcast detection)
on('alert_triggered')           ← routes/alerts.py (send alert)
```

---

## 📚 Documentation Cross-References

### By Topic

#### Getting Started:
- [README.md](README.md) - Main project overview
- [HARDWARE_REQUIREMENTS.md](HARDWARE_REQUIREMENTS.md) - Hardware setup
- [QUICK_START_MULTI_CLOUD.md](QUICK_START_MULTI_CLOUD.md) - Cloud deployment

#### Implementation Guides:
- [IMPLEMENTATION_COMPLETE.md](IMPLEMENTATION_COMPLETE.md) - Complete features
- [COLLABORATION_IMPLEMENTATION_COMPLETE.md](COLLABORATION_IMPLEMENTATION_COMPLETE.md) - Collaboration features
- [firmware/IMPLEMENTATION_NOTES.md](firmware/IMPLEMENTATION_NOTES.md) - Firmware fixes and details

#### Feature Documentation:
- [QUANTUM_SAFE_SECURITY.md](QUANTUM_SAFE_SECURITY.md) - Security features
- [GLOBAL_CONSERVATION_NETWORK.md](GLOBAL_CONSERVATION_NETWORK.md) - Global network
- [ALERT_SYSTEM_README.md](ALERT_SYSTEM_README.md) - Alert system
- [ML_INTEGRATION_README.md](ML_INTEGRATION_README.md) - AI/ML integration

#### Development Guides:
- [firmware/CRITICAL_FIXES_README.md](firmware/CRITICAL_FIXES_README.md) - Critical fixes summary
- [PR_REVIEW_GUIDE.md](PR_REVIEW_GUIDE.md) - Code review guidelines
- [WORKFLOW_SUMMARY.md](WORKFLOW_SUMMARY.md) - Development workflow

#### Deployment:
- [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) - Production deployment
- [PRODUCTION_DEPLOYMENT.md](PRODUCTION_DEPLOYMENT.md) - Production setup
- [SCALING_GUIDE.md](SCALING_GUIDE.md) - Scaling strategies

---

## 🧪 Testing & Validation

### Test File Relationships

| Test File | Tests What | Related Production Code |
|-----------|------------|------------------------|
| `firmware/test/test_camera_manager.cpp` | Camera functionality | `firmware/src/camera/camera_manager.cpp` |
| `firmware/test/test_encryption.cpp` | Encryption/decryption | `firmware/networking/lora_encryption.cpp`, `firmware/security/quantum_safe_crypto.cpp` |
| `firmware/test/test_error_handling.cpp` | Error recovery | All firmware components |
| `firmware/test/test_memory_management.cpp` | Memory leaks | `firmware/main.cpp` initialization |
| `backend/examples/test_rbac.py` | RBAC permissions | `backend/rbac.py`, `backend/app.py` |
| `examples/test_integration.py` | End-to-end flows | Entire system |

### Running Tests

**Firmware Tests:**
```bash
cd firmware
pio test -e esp32cam_advanced
```

**Backend Tests:**
```bash
cd backend
python examples/test_rbac.py
```

**Integration Tests:**
```bash
python examples/test_integration.py
```

---

## 📊 Quick Reference Tables

### Component Communication Matrix

| From | To | Protocol | Code Location |
|------|----|-----------|--------------------|
| Firmware | Backend | MQTT | `firmware/networking/mqtt_client.cpp` → `backend/app.py` |
| Firmware | Backend | HTTP | `firmware/networking/api_client.cpp` → `backend/app.py` |
| Backend | Frontend | REST API | `backend/app.py` → `frontend/dashboard/src/services/api.js` |
| Backend | Frontend | WebSocket | `backend/collaboration.py` → `frontend/dashboard/src/services/WebSocketService.js` |
| Frontend | Frontend | Redux | `frontend/dashboard/src/store/` → React components |
| Firmware | Firmware | LoRa | `firmware/networking/lora_manager.cpp` → peer nodes |
| Backend | Database | SQLAlchemy | `backend/models.py` → PostgreSQL |
| Backend | Time-Series DB | InfluxDB Client | `backend/services/influxdb_service.py` → InfluxDB |

---

### Database Models Cross-Reference

| Model | Defined In | Used By Backend | Used By Frontend | Related Feature |
|-------|------------|-----------------|------------------|-----------------|
| `User` | `backend/models.py` | `auth.py`, `rbac.py` | All authenticated pages | Authentication |
| `WildlifeDetection` | `backend/models.py` | `app.py` (detection endpoints) | `LiveDetections.js` | AI Detection |
| `UserSession` | `backend/models.py` | `collaboration.py` | `UserPresence.js` | Collaboration |
| `ChatMessage` | `backend/models.py` | `collaboration.py` | `TeamChat.js` | Team Chat |
| `Task` | `backend/models.py` | `collaboration.py` | `TaskManager.js` | Task Management |
| `Annotation` | `backend/models.py` | `app.py` | `AnnotationEditor.js` | Data Annotation |
| `Alert` | `backend/models.py` | `routes/alerts.py` | `AlertPanel.js` | Alert System |
| `Camera` | `backend/models.py` | `app.py` | `CameraManagement.js` | Camera Management |

---

### Configuration Files

| File | Purpose | Related Components |
|------|---------|-------------------|
| `firmware/platformio.ini` | Firmware build config | All firmware code |
| `backend/config.py` | Backend configuration | `backend/app.py` |
| `.env.example` | Environment variables | Backend, frontend, deployment |
| `frontend/dashboard/package.json` | Frontend dependencies | React app |
| `docker-compose.yml` | Local development | All services |
| `docker-compose.prod.yml` | Production deployment | All services |
| `k8s/*.yaml` | Kubernetes deployment | Cloud infrastructure |

---

## 🚀 Development Workflow

### Adding a New Feature

1. **Firmware Changes**
   - Modify: `firmware/src/` or `firmware/drivers/`
   - Test: Add tests in `firmware/test/`
   - Document: Update `firmware/IMPLEMENTATION_NOTES.md`

2. **Backend API**
   - Add model: `backend/models.py`
   - Add endpoint: `backend/app.py` or `backend/routes/`
   - Add service: `backend/services/`
   - Test: Add to `backend/examples/test_*.py`

3. **Frontend UI**
   - Add component: `frontend/dashboard/src/components/`
   - Add API call: `frontend/dashboard/src/services/api.js`
   - Add state: `frontend/dashboard/src/store/` (if needed)

4. **Documentation**
   - Update main: `README.md`
   - Add feature doc: New `*_IMPLEMENTATION_SUMMARY.md`
   - Update this file: `RELATED_CODE_CONTENT.md`

---

## 🔍 Finding Related Code

### By Feature Name

Use these grep commands to find all references:

```bash
# Find all code related to "RBAC"
grep -r "rbac\|RBAC\|permission\|Permission" --include="*.py" --include="*.js" --include="*.h" --include="*.cpp"

# Find all code related to "collaboration"
grep -r "collab\|Collab\|websocket\|WebSocket" --include="*.py" --include="*.js"

# Find all code related to "encryption"
grep -r "encrypt\|crypt\|AES\|security" --include="*.py" --include="*.cpp" --include="*.h"

# Find all code related to "detection"
grep -r "detect\|yolo\|YOLO\|wildlife" --include="*.py" --include="*.cpp" --include="*.h" --include="*.js"
```

### By File Type

```bash
# All backend Python files
find backend -name "*.py" -type f

# All firmware C++ files
find firmware -name "*.cpp" -o -name "*.h" -type f

# All React components
find frontend/dashboard/src/components -name "*.js" -type f

# All documentation
find . -name "*.md" -type f | grep -v node_modules | grep -v ".git"
```

---

## 📞 Support

For questions about code relationships or architecture:
- **GitHub Issues**: https://github.com/thewriterben/WildCAM_ESP32/issues
- **Documentation**: See [README.md](README.md) for contact information
- **Code Examples**: Check `examples/` directory

---

## 📝 Contributing

When adding new features:

1. **Update This Document**: Add your new component to the relevant sections
2. **Cross-Reference**: Link related files in your PR description
3. **Document Relationships**: Explain how your code integrates with existing components
4. **Update Tests**: Add test cross-references
5. **Follow Patterns**: Match existing architectural patterns

---

**Last Updated**: 2025-10-14  
**Version**: 3.0.0  
**Maintainers**: WildCAM ESP32 Team

---

<div align="center">

**🦌 Navigate the WildCAM codebase with confidence! 🦌**

*This document is automatically maintained as part of the WildCAM development process*

</div>
