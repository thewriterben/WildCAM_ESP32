/**
 * @file mock_camera.h
 * @brief Mock camera interface for unit testing
 * 
 * Provides mock implementations of ESP32-CAM functions for testing
 * without requiring actual hardware.
 */

#ifndef MOCK_CAMERA_H
#define MOCK_CAMERA_H

#include <stdint.h>
#include <stddef.h>

#ifdef NATIVE_TEST

// Mock camera frame buffer structure
typedef struct {
    uint8_t* buf;
    size_t len;
    size_t width;
    size_t height;
    int format;
} camera_fb_t;

// Mock camera configuration structure
typedef struct {
    int pin_pwdn;
    int pin_reset;
    int pin_xclk;
    int pin_sscb_sda;
    int pin_sscb_scl;
    int pin_d7;
    int pin_d6;
    int pin_d5;
    int pin_d4;
    int pin_d3;
    int pin_d2;
    int pin_d1;
    int pin_d0;
    int pin_vsync;
    int pin_href;
    int pin_pclk;
    int xclk_freq_hz;
    int ledc_timer;
    int ledc_channel;
    int pixel_format;
    int frame_size;
    int jpeg_quality;
    size_t fb_count;
    bool grab_mode;
} camera_config_t;

// Mock camera initialization function
inline int esp_camera_init(camera_config_t* config) {
    (void)config;
    return 0; // Success
}

// Mock camera capture function
inline camera_fb_t* esp_camera_fb_get() {
    static uint8_t dummy_buffer[1024];
    static camera_fb_t fb = {
        .buf = dummy_buffer,
        .len = 1024,
        .width = 320,
        .height = 240,
        .format = 4 // JPEG
    };
    return &fb;
}

// Mock camera frame buffer return function
inline void esp_camera_fb_return(camera_fb_t* fb) {
    (void)fb;
}

#endif // NATIVE_TEST

#endif // MOCK_CAMERA_H
