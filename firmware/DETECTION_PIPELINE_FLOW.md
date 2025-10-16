# Detection Pipeline Flow Diagram

## Overview
This document provides a visual representation of the wildlife detection processing pipeline flow.

```
┌─────────────────────────────────────────────────────────────────────┐
│                     Wildlife Detection Pipeline                      │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────┐
│  Camera Capture │
│   (320x240)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  YOLO Detection │
│   (AI Inference)│
└────────┬────────┘
         │
         │ Wildlife Detected?
         ├─── No ──────────────────────────┐
         │                                  │
         ▼ Yes                              ▼
┌─────────────────────────────┐    ┌──────────────┐
│ processWildlifeDetection()  │    │ Continue Loop│
└────────┬────────────────────┘    └──────────────┘
         │
         ▼
┌─────────────────────────────┐
│ Step 1: Generate Filename   │
│ Format: YYYYMMDD_HHMMSS.jpg │
│ Example: 20251016_143052    │
│         _deer.jpg            │
└────────┬────────────────────┘
         │
         ▼
┌─────────────────────────────┐
│ Step 2: Check Storage Ready │
└────────┬────────────────────┘
         │
         ├─── Not Ready ───┐
         │                 │
         ▼ Ready           ▼
┌─────────────────┐  ┌────────────────┐
│  Save Image to  │  │  Log Error &   │
│  /images/ dir   │  │  Continue      │
└────────┬────────┘  └────────────────┘
         │
         ├─── Failed ──────┐
         │                 │
         ▼ Success         ▼
┌─────────────────┐  ┌────────────────┐
│ Step 3: Log     │  │  Log Error &   │
│ Save Operation  │  │  Continue      │
│ (filename+size) │  └────────────────┘
└────────┬────────┘
         │
         ▼
┌─────────────────────────────┐
│ Step 4: Save Metadata JSON  │
│ (filename, species,         │
│  confidence, bbox, etc.)    │
└────────┬────────────────────┘
         │
         ├─── Failed ──────┐
         │                 │
         ▼ Success         ▼
┌─────────────────┐  ┌────────────────┐
│  Log Metadata   │  │  Log Warning & │
│  Save Success   │  │  Continue      │
└─────────────────┘  └────────────────┘
         │                 │
         └────────┬────────┘
                  │
                  ▼
         ┌────────────────┐
         │ Continue to    │
         │ Next Detection │
         └────────────────┘
```

## Component Interaction Diagram

```
┌────────────────────────────────────────────────────────────────┐
│                        Main Components                          │
└────────────────────────────────────────────────────────────────┘

┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   Camera     │────▶│    YOLO      │────▶│   Detection  │
│   Manager    │     │   Detector   │     │   Pipeline   │
└──────────────┘     └──────────────┘     └──────┬───────┘
                                                  │
                     ┌────────────────────────────┼────────────────┐
                     │                            │                │
                     ▼                            ▼                ▼
            ┌──────────────┐          ┌──────────────┐  ┌──────────────┐
            │   Filename   │          │   Storage    │  │   Metadata   │
            │   Generator  │          │   Manager    │  │   Generator  │
            └──────────────┘          └──────┬───────┘  └──────────────┘
                                              │
                          ┌───────────────────┼───────────────────┐
                          │                   │                   │
                          ▼                   ▼                   ▼
                  ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
                  │   SD Card    │   │   LittleFS   │   │    Logger    │
                  │  (Primary)   │   │  (Fallback)  │   │              │
                  └──────────────┘   └──────────────┘   └──────────────┘
```

## Data Flow

```
Camera Frame (uint8_t*)
         │
         ▼
YOLO Detector
         │
         ▼
BoundingBox[] (detections)
         │
         ├─────────────────┬──────────────────────┐
         │                 │                      │
         ▼                 ▼                      ▼
    class_name        confidence            x,y,w,h
         │                 │                      │
         └─────────────────┴──────────────────────┘
                           │
                           ▼
              processWildlifeDetection()
                           │
         ┌─────────────────┴─────────────────┐
         │                                    │
         ▼                                    ▼
   Image File                          Metadata File
   YYYYMMDD_HHMMSS_species.jpg        YYYYMMDD_HHMMSS_species.json
   Size: ~15-30KB                      Size: ~200 bytes
```

## Error Handling Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                      Error Scenarios                             │
└─────────────────────────────────────────────────────────────────┘

    Detection Event
         │
         ├─── Invalid Data ───┐
         │                    │
         │                    ▼
         │           ┌────────────────┐
         │           │  Return false  │
         │           │  (Stop process)│
         │           └────────────────┘
         │
         ├─── Filename Gen Fails ──┐
         │                          │
         │                          ▼
         │                 ┌────────────────┐
         │                 │  Return false  │
         │                 │  (Stop process)│
         │                 └────────────────┘
         │
         ├─── Storage Not Ready ──┐
         │                        │
         │                        ▼
         │               ┌────────────────┐
         │               │  Log Error &   │
         │               │  Return true   │
         │               │  (Continue)    │
         │               └────────────────┘
         │
         ├─── Image Save Fails ──┐
         │                       │
         │                       ▼
         │              ┌────────────────┐
         │              │  Log Error &   │
         │              │  Return true   │
         │              │  (Continue)    │
         │              └────────────────┘
         │
         └─── Metadata Fails ───┐
                                │
                                ▼
                       ┌────────────────┐
                       │  Log Warning & │
                       │  Return true   │
                       │  (Continue)    │
                       └────────────────┘
```

## Storage Decision Tree

```
┌─────────────────────────────────────────────────────────────────┐
│                    Storage Selection                             │
└─────────────────────────────────────────────────────────────────┘

        System Init
             │
             ▼
        Try SD Card
             │
             ├─── Success ────┐
             │                │
             ▼ Failed         ▼
        Try LittleFS    Use SD Card
             │          (PRIMARY)
             │
             ├─── Success ────┐
             │                │
             ▼ Failed         ▼
        No Storage      Use LittleFS
        Available       (FALLBACK)
             │
             ▼
        Log Error
        Continue
        (Detection works
         but no saves)
```

## File Structure

```
/wildlife/                          <- Root directory
    ├── images/                     <- Image storage
    │   ├── 20251016_143052_deer.jpg
    │   ├── 20251016_143052_deer.json
    │   ├── 20251016_143215_fox.jpg
    │   ├── 20251016_143215_fox.json
    │   └── ...
    ├── logs/                       <- System logs
    │   └── ...
    ├── config/                     <- Configuration
    │   └── ...
    └── data/                       <- Other data
        └── ...
```

## Timing Diagram

```
Time (ms)  Event
──────────────────────────────────────────────────────────────────
    0      │ Camera Capture Start
           │
   50      │ Camera Frame Ready
           │
   51      │ YOLO Detection Start
           │
  200      │ YOLO Detection Complete (Wildlife Found!)
           │
  201      │ processWildlifeDetection() Start
           │
  202      │   └─ generateDetectionFilename() [~1ms]
           │
  203      │   └─ Storage Check [~1ms]
           │
  204      │   └─ Image Save Start
           │
  284      │   └─ Image Save Complete [~80ms]
           │
  285      │   └─ Log Save Operation [~1ms]
           │
  286      │   └─ saveDetectionMetadata() Start
           │
  295      │   └─ Metadata Save Complete [~9ms]
           │
  296      │ processWildlifeDetection() Complete
           │
  297      │ Continue Detection Loop
──────────────────────────────────────────────────────────────────
Total Processing Time: ~97ms per detection
```

## Memory Usage

```
┌─────────────────────────────────────────────────────────────────┐
│                      Memory Footprint                            │
└─────────────────────────────────────────────────────────────────┘

Stack Usage (during detection):
├── Filename buffer:           64 bytes
├── Metadata filename buffer:  64 bytes
├── JSON buffer:              512 bytes
├── Function call stack:      ~100 bytes
└── Total:                    ~740 bytes

Heap Usage:
├── StorageManager instance:   ~150 bytes
├── BoundingBox array:         ~280 bytes (10 detections)
└── Camera frame:             76,800 bytes (QVGA, temp)

Flash Usage:
├── Detection functions:      ~2 KB code
├── Storage manager:          ~4 KB code
└── Total:                    ~6 KB
```

## Key Performance Metrics

| Metric | Value | Notes |
|--------|-------|-------|
| Detection Rate | 10 FPS | When AI enabled |
| Processing Time | 50-100ms | Per detection |
| Image Size | 15-30KB | JPEG 320x240 |
| Metadata Size | ~200 bytes | JSON format |
| Storage/Hour | ~100MB | @ 10 detections/hour |
| Memory Impact | <1KB | Stack usage |

## Integration Points

```
Main Firmware (main.cpp)
    │
    ├─ setup()
    │   └─ initializeSystem()
    │       └─ g_storage.initialize()
    │
    └─ aiProcessingTask()
        ├─ captureFrame()
        ├─ detect()
        └─ FOR EACH detection:
            └─ processWildlifeDetection()  ← PIPELINE ENTRY
                ├─ generateDetectionFilename()
                ├─ g_storage.saveImage()
                └─ saveDetectionMetadata()
```

## Success Criteria Checklist

- ✅ Unique filename with timestamp
- ✅ Images saved to /images/ directory  
- ✅ Comprehensive logging at each step
- ✅ Graceful error handling
- ✅ Optional metadata generation
- ✅ No interruption on storage failures
- ✅ Unit test coverage
- ✅ Documentation complete
