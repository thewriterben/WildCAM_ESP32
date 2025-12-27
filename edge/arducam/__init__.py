"""
Arducam Pivistation 5 (IMX500) Edge AI Integration

This module provides integration for the Arducam Pivistation 5 camera with
Sony IMX500 smart sensor for real-time wildlife monitoring at the edge.
"""

__version__ = "1.0.0"
__author__ = "WildCAM Team"

from .imx500_wildlife_detector import IMX500WildlifeDetector
from .config import IMX500Config

__all__ = [
    "IMX500WildlifeDetector",
    "IMX500Config",
]
