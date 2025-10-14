/**
 * @file test_memory_management.cpp
 * @brief Unit tests for memory management and leak detection
 */

#include <unity.h>
#include <Arduino.h>
#include "../src/camera/camera_manager.h"

size_t initial_heap = 0;

void setUp(void) {
    // Record initial heap
    initial_heap = ESP.getFreeHeap();
}

void tearDown(void) {
    // Check for memory leaks
    delay(100); // Allow cleanup to complete
    size_t final_heap = ESP.getFreeHeap();
    
    // Allow small variation (fragmentation)
    int leak = initial_heap - final_heap;
    TEST_ASSERT_LESS_THAN(1024, abs(leak)); // Less than 1KB leak
}

/**
 * Test no memory leak on camera initialization
 */
void test_no_leak_on_init(void) {
    size_t heap_before = ESP.getFreeHeap();
    
    CameraManager* camera = new CameraManager();
    camera->initialize();
    delete camera;
    
    delay(100);
    size_t heap_after = ESP.getFreeHeap();
    
    // Should recover most memory (allow for fragmentation)
    int leak = heap_before - heap_after;
    TEST_ASSERT_LESS_THAN(512, abs(leak));
}

/**
 * Test no memory leak on repeated captures
 */
void test_no_leak_on_repeated_captures(void) {
    CameraManager camera;
    camera.initialize();
    
    size_t heap_before = ESP.getFreeHeap();
    
    // Perform 10 captures
    for (int i = 0; i < 10; i++) {
        int width = 0, height = 0;
        uint8_t* image_data = camera.captureFrame(&width, &height);
        
        if (image_data) {
            camera.releaseFrame(image_data);
        }
        
        delay(10);
    }
    
    delay(100);
    size_t heap_after = ESP.getFreeHeap();
    
    // Memory should be stable
    int leak = heap_before - heap_after;
    TEST_ASSERT_LESS_THAN(256, abs(leak));
}

/**
 * Test proper cleanup on failed initialization
 */
void test_cleanup_on_failed_init(void) {
    size_t heap_before = ESP.getFreeHeap();
    
    // Multiple init attempts with cleanup
    for (int i = 0; i < 5; i++) {
        CameraManager* camera = new CameraManager();
        // May or may not succeed, but should clean up
        camera->initialize();
        delete camera;
        delay(50);
    }
    
    delay(100);
    size_t heap_after = ESP.getFreeHeap();
    
    // Should not accumulate leaks
    int leak = heap_before - heap_after;
    TEST_ASSERT_LESS_THAN(1024, abs(leak));
}

/**
 * Test frame buffer release prevents memory leak
 */
void test_frame_buffer_release(void) {
    CameraManager camera;
    
    if (camera.initialize()) {
        size_t heap_before = ESP.getFreeHeap();
        
        int width = 0, height = 0;
        uint8_t* image_data = camera.captureFrame(&width, &height);
        
        if (image_data) {
            size_t heap_during = ESP.getFreeHeap();
            TEST_ASSERT_LESS_THAN(heap_before, heap_during); // Memory used
            
            camera.releaseFrame(image_data);
            delay(50);
            
            size_t heap_after = ESP.getFreeHeap();
            // Memory should be recovered
            TEST_ASSERT_GREATER_OR_EQUAL(heap_during, heap_after);
        }
    }
}

/**
 * Test RAII pattern - automatic cleanup
 */
void test_raii_cleanup(void) {
    size_t heap_before = ESP.getFreeHeap();
    
    {
        // Camera in local scope - should auto-cleanup
        CameraManager camera;
        camera.initialize();
        
        int width = 0, height = 0;
        uint8_t* image_data = camera.captureFrame(&width, &height);
        
        // Don't manually release - test destructor cleanup
        // camera.releaseFrame(image_data);
    } // Camera destructor called here
    
    delay(100);
    size_t heap_after = ESP.getFreeHeap();
    
    // RAII should have cleaned up
    int leak = heap_before - heap_after;
    TEST_ASSERT_LESS_THAN(512, abs(leak));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_no_leak_on_init);
    RUN_TEST(test_no_leak_on_repeated_captures);
    RUN_TEST(test_cleanup_on_failed_init);
    RUN_TEST(test_frame_buffer_release);
    RUN_TEST(test_raii_cleanup);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
