"""
Arducam Pivistation 5 (Sony IMX500) Integration
Wildlife monitoring with on-sensor AI inference
"""

__version__ = "1.0.0"
__author__ = "WildCAM Project"

from .inference.imx500_wildlife_detector import IMX500WildlifeDetector, IMX500Detection
from .inference.species_mapping import WILDLIFE_SPECIES, get_species_name, get_species_id

__all__ = [
    "IMX500WildlifeDetector",
    "IMX500Detection",
    "WILDLIFE_SPECIES",
    "get_species_name",
    "get_species_id",
]
