"""
Inference module for IMX500 wildlife detection
"""

from .imx500_wildlife_detector import IMX500WildlifeDetector, IMX500Detection
from .species_mapping import WILDLIFE_SPECIES, get_species_name, get_species_id

__all__ = [
    "IMX500WildlifeDetector",
    "IMX500Detection",
    "WILDLIFE_SPECIES",
    "get_species_name",
    "get_species_id",
]
