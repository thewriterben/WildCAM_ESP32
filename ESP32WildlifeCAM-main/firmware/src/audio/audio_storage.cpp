/**
 * @file audio_storage.cpp
 * @brief Audio file storage and management system implementation
 */

#include "audio_storage.h"
#include "../debug_utils.h"
#include <ArduinoJson.h>
#include <time.h>

AudioStorage::AudioStorage() 
    : initialized(false)
    , recording(false)
    , fileSystem(nullptr)
    , audioFolder("/audio")
    , recordingStartTime(0)
    , samplesWritten(0) {
}

AudioStorage::~AudioStorage() {
    if (recording) {
        stopRecording();
    }
}

bool AudioStorage::init(const AudioRecordingConfig& recordingConfig) {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing audio storage system...");
    
    config = recordingConfig;
    
    // Determine file system to use
    if (SD_CARD_ENABLED && SD_MMC.cardType() != CARD_NONE) {
        fileSystem = &SD_MMC;
        audioFolder = "/audio";
        DEBUG_PRINTLN("Using SD card for audio storage");
    } else {
        fileSystem = &LittleFS;
        audioFolder = "/audio";
        DEBUG_PRINTLN("Using internal flash for audio storage");
    }
    
    // Create audio directory
    if (!createDirectories()) {
        DEBUG_PRINTLN("Failed to create audio directories");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Audio storage system initialized successfully");
    return true;
}

bool AudioStorage::createDirectories() {
    if (!fileSystem) return false;
    
    // Create main audio directory
    if (!fileSystem->exists(audioFolder)) {
        if (!fileSystem->mkdir(audioFolder)) {
            DEBUG_PRINTF("Failed to create directory: %s\n", audioFolder.c_str());
            return false;
        }
    }
    
    // Create subdirectories for organization
    String metadataDir = audioFolder + "/metadata";
    if (!fileSystem->exists(metadataDir)) {
        fileSystem->mkdir(metadataDir);
    }
    
    return true;
}

String AudioStorage::createAudioFilename(const String& prefix, const String& extension) {
    // Create timestamp-based filename
    unsigned long timestamp = millis(); // In a real implementation, use RTC
    
    char filename[64];
    snprintf(filename, sizeof(filename), "%s_%lu.%s", 
             prefix.c_str(), timestamp, extension.c_str());
    
    return audioFolder + "/" + String(filename);
}

bool AudioStorage::startRecording(const String& triggerSource, const AudioFileMetadata& metadata) {
    if (!initialized || recording) {
        DEBUG_PRINTLN("Cannot start recording: not initialized or already recording");
        return false;
    }
    
    // Create filename
    String extension = AudioStorageUtils::formatToExtension(config.format);
    currentFilename = createAudioFilename("wildlife", extension);
    
    DEBUG_PRINTF("Starting audio recording: %s\n", currentFilename.c_str());
    
    // Open file for writing
    currentFile = fileSystem->open(currentFilename, FILE_WRITE);
    if (!currentFile) {
        DEBUG_PRINTF("Failed to create audio file: %s\n", currentFilename.c_str());
        return false;
    }
    
    // Initialize metadata
    currentMetadata = metadata;
    currentMetadata.filename = currentFilename;
    currentMetadata.format = config.format;
    currentMetadata.sampleRate_Hz = config.sampleRate_Hz;
    currentMetadata.channels = config.channels;
    currentMetadata.bitsPerSample = config.bitsPerSample;
    currentMetadata.timestamp = millis();
    currentMetadata.triggerSource = triggerSource;
    
    // Write WAV header (will be updated when recording stops)
    if (config.format == AudioFormat::WAV_PCM_16 || config.format == AudioFormat::WAV_PCM_8) {
        WAVHeader header = createWAVHeader(0); // Data size unknown at start
        currentFile.write((uint8_t*)&header, sizeof(header));
    }
    
    // Initialize recording state
    recording = true;
    recordingStartTime = millis();
    samplesWritten = 0;
    
    DEBUG_PRINTLN("Audio recording started successfully");
    return true;
}

AudioFileMetadata AudioStorage::stopRecording() {
    if (!recording) {
        DEBUG_PRINTLN("No recording in progress");
        return currentMetadata;
    }
    
    DEBUG_PRINTLN("Stopping audio recording...");
    
    // Calculate final metadata
    currentMetadata.duration_ms = millis() - recordingStartTime;
    currentMetadata.fileSize_bytes = currentFile.size();
    
    // Update WAV header with correct data size
    if (config.format == AudioFormat::WAV_PCM_16 || config.format == AudioFormat::WAV_PCM_8) {
        uint32_t dataSize = samplesWritten * config.channels * (config.bitsPerSample / 8);
        updateWAVHeader(currentFile, dataSize);
    }
    
    // Close file
    currentFile.close();
    
    // Save metadata file
    saveMetadataFile(currentMetadata);
    
    DEBUG_PRINTF("Recording stopped. Duration: %d ms, Size: %d bytes\n", 
                currentMetadata.duration_ms, currentMetadata.fileSize_bytes);
    
    recording = false;
    AudioFileMetadata result = currentMetadata;
    
    // Clear current recording state
    currentFilename = "";
    samplesWritten = 0;
    
    return result;
}

bool AudioStorage::writeAudioData(const int16_t* audioData, uint32_t sampleCount) {
    if (!recording || !currentFile) {
        return false;
    }
    
    // Check maximum duration
    if (getCurrentRecordingDuration() > config.maxDuration_s * 1000) {
        DEBUG_PRINTLN("Maximum recording duration reached");
        stopRecording();
        return false;
    }
    
    // Write audio data based on format
    size_t bytesWritten = 0;
    
    if (config.format == AudioFormat::WAV_PCM_16) {
        // Write 16-bit samples directly
        bytesWritten = currentFile.write((uint8_t*)audioData, sampleCount * sizeof(int16_t));
    } else if (config.format == AudioFormat::WAV_PCM_8) {
        // Convert 16-bit to 8-bit
        for (uint32_t i = 0; i < sampleCount; i++) {
            int8_t sample8 = (int8_t)(audioData[i] >> 8); // Simple downsampling
            currentFile.write((uint8_t*)&sample8, 1);
            bytesWritten++;
        }
    }
    
    samplesWritten += sampleCount;
    
    // Flush periodically to ensure data is written
    if (samplesWritten % 1024 == 0) {
        currentFile.flush();
    }
    
    return bytesWritten > 0;
}

uint32_t AudioStorage::getCurrentRecordingDuration() const {
    if (!recording) return 0;
    return millis() - recordingStartTime;
}

WAVHeader AudioStorage::createWAVHeader(uint32_t dataSize) {
    WAVHeader header;
    
    // RIFF Header
    memcpy(header.riff, "RIFF", 4);
    header.fileSize = sizeof(WAVHeader) - 8 + dataSize;
    memcpy(header.wave, "WAVE", 4);
    
    // Format Chunk
    memcpy(header.fmt, "fmt ", 4);
    header.fmtSize = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = config.channels;
    header.sampleRate = config.sampleRate_Hz;
    header.bitsPerSample = config.bitsPerSample;
    header.blockAlign = config.channels * (config.bitsPerSample / 8);
    header.byteRate = config.sampleRate_Hz * header.blockAlign;
    
    // Data Chunk
    memcpy(header.data, "data", 4);
    header.dataSize = dataSize;
    
    return header;
}

bool AudioStorage::updateWAVHeader(File& file, uint32_t dataSize) {
    // Seek to beginning of file
    file.seek(0);
    
    // Create updated header
    WAVHeader header = createWAVHeader(dataSize);
    
    // Write updated header
    size_t written = file.write((uint8_t*)&header, sizeof(header));
    
    return written == sizeof(header);
}

bool AudioStorage::saveMetadataFile(const AudioFileMetadata& metadata) {
    String metadataFilename = getMetadataFilename(metadata.filename);
    
    File metadataFile = fileSystem->open(metadataFilename, FILE_WRITE);
    if (!metadataFile) {
        DEBUG_PRINTF("Failed to create metadata file: %s\n", metadataFilename.c_str());
        return false;
    }
    
    // Create JSON metadata
    String jsonMetadata = AudioStorageUtils::createMetadataJSON(metadata);
    metadataFile.print(jsonMetadata);
    metadataFile.close();
    
    return true;
}

String AudioStorage::getMetadataFilename(const String& audioFilename) {
    // Convert audio filename to metadata filename
    String baseName = audioFilename;
    int lastDot = baseName.lastIndexOf('.');
    if (lastDot > 0) {
        baseName = baseName.substring(0, lastDot);
    }
    return baseName + ".json";
}

std::vector<AudioFileMetadata> AudioStorage::getAudioFileList(uint32_t maxFiles) {
    std::vector<AudioFileMetadata> fileList;
    
    if (!initialized || !fileSystem) return fileList;
    
    File dir = fileSystem->open(audioFolder);
    if (!dir || !dir.isDirectory()) {
        return fileList;
    }
    
    File file = dir.openNextFile();
    while (file && fileList.size() < maxFiles) {
        String filename = file.name();
        
        // Check if it's an audio file
        if (filename.endsWith(".wav") || filename.endsWith(".mp3")) {
            AudioFileMetadata metadata = loadMetadataFile(audioFolder + "/" + filename);
            if (metadata.filename.length() > 0) {
                fileList.push_back(metadata);
            }
        }
        
        file = dir.openNextFile();
    }
    
    dir.close();
    return fileList;
}

AudioFileMetadata AudioStorage::loadMetadataFile(const String& audioFilename) {
    String metadataFilename = getMetadataFilename(audioFilename);
    AudioFileMetadata metadata;
    
    File metadataFile = fileSystem->open(metadataFilename, FILE_READ);
    if (!metadataFile) {
        // Create basic metadata from filename if metadata file doesn't exist
        metadata.filename = audioFilename;
        metadata.timestamp = millis(); // Default timestamp
        return metadata;
    }
    
    String jsonContent = metadataFile.readString();
    metadataFile.close();
    
    return AudioStorageUtils::parseMetadataJSON(jsonContent);
}

uint32_t AudioStorage::cleanupOldFiles(uint32_t maxFiles, unsigned long maxAge) {
    std::vector<AudioFileMetadata> files = getAudioFileList(1000); // Get all files
    uint32_t deletedCount = 0;
    unsigned long currentTime = millis();
    
    // Delete files older than maxAge
    for (const auto& metadata : files) {
        if (currentTime - metadata.timestamp > maxAge) {
            if (fileSystem->remove(metadata.filename)) {
                // Also remove metadata file
                String metadataFile = getMetadataFilename(metadata.filename);
                fileSystem->remove(metadataFile);
                deletedCount++;
            }
        }
    }
    
    // If still too many files, delete oldest ones
    if (files.size() - deletedCount > maxFiles) {
        // Sort by timestamp (oldest first) and delete excess
        // Simplified implementation - in practice would sort by timestamp
        uint32_t toDelete = files.size() - deletedCount - maxFiles;
        for (uint32_t i = 0; i < toDelete && i < files.size(); i++) {
            if (fileSystem->remove(files[i].filename)) {
                String metadataFile = getMetadataFilename(files[i].filename);
                fileSystem->remove(metadataFile);
                deletedCount++;
            }
        }
    }
    
    return deletedCount;
}

// Utility functions implementation
namespace AudioStorageUtils {
    String formatToExtension(AudioFormat format) {
        switch (format) {
            case AudioFormat::WAV_PCM_16:
            case AudioFormat::WAV_PCM_8:
                return "wav";
            case AudioFormat::COMPRESSED_ADPCM:
                return "adpcm";
            case AudioFormat::COMPRESSED_OPUS:
                return "opus";
            default:
                return "wav";
        }
    }
    
    AudioFormat extensionToFormat(const String& extension) {
        if (extension == "wav") return AudioFormat::WAV_PCM_16;
        if (extension == "adpcm") return AudioFormat::COMPRESSED_ADPCM;
        if (extension == "opus") return AudioFormat::COMPRESSED_OPUS;
        return AudioFormat::WAV_PCM_16; // Default
    }
    
    uint32_t estimateFileSize(uint32_t duration_ms, uint32_t sampleRate, 
                             uint8_t channels, uint8_t bitsPerSample) {
        uint32_t samplesPerSecond = sampleRate * channels;
        uint32_t bytesPerSample = bitsPerSample / 8;
        uint32_t duration_s = duration_ms / 1000;
        
        return sizeof(WAVHeader) + (samplesPerSecond * bytesPerSample * duration_s);
    }
    
    String createMetadataJSON(const AudioFileMetadata& metadata) {
        DynamicJsonDocument doc(1024);
        
        doc["filename"] = metadata.filename;
        doc["format"] = static_cast<int>(metadata.format);
        doc["sampleRate"] = metadata.sampleRate_Hz;
        doc["channels"] = metadata.channels;
        doc["bitsPerSample"] = metadata.bitsPerSample;
        doc["duration"] = metadata.duration_ms;
        doc["fileSize"] = metadata.fileSize_bytes;
        doc["timestamp"] = metadata.timestamp;
        doc["species"] = static_cast<int>(metadata.detectedSpecies);
        doc["callType"] = static_cast<int>(metadata.callType);
        doc["confidence"] = metadata.confidence;
        doc["triggerSource"] = metadata.triggerSource;
        doc["location"] = metadata.location;
        doc["temperature"] = metadata.temperatureC;
        doc["humidity"] = metadata.humidityPercent;
        doc["windSpeed"] = metadata.windSpeed_kmh;
        
        String jsonString;
        serializeJson(doc, jsonString);
        return jsonString;
    }
    
    AudioFileMetadata parseMetadataJSON(const String& json) {
        AudioFileMetadata metadata;
        DynamicJsonDocument doc(1024);
        
        if (deserializeJson(doc, json) != DeserializationError::Ok) {
            return metadata; // Return empty metadata on parse error
        }
        
        metadata.filename = doc["filename"].as<String>();
        metadata.format = static_cast<AudioFormat>(doc["format"].as<int>());
        metadata.sampleRate_Hz = doc["sampleRate"];
        metadata.channels = doc["channels"];
        metadata.bitsPerSample = doc["bitsPerSample"];
        metadata.duration_ms = doc["duration"];
        metadata.fileSize_bytes = doc["fileSize"];
        metadata.timestamp = doc["timestamp"];
        metadata.detectedSpecies = static_cast<RaptorSpecies>(doc["species"].as<int>());
        metadata.callType = static_cast<RaptorCallType>(doc["callType"].as<int>());
        metadata.confidence = doc["confidence"];
        metadata.triggerSource = doc["triggerSource"].as<String>();
        metadata.location = doc["location"].as<String>();
        metadata.temperatureC = doc["temperature"];
        metadata.humidityPercent = doc["humidity"];
        metadata.windSpeed_kmh = doc["windSpeed"];
        
        return metadata;
    }
}