/**
 * @file board_detector.h
 * @brief Hardware detection and board identification system
 */

#ifndef BOARD_DETECTOR_H
#define BOARD_DETECTOR_H

#include "camera_board.h"
#include <memory>

class BoardDetector {
public:
    /**
     * Detect the current board type based on hardware fingerprinting
     * @return Detected board type
     */
    static BoardType detectBoardType();
    
    /**
     * Create appropriate board instance based on detected hardware
     * @return Unique pointer to board implementation
     */
    static std::unique_ptr<CameraBoard> createBoard();
    
    /**
     * Create board instance for specific board type
     * @param type Board type to create
     * @return Unique pointer to board implementation
     */
    static std::unique_ptr<CameraBoard> createBoard(BoardType type);
    
    /**
     * Get board name string for given type
     * @param type Board type
     * @return Board name string
     */
    static const char* getBoardName(BoardType type);
    
    /**
     * Check if given board type is supported
     * @param type Board type to check
     * @return True if supported
     */
    static bool isBoardSupported(BoardType type);
    
    /**
     * Detect ESP32 chip variant
     * @return Chip model string
     */
    static const char* getChipModel();
    
    /**
     * Check PSRAM availability
     * @return True if PSRAM is available
     */
    static bool hasPSRAM();
    
    /**
     * Get chip ID for hardware fingerprinting
     * @return Chip ID
     */
    static uint32_t getChipId();

private:
    /**
     * Test pin configuration to identify board type
     * @return Detected board type based on pin configuration
     */
    static BoardType detectByPinConfiguration();
    
    /**
     * Test GPIO pin state for board identification
     * @param pin GPIO pin number
     * @param expected_state Expected pin state
     * @return True if pin matches expected state
     */
    static bool testGPIOPin(int pin, bool expected_state);
    
    /**
     * Detect board based on I2C devices present
     * @return Board type if identifiable by I2C devices
     */
    static BoardType detectByI2CDevices();
    
    /**
     * Check if specific GPIO configuration is valid for a board
     * @param gpio_map GPIO mapping to test
     * @return True if configuration is valid
     */
    static bool validateGPIOConfiguration(const GPIOMap& gpio_map);
};

#endif // BOARD_DETECTOR_H