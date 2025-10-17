/**
 * @file mock_arduino.h
 * @brief Mock Arduino framework for unit testing
 * 
 * Provides mock implementations of Arduino core functions for testing
 * without requiring the Arduino framework.
 */

#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#ifdef NATIVE_TEST

#include <stdint.h>
#include <string>
#include <cstring>
#include <cmath>

// Arduino types
using String = std::string;

// Pin modes
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Pin states
#define LOW 0
#define HIGH 1

// Mock time functions
inline unsigned long millis() {
    static unsigned long mock_millis = 0;
    return mock_millis += 100; // Increment by 100ms each call
}

inline unsigned long micros() {
    return millis() * 1000;
}

inline void delay(unsigned long ms) {
    (void)ms;
}

inline void delayMicroseconds(unsigned long us) {
    (void)us;
}

// Mock pin functions
inline void pinMode(uint8_t pin, uint8_t mode) {
    (void)pin;
    (void)mode;
}

inline void digitalWrite(uint8_t pin, uint8_t value) {
    (void)pin;
    (void)value;
}

inline int digitalRead(uint8_t pin) {
    (void)pin;
    return LOW;
}

inline void analogWrite(uint8_t pin, int value) {
    (void)pin;
    (void)value;
}

inline int analogRead(uint8_t pin) {
    (void)pin;
    return 512; // Mid-range value
}

// Mock serial class
class MockSerial {
public:
    void begin(unsigned long baud) { (void)baud; }
    void print(const char* str) { (void)str; }
    void print(int val) { (void)val; }
    void print(unsigned long val) { (void)val; }
    void print(double val) { (void)val; }
    void println(const char* str) { (void)str; }
    void println(int val) { (void)val; }
    void println(unsigned long val) { (void)val; }
    void println(double val) { (void)val; }
    void println() { }
    int available() { return 0; }
    int read() { return -1; }
};

extern MockSerial Serial;

#endif // NATIVE_TEST

#endif // MOCK_ARDUINO_H
