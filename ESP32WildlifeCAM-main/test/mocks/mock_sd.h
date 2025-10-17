/**
 * @file mock_sd.h
 * @brief Mock SD card interface for unit testing
 * 
 * Provides mock implementations of SD card functions for testing
 * without requiring actual hardware.
 */

#ifndef MOCK_SD_H
#define MOCK_SD_H

#include <string>
#include <map>

#ifdef NATIVE_TEST

// Mock SD card class
class MockSD {
public:
    static bool begin(int pin = -1) {
        initialized = true;
        return true;
    }
    
    static bool exists(const char* path) {
        return fileSystem.find(std::string(path)) != fileSystem.end();
    }
    
    static bool mkdir(const char* path) {
        directories.insert(std::string(path));
        return true;
    }
    
    static bool remove(const char* path) {
        fileSystem.erase(std::string(path));
        return true;
    }
    
    static uint64_t totalBytes() {
        return 8ULL * 1024 * 1024 * 1024; // 8GB
    }
    
    static uint64_t usedBytes() {
        return 100ULL * 1024 * 1024; // 100MB
    }
    
    // Helper for tests
    static void reset() {
        fileSystem.clear();
        directories.clear();
        initialized = false;
    }
    
    static void addFile(const std::string& path, const std::string& content) {
        fileSystem[path] = content;
    }
    
private:
    static bool initialized;
    static std::map<std::string, std::string> fileSystem;
    static std::set<std::string> directories;
};

bool MockSD::initialized = false;
std::map<std::string, std::string> MockSD::fileSystem;
std::set<std::string> MockSD::directories;

#define SD MockSD

#endif // NATIVE_TEST

#endif // MOCK_SD_H
