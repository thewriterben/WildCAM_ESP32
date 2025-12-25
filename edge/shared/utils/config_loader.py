"""
Configuration loader utilities
"""

import yaml
from pathlib import Path
from typing import Dict, Any
import os


def load_config(config_path: str, environment: str = "production") -> Dict[str, Any]:
    """
    Load YAML configuration file
    
    Args:
        config_path: Path to config directory or file
        environment: Environment name (production, development, etc.)
        
    Returns:
        Configuration dictionary
    """
    # If path is a directory, look for environment-specific file
    path = Path(config_path)
    if path.is_dir():
        config_file = path / f"{environment}.yaml"
        if not config_file.exists():
            # Fallback to config.yaml
            config_file = path / "config.yaml"
    else:
        config_file = path
        
    if not config_file.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_file}")
        
    with open(config_file, 'r') as f:
        config = yaml.safe_load(f)
        
    # Environment variable substitution
    config = _substitute_env_vars(config)
    
    return config


def _substitute_env_vars(config: Any) -> Any:
    """
    Recursively substitute environment variables in config
    
    Args:
        config: Configuration value (dict, list, str, etc.)
        
    Returns:
        Configuration with environment variables substituted
    """
    if isinstance(config, dict):
        return {k: _substitute_env_vars(v) for k, v in config.items()}
    elif isinstance(config, list):
        return [_substitute_env_vars(item) for item in config]
    elif isinstance(config, str):
        # Support ${VAR_NAME} or ${VAR_NAME:default} syntax
        if config.startswith('${') and config.endswith('}'):
            var_spec = config[2:-1]
            if ':' in var_spec:
                var_name, default = var_spec.split(':', 1)
                return os.getenv(var_name, default)
            else:
                return os.getenv(var_spec, config)
        return config
    else:
        return config
