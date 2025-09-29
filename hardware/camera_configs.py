"""
Configuration for various camera hardware options
"""

CAMERA_MODULES = {
    "esp32_cam_configurations": {
        "OV2640": {
            "resolution": "1600x1200",
            "fps": 15,
            "night_vision": False,
            "price": "$10-15",
            "power_consumption": "310mA active"
        },
        "OV5640": {
            "resolution": "2592x1944",
            "fps": 15,
            "night_vision": False,
            "price": "$25-30",
            "power_consumption": "420mA active",
            "autofocus": True
        },
        "OV7725": {
            "resolution": "640x480",
            "fps": 60,
            "night_vision": True,
            "price": "$15-20",
            "power_consumption": "250mA active",
            "global_shutter": True
        }
    },
    
    "specialized_wildlife_cameras": {
        "FLIR_Lepton_3.5": {
            "type": "Thermal",
            "resolution": "160x120",
            "detection_range": "100m",
            "price": "$200-250",
            "use_case": "Night detection, heat signatures"
        },
        "OpenMV_Cam_H7_Plus": {
            "processor": "STM32H743",
            "resolution": "5MP",
            "onboard_ml": True,
            "price": "$65-85",
            "tensorflow_lite": True
        },
        "Luxonis_OAK-D-Lite": {
            "type": "Stereo depth + RGB",
            "ai_accelerator": "Intel Movidius Myriad X",
            "depth_perception": True,
            "price": "$149",
            "fps": "30 @ 1080p"
        },
        "NVIDIA_Jetson_Nano": {
            "type": "AI Computing Platform",
            "gpu": "128-core Maxwell",
            "power": "5W-10W",
            "price": "$99",
            "use_case": "Edge AI processing hub"
        }
    }
}

# Power Solutions
POWER_SYSTEMS = {
    "solar_panels": {
        "6W_Panel": {
            "voltage": "6V",
            "current": "1A max",
            "size": "190x170mm",
            "weatherproof": "IP65"
        },
        "20W_Panel": {
            "voltage": "18V",
            "current": "1.11A max",
            "size": "350x350mm",
            "weatherproof": "IP67"
        }
    },
    
    "battery_systems": {
        "18650_Pack": {
            "capacity": "12000mAh",
            "voltage": "3.7V nominal",
            "cells": "4x 18650",
            "protection": "BMS integrated"
        },
        "LiFePO4": {
            "capacity": "20Ah",
            "voltage": "12.8V",
            "cycles": "2000+",
            "temperature_range": "-20°C to 60°C"
        }
    }
}