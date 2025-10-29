# Logging System Architecture

## System Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Code                          │
│  (main.cpp, CameraManager, PowerManager, MotionDetector, etc.)  │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             │ Uses logging macros:
                             │ LOG_DEBUG(), LOG_INFO(),
                             │ LOG_WARN(), LOG_ERROR()
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                         Logger Macros                            │
│         Automatically capture: __FILE__, __func__, __LINE__      │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Logger::log() Method                        │
│                                                                   │
│  1. Check if logging enabled                                     │
│  2. Check if level >= minLogLevel (filter)                       │
│  3. Generate timestamp                                           │
│  4. Format message with vsnprintf                                │
│  5. Route to outputs                                             │
└──────────────────┬─────────────────────────┬────────────────────┘
                   │                         │
                   ▼                         ▼
        ┌──────────────────┐      ┌──────────────────┐
        │  Serial Output   │      │   SD Card File   │
        │  (if enabled)    │      │   (if enabled)   │
        └──────────────────┘      └──────────────────┘
                   │                         │
                   ▼                         ▼
        ┌──────────────────┐      ┌──────────────────┐
        │  Serial Monitor  │      │  /system.log     │
        │   (USB/UART)     │      │   (persistent)   │
        └──────────────────┘      └──────────────────┘
```

## Log Message Format

```
┌──────────────┬───────┬────────────────┬─────────────────────┐
│  Timestamp   │ Level │  Function:Line │      Message        │
└──────────────┴───────┴────────────────┴─────────────────────┘
   [0d 00:12:34.567] [INFO] [setup:125] Power Manager initialized
    └─┬──┘ └──┬───┘   └─┬─┘  └────┬───┘ └──────────┬──────────┘
      │       │          │         │                 │
   Days    Time(HH:MM:SS.mmm)   Function          User
  uptime   since boot    Level    name +          Message
                                  line number
```

## Configuration Hierarchy

```
┌─────────────────────────────────────────────────────────────────┐
│                         config.h                                 │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ LOGGING_ENABLED = true/false                             │   │
│  │ DEFAULT_LOG_LEVEL = 0(DEBUG), 1(INFO), 2(WARN), 3(ERROR)│   │
│  │ LOG_TO_SERIAL = true/false                               │   │
│  │ LOG_TO_SD = true/false                                   │   │
│  │ LOG_FILE_PATH = "/system.log"                            │   │
│  └──────────────────────────────────────────────────────────┘   │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    main.cpp setup()                              │
│  Logger::init((LogLevel)DEFAULT_LOG_LEVEL,                      │
│               LOG_TO_SERIAL,                                     │
│               LOG_TO_SD,                                         │
│               LOG_FILE_PATH);                                    │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Runtime Configuration                         │
│  Logger::setLogLevel(LOG_DEBUG);                                │
│  Logger::setSerialOutput(false);                                │
│  Logger::setSDOutput(true);                                     │
│  Logger::setLogFilePath("/custom.log");                         │
└─────────────────────────────────────────────────────────────────┘
```

## Log Level Filtering

```
Application Code              Logger Filter                Output
─────────────────────────────────────────────────────────────────
                          minLogLevel = INFO
                                 │
LOG_DEBUG("...")  ────────►  │ DEBUG < INFO? ────► Filtered (no output)
                             │
LOG_INFO("...")   ────────►  │ INFO >= INFO? ────► Output to Serial/SD
                             │
LOG_WARN("...")   ────────►  │ WARN >= INFO? ────► Output to Serial/SD
                             │
LOG_ERROR("...")  ────────►  │ ERROR >= INFO? ───► Output to Serial/SD


                          minLogLevel = ERROR
                                 │
LOG_DEBUG("...")  ────────►  │ DEBUG < ERROR? ───► Filtered (no output)
                             │
LOG_INFO("...")   ────────►  │ INFO < ERROR? ────► Filtered (no output)
                             │
LOG_WARN("...")   ────────►  │ WARN < ERROR? ────► Filtered (no output)
                             │
LOG_ERROR("...")  ────────►  │ ERROR >= ERROR? ──► Output to Serial/SD
```

## Integration Points in main.cpp

```
setup()
│
├─► Initialize Logger ──────────► LOG_INFO("Starting up...")
│
├─► Initialize Watchdog ────────► LOG_INFO("Watchdog initialized")
│
├─► Initialize Power Manager ───► LOG_INFO("Power Manager initialized")
│                                  LOG_INFO("Battery: %.2fV (%d%%)")
│                                  LOG_WARN("Battery low!")
│
├─► Initialize Camera ──────────► LOG_INFO("Camera initialized")
│                                  LOG_ERROR("Camera init failed!")
│
├─► Initialize Storage ─────────► LOG_INFO("Storage initialized")
│                                  LOG_ERROR("Storage init failed!")
│
├─► Initialize Motion Detector ─► LOG_INFO("Motion detector ready")
│
└─► Initialize Web Server ──────► LOG_INFO("WiFi connected")
                                   LOG_WARN("WiFi connection failed")

loop()
│
├─► Motion Detection ───────────► LOG_INFO("Motion detected")
│                                  LOG_DEBUG("Stabilizing camera...")
│
├─► Image Capture ──────────────► LOG_INFO("Capturing image...")
│                                  LOG_INFO("Image saved: /IMG_0001.jpg")
│                                  LOG_ERROR("Failed to capture image")
│
├─► Battery Check ──────────────► LOG_INFO("Battery check: %.2fV")
│                                  LOG_WARN("Battery critically low")
│
└─► Deep Sleep ─────────────────► LOG_INFO("Entering deep sleep")
```

## File Structure

```
WildCAM_ESP32/
├── include/
│   ├── config.h ─────────────── Configuration (LOGGING_ENABLED, etc.)
│   └── Logger.h ─────────────── Logger class definition & macros
│
├── src/
│   ├── Logger.cpp ───────────── Logger implementation
│   └── main.cpp ────────────────  Logging integration
│
├── test/
│   └── test_logger.cpp ──────── Unit tests for logging system
│
├── examples/
│   └── logging_example.cpp ──── Example usage demonstration
│
└── docs/
    ├── LOGGING_SYSTEM_README.md ─ Comprehensive documentation
    ├── LOGGING_INTEGRATION.md ─── Integration guide
    └── LOGGING_ARCHITECTURE.md ── This file
```
