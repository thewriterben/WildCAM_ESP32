#!/bin/bash
# ESP32WildlifeCAM Project Setup Script
# Created: 2025-08-31 17:37:10 UTC
# Author: thewriterben

echo "🚀 Setting up ESP32WildlifeCAM development environment..."

# Create project directory structure
mkdir -p ESP32WildlifeCAM/{src/{core,network,ai,utils,hardware,camera,sensors,storage,power,web,blockchain,telemetry},lib,test,data/{html,css,js},docs/{user-guide,api-reference,hardware-guide,developer-guide},tools,config}

cd ESP32WildlifeCAM

# Create .gitignore
cat > .gitignore << 'EOF'
# PlatformIO
.pio/
.pioenvs/
.piolibdeps/
.vscode/
.clang_complete
.gcc-flags.json

# Build files
build/
*.bin
*.elf

# IDE
.idea/
*.swp
*.swo
*~

# OS
.DS_Store
Thumbs.db

# Credentials and secrets
secrets.h
wifi_credentials.h
*_private.h

# Logs
*.log
debug.txt

# Temporary files
*.tmp
*.bak

# Python
__pycache__/
*.pyc
.env
venv/

# Documentation build
docs/_build/
*.pdf

# Test coverage
coverage/
*.gcov
*.gcda
*.gcno

# Local configuration
local_config.ini
platformio_override.ini
EOF

echo "✅ Created .gitignore"