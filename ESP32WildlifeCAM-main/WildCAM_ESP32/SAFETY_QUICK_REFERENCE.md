# WildCAM ESP32 Safety Quick Reference

Quick reference for common safety patterns. For detailed information, see [RESOURCE_MANAGEMENT_GUIDE.md](RESOURCE_MANAGEMENT_GUIDE.md).

## Memory Management

### Dynamic Allocation
```cpp
// ✅ DO: Check for nullptr
MyClass* obj = new (std::nothrow) MyClass();
if (!obj) {
    Logger::error("Allocation failed");
    return false;
}

// ❌ DON'T: Assume success
MyClass* obj = new MyClass();
obj->method();  // Crashes if allocation fails
```

### PSRAM
```cpp
// ✅ DO: Check availability
if (psramFound()) {
    void* buffer = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (buffer) {
        // Use buffer
        heap_caps_free(buffer);
    }
}
```

## Camera Frame Buffers

### Safe Capture (RAII)
```cpp
#include "camera/camera_safety_impl.h"

void captureImage() {
    SafeCameraFrameBuffer frame;
    if (frame.capture()) {
        processImage(frame.get());
    }
    // Automatically released
}
```

### Manual Capture
```cpp
camera_fb_t* fb = esp_camera_fb_get();
if (fb && fb->buf && fb->len > 0) {
    processImage(fb);
}
// CRITICAL: Always return
if (fb) {
    esp_camera_fb_return(fb);
    fb = nullptr;
}
```

## RTOS Resources

### Mutex with RAII
```cpp
#include "camera/camera_safety_impl.h"

void criticalSection() {
    SafeMutexLock lock(m_mutex, "MyFunction");
    if (lock.isLocked()) {
        // Protected code
    }
    // Automatically released
}
```

### Mutex Manual
```cpp
// Create with check
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
if (!mutex) {
    Logger::error("Mutex creation failed");
    return false;
}

// Use
if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
    // Critical section
    xSemaphoreGive(mutex);
}

// Cleanup in destructor
~MyClass() {
    if (mutex) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}
```

### Task Creation
```cpp
TaskHandle_t task = nullptr;
BaseType_t result = xTaskCreatePinnedToCore(
    taskFunc,           // Function
    "TaskName",         // Name
    4096,               // Stack (bytes)
    this,               // Parameter
    1,                  // Priority
    &task,              // Handle
    1                   // Core (0 or 1)
);

if (result != pdPASS || !task) {
    Logger::error("Task creation failed");
    return false;
}

// Cleanup before destruction
~MyClass() {
    if (task) {
        vTaskDelete(task);
        task = nullptr;
    }
}
```

## Buffer Safety

### String Operations
```cpp
// ✅ DO: Use safe functions
char buffer[64];
snprintf(buffer, sizeof(buffer), "Value: %d", value);
buffer[sizeof(buffer) - 1] = '\0';

// ❌ DON'T: Use unsafe functions
sprintf(buffer, "Value: %d", value);  // Overflow risk!
strcpy(buffer, string);               // Overflow risk!
```

### Array Access
```cpp
// ✅ DO: Check bounds
bool setValue(size_t index, int value) {
    if (index >= ARRAY_SIZE) {
        return false;
    }
    array[index] = value;
    return true;
}
```

## Power Management

### Battery Monitoring
```cpp
#include "power/power_safety_impl.h"

SafeBatteryMonitor battery(ADC_PIN, 2.0f);
float voltage = battery.readVoltage();
if (voltage > 0.0f) {
    int percent = SafeBatteryMonitor::voltageToPercentage(voltage);
    Logger::info("Battery: %.2fV (%d%%)", voltage, percent);
}
```

### Watchdog
```cpp
#include "power/power_safety_impl.h"

SafeWatchdog watchdog(30);  // 30 second timeout
if (watchdog.enable()) {
    for (int i = 0; i < 100; i++) {
        doWork();
        watchdog.reset();  // Reset periodically
    }
}
// Automatically disabled
```

## Error Handling

### Function Returns
```cpp
bool initialize() {
    if (!validateConfig()) {
        setError("Invalid configuration");
        return false;
    }
    
    if (!allocateResources()) {
        setError("Resource allocation failed");
        cleanup();  // Clean partial state
        return false;
    }
    
    return true;
}

// Caller checks return
if (!myObject.initialize()) {
    Logger::error("Init failed: %s", myObject.getLastError());
    return false;
}
```

## Thread Safety

### ISR Variables
```cpp
volatile bool g_flag = false;

void IRAM_ATTR myISR() {
    g_flag = true;  // Fast, no blocking
}

void setup() {
    attachInterrupt(digitalPinToInterrupt(PIN), myISR, RISING);
}
```

### Shared State
```cpp
class ThreadSafe {
private:
    int m_value;
    SemaphoreHandle_t m_mutex;
    
public:
    void setValue(int v) {
        SafeMutexLock lock(m_mutex, "setValue");
        if (lock.isLocked()) {
            m_value = v;
        }
    }
    
    int getValue() const {
        int result = 0;
        SafeMutexLock lock(m_mutex, "getValue");
        if (lock.isLocked()) {
            result = m_value;
        }
        return result;
    }
};
```

## Common Pitfalls

### ❌ Frame Buffer Leak
```cpp
camera_fb_t* fb = esp_camera_fb_get();
if (!fb) return;
if (!processImage(fb)) {
    return;  // LEAK! Forgot to return buffer
}
esp_camera_fb_return(fb);
```

### ❌ Missing Null Check
```cpp
MyClass* obj = new MyClass();
obj->method();  // Crashes if new fails
```

### ❌ Buffer Overflow
```cpp
char buffer[32];
sprintf(buffer, "%s", longString);  // Overflow!
```

### ❌ Mutex Not Released
```cpp
if (xSemaphoreTake(mutex, timeout) == pdTRUE) {
    if (error) {
        return;  // DEADLOCK! Forgot to give
    }
    xSemaphoreGive(mutex);
}
```

### ❌ Resource Leak
```cpp
~MyClass() {
    // Forgot to delete task/mutex/queue
}
```

## Quick Checklist

Before committing code, verify:

- [ ] All `new` uses `(std::nothrow)` with null check
- [ ] All frame buffers returned in all paths
- [ ] All RTOS resources checked on creation
- [ ] All RTOS resources cleaned up in destructor
- [ ] Shared variables protected by mutex
- [ ] ISR variables declared `volatile`
- [ ] Buffer operations use safe functions
- [ ] Array accesses are bounds-checked
- [ ] Error paths cleanup resources
- [ ] Functions document thread-safety

## Examples

See [safe_implementation_example.cpp](examples/safe_implementation_example.cpp) for complete working examples.

## Full Documentation

- [RESOURCE_MANAGEMENT_GUIDE.md](RESOURCE_MANAGEMENT_GUIDE.md) - Complete guide
- [CODE_REVIEW_CHECKLIST.md](CODE_REVIEW_CHECKLIST.md) - Detailed checklist
- [SECURITY_REVIEW_FINDINGS.md](SECURITY_REVIEW_FINDINGS.md) - Review findings
- [camera_safety_impl.h](src/camera/camera_safety_impl.h) - Camera utilities
- [power_safety_impl.h](src/power/power_safety_impl.h) - Power utilities

---

**When in doubt, prefer safety over performance!**
