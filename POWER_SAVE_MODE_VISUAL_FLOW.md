# Power Save Mode - Visual Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    WildCAM ESP32 Power Management                │
└─────────────────────────────────────────────────────────────────┘

                          ┌──────────────┐
                          │   STARTUP    │
                          │  Battery OK  │
                          └──────┬───────┘
                                 │
                                 v
                    ┌────────────────────────┐
                    │    NORMAL MODE         │
                    │  ────────────────────  │
                    │  CPU: 240 MHz          │
                    │  Sleep: 300s (5 min)   │
                    │  WiFi: Enabled         │
                    │  Camera: Normal        │
                    │  Power: 100%           │
                    └────────┬───────────────┘
                             │
                             │ Battery monitoring
                             │ (every 5 seconds)
                             │
              ┌──────────────┴──────────────┐
              │                             │
              v                             v
    Battery < 3.0V                  Battery > 3.0V
              │                             │
              v                             │
    ┌─────────────────────┐                │
    │  POWER SAVE MODE    │                │
    │  ─────────────────  │                │
    │  CPU: 80 MHz        │                │
    │  Sleep: 600s (10min)│                │
    │  WiFi: DISABLED     │                │
    │  Camera: Reduced*   │                │
    │  Power: <60%        │                │
    └─────────┬───────────┘                │
              │                             │
              │ Battery monitoring          │
              │ (every 5 seconds)           │
              │                             │
              ├─────────────────────────────┘
              │                             
              │                             
    ┌─────────┴────────────┐               
    │                      │               
    v                      v               
Battery > 3.4V     Battery 3.0V-3.4V      
    │                      │               
    │                      │               
    │              Stay in current mode    
    │              (Hysteresis Zone)       
    │                      │               
    │                      └──────────┐    
    │                                 │    
    v                                 │    
Back to NORMAL MODE ◄─────────────────┘    
```

## State Machine Diagram

```
          Enter                    Exit
          < 3.0V                  > 3.4V
            ↓                       ↑
    ┌───────────────────────────────────────┐
    │                                       │
    │   ┌─────────┐           ┌─────────┐  │
    │   │ NORMAL  │           │  POWER  │  │
    └──→│  MODE   │──────────→│  SAVE   │──┘
        └─────────┘           └─────────┘
            ↑                       │
            │    Hysteresis Zone    │
            │    (3.0V - 3.4V)      │
            └───────────────────────┘
```

## Battery Voltage Timeline

```
Voltage
(V)     
  4.2   ──────────────────────────────────────────
  4.0   
  3.8   ────┐
  3.6       │
  3.4   ────┼──────────────────EXIT POWER SAVE──→
  3.2       │                        ↑
  3.0   ────┼────ENTER POWER SAVE────┤
  2.8       │         ↓               │
  2.6   ────┘                         │
  2.4                                 │
        ──────Time──────────────────────→

        Normal    Power Save    Hysteresis    Normal
         Mode       Mode          Zone         Mode
```

## Decision Flow

```
┌─────────────────────────┐
│  Battery Check          │
│  (every 5 seconds)      │
└────────┬────────────────┘
         │
         v
┌─────────────────────────┐
│  Read Battery Voltage   │
│  from MPPT Controller   │
└────────┬────────────────┘
         │
         v
    ┌────────────┐
    │ Voltage?   │
    └────┬───────┘
         │
    ┌────┴────┬────────┬────────┐
    │         │        │        │
    v         v        v        v
  < 3.0V   3.0-3.4V  > 3.4V   Other
    │         │        │        │
    v         v        v        v
┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
│Enter │  │Keep  │  │Exit  │  │Keep  │
│Power │  │Current│ │Power │  │Normal│
│Save  │  │Mode  │  │Save  │  │Mode  │
└──────┘  └──────┘  └──────┘  └──────┘
```

## Power Consumption Over Time

```
Power
(%)
100  ████████╗               ╔════════
     █       ║               ║
 80  █       ║               ║
     █       ║               ║
 60  █       ╚═══════════════╝
     █            Power Save
 40  █              Mode
     █
 20  █
     █
  0  ████████████████████████████████
     0     5    10   15   20   25   30  Time (min)
           ↑                   ↑
           Enter               Exit
         (< 3.0V)           (> 3.4V)

     Normal → Power Save → Normal
     100%      <60%         100%
```

## Component States

```
┌──────────────┬──────────────┬──────────────┐
│              │ NORMAL MODE  │ POWER SAVE   │
├──────────────┼──────────────┼──────────────┤
│ CPU Freq     │   240 MHz    │    80 MHz    │
│ Reduction    │      -       │     67%      │
├──────────────┼──────────────┼──────────────┤
│ Sleep Time   │   300 sec    │   600 sec    │
│ Active %     │    100%      │     50%      │
├──────────────┼──────────────┼──────────────┤
│ WiFi         │   Enabled    │  Disabled    │
│ Power Save   │      -       │    100%      │
├──────────────┼──────────────┼──────────────┤
│ Camera       │   Normal     │  Reduced*    │
│ Quality      │    100%      │     TBD      │
├──────────────┼──────────────┼──────────────┤
│ Total Power  │    100%      │    <60%      │
│ Reduction    │      -       │    >40%      │
├──────────────┼──────────────┼──────────────┤
│ Battery Life │  Baseline    │   >50%       │
│ Extension    │      -       │   longer     │
└──────────────┴──────────────┴──────────────┘

* Logged for future implementation
```

## Logging Timeline

```
Time    Log Message
─────   ────────────────────────────────────────────────────
00:00   Power Status - Battery: 3.75V, Solar: 2.5W...
00:05   Power Status - Battery: 3.45V, Solar: 2.3W...
00:10   Power Status - Battery: 3.15V, Solar: 1.8W...
00:15   Power Status - Battery: 2.95V, Solar: 1.2W...
        ⚠️  Entering power save mode (battery: 2.95V)
        ✓ CPU frequency reduced to 80MHz
        ✓ Deep sleep duration increased to 600 seconds
        ✓ WiFi disabled for power saving
        ✓ Camera quality will be reduced on next capture
        ✓ Power save mode activated
00:20   Power Status - Battery: 2.88V, Solar: 0.8W...
...
10:00   Power Status - Battery: 3.52V, Solar: 3.1W...
        ✓ Exiting power save mode (battery: 3.52V)
        ✓ CPU frequency restored to 240MHz
        ✓ Deep sleep duration restored to 300 seconds
        ✓ Camera quality will be restored on next capture
        ✓ Power save mode deactivated - normal operation resumed
```

## Implementation Locations

```
firmware/main.cpp
├── Lines 16-23:     Includes (WiFi, HTTPClient, SD, Preferences)
├── Line 86:         Global variable (deep_sleep_duration)
├── Lines 85-119:    SystemState struct (power_save_mode field)
└── Lines 368-413:   Power save logic implementation
```

---

**Legend:**
- ─── : Normal state
- ═══ : Power save state
- → : State transition
- * : Future enhancement
