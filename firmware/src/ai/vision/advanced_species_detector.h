/**
 * @file advanced_species_detector.h
 * @brief Advanced AI-powered species detection and classification system
 * @version 1.0.0
 * 
 * This module provides advanced species detection capabilities supporting 150+ wildlife species
 * with hierarchical classification, regional filtering, and confidence boosting algorithms.
 */

#ifndef ADVANCED_SPECIES_DETECTOR_H
#define ADVANCED_SPECIES_DETECTOR_H

#include "../ai_common.h"
#include <vector>
#include <map>
#include <memory>

namespace AdvancedAI {

/**
 * @brief Extended species taxonomy for 150+ species support
 */
enum class DetailedSpecies {
    // Unknown / Unclassified
    UNKNOWN = 0,
    
    // ===== MAMMALS - Deer Family (Cervidae) =====
    WHITE_TAILED_DEER,
    MULE_DEER,
    ELK,
    MOOSE,
    CARIBOU,
    RED_DEER,
    FALLOW_DEER,
    SIKA_DEER,
    AXIS_DEER,
    SAMBAR_DEER,
    
    // ===== MAMMALS - Bears (Ursidae) =====
    BLACK_BEAR,
    GRIZZLY_BEAR,
    BROWN_BEAR,
    POLAR_BEAR,
    ASIAN_BLACK_BEAR,
    SUN_BEAR,
    SPECTACLED_BEAR,
    SLOTH_BEAR,
    
    // ===== MAMMALS - Canids (Canidae) =====
    GRAY_WOLF,
    RED_WOLF,
    COYOTE,
    RED_FOX,
    GRAY_FOX,
    ARCTIC_FOX,
    KIT_FOX,
    SWIFT_FOX,
    
    // ===== MAMMALS - Felines (Felidae) =====
    MOUNTAIN_LION,
    BOBCAT,
    LYNX,
    OCELOT,
    JAGUAR,
    LEOPARD,
    CHEETAH,
    AFRICAN_LION,
    TIGER,
    SNOW_LEOPARD,
    CLOUDED_LEOPARD,
    WILD_CAT,
    SERVAL,
    CARACAL,
    
    // ===== MAMMALS - Small Carnivores =====
    RACCOON,
    FISHER,
    PINE_MARTEN,
    AMERICAN_MARTEN,
    WOLVERINE,
    BADGER,
    HONEY_BADGER,
    RIVER_OTTER,
    SEA_OTTER,
    STRIPED_SKUNK,
    SPOTTED_SKUNK,
    WEASEL,
    ERMINE,
    MINK,
    COATI,
    KINKAJOU,
    RINGTAIL,
    
    // ===== MAMMALS - Rodents =====
    EASTERN_GRAY_SQUIRREL,
    RED_SQUIRREL,
    FOX_SQUIRREL,
    FLYING_SQUIRREL,
    CHIPMUNK,
    GROUNDHOG,
    BEAVER,
    MUSKRAT,
    PORCUPINE,
    PRAIRIE_DOG,
    CAPYBARA,
    MARMOT,
    
    // ===== MAMMALS - Lagomorphs =====
    EASTERN_COTTONTAIL,
    JACKRABBIT,
    SNOWSHOE_HARE,
    EUROPEAN_RABBIT,
    PIKA,
    
    // ===== MAMMALS - Marsupials =====
    OPOSSUM,
    KANGAROO,
    WALLABY,
    KOALA,
    WOMBAT,
    TASMANIAN_DEVIL,
    
    // ===== MAMMALS - Ungulates =====
    WILD_BOAR,
    FERAL_PIG,
    PECCARY,
    AMERICAN_BISON,
    EUROPEAN_BISON,
    MOUNTAIN_GOAT,
    BIGHORN_SHEEP,
    DALL_SHEEP,
    PRONGHORN,
    AFRICAN_ELEPHANT,
    ASIAN_ELEPHANT,
    HIPPO,
    RHINO,
    TAPIR,
    ZEBRA,
    WILDEBEEST,
    
    // ===== MAMMALS - Primates =====
    CHIMPANZEE,
    GORILLA,
    ORANGUTAN,
    BABOON,
    MACAQUE,
    LEMUR,
    
    // ===== MAMMALS - Other =====
    ARMADILLO,
    PANGOLIN,
    AARDVARK,
    ANTEATER,
    SLOTH,
    
    // ===== BIRDS - Raptors =====
    BALD_EAGLE,
    GOLDEN_EAGLE,
    RED_TAILED_HAWK,
    COOPERS_HAWK,
    SHARP_SHINNED_HAWK,
    OSPREY,
    PEREGRINE_FALCON,
    AMERICAN_KESTREL,
    GREAT_HORNED_OWL,
    BARRED_OWL,
    BARN_OWL,
    SNOWY_OWL,
    SCREECH_OWL,
    VULTURE,
    CONDOR,
    
    // ===== BIRDS - Game Birds =====
    WILD_TURKEY,
    RUFFED_GROUSE,
    SHARP_TAILED_GROUSE,
    PRAIRIE_CHICKEN,
    RING_NECKED_PHEASANT,
    NORTHERN_BOBWHITE,
    CALIFORNIA_QUAIL,
    GAMBELS_QUAIL,
    CHUKAR,
    PTARMIGAN,
    
    // ===== BIRDS - Waterfowl =====
    MALLARD,
    WOOD_DUCK,
    CANADA_GOOSE,
    SNOW_GOOSE,
    TRUMPETER_SWAN,
    MUTE_SWAN,
    GREAT_BLUE_HERON,
    GREAT_EGRET,
    SANDHILL_CRANE,
    WHOOPING_CRANE,
    
    // ===== BIRDS - Songbirds =====
    AMERICAN_ROBIN,
    NORTHERN_CARDINAL,
    BLUE_JAY,
    AMERICAN_CROW,
    COMMON_RAVEN,
    BLACK_CAPPED_CHICKADEE,
    WHITE_BREASTED_NUTHATCH,
    DOWNY_WOODPECKER,
    PILEATED_WOODPECKER,
    RED_BELLIED_WOODPECKER,
    
    // ===== REPTILES =====
    AMERICAN_ALLIGATOR,
    SALTWATER_CROCODILE,
    SNAPPING_TURTLE,
    BOX_TURTLE,
    GOPHER_TORTOISE,
    RATTLESNAKE,
    COPPERHEAD,
    WATER_MOCCASIN,
    CORN_SNAKE,
    GARTER_SNAKE,
    KING_SNAKE,
    IGUANA,
    MONITOR_LIZARD,
    GILA_MONSTER,
    
    // ===== AMPHIBIANS =====
    BULLFROG,
    GREEN_FROG,
    TREE_FROG,
    SALAMANDER,
    NEWT,
    
    // ===== HUMAN & VEHICLE =====
    HUMAN_ADULT,
    HUMAN_CHILD,
    VEHICLE_CAR,
    VEHICLE_TRUCK,
    VEHICLE_ATV,
    
    // End marker for iteration
    SPECIES_COUNT
};

/**
 * @brief Species category for hierarchical classification
 */
enum class SpeciesCategory {
    UNKNOWN = 0,
    MAMMAL_DEER_FAMILY,
    MAMMAL_BEAR,
    MAMMAL_CANID,
    MAMMAL_FELINE,
    MAMMAL_SMALL_CARNIVORE,
    MAMMAL_RODENT,
    MAMMAL_LAGOMORPH,
    MAMMAL_MARSUPIAL,
    MAMMAL_UNGULATE,
    MAMMAL_PRIMATE,
    MAMMAL_OTHER,
    BIRD_RAPTOR,
    BIRD_GAME,
    BIRD_WATERFOWL,
    BIRD_SONGBIRD,
    REPTILE,
    AMPHIBIAN,
    HUMAN,
    VEHICLE
};

/**
 * @brief Conservation status for species
 */
enum class ConservationStatus {
    UNKNOWN = 0,
    LEAST_CONCERN,
    NEAR_THREATENED,
    VULNERABLE,
    ENDANGERED,
    CRITICALLY_ENDANGERED,
    EXTINCT_IN_WILD
};

/**
 * @brief Detailed detection result with species information
 */
struct AdvancedDetectionResult {
    DetailedSpecies species;
    SpeciesCategory category;
    String commonName;
    String scientificName;
    float confidence;
    ConfidenceLevel confidenceLevel;
    ConservationStatus conservationStatus;
    
    // Bounding box
    uint16_t x, y, width, height;
    
    // Additional detection info
    bool isDangerous;
    bool isProtected;
    bool isNocturnal;
    uint8_t estimatedCount;
    float sizeEstimate;
    
    // Behavior hints
    String behaviorHint;
    
    // Timestamp
    unsigned long timestamp;
    
    AdvancedDetectionResult() :
        species(DetailedSpecies::UNKNOWN),
        category(SpeciesCategory::UNKNOWN),
        confidence(0.0f),
        confidenceLevel(ConfidenceLevel::VERY_LOW),
        conservationStatus(ConservationStatus::UNKNOWN),
        x(0), y(0), width(0), height(0),
        isDangerous(false), isProtected(false), isNocturnal(false),
        estimatedCount(1), sizeEstimate(0.0f),
        timestamp(0) {}
};

/**
 * @brief Species database entry
 */
struct SpeciesInfo {
    DetailedSpecies id;
    SpeciesCategory category;
    const char* commonName;
    const char* scientificName;
    ConservationStatus conservationStatus;
    bool isDangerous;
    bool isProtected;
    bool isNocturnal;
    float typicalSizeCm;
    const char* habitatRegions;  // Comma-separated list
};

/**
 * @brief Configuration for advanced species detection
 */
struct AdvancedDetectorConfig {
    float confidenceThreshold = 0.6f;
    float hierarchicalBoostFactor = 1.15f;
    bool enableHierarchicalClassification = true;
    bool enableRegionalFiltering = true;
    bool enableConservationAlerts = true;
    bool enableDangerousSpeciesAlerts = true;
    bool enableConfidenceBoosting = true;
    uint8_t maxDetectionsPerFrame = 10;
    uint32_t processingTimeoutMs = 3000;
    String currentRegion = "north_america";
};

/**
 * @brief Advanced Species Detector class
 * 
 * Provides comprehensive wildlife species detection with:
 * - 150+ species support
 * - Hierarchical classification
 * - Regional filtering
 * - Conservation status tracking
 * - Dangerous species alerts
 */
class AdvancedSpeciesDetector {
public:
    AdvancedSpeciesDetector();
    ~AdvancedSpeciesDetector();
    
    // Initialization
    bool init();
    bool loadModel(const char* modelPath);
    void configure(const AdvancedDetectorConfig& config);
    
    // Core detection
    std::vector<AdvancedDetectionResult> detect(const uint8_t* frameData, 
                                                 size_t frameSize,
                                                 uint16_t width,
                                                 uint16_t height);
    
    // Single species identification
    AdvancedDetectionResult identifySpecies(const uint8_t* frameData,
                                            size_t frameSize,
                                            uint16_t width,
                                            uint16_t height);
    
    // Hierarchical classification
    SpeciesCategory classifyCategory(const uint8_t* frameData,
                                      size_t frameSize,
                                      uint16_t width,
                                      uint16_t height);
    
    // Species database queries
    const SpeciesInfo* getSpeciesInfo(DetailedSpecies species) const;
    std::vector<DetailedSpecies> getSpeciesByCategory(SpeciesCategory category) const;
    std::vector<DetailedSpecies> getSpeciesByRegion(const String& region) const;
    std::vector<DetailedSpecies> getDangerousSpecies() const;
    std::vector<DetailedSpecies> getProtectedSpecies() const;
    
    // Configuration
    void setRegion(const String& region);
    void setConfidenceThreshold(float threshold);
    void enableHierarchicalClassification(bool enable);
    void enableRegionalFiltering(bool enable);
    void enableConservationAlerts(bool enable);
    
    // Statistics
    struct DetectionStatistics {
        uint32_t totalDetections;
        uint32_t speciesDetections[static_cast<int>(DetailedSpecies::SPECIES_COUNT)];
        uint32_t categoryDetections[20];  // SpeciesCategory count
        float averageConfidence;
        uint32_t dangerousSpeciesAlerts;
        uint32_t conservationAlerts;
        unsigned long lastDetectionTime;
    };
    
    DetectionStatistics getStatistics() const;
    void resetStatistics();
    
    // Utility
    static const char* getSpeciesName(DetailedSpecies species);
    static const char* getCategoryName(SpeciesCategory category);
    static const char* getConservationStatusName(ConservationStatus status);
    static bool isSpeciesDangerous(DetailedSpecies species);
    static bool isSpeciesProtected(DetailedSpecies species);
    static bool isSpeciesNocturnal(DetailedSpecies species);
    
    // Model information
    int getSupportedSpeciesCount() const;
    std::vector<String> getSupportedSpeciesList() const;
    String getModelVersion() const;
    
    // Status
    bool isInitialized() const { return initialized_; }
    bool isModelLoaded() const { return modelLoaded_; }

private:
    bool initialized_;
    bool modelLoaded_;
    AdvancedDetectorConfig config_;
    DetectionStatistics stats_;
    String modelVersion_;
    
    // Internal species database
    void initSpeciesDatabase();
    static const SpeciesInfo speciesDatabase_[];
    static const size_t speciesDatabaseSize_;
    
    // Classification helpers
    SpeciesCategory classifyCategoryFromFeatures(const float* features);
    DetailedSpecies classifySpeciesInCategory(const float* features, SpeciesCategory category);
    float applyConfidenceBoost(float baseConfidence, SpeciesCategory category, DetailedSpecies species);
    
    // Regional filtering
    bool isSpeciesInRegion(DetailedSpecies species, const String& region) const;
    float getRegionalPrior(DetailedSpecies species, const String& region) const;
    
    // Alert checking
    void checkDangerousSpeciesAlert(const AdvancedDetectionResult& result);
    void checkConservationAlert(const AdvancedDetectionResult& result);
    
    // Statistics update
    void updateStatistics(const AdvancedDetectionResult& result);
};

} // namespace AdvancedAI

#endif // ADVANCED_SPECIES_DETECTOR_H
