"""
Wildlife Species Mapping for IMX500 Detector
Maps species names to IDs for compact LoRa transmission
"""

from typing import Optional

# Wildlife species mapping (species_name -> species_id)
WILDLIFE_SPECIES = {
    # Common mammals
    "deer": 0,
    "elk": 1,
    "moose": 2,
    "bear": 3,
    "wolf": 4,
    "coyote": 5,
    "fox": 6,
    "rabbit": 7,
    "raccoon": 8,
    "bobcat": 9,
    
    # Large predators and ungulates
    "mountain_lion": 10,
    "wild_boar": 11,
    
    # Birds
    "turkey": 12,
    "eagle": 13,
    "owl": 14,
    "hawk": 15,
    
    # Small mammals
    "squirrel": 16,
    "skunk": 17,
    "opossum": 18,
    "beaver": 19,
    
    # Additional species (can be extended)
    "porcupine": 20,
    "badger": 21,
    "marmot": 22,
    "lynx": 23,
    "wolverine": 24,
    "otter": 25,
    "mink": 26,
    "fisher": 27,
    "marten": 28,
    "weasel": 29,
    
    # Reserved for special cases
    "human": 250,
    "vehicle": 251,
    "domestic_animal": 252,
    "bird_other": 253,
    "unknown": 255,
}

# Reverse mapping (species_id -> species_name)
SPECIES_ID_TO_NAME = {v: k for k, v in WILDLIFE_SPECIES.items()}


def get_species_id(species_name: str) -> int:
    """
    Get species ID from species name
    
    Args:
        species_name: Name of the species (e.g., "deer", "bear")
        
    Returns:
        Species ID (0-255), returns 255 (unknown) if not found
    """
    return WILDLIFE_SPECIES.get(species_name.lower(), 255)


def get_species_name(species_id: int) -> Optional[str]:
    """
    Get species name from species ID
    
    Args:
        species_id: Species ID (0-255)
        
    Returns:
        Species name, or None if not found
    """
    return SPECIES_ID_TO_NAME.get(species_id)


def is_valid_species(species_name: str) -> bool:
    """
    Check if a species name is valid
    
    Args:
        species_name: Name of the species
        
    Returns:
        True if species is in the mapping, False otherwise
    """
    return species_name.lower() in WILDLIFE_SPECIES


def get_all_species_names():
    """
    Get list of all valid species names
    
    Returns:
        List of species names
    """
    return sorted([name for name in WILDLIFE_SPECIES.keys() if name not in ["unknown", "human", "vehicle", "domestic_animal"]])


def get_priority_species():
    """
    Get list of priority species for conservation monitoring
    
    Returns:
        List of priority species names
    """
    priority = [
        "bear", "wolf", "mountain_lion", "elk", "moose",
        "bobcat", "lynx", "wolverine", "eagle", "owl"
    ]
    return [s for s in priority if s in WILDLIFE_SPECIES]
