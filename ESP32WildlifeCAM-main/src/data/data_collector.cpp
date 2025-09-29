#include "data_collector.h"
#include "../utils/logger.h"
#include <time.h>
#include <esp_system.h>

bool DataCollector::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing data collector...");

    // Ensure base directories exist
    if (!ensureDirectory(IMAGE_FOLDER)) {
        LOG_ERROR("Failed to create image directory");
        return false;
    }

    if (!ensureDirectory(DATA_FOLDER)) {
        LOG_ERROR("Failed to create data directory");
        return false;
    }

    // Create species folders if enabled
    if (speciesOrganizationEnabled) {
        LOG_INFO("Creating species organization folders...");
        // Create folders for common species
        ensureDirectory(String(IMAGE_FOLDER) + "/deer");
        ensureDirectory(String(IMAGE_FOLDER) + "/bear");
        ensureDirectory(String(IMAGE_FOLDER) + "/fox");
        ensureDirectory(String(IMAGE_FOLDER) + "/unknown");
    }

    // Reset statistics
    resetStatistics();

    initialized = true;
    enabled = true;

    LOG_INFO("Data collector initialized successfully");
    LOG_INFO("Species organization: " + String(speciesOrganizationEnabled ? "enabled" : "disabled"));
    LOG_INFO("Time organization: " + String(timeOrganizationEnabled ? "enabled" : "disabled"));

    return true;
}

DataCollector::CollectionResult DataCollector::collectMotionData(const uint8_t* imageData, 
                                                                size_t imageSize,
                                                                float motionLevel, 
                                                                const String& motionSource) {
    CollectionResult result = {};
    result.eventType = EventType::MOTION_TRIGGER;
    
    if (!initialized || !enabled || imageData == nullptr) {
        return result;
    }

    uint32_t startTime = millis();

    // Generate filenames
    String imageFilename = generateFilename("motion", ".jpg");
    String metadataFilename = generateFilename("motion", ".json");

    // Create metadata
    DataMetadata metadata = createMetadata(EventType::MOTION_TRIGGER, imageSize);
    metadata.motionDetected = true;
    metadata.motionLevel = motionLevel;
    metadata.motionSource = motionSource;
    metadata.filename = imageFilename;

    // Save image data
    if (saveImageData(imageData, imageSize, imageFilename)) {
        result.imageFilename = imageFilename;
        result.imageSize = imageSize;
        
        // Save metadata
        if (saveMetadata(metadata, metadataFilename)) {
            result.metadataFilename = metadataFilename;
            result.success = true;
        }
        
        // Organize data if enabled
        if (timeOrganizationEnabled) {
            organizeByDate(imageFilename);
        }
    }

    result.processingTime = millis() - startTime;
    updateStatistics(result);

    if (result.success) {
        LOG_INFO("Motion data collected: " + imageFilename);
    } else {
        LOG_ERROR("Failed to collect motion data");
    }

    return result;
}

DataCollector::CollectionResult DataCollector::collectClassificationData(const uint8_t* imageData, 
                                                                        size_t imageSize,
                                                                        const WildlifeClassifier::ClassificationResult& classificationResult) {
    CollectionResult result = {};
    result.eventType = EventType::AI_CLASSIFICATION;
    
    if (!initialized || !enabled || imageData == nullptr) {
        return result;
    }

    uint32_t startTime = millis();

    // Generate filenames
    String speciesPrefix = WildlifeClassifier::getSpeciesName(classificationResult.species);
    speciesPrefix.toLowerCase();
    speciesPrefix.replace(" ", "_");
    
    String imageFilename = generateFilename(speciesPrefix, ".jpg");
    String metadataFilename = generateFilename(speciesPrefix, ".json");

    // Create metadata
    DataMetadata metadata = createMetadata(EventType::AI_CLASSIFICATION, imageSize);
    metadata.species = classificationResult.species;
    metadata.speciesConfidence = classificationResult.confidence;
    metadata.speciesName = classificationResult.speciesName;
    metadata.animalCount = classificationResult.animalCount;
    metadata.isDangerous = WildlifeClassifier::isDangerousSpecies(classificationResult.species);
    metadata.filename = imageFilename;

    // Save image data
    if (saveImageData(imageData, imageSize, imageFilename)) {
        result.imageFilename = imageFilename;
        result.imageSize = imageSize;
        
        // Save metadata
        if (saveMetadata(metadata, metadataFilename)) {
            result.metadataFilename = metadataFilename;
            result.success = true;
        }
        
        // Organize by species if enabled
        if (speciesOrganizationEnabled) {
            organizeBySpecies(imageFilename, classificationResult.species);
        }
        
        // Organize by date if enabled
        if (timeOrganizationEnabled) {
            organizeByDate(imageFilename);
        }
    }

    result.processingTime = millis() - startTime;
    updateStatistics(result);

    if (result.success) {
        LOG_INFO("Classification data collected: " + imageFilename + " (" + metadata.speciesName + ")");
    } else {
        LOG_ERROR("Failed to collect classification data");
    }

    return result;
}

DataCollector::CollectionResult DataCollector::collectScheduledData(const uint8_t* imageData, size_t imageSize) {
    CollectionResult result = {};
    result.eventType = EventType::SCHEDULED_CAPTURE;
    
    if (!initialized || !enabled || imageData == nullptr) {
        return result;
    }

    uint32_t startTime = millis();

    // Generate filenames
    String imageFilename = generateFilename("scheduled", ".jpg");
    String metadataFilename = generateFilename("scheduled", ".json");

    // Create metadata
    DataMetadata metadata = createMetadata(EventType::SCHEDULED_CAPTURE, imageSize);
    metadata.filename = imageFilename;

    // Save image data
    if (saveImageData(imageData, imageSize, imageFilename)) {
        result.imageFilename = imageFilename;
        result.imageSize = imageSize;
        
        // Save metadata
        if (saveMetadata(metadata, metadataFilename)) {
            result.metadataFilename = metadataFilename;
            result.success = true;
        }
        
        // Organize by date if enabled
        if (timeOrganizationEnabled) {
            organizeByDate(imageFilename);
        }
    }

    result.processingTime = millis() - startTime;
    updateStatistics(result);

    return result;
}

bool DataCollector::saveMetadata(const DataMetadata& metadata, const String& filename) {
    DynamicJsonDocument doc(2048);
    
    // Timestamp and identification
    doc["timestamp"] = metadata.timestamp;
    doc["datetime"] = metadata.dateTime;
    doc["filename"] = metadata.filename;
    doc["event_type"] = static_cast<int>(metadata.eventType);
    
    // Environmental data
    JsonObject env = doc.createNestedObject("environment");
    env["battery_voltage"] = metadata.batteryVoltage;
    env["battery_percentage"] = metadata.batteryPercentage;
    env["solar_voltage"] = metadata.solarVoltage;
    env["is_charging"] = metadata.isCharging;
    env["temperature"] = metadata.temperature;
    env["humidity"] = metadata.humidity;
    env["pressure"] = metadata.pressure;
    
    // Motion data
    if (metadata.motionDetected) {
        JsonObject motion = doc.createNestedObject("motion");
        motion["detected"] = metadata.motionDetected;
        motion["level"] = metadata.motionLevel;
        motion["source"] = metadata.motionSource;
    }
    
    // Species data
    if (metadata.species != WildlifeClassifier::SpeciesType::UNKNOWN) {
        JsonObject species = doc.createNestedObject("species");
        species["type"] = metadata.speciesName;
        species["confidence"] = metadata.speciesConfidence;
        species["count"] = metadata.animalCount;
        species["dangerous"] = metadata.isDangerous;
    }
    
    // Image data
    JsonObject image = doc.createNestedObject("image");
    image["size"] = metadata.imageSize;
    image["width"] = metadata.imageWidth;
    image["height"] = metadata.imageHeight;
    image["quality"] = metadata.jpegQuality;
    
    // System data
    JsonObject system = doc.createNestedObject("system");
    system["uptime"] = metadata.uptime;
    system["cpu_frequency"] = metadata.cpuFrequency;
    system["free_heap"] = metadata.freeHeap;
    system["power_state"] = static_cast<int>(metadata.powerState);
    
    // Save to file
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        LOG_ERROR("Failed to create metadata file: " + filename);
        return false;
    }
    
    size_t written = serializeJson(doc, file);
    file.close();
    
    if (written == 0) {
        LOG_ERROR("Failed to write metadata: " + filename);
        return false;
    }
    
    return true;
}

bool DataCollector::organizeBySpecies(const String& imageFilename, WildlifeClassifier::SpeciesType species) {
    String speciesFolder = getSpeciesFolder(species);
    String targetPath = String(IMAGE_FOLDER) + "/" + speciesFolder;
    
    if (!ensureDirectory(targetPath)) {
        LOG_WARNING("Failed to create species folder: " + targetPath);
        return false;
    }
    
    // For simplicity, we'll just log the organization
    // In a full implementation, you might create symlinks or copy files
    LOG_DEBUG("Image organized to species folder: " + speciesFolder);
    return true;
}

bool DataCollector::organizeByDate(const String& imageFilename) {
    String dateFolder = getDateFolder();
    String targetPath = String(IMAGE_FOLDER) + "/" + dateFolder;
    
    if (!ensureDirectory(targetPath)) {
        LOG_WARNING("Failed to create date folder: " + targetPath);
        return false;
    }
    
    LOG_DEBUG("Image organized to date folder: " + dateFolder);
    return true;
}

void DataCollector::resetStatistics() {
    stats = {};
    collectionCounter = 0;
    LOG_INFO("Data collection statistics reset");
}

void DataCollector::cleanup() {
    if (initialized) {
        LOG_INFO("Data collector shutting down");
        LOG_INFO("Total collections: " + String(stats.totalCollections));
        LOG_INFO("Success rate: " + String(stats.successRate * 100, 1) + "%");
        
        initialized = false;
    }
}

String DataCollector::generateFilename(const String& prefix, const String& extension) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char filename[100];
    snprintf(filename, sizeof(filename), "%s/%s_%04d%02d%02d_%02d%02d%02d_%04d%s",
             IMAGE_FOLDER,
             prefix.c_str(),
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec,
             collectionCounter++,
             extension.c_str());

    return String(filename);
}

DataCollector::DataMetadata DataCollector::createMetadata(EventType eventType, size_t imageSize) {
    DataMetadata metadata = {};
    
    // Timestamp
    time_t now;
    time(&now);
    metadata.timestamp = now;
    metadata.dateTime = getCurrentTimestamp();
    metadata.eventType = eventType;
    
    // Environmental data (placeholder - would come from sensors)
    getEnvironmentalData(metadata.temperature, metadata.humidity, metadata.pressure);
    
    // Power data (would come from PowerManager)
    metadata.batteryVoltage = 3.8f; // Placeholder
    metadata.batteryPercentage = 75.0f; // Placeholder
    metadata.solarVoltage = 5.2f; // Placeholder
    metadata.isCharging = true; // Placeholder
    
    // Image data
    metadata.imageSize = imageSize;
    metadata.imageWidth = 1600; // Placeholder
    metadata.imageHeight = 1200; // Placeholder
    metadata.jpegQuality = CAMERA_JPEG_QUALITY;
    
    // System data
    metadata.uptime = millis();
    metadata.cpuFrequency = 240; // Placeholder
    metadata.freeHeap = ESP.getFreeHeap();
    metadata.powerState = PowerManager::PowerState::NORMAL; // Placeholder
    
    return metadata;
}

bool DataCollector::saveImageData(const uint8_t* imageData, size_t imageSize, const String& filename) {
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        LOG_ERROR("Failed to create image file: " + filename);
        return false;
    }
    
    size_t written = file.write(imageData, imageSize);
    file.close();
    
    if (written != imageSize) {
        LOG_ERROR("Incomplete image write: " + filename);
        return false;
    }
    
    return true;
}

void DataCollector::updateStatistics(const CollectionResult& result) {
    stats.totalCollections++;
    
    if (result.success) {
        stats.successfulCollections++;
        stats.totalDataSize += result.imageSize;
    } else {
        stats.failedCollections++;
    }
    
    // Update event type counts
    int eventIndex = static_cast<int>(result.eventType);
    if (eventIndex >= 0 && eventIndex < 5) {
        stats.eventTypeCounts[eventIndex]++;
    }
    
    // Update success rate
    stats.successRate = (float)stats.successfulCollections / stats.totalCollections;
    
    // Update average processing time
    stats.averageProcessingTime = ((stats.averageProcessingTime * (stats.totalCollections - 1)) + 
                                  result.processingTime) / stats.totalCollections;
}

String DataCollector::getSpeciesFolder(WildlifeClassifier::SpeciesType species) {
    String folderName = WildlifeClassifier::getSpeciesName(species);
    folderName.toLowerCase();
    folderName.replace(" ", "_");
    folderName.replace("-", "_");
    
    if (folderName == "unknown") {
        folderName = "unknown";
    }
    
    return folderName;
}

String DataCollector::getDateFolder() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char dateFolder[20];
    snprintf(dateFolder, sizeof(dateFolder), "%04d_%02d_%02d",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday);

    return String(dateFolder);
}

bool DataCollector::ensureDirectory(const String& path) {
    if (SD_MMC.exists(path.c_str())) {
        return true;
    }
    
    return SD_MMC.mkdir(path.c_str());
}

String DataCollector::getCurrentTimestamp() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    return String(timestamp);
}

void DataCollector::getEnvironmentalData(float& temperature, float& humidity, float& pressure) {
    // Placeholder for environmental sensor data
    // In a real implementation, this would read from BME280 or similar sensor
    temperature = 22.5f;
    humidity = 65.0f;
    pressure = 1013.25f;
}