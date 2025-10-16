# WildCAM ESP32 Resource Management Guide

## Overview
This guide provides best practices and requirements for safe resource management in the WildCAM ESP32 project.

## Memory Management

### Dynamic Allocation Best Practices

1. **Always check for allocation failures:**
```cpp
// ✅ CORRECT - Use nothrow and check for nullptr
MyClass* obj = new (std::nothrow) MyClass();
if (!obj) {
    Logger::error("Failed to allocate MyClass");
    return false;
}

// ❌ INCORRECT - No null check
MyClass* obj = new MyClass();
obj->doSomething(); // Crash if allocation failed
```

2. **Always pair allocations with deallocations:**
```cpp
// ✅ CORRECT - Cleanup in destructor
class MyManager {
private:
    uint8_t* buffer;
public:
    MyManager() : buffer(nullptr) {
        buffer = new (std::nothrow) uint8_t[1024];
    }
    
    ~MyManager() {
        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
        }
    }
};
```

3. **PSRAM Allocations:**
```cpp
// ✅ CORRECT - Check PSRAM availability and allocation
if (psramFound()) {
    uint8_t* psram_buffer = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (psram_buffer) {
        // Use buffer
        heap_caps_free(psram_buffer);
    }
}
```

## Camera Frame Buffer Management

### Critical Rules

1. **ALWAYS return frame buffers:**
```cpp
// ✅ CORRECT - Return buffer in all paths
camera_fb_t* fb = esp_camera_fb_get();
if (fb) {
    // Process image
    processImage(fb);
    
    // MUST return buffer
    esp_camera_fb_return(fb);
    fb = nullptr; // Good practice
}

// ❌ INCORRECT - Missing return on error path
camera_fb_t* fb = esp_camera_fb_get();
if (fb) {
    if (!processImage(fb)) {
        return false; // LEAK! Buffer not returned
    }
    esp_camera_fb_return(fb);
}
```

2. **Check for nullptr before use:**
```cpp
// ✅ CORRECT
camera_fb_t* fb = getLastFrameBuffer();
if (fb && fb->buf && fb->len > 0) {
    // Safe to use
    saveToSD(fb);
}
returnFrameBuffer(fb);
```

## RTOS Resource Management

### Semaphores and Mutexes

1. **Create with error checking:**
```cpp
// ✅ CORRECT - Check creation result
SemaphoreHandle_t m_mutex = xSemaphoreCreateMutex();
if (!m_mutex) {
    Logger::error("Failed to create mutex");
    return false;
}
```

2. **Always cleanup in destructor:**
```cpp
// ✅ CORRECT
~MyClass() {
    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
        m_mutex = nullptr;
    }
}
```

3. **Take and give semaphores properly:**
```cpp
// ✅ CORRECT - Always give after take
if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
    // Critical section
    updateSharedState();
    
    xSemaphoreGive(m_mutex); // MUST give back
}
```

### Queues

1. **Create with validation:**
```cpp
// ✅ CORRECT
QueueHandle_t m_queue = xQueueCreate(10, sizeof(Event));
if (!m_queue) {
    Logger::error("Failed to create queue");
    return false;
}
```

2. **Cleanup in destructor:**
```cpp
// ✅ CORRECT
~MyClass() {
    if (m_queue) {
        vQueueDelete(m_queue);
        m_queue = nullptr;
    }
}
```

### Tasks

1. **Create with error checking:**
```cpp
// ✅ CORRECT
TaskHandle_t task_handle = nullptr;
BaseType_t result = xTaskCreatePinnedToCore(
    taskFunction,
    "TaskName",
    4096,
    this,
    1,
    &task_handle,
    1  // Core 1
);

if (result != pdPASS || !task_handle) {
    Logger::error("Failed to create task");
    return false;
}
```

2. **Cleanup tasks properly:**
```cpp
// ✅ CORRECT - Delete task before class destruction
~MyClass() {
    if (task_handle) {
        vTaskDelete(task_handle);
        task_handle = nullptr;
    }
}
```

## Thread Safety

### Shared Variable Access

1. **Protect shared state:**
```cpp
// ✅ CORRECT - Mutex protected
class ThreadSafeCounter {
private:
    int m_count;
    SemaphoreHandle_t m_mutex;
    
public:
    void increment() {
        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
            m_count++;
            xSemaphoreGive(m_mutex);
        }
    }
    
    int getCount() const {
        int value = 0;
        if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
            value = m_count;
            xSemaphoreGive(m_mutex);
        }
        return value;
    }
};
```

2. **Use volatile for ISR-accessed variables:**
```cpp
// ✅ CORRECT - volatile for ISR
volatile bool g_motion_detected = false;

void IRAM_ATTR motionISR() {
    g_motion_detected = true;
}
```

## Error Handling

### Function Return Values

1. **Check all function returns:**
```cpp
// ✅ CORRECT - Check returns
if (!initializeCamera()) {
    Logger::error("Camera init failed");
    cleanup();
    return false;
}

if (!initializeStorage()) {
    Logger::error("Storage init failed");
    cleanup();
    return false;
}
```

2. **Provide error context:**
```cpp
// ✅ CORRECT - Detailed error messages
bool initialize() {
    if (!validateHardware()) {
        setError("Hardware validation failed: Pin conflict detected");
        return false;
    }
    
    if (!allocateBuffers()) {
        setError("Buffer allocation failed: Insufficient heap memory");
        return false;
    }
    
    return true;
}
```

## Buffer Management

### Fixed-Size Buffers

1. **Always validate bounds:**
```cpp
// ✅ CORRECT - Bounds checking
void setError(const char* error) {
    if (error) {
        strncpy(m_lastError, error, sizeof(m_lastError) - 1);
        m_lastError[sizeof(m_lastError) - 1] = '\0'; // Ensure null termination
    }
}

// ❌ INCORRECT - No bounds check
void setError(const char* error) {
    strcpy(m_lastError, error); // Buffer overflow risk!
}
```

2. **Check input validation:**
```cpp
// ✅ CORRECT - Validate array access
bool setPixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= width || y >= height) {
        return false; // Out of bounds
    }
    buffer[y * width + x] = color;
    return true;
}
```

## Best Practices Summary

### DO:
- ✅ Use `new (std::nothrow)` and check for `nullptr`
- ✅ Return camera frame buffers in ALL code paths
- ✅ Check RTOS resource creation success
- ✅ Cleanup all resources in destructors
- ✅ Use mutexes for shared state access
- ✅ Validate all pointers before dereferencing
- ✅ Check array bounds before access
- ✅ Provide detailed error messages
- ✅ Document thread-safety requirements

### DON'T:
- ❌ Assume allocations always succeed
- ❌ Forget to return frame buffers
- ❌ Ignore RTOS function return values
- ❌ Access shared variables without protection
- ❌ Use `strcpy` or `sprintf` without bounds checking
- ❌ Dereference pointers without null checks
- ❌ Leave resources unfreed in error paths
- ❌ Create tasks without error checking

## Code Review Checklist

When reviewing code, verify:

1. [ ] All dynamic allocations are checked for null
2. [ ] All camera frame buffers are returned
3. [ ] All RTOS resources are validated after creation
4. [ ] All RTOS resources are cleaned up in destructors
5. [ ] Shared variables are protected by mutexes
6. [ ] All pointers are checked before dereferencing
7. [ ] Array accesses are bounds-checked
8. [ ] Error paths cleanup resources properly
9. [ ] Functions document thread-safety requirements
10. [ ] Buffer operations use safe string functions

## Additional Resources

- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/
- FreeRTOS Documentation: https://www.freertos.org/Documentation/
- ESP32 Camera Driver: https://github.com/espressif/esp32-camera

## Support

For questions or issues related to resource management, please refer to:
- Project README.md
- IMPLEMENTATION_SUMMARY.md
- Issue tracker on GitHub
