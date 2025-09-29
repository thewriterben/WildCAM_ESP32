/**
 * @file audio_storage.h
 * @brief Audio file storage and management system
 * 
 * Provides WAV file recording, audio compression, and file management
 * for storing wildlife sound recordings on SD card or internal storage.
 */

#ifndef AUDIO_STORAGE_H
#define AUDIO_STORAGE_H

#include "../config.h"
#include "acoustic_detection.h"
#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>

/**
 * Audio File Format Types
 */
enum class AudioFormat {
    WAV_PCM_16,         // 16-bit PCM WAV format
    WAV_PCM_8,          // 8-bit PCM WAV format  
    COMPRESSED_ADPCM,   // ADPCM compressed format
    COMPRESSED_OPUS     // Opus compressed format (future)
};

/**
 * Audio Recording Configuration
 */
struct AudioRecordingConfig {
    AudioFormat format;
    uint32_t sampleRate_Hz;
    uint8_t channels;
    uint8_t bitsPerSample;
    uint32_t maxDuration_s;
    uint32_t preTriggerDuration_s;
    uint32_t postTriggerDuration_s;
    bool enableCompression;
    uint8_t compressionQuality;  // 1-10, higher = better quality
    
    AudioRecordingConfig() : format(AudioFormat::WAV_PCM_16), 
                           sampleRate_Hz(AUDIO_DEFAULT_SAMPLE_RATE),
                           channels(1), bitsPerSample(16),
                           maxDuration_s(AUDIO_MAX_RECORDING_DURATION),
                           preTriggerDuration_s(AUDIO_PRE_TRIGGER_DURATION),
                           postTriggerDuration_s(AUDIO_POST_TRIGGER_DURATION),
                           enableCompression(AUDIO_COMPRESSION_ENABLED),
                           compressionQuality(AUDIO_COMPRESSION_QUALITY) {}
};

/**
 * Audio File Metadata
 */
struct AudioFileMetadata {
    String filename;
    AudioFormat format;
    uint32_t sampleRate_Hz;
    uint8_t channels;
    uint8_t bitsPerSample;
    uint32_t duration_ms;
    uint32_t fileSize_bytes;
    unsigned long timestamp;
    RaptorSpecies detectedSpecies;
    RaptorCallType callType;
    float confidence;
    String triggerSource;       // "motion", "audio", "manual", "scheduled"
    String location;            // GPS coordinates if available
    float temperatureC;
    float humidityPercent;
    float windSpeed_kmh;
    
    AudioFileMetadata() : format(AudioFormat::WAV_PCM_16), sampleRate_Hz(16000),
                         channels(1), bitsPerSample(16), duration_ms(0),
                         fileSize_bytes(0), timestamp(0),
                         detectedSpecies(RaptorSpecies::UNKNOWN_RAPTOR),
                         callType(RaptorCallType::UNKNOWN_CALL),
                         confidence(0.0f), temperatureC(0.0f),
                         humidityPercent(0.0f), windSpeed_kmh(0.0f) {}
};

/**
 * WAV File Header Structure
 */
struct __attribute__((packed)) WAVHeader {
    // RIFF Header
    char riff[4];               // "RIFF"
    uint32_t fileSize;          // File size - 8 bytes
    char wave[4];               // "WAVE"
    
    // Format Chunk
    char fmt[4];                // "fmt "
    uint32_t fmtSize;           // Format chunk size (16 for PCM)
    uint16_t audioFormat;       // Audio format (1 = PCM)
    uint16_t numChannels;       // Number of channels
    uint32_t sampleRate;        // Sample rate
    uint32_t byteRate;          // Byte rate
    uint16_t blockAlign;        // Block alignment
    uint16_t bitsPerSample;     // Bits per sample
    
    // Data Chunk
    char data[4];               // "data"
    uint32_t dataSize;          // Data size
};

/**
 * Audio Storage System
 */
class AudioStorage {
public:
    /**
     * Constructor
     */
    AudioStorage();
    
    /**
     * Destructor
     */
    ~AudioStorage();
    
    /**
     * Initialize audio storage system
     * @param config Recording configuration
     * @return true if initialization successful
     */
    bool init(const AudioRecordingConfig& config = AudioRecordingConfig());
    
    /**
     * Start recording audio to file
     * @param triggerSource Source of the recording trigger
     * @param metadata Additional metadata for the recording
     * @return true if recording started successfully
     */
    bool startRecording(const String& triggerSource, 
                       const AudioFileMetadata& metadata = AudioFileMetadata());
    
    /**
     * Stop current recording
     * @return Metadata of the completed recording
     */
    AudioFileMetadata stopRecording();
    
    /**
     * Write audio samples to current recording
     * @param audioData Raw audio sample data
     * @param sampleCount Number of audio samples
     * @return true if data written successfully
     */
    bool writeAudioData(const int16_t* audioData, uint32_t sampleCount);
    
    /**
     * Check if currently recording
     */
    bool isRecording() const { return recording; }
    
    /**
     * Get current recording duration in milliseconds
     */
    uint32_t getCurrentRecordingDuration() const;
    
    /**
     * Create audio filename with timestamp
     * @param prefix Filename prefix (default: "audio")
     * @param extension File extension (default: "wav")
     * @return Generated filename
     */
    String createAudioFilename(const String& prefix = "audio", 
                              const String& extension = "wav");
    
    /**
     * Get list of audio files
     * @param maxFiles Maximum number of files to return
     * @return Vector of audio file metadata
     */
    std::vector<AudioFileMetadata> getAudioFileList(uint32_t maxFiles = 100);
    
    /**
     * Delete old audio files to free space
     * @param maxFiles Maximum number of files to keep
     * @param maxAge Maximum age in milliseconds
     * @return Number of files deleted
     */
    uint32_t cleanupOldFiles(uint32_t maxFiles = 50, unsigned long maxAge = 604800000); // 7 days
    
    /**
     * Get storage statistics
     */
    struct StorageStats {
        uint32_t totalFiles;
        uint64_t totalSize_bytes;
        uint64_t availableSpace_bytes;
        uint32_t oldestFile_age_ms;
        uint32_t newestFile_age_ms;
    } getStorageStats();
    
    /**
     * Export audio file metadata to CSV
     * @param filename Output CSV filename
     * @return true if export successful
     */
    bool exportMetadataToCSV(const String& filename);
    
    /**
     * Compress existing audio file
     * @param inputFilename Input WAV filename
     * @param outputFilename Output compressed filename
     * @param quality Compression quality (1-10)
     * @return true if compression successful
     */
    bool compressAudioFile(const String& inputFilename, 
                          const String& outputFilename,
                          uint8_t quality = 5);

private:
    // Configuration
    AudioRecordingConfig config;
    bool initialized;
    bool recording;
    
    // Current recording state
    File currentFile;
    String currentFilename;
    AudioFileMetadata currentMetadata;
    unsigned long recordingStartTime;
    uint32_t samplesWritten;
    
    // File system
    fs::FS* fileSystem;
    String audioFolder;
    
    // Private methods
    bool createDirectories();
    WAVHeader createWAVHeader(uint32_t dataSize);
    bool writeWAVHeader(File& file, uint32_t dataSize);
    bool updateWAVHeader(File& file, uint32_t dataSize);
    String formatTimestamp(unsigned long timestamp);
    bool validateAudioFile(const String& filename);
    uint32_t calculateFileSize(uint32_t sampleCount, uint8_t bitsPerSample, uint8_t channels);
    
    // Compression helpers
    bool compressWithADPCM(const int16_t* input, uint8_t* output, uint32_t sampleCount);
    bool decompressADPCM(const uint8_t* input, int16_t* output, uint32_t compressedSize);
    
    // Metadata management
    bool saveMetadataFile(const AudioFileMetadata& metadata);
    AudioFileMetadata loadMetadataFile(const String& audioFilename);
    String getMetadataFilename(const String& audioFilename);
};

/**
 * Audio Storage Utilities
 */
namespace AudioStorageUtils {
    /**
     * Convert audio format to file extension
     */
    String formatToExtension(AudioFormat format);
    
    /**
     * Get audio format from file extension
     */
    AudioFormat extensionToFormat(const String& extension);
    
    /**
     * Calculate estimated file size
     */
    uint32_t estimateFileSize(uint32_t duration_ms, uint32_t sampleRate, 
                             uint8_t channels, uint8_t bitsPerSample);
    
    /**
     * Validate WAV file header
     */
    bool validateWAVHeader(const WAVHeader& header);
    
    /**
     * Create metadata JSON string
     */
    String createMetadataJSON(const AudioFileMetadata& metadata);
    
    /**
     * Parse metadata from JSON string
     */
    AudioFileMetadata parseMetadataJSON(const String& json);
}

#endif // AUDIO_STORAGE_H