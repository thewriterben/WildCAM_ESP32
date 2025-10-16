# WildCAM ESP32 Code Review Checklist

## Overview
This checklist should be used when reviewing code changes to ensure memory safety, proper resource management, and ESP32 best practices.

## Memory Management

### Dynamic Allocation
- [ ] All `new` operations use `new (std::nothrow)` and check for nullptr
- [ ] All `malloc` / `heap_caps_malloc` calls check return value for nullptr
- [ ] All PSRAM allocations verify PSRAM availability with `psramFound()`
- [ ] Every allocation has a corresponding deallocation in all code paths
- [ ] Array allocations use `new[]` and deallocations use `delete[]`
- [ ] No memory leaks in error handling paths

### PSRAM Usage
- [ ] PSRAM availability checked before allocation: `if (psramFound())`
- [ ] PSRAM allocations use appropriate functions: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- [ ] PSRAM buffers are freed with: `heap_caps_free(buffer)`
- [ ] Frame buffer location set to PSRAM when available: `config.fb_location = CAMERA_FB_IN_PSRAM`

## Pointer Safety

### Null Pointer Checks
- [ ] All pointers validated before dereferencing
- [ ] Function parameters that are pointers checked for nullptr
- [ ] Pointers set to nullptr after deletion/free
- [ ] Double-free prevented by nullptr checks

### Camera Frame Buffers
- [ ] All `esp_camera_fb_get()` results checked for nullptr
- [ ] Frame buffer `buf` member checked: `if (fb && fb->buf && fb->len > 0)`
- [ ] Frame buffers returned in ALL code paths: `esp_camera_fb_return(fb)`
- [ ] Frame buffer pointer set to nullptr after return
- [ ] No frame buffer returned twice

Example:
```cpp
// ✅ CORRECT
camera_fb_t* fb = esp_camera_fb_get();
if (fb && fb->buf && fb->len > 0) {
    processImage(fb);
}
if (fb) {
    esp_camera_fb_return(fb);
    fb = nullptr;
}

// ❌ INCORRECT
camera_fb_t* fb = esp_camera_fb_get();
if (processImage(fb)) {  // No null check
    return true;  // LEAK: fb not returned
}
```

## RTOS Resources

### Semaphores and Mutexes
- [ ] Creation checked for failure: `if (!mutex) { /* error */ }`
- [ ] Every `xSemaphoreCreateMutex()` has cleanup in destructor
- [ ] Every `xSemaphoreTake()` has matching `xSemaphoreGive()`
- [ ] Mutex take has timeout to prevent deadlock
- [ ] Deleted in destructor: `vSemaphoreDelete(mutex)`
- [ ] Set to nullptr after deletion

Example:
```cpp
// ✅ CORRECT
class MyClass {
private:
    SemaphoreHandle_t m_mutex;
public:
    MyClass() : m_mutex(nullptr) {
        m_mutex = xSemaphoreCreateMutex();
        if (!m_mutex) {
            Logger::error("Failed to create mutex");
        }
    }
    
    ~MyClass() {
        if (m_mutex) {
            vSemaphoreDelete(m_mutex);
            m_mutex = nullptr;
        }
    }
    
    void criticalSection() {
        if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Critical section
            xSemaphoreGive(m_mutex);
        }
    }
};
```

### Queues
- [ ] Queue creation checked: `if (!queue) { /* error */ }`
- [ ] Queue size appropriate for use case
- [ ] Queue deleted in destructor: `vQueueDelete(queue)`
- [ ] Set to nullptr after deletion

### Tasks
- [ ] Task creation checked: `if (result != pdPASS) { /* error */ }`
- [ ] Task handle stored for cleanup
- [ ] Task deleted before object destruction: `vTaskDelete(task_handle)`
- [ ] Task function handles nullptr parameter gracefully
- [ ] Task stack size validated (minimum 2KB, typical 4KB)
- [ ] Task priority appropriate (0-configMAX_PRIORITIES-1)
- [ ] Task pinned to appropriate core (0, 1, or tskNO_AFFINITY)

Example:
```cpp
// ✅ CORRECT
TaskHandle_t m_task = nullptr;
BaseType_t result = xTaskCreatePinnedToCore(
    taskFunction,
    "TaskName",
    4096,        // Stack size
    this,        // Parameter
    1,           // Priority
    &m_task,
    1            // Core 1
);

if (result != pdPASS || !m_task) {
    Logger::error("Failed to create task");
    return false;
}

// Cleanup in destructor
~MyClass() {
    if (m_task) {
        vTaskDelete(m_task);
        m_task = nullptr;
    }
}
```

## Thread Safety

### Shared Variables
- [ ] All shared variables protected by mutex or atomic operations
- [ ] ISR-accessed variables declared `volatile`
- [ ] Thread-safety requirements documented in comments
- [ ] Critical sections kept as short as possible
- [ ] No nested mutex locks (deadlock prevention)

### ISR (Interrupt Service Routine) Safety
- [ ] ISR functions marked with `IRAM_ATTR`
- [ ] ISR functions minimal and fast
- [ ] ISR-accessed variables declared `volatile`
- [ ] No blocking calls in ISR (no delays, prints, etc.)
- [ ] Use ISR-safe FreeRTOS functions (`...FromISR`)

Example:
```cpp
// ✅ CORRECT
volatile bool g_motion_detected = false;

void IRAM_ATTR motionISR() {
    g_motion_detected = true;
    // Don't do anything complex here
}

void setup() {
    attachInterrupt(digitalPinToInterrupt(MOTION_PIN), motionISR, RISING);
}
```

## Buffer Management

### Fixed-Size Buffers
- [ ] All string operations use safe functions: `strncpy`, `snprintf`
- [ ] Buffer size constants defined and used consistently
- [ ] Null termination guaranteed: `buffer[size-1] = '\0'`
- [ ] Array bounds checked before access
- [ ] No use of unsafe functions: `strcpy`, `sprintf`, `gets`

Example:
```cpp
// ✅ CORRECT
char buffer[64];
snprintf(buffer, sizeof(buffer), "Value: %d", value);
buffer[sizeof(buffer) - 1] = '\0';  // Ensure termination

// ❌ INCORRECT
char buffer[64];
sprintf(buffer, "Value: %d", value);  // Buffer overflow risk!
```

### Dynamic Buffers
- [ ] Size validated before allocation
- [ ] Allocated size tracked and respected
- [ ] Bounds checked on all accesses
- [ ] Freed in all code paths including error paths

## Error Handling

### Function Return Values
- [ ] All error conditions have return values or exceptions
- [ ] Error codes documented in comments
- [ ] Caller checks return values
- [ ] Error messages logged with context
- [ ] Resources cleaned up on error

### Error Messages
- [ ] Errors logged with appropriate level (error, warning, info)
- [ ] Error messages include context and values
- [ ] Error state tracked and available via getter
- [ ] Error messages help with debugging

Example:
```cpp
// ✅ CORRECT
bool initialize() {
    if (!validateConfig()) {
        setError("Configuration validation failed: Invalid pin assignment");
        return false;
    }
    
    if (!allocateBuffers()) {
        setError("Buffer allocation failed: Insufficient memory");
        cleanup();  // Clean up partial state
        return false;
    }
    
    return true;
}
```

## Camera-Specific Checks

### Initialization
- [ ] Pin configuration validated before init
- [ ] PSRAM requirement checked if needed
- [ ] Camera init return value checked
- [ ] Sensor availability verified
- [ ] Cleanup performed on init failure

### Image Capture
- [ ] Memory availability checked before capture
- [ ] Capture timeout configured
- [ ] Frame buffer validity checked
- [ ] Frame buffer returned in all paths
- [ ] Capture statistics updated

### Configuration
- [ ] Frame size appropriate for available memory
- [ ] JPEG quality balanced for size/quality
- [ ] Frame buffer count appropriate (1-2 typically)
- [ ] Clock frequency valid for sensor

## Power Management

### Battery Monitoring
- [ ] Voltage readings filtered/averaged
- [ ] Voltage range validated (2.5V - 4.3V for LiPo)
- [ ] Battery percentage calculated correctly
- [ ] Low battery conditions detected
- [ ] Critical battery triggers shutdown

### Sleep Modes
- [ ] Sleep prerequisites validated
- [ ] Wake sources configured correctly
- [ ] WiFi/Bluetooth disabled before sleep
- [ ] Peripherals powered down appropriately
- [ ] Wake up cause logged

### Watchdog Timer
- [ ] Timeout appropriate for operations
- [ ] Reset called regularly in long operations
- [ ] Disabled or reset before long blocking operations
- [ ] Error handling for watchdog failures

## Documentation

### Code Comments
- [ ] Complex algorithms explained
- [ ] Thread-safety requirements documented
- [ ] Resource ownership documented
- [ ] Caller responsibilities documented (e.g., "must free result")
- [ ] Assumptions documented

### Function Documentation
- [ ] Purpose described
- [ ] Parameters documented
- [ ] Return values documented
- [ ] Error conditions documented
- [ ] Thread-safety documented
- [ ] Resource management documented

Example:
```cpp
/**
 * @brief Capture image from camera
 * @param metadata Optional metadata storage (can be nullptr)
 * @return Frame buffer pointer or nullptr on failure
 * @note Thread-safe: Protected by internal mutex
 * @warning Caller MUST call returnFrameBuffer() to prevent memory leak
 * @warning Always check return value for nullptr before use
 */
camera_fb_t* captureImage(ImageMetadata* metadata = nullptr);
```

## Best Practices

### ESP32-Specific
- [ ] Core affinity considered for tasks (Core 0: WiFi/BT, Core 1: App)
- [ ] Stack sizes appropriate (min 2KB, typical 4KB, network 8KB+)
- [ ] IRAM usage minimized (limited to 128KB)
- [ ] Flash access minimized in time-critical code
- [ ] Hardware timers used appropriately

### FreeRTOS
- [ ] Task priorities appropriate and documented
- [ ] Blocking operations use timeouts
- [ ] Task synchronization uses appropriate primitives
- [ ] Stack overflow checking enabled in debug builds
- [ ] Idle task not blocked

### Performance
- [ ] Memory allocations minimized in hot paths
- [ ] Large buffers allocated at initialization
- [ ] String operations efficient
- [ ] Logging level appropriate (verbose in debug only)
- [ ] Delays/sleeps used appropriately

## Testing

### Unit Tests
- [ ] Error paths tested
- [ ] Boundary conditions tested
- [ ] Resource cleanup tested
- [ ] Thread safety tested (if applicable)
- [ ] Memory leaks verified (none)

### Integration Tests
- [ ] Full initialization/cleanup cycle tested
- [ ] Error recovery tested
- [ ] Long-running stability tested
- [ ] Memory usage monitored
- [ ] Power consumption acceptable

## Final Review

### Pre-Merge Checklist
- [ ] All compiler warnings addressed
- [ ] Code builds successfully
- [ ] Tests pass
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] No debug code left in
- [ ] Logging levels appropriate
- [ ] Code formatted consistently

### Reviewer Questions
- Does this change introduce any memory leaks?
- Are all error paths handled correctly?
- Is resource cleanup guaranteed?
- Are thread-safety requirements met?
- Is the documentation accurate and complete?
- Are there any security implications?
- Is the performance acceptable?
- Does this follow project conventions?

## Common Issues to Watch For

### Memory Issues
- ❌ Forgetting to return camera frame buffers
- ❌ Not checking allocation results
- ❌ Memory leaks in error paths
- ❌ PSRAM allocations without checking availability
- ❌ Double-free of resources

### Thread Safety Issues
- ❌ Unprotected shared variable access
- ❌ Deadlocks from nested locks
- ❌ Race conditions on state changes
- ❌ ISR functions doing too much work
- ❌ Non-volatile variables in ISR

### Resource Management Issues
- ❌ RTOS resources not cleaned up
- ❌ Tasks not deleted before destruction
- ❌ Semaphores taken but not given
- ❌ Queues not deleted
- ❌ File handles not closed

### Error Handling Issues
- ❌ Ignoring return values
- ❌ No error logging
- ❌ Resources not cleaned up on error
- ❌ Error messages not helpful
- ❌ Errors not propagated to caller

## Resources

- [RESOURCE_MANAGEMENT_GUIDE.md](RESOURCE_MANAGEMENT_GUIDE.md) - Detailed best practices
- [camera_safety_impl.h](src/camera/camera_safety_impl.h) - Safe camera patterns
- [power_safety_impl.h](src/power/power_safety_impl.h) - Safe power patterns
- [safe_implementation_example.cpp](examples/safe_implementation_example.cpp) - Example code

## Version History

- v1.0 - Initial checklist (2025-10-16)

---

**Remember:** When in doubt, prefer safety over performance. It's easier to optimize safe code than to fix unsafe code in production.
