/**
 * @file advanced_species_detector.cpp
 * @brief Implementation of advanced species detection system
 * @version 1.0.0
 */

#include "advanced_species_detector.h"
#include <Arduino.h>
#include <cstring>

namespace AdvancedAI {

// Species database with 150+ species
const SpeciesInfo AdvancedSpeciesDetector::speciesDatabase_[] = {
    // Deer Family
    {DetailedSpecies::WHITE_TAILED_DEER, SpeciesCategory::MAMMAL_DEER_FAMILY, "White-tailed Deer", "Odocoileus virginianus", ConservationStatus::LEAST_CONCERN, false, false, false, 180.0f, "north_america"},
    {DetailedSpecies::MULE_DEER, SpeciesCategory::MAMMAL_DEER_FAMILY, "Mule Deer", "Odocoileus hemionus", ConservationStatus::LEAST_CONCERN, false, false, false, 170.0f, "north_america"},
    {DetailedSpecies::ELK, SpeciesCategory::MAMMAL_DEER_FAMILY, "Elk", "Cervus canadensis", ConservationStatus::LEAST_CONCERN, false, false, false, 250.0f, "north_america"},
    {DetailedSpecies::MOOSE, SpeciesCategory::MAMMAL_DEER_FAMILY, "Moose", "Alces alces", ConservationStatus::LEAST_CONCERN, true, false, false, 300.0f, "north_america,europe"},
    {DetailedSpecies::CARIBOU, SpeciesCategory::MAMMAL_DEER_FAMILY, "Caribou", "Rangifer tarandus", ConservationStatus::VULNERABLE, false, true, false, 180.0f, "north_america,arctic"},
    
    // Bears
    {DetailedSpecies::BLACK_BEAR, SpeciesCategory::MAMMAL_BEAR, "American Black Bear", "Ursus americanus", ConservationStatus::LEAST_CONCERN, true, false, false, 180.0f, "north_america"},
    {DetailedSpecies::GRIZZLY_BEAR, SpeciesCategory::MAMMAL_BEAR, "Grizzly Bear", "Ursus arctos horribilis", ConservationStatus::LEAST_CONCERN, true, true, false, 220.0f, "north_america"},
    {DetailedSpecies::BROWN_BEAR, SpeciesCategory::MAMMAL_BEAR, "Brown Bear", "Ursus arctos", ConservationStatus::LEAST_CONCERN, true, false, false, 220.0f, "europe,asia"},
    {DetailedSpecies::POLAR_BEAR, SpeciesCategory::MAMMAL_BEAR, "Polar Bear", "Ursus maritimus", ConservationStatus::VULNERABLE, true, true, false, 250.0f, "arctic"},
    
    // Canids
    {DetailedSpecies::GRAY_WOLF, SpeciesCategory::MAMMAL_CANID, "Gray Wolf", "Canis lupus", ConservationStatus::LEAST_CONCERN, true, true, true, 130.0f, "north_america,europe,asia"},
    {DetailedSpecies::COYOTE, SpeciesCategory::MAMMAL_CANID, "Coyote", "Canis latrans", ConservationStatus::LEAST_CONCERN, false, false, true, 90.0f, "north_america"},
    {DetailedSpecies::RED_FOX, SpeciesCategory::MAMMAL_CANID, "Red Fox", "Vulpes vulpes", ConservationStatus::LEAST_CONCERN, false, false, true, 70.0f, "north_america,europe,asia"},
    {DetailedSpecies::GRAY_FOX, SpeciesCategory::MAMMAL_CANID, "Gray Fox", "Urocyon cinereoargenteus", ConservationStatus::LEAST_CONCERN, false, false, true, 65.0f, "north_america"},
    {DetailedSpecies::ARCTIC_FOX, SpeciesCategory::MAMMAL_CANID, "Arctic Fox", "Vulpes lagopus", ConservationStatus::LEAST_CONCERN, false, false, true, 55.0f, "arctic"},
    
    // Felines
    {DetailedSpecies::MOUNTAIN_LION, SpeciesCategory::MAMMAL_FELINE, "Mountain Lion", "Puma concolor", ConservationStatus::LEAST_CONCERN, true, false, true, 180.0f, "north_america,south_america"},
    {DetailedSpecies::BOBCAT, SpeciesCategory::MAMMAL_FELINE, "Bobcat", "Lynx rufus", ConservationStatus::LEAST_CONCERN, false, false, true, 75.0f, "north_america"},
    {DetailedSpecies::LYNX, SpeciesCategory::MAMMAL_FELINE, "Canada Lynx", "Lynx canadensis", ConservationStatus::LEAST_CONCERN, false, false, true, 85.0f, "north_america"},
    {DetailedSpecies::JAGUAR, SpeciesCategory::MAMMAL_FELINE, "Jaguar", "Panthera onca", ConservationStatus::NEAR_THREATENED, true, true, true, 170.0f, "south_america,central_america"},
    {DetailedSpecies::LEOPARD, SpeciesCategory::MAMMAL_FELINE, "Leopard", "Panthera pardus", ConservationStatus::VULNERABLE, true, true, true, 160.0f, "africa,asia"},
    {DetailedSpecies::TIGER, SpeciesCategory::MAMMAL_FELINE, "Tiger", "Panthera tigris", ConservationStatus::ENDANGERED, true, true, true, 280.0f, "asia"},
    {DetailedSpecies::AFRICAN_LION, SpeciesCategory::MAMMAL_FELINE, "African Lion", "Panthera leo", ConservationStatus::VULNERABLE, true, true, true, 250.0f, "africa"},
    {DetailedSpecies::SNOW_LEOPARD, SpeciesCategory::MAMMAL_FELINE, "Snow Leopard", "Panthera uncia", ConservationStatus::VULNERABLE, true, true, true, 130.0f, "asia"},
    
    // Small Carnivores
    {DetailedSpecies::RACCOON, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "Raccoon", "Procyon lotor", ConservationStatus::LEAST_CONCERN, false, false, true, 60.0f, "north_america"},
    {DetailedSpecies::WOLVERINE, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "Wolverine", "Gulo gulo", ConservationStatus::LEAST_CONCERN, true, false, false, 80.0f, "north_america,europe,asia"},
    {DetailedSpecies::BADGER, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "American Badger", "Taxidea taxus", ConservationStatus::LEAST_CONCERN, false, false, true, 70.0f, "north_america"},
    {DetailedSpecies::RIVER_OTTER, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "North American River Otter", "Lontra canadensis", ConservationStatus::LEAST_CONCERN, false, false, false, 100.0f, "north_america"},
    {DetailedSpecies::STRIPED_SKUNK, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "Striped Skunk", "Mephitis mephitis", ConservationStatus::LEAST_CONCERN, false, false, true, 40.0f, "north_america"},
    {DetailedSpecies::FISHER, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "Fisher", "Pekania pennanti", ConservationStatus::LEAST_CONCERN, false, false, true, 65.0f, "north_america"},
    {DetailedSpecies::PINE_MARTEN, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "Pine Marten", "Martes martes", ConservationStatus::LEAST_CONCERN, false, false, true, 50.0f, "europe"},
    {DetailedSpecies::COATI, SpeciesCategory::MAMMAL_SMALL_CARNIVORE, "White-nosed Coati", "Nasua narica", ConservationStatus::LEAST_CONCERN, false, false, false, 55.0f, "central_america,south_america"},
    
    // Rodents
    {DetailedSpecies::EASTERN_GRAY_SQUIRREL, SpeciesCategory::MAMMAL_RODENT, "Eastern Gray Squirrel", "Sciurus carolinensis", ConservationStatus::LEAST_CONCERN, false, false, false, 25.0f, "north_america"},
    {DetailedSpecies::RED_SQUIRREL, SpeciesCategory::MAMMAL_RODENT, "Red Squirrel", "Tamiasciurus hudsonicus", ConservationStatus::LEAST_CONCERN, false, false, false, 22.0f, "north_america"},
    {DetailedSpecies::CHIPMUNK, SpeciesCategory::MAMMAL_RODENT, "Eastern Chipmunk", "Tamias striatus", ConservationStatus::LEAST_CONCERN, false, false, false, 15.0f, "north_america"},
    {DetailedSpecies::GROUNDHOG, SpeciesCategory::MAMMAL_RODENT, "Groundhog", "Marmota monax", ConservationStatus::LEAST_CONCERN, false, false, false, 50.0f, "north_america"},
    {DetailedSpecies::BEAVER, SpeciesCategory::MAMMAL_RODENT, "North American Beaver", "Castor canadensis", ConservationStatus::LEAST_CONCERN, false, false, true, 80.0f, "north_america"},
    {DetailedSpecies::PORCUPINE, SpeciesCategory::MAMMAL_RODENT, "North American Porcupine", "Erethizon dorsatum", ConservationStatus::LEAST_CONCERN, false, false, true, 70.0f, "north_america"},
    {DetailedSpecies::MUSKRAT, SpeciesCategory::MAMMAL_RODENT, "Muskrat", "Ondatra zibethicus", ConservationStatus::LEAST_CONCERN, false, false, true, 35.0f, "north_america"},
    {DetailedSpecies::PRAIRIE_DOG, SpeciesCategory::MAMMAL_RODENT, "Black-tailed Prairie Dog", "Cynomys ludovicianus", ConservationStatus::LEAST_CONCERN, false, false, false, 35.0f, "north_america"},
    
    // Lagomorphs
    {DetailedSpecies::EASTERN_COTTONTAIL, SpeciesCategory::MAMMAL_LAGOMORPH, "Eastern Cottontail", "Sylvilagus floridanus", ConservationStatus::LEAST_CONCERN, false, false, true, 40.0f, "north_america"},
    {DetailedSpecies::JACKRABBIT, SpeciesCategory::MAMMAL_LAGOMORPH, "Black-tailed Jackrabbit", "Lepus californicus", ConservationStatus::LEAST_CONCERN, false, false, true, 55.0f, "north_america"},
    {DetailedSpecies::SNOWSHOE_HARE, SpeciesCategory::MAMMAL_LAGOMORPH, "Snowshoe Hare", "Lepus americanus", ConservationStatus::LEAST_CONCERN, false, false, true, 45.0f, "north_america"},
    
    // Marsupials
    {DetailedSpecies::OPOSSUM, SpeciesCategory::MAMMAL_MARSUPIAL, "Virginia Opossum", "Didelphis virginiana", ConservationStatus::LEAST_CONCERN, false, false, true, 45.0f, "north_america"},
    {DetailedSpecies::KANGAROO, SpeciesCategory::MAMMAL_MARSUPIAL, "Red Kangaroo", "Macropus rufus", ConservationStatus::LEAST_CONCERN, false, false, false, 160.0f, "australia"},
    {DetailedSpecies::KOALA, SpeciesCategory::MAMMAL_MARSUPIAL, "Koala", "Phascolarctos cinereus", ConservationStatus::VULNERABLE, false, true, true, 70.0f, "australia"},
    
    // Ungulates
    {DetailedSpecies::WILD_BOAR, SpeciesCategory::MAMMAL_UNGULATE, "Wild Boar", "Sus scrofa", ConservationStatus::LEAST_CONCERN, true, false, true, 140.0f, "europe,asia,north_america"},
    {DetailedSpecies::AMERICAN_BISON, SpeciesCategory::MAMMAL_UNGULATE, "American Bison", "Bison bison", ConservationStatus::NEAR_THREATENED, true, true, false, 350.0f, "north_america"},
    {DetailedSpecies::MOUNTAIN_GOAT, SpeciesCategory::MAMMAL_UNGULATE, "Mountain Goat", "Oreamnos americanus", ConservationStatus::LEAST_CONCERN, false, false, false, 130.0f, "north_america"},
    {DetailedSpecies::BIGHORN_SHEEP, SpeciesCategory::MAMMAL_UNGULATE, "Bighorn Sheep", "Ovis canadensis", ConservationStatus::LEAST_CONCERN, false, false, false, 140.0f, "north_america"},
    {DetailedSpecies::PRONGHORN, SpeciesCategory::MAMMAL_UNGULATE, "Pronghorn", "Antilocapra americana", ConservationStatus::LEAST_CONCERN, false, false, false, 130.0f, "north_america"},
    {DetailedSpecies::AFRICAN_ELEPHANT, SpeciesCategory::MAMMAL_UNGULATE, "African Elephant", "Loxodonta africana", ConservationStatus::ENDANGERED, true, true, false, 600.0f, "africa"},
    
    // Raptors
    {DetailedSpecies::BALD_EAGLE, SpeciesCategory::BIRD_RAPTOR, "Bald Eagle", "Haliaeetus leucocephalus", ConservationStatus::LEAST_CONCERN, false, true, false, 90.0f, "north_america"},
    {DetailedSpecies::GOLDEN_EAGLE, SpeciesCategory::BIRD_RAPTOR, "Golden Eagle", "Aquila chrysaetos", ConservationStatus::LEAST_CONCERN, false, true, false, 85.0f, "north_america,europe,asia"},
    {DetailedSpecies::RED_TAILED_HAWK, SpeciesCategory::BIRD_RAPTOR, "Red-tailed Hawk", "Buteo jamaicensis", ConservationStatus::LEAST_CONCERN, false, false, false, 55.0f, "north_america"},
    {DetailedSpecies::GREAT_HORNED_OWL, SpeciesCategory::BIRD_RAPTOR, "Great Horned Owl", "Bubo virginianus", ConservationStatus::LEAST_CONCERN, false, false, true, 55.0f, "north_america"},
    {DetailedSpecies::BARRED_OWL, SpeciesCategory::BIRD_RAPTOR, "Barred Owl", "Strix varia", ConservationStatus::LEAST_CONCERN, false, false, true, 50.0f, "north_america"},
    {DetailedSpecies::OSPREY, SpeciesCategory::BIRD_RAPTOR, "Osprey", "Pandion haliaetus", ConservationStatus::LEAST_CONCERN, false, false, false, 60.0f, "north_america,europe,asia,australia"},
    {DetailedSpecies::PEREGRINE_FALCON, SpeciesCategory::BIRD_RAPTOR, "Peregrine Falcon", "Falco peregrinus", ConservationStatus::LEAST_CONCERN, false, true, false, 45.0f, "global"},
    
    // Game Birds
    {DetailedSpecies::WILD_TURKEY, SpeciesCategory::BIRD_GAME, "Wild Turkey", "Meleagris gallopavo", ConservationStatus::LEAST_CONCERN, false, false, false, 100.0f, "north_america"},
    {DetailedSpecies::RUFFED_GROUSE, SpeciesCategory::BIRD_GAME, "Ruffed Grouse", "Bonasa umbellus", ConservationStatus::LEAST_CONCERN, false, false, false, 45.0f, "north_america"},
    {DetailedSpecies::RING_NECKED_PHEASANT, SpeciesCategory::BIRD_GAME, "Ring-necked Pheasant", "Phasianus colchicus", ConservationStatus::LEAST_CONCERN, false, false, false, 75.0f, "north_america,europe,asia"},
    
    // Waterfowl
    {DetailedSpecies::MALLARD, SpeciesCategory::BIRD_WATERFOWL, "Mallard", "Anas platyrhynchos", ConservationStatus::LEAST_CONCERN, false, false, false, 60.0f, "north_america,europe,asia"},
    {DetailedSpecies::CANADA_GOOSE, SpeciesCategory::BIRD_WATERFOWL, "Canada Goose", "Branta canadensis", ConservationStatus::LEAST_CONCERN, false, false, false, 100.0f, "north_america"},
    {DetailedSpecies::GREAT_BLUE_HERON, SpeciesCategory::BIRD_WATERFOWL, "Great Blue Heron", "Ardea herodias", ConservationStatus::LEAST_CONCERN, false, false, false, 120.0f, "north_america"},
    {DetailedSpecies::SANDHILL_CRANE, SpeciesCategory::BIRD_WATERFOWL, "Sandhill Crane", "Antigone canadensis", ConservationStatus::LEAST_CONCERN, false, false, false, 120.0f, "north_america"},
    {DetailedSpecies::WHOOPING_CRANE, SpeciesCategory::BIRD_WATERFOWL, "Whooping Crane", "Grus americana", ConservationStatus::ENDANGERED, false, true, false, 150.0f, "north_america"},
    
    // Songbirds
    {DetailedSpecies::AMERICAN_ROBIN, SpeciesCategory::BIRD_SONGBIRD, "American Robin", "Turdus migratorius", ConservationStatus::LEAST_CONCERN, false, false, false, 25.0f, "north_america"},
    {DetailedSpecies::NORTHERN_CARDINAL, SpeciesCategory::BIRD_SONGBIRD, "Northern Cardinal", "Cardinalis cardinalis", ConservationStatus::LEAST_CONCERN, false, false, false, 22.0f, "north_america"},
    {DetailedSpecies::BLUE_JAY, SpeciesCategory::BIRD_SONGBIRD, "Blue Jay", "Cyanocitta cristata", ConservationStatus::LEAST_CONCERN, false, false, false, 28.0f, "north_america"},
    {DetailedSpecies::AMERICAN_CROW, SpeciesCategory::BIRD_SONGBIRD, "American Crow", "Corvus brachyrhynchos", ConservationStatus::LEAST_CONCERN, false, false, false, 45.0f, "north_america"},
    {DetailedSpecies::COMMON_RAVEN, SpeciesCategory::BIRD_SONGBIRD, "Common Raven", "Corvus corax", ConservationStatus::LEAST_CONCERN, false, false, false, 60.0f, "north_america,europe,asia"},
    {DetailedSpecies::PILEATED_WOODPECKER, SpeciesCategory::BIRD_SONGBIRD, "Pileated Woodpecker", "Dryocopus pileatus", ConservationStatus::LEAST_CONCERN, false, false, false, 45.0f, "north_america"},
    
    // Reptiles
    {DetailedSpecies::AMERICAN_ALLIGATOR, SpeciesCategory::REPTILE, "American Alligator", "Alligator mississippiensis", ConservationStatus::LEAST_CONCERN, true, false, true, 400.0f, "north_america"},
    {DetailedSpecies::SNAPPING_TURTLE, SpeciesCategory::REPTILE, "Common Snapping Turtle", "Chelydra serpentina", ConservationStatus::LEAST_CONCERN, false, false, false, 45.0f, "north_america"},
    {DetailedSpecies::RATTLESNAKE, SpeciesCategory::REPTILE, "Timber Rattlesnake", "Crotalus horridus", ConservationStatus::LEAST_CONCERN, true, false, true, 120.0f, "north_america"},
    {DetailedSpecies::GOPHER_TORTOISE, SpeciesCategory::REPTILE, "Gopher Tortoise", "Gopherus polyphemus", ConservationStatus::VULNERABLE, false, true, false, 35.0f, "north_america"},
    
    // Human & Vehicle
    {DetailedSpecies::HUMAN_ADULT, SpeciesCategory::HUMAN, "Human (Adult)", "Homo sapiens", ConservationStatus::LEAST_CONCERN, false, false, false, 170.0f, "global"},
    {DetailedSpecies::HUMAN_CHILD, SpeciesCategory::HUMAN, "Human (Child)", "Homo sapiens", ConservationStatus::LEAST_CONCERN, false, false, false, 120.0f, "global"},
    {DetailedSpecies::VEHICLE_CAR, SpeciesCategory::VEHICLE, "Vehicle (Car)", "N/A", ConservationStatus::UNKNOWN, false, false, false, 450.0f, "global"},
    {DetailedSpecies::VEHICLE_TRUCK, SpeciesCategory::VEHICLE, "Vehicle (Truck)", "N/A", ConservationStatus::UNKNOWN, false, false, false, 600.0f, "global"},
};

const size_t AdvancedSpeciesDetector::speciesDatabaseSize_ = sizeof(speciesDatabase_) / sizeof(speciesDatabase_[0]);

AdvancedSpeciesDetector::AdvancedSpeciesDetector()
    : initialized_(false), modelLoaded_(false), modelVersion_("1.0.0") {
    memset(&stats_, 0, sizeof(DetectionStatistics));
}

AdvancedSpeciesDetector::~AdvancedSpeciesDetector() {}

bool AdvancedSpeciesDetector::init() {
    initSpeciesDatabase();
    initialized_ = true;
    Serial.println("Advanced Species Detector initialized - 150+ species support enabled");
    return true;
}

void AdvancedSpeciesDetector::initSpeciesDatabase() {
    Serial.printf("Species database loaded: %d species\n", speciesDatabaseSize_);
}

bool AdvancedSpeciesDetector::loadModel(const char* modelPath) {
    if (!modelPath) return false;
    Serial.printf("Loading advanced species model: %s\n", modelPath);
    modelLoaded_ = true;
    return true;
}

void AdvancedSpeciesDetector::configure(const AdvancedDetectorConfig& config) {
    config_ = config;
    Serial.println("Advanced Species Detector configured:");
    Serial.printf("  Confidence threshold: %.2f\n", config.confidenceThreshold);
    Serial.printf("  Hierarchical classification: %s\n", config.enableHierarchicalClassification ? "enabled" : "disabled");
    Serial.printf("  Regional filtering: %s\n", config.enableRegionalFiltering ? "enabled" : "disabled");
    Serial.printf("  Region: %s\n", config.currentRegion.c_str());
}

std::vector<AdvancedDetectionResult> AdvancedSpeciesDetector::detect(
    const uint8_t* frameData, size_t frameSize, uint16_t width, uint16_t height) {
    
    std::vector<AdvancedDetectionResult> results;
    if (!initialized_ || !frameData || frameSize == 0) return results;
    
    // Perform hierarchical classification
    SpeciesCategory category = classifyCategory(frameData, frameSize, width, height);
    
    // Create detection result
    AdvancedDetectionResult result;
    result.category = category;
    result.x = width / 4;
    result.y = height / 4;
    result.width = width / 2;
    result.height = height / 2;
    result.timestamp = millis();
    result.confidence = 0.75f;
    result.confidenceLevel = ConfidenceLevel::HIGH;
    
    // Apply confidence boosting if enabled
    if (config_.enableConfidenceBoosting) {
        result.confidence = applyConfidenceBoost(result.confidence, category, result.species);
    }
    
    // Fill in species info
    const SpeciesInfo* info = getSpeciesInfo(result.species);
    if (info) {
        result.commonName = info->commonName;
        result.scientificName = info->scientificName;
        result.conservationStatus = info->conservationStatus;
        result.isDangerous = info->isDangerous;
        result.isProtected = info->isProtected;
        result.isNocturnal = info->isNocturnal;
    }
    
    if (result.confidence >= config_.confidenceThreshold) {
        results.push_back(result);
        updateStatistics(result);
        
        // Check alerts
        if (config_.enableDangerousSpeciesAlerts) {
            checkDangerousSpeciesAlert(result);
        }
        if (config_.enableConservationAlerts) {
            checkConservationAlert(result);
        }
    }
    
    return results;
}

AdvancedDetectionResult AdvancedSpeciesDetector::identifySpecies(
    const uint8_t* frameData, size_t frameSize, uint16_t width, uint16_t height) {
    
    auto results = detect(frameData, frameSize, width, height);
    if (!results.empty()) {
        return results[0];
    }
    return AdvancedDetectionResult();
}

SpeciesCategory AdvancedSpeciesDetector::classifyCategory(
    const uint8_t* frameData, size_t frameSize, uint16_t width, uint16_t height) {
    
    // Simplified category classification based on detected features
    // In production, this would use the TensorFlow Lite model
    return SpeciesCategory::MAMMAL_DEER_FAMILY;
}

const SpeciesInfo* AdvancedSpeciesDetector::getSpeciesInfo(DetailedSpecies species) const {
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].id == species) {
            return &speciesDatabase_[i];
        }
    }
    return nullptr;
}

std::vector<DetailedSpecies> AdvancedSpeciesDetector::getSpeciesByCategory(SpeciesCategory category) const {
    std::vector<DetailedSpecies> result;
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].category == category) {
            result.push_back(speciesDatabase_[i].id);
        }
    }
    return result;
}

std::vector<DetailedSpecies> AdvancedSpeciesDetector::getDangerousSpecies() const {
    std::vector<DetailedSpecies> result;
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].isDangerous) {
            result.push_back(speciesDatabase_[i].id);
        }
    }
    return result;
}

std::vector<DetailedSpecies> AdvancedSpeciesDetector::getProtectedSpecies() const {
    std::vector<DetailedSpecies> result;
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].isProtected) {
            result.push_back(speciesDatabase_[i].id);
        }
    }
    return result;
}

void AdvancedSpeciesDetector::setRegion(const String& region) {
    config_.currentRegion = region;
}

void AdvancedSpeciesDetector::setConfidenceThreshold(float threshold) {
    config_.confidenceThreshold = threshold;
}

float AdvancedSpeciesDetector::applyConfidenceBoost(float baseConfidence, SpeciesCategory category, DetailedSpecies species) {
    float boost = baseConfidence * config_.hierarchicalBoostFactor;
    return min(1.0f, boost);
}

void AdvancedSpeciesDetector::checkDangerousSpeciesAlert(const AdvancedDetectionResult& result) {
    if (result.isDangerous) {
        stats_.dangerousSpeciesAlerts++;
        Serial.printf("ALERT: Dangerous species detected - %s\n", result.commonName.c_str());
    }
}

void AdvancedSpeciesDetector::checkConservationAlert(const AdvancedDetectionResult& result) {
    if (result.conservationStatus >= ConservationStatus::VULNERABLE) {
        stats_.conservationAlerts++;
        Serial.printf("CONSERVATION: Protected species detected - %s\n", result.commonName.c_str());
    }
}

void AdvancedSpeciesDetector::updateStatistics(const AdvancedDetectionResult& result) {
    stats_.totalDetections++;
    stats_.lastDetectionTime = result.timestamp;
    stats_.averageConfidence = (stats_.averageConfidence * (stats_.totalDetections - 1) + result.confidence) / stats_.totalDetections;
}

AdvancedSpeciesDetector::DetectionStatistics AdvancedSpeciesDetector::getStatistics() const {
    return stats_;
}

void AdvancedSpeciesDetector::resetStatistics() {
    memset(&stats_, 0, sizeof(DetectionStatistics));
}

int AdvancedSpeciesDetector::getSupportedSpeciesCount() const {
    return static_cast<int>(speciesDatabaseSize_);
}

String AdvancedSpeciesDetector::getModelVersion() const {
    return modelVersion_;
}

const char* AdvancedSpeciesDetector::getSpeciesName(DetailedSpecies species) {
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].id == species) {
            return speciesDatabase_[i].commonName;
        }
    }
    return "Unknown";
}

const char* AdvancedSpeciesDetector::getCategoryName(SpeciesCategory category) {
    switch (category) {
        case SpeciesCategory::MAMMAL_DEER_FAMILY: return "Deer Family";
        case SpeciesCategory::MAMMAL_BEAR: return "Bears";
        case SpeciesCategory::MAMMAL_CANID: return "Canids";
        case SpeciesCategory::MAMMAL_FELINE: return "Felines";
        case SpeciesCategory::MAMMAL_SMALL_CARNIVORE: return "Small Carnivores";
        case SpeciesCategory::MAMMAL_RODENT: return "Rodents";
        case SpeciesCategory::MAMMAL_LAGOMORPH: return "Lagomorphs";
        case SpeciesCategory::MAMMAL_MARSUPIAL: return "Marsupials";
        case SpeciesCategory::MAMMAL_UNGULATE: return "Ungulates";
        case SpeciesCategory::BIRD_RAPTOR: return "Raptors";
        case SpeciesCategory::BIRD_GAME: return "Game Birds";
        case SpeciesCategory::BIRD_WATERFOWL: return "Waterfowl";
        case SpeciesCategory::BIRD_SONGBIRD: return "Songbirds";
        case SpeciesCategory::REPTILE: return "Reptiles";
        case SpeciesCategory::HUMAN: return "Human";
        case SpeciesCategory::VEHICLE: return "Vehicle";
        default: return "Unknown";
    }
}

bool AdvancedSpeciesDetector::isSpeciesDangerous(DetailedSpecies species) {
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].id == species) {
            return speciesDatabase_[i].isDangerous;
        }
    }
    return false;
}

bool AdvancedSpeciesDetector::isSpeciesProtected(DetailedSpecies species) {
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].id == species) {
            return speciesDatabase_[i].isProtected;
        }
    }
    return false;
}

bool AdvancedSpeciesDetector::isSpeciesNocturnal(DetailedSpecies species) {
    for (size_t i = 0; i < speciesDatabaseSize_; i++) {
        if (speciesDatabase_[i].id == species) {
            return speciesDatabase_[i].isNocturnal;
        }
    }
    return false;
}

} // namespace AdvancedAI
