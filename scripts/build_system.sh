#!/usr/bin/env bash
# Build orchestration script for the WildCAM ESP32 platform.
# It validates toolchain prerequisites and builds the firmware,
# backend services, and frontend dashboard in one pass.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FIRMWARE_DIR="$ROOT_DIR/firmware"
BACKEND_DIR="$ROOT_DIR/backend"
FRONTEND_DIR="$ROOT_DIR/frontend/dashboard"

SKIP_FIRMWARE=0
SKIP_BACKEND=0
SKIP_FRONTEND=0
INSTALL_ONLY=0

usage() {
  cat <<USAGE
Usage: $(basename "$0") [options]

Options:
  --skip-firmware       Skip the PlatformIO firmware build
  --skip-backend        Skip backend dependency install and checks
  --skip-frontend       Skip frontend dashboard build
  --install-only        Install dependencies without running build steps
  -h, --help            Show this help message and exit
USAGE
}

log_step() { printf '\n\033[1;34m▶ %s\033[0m\n' "$1"; }
log_info() { printf '\033[0;32m✓ %s\033[0m\n' "$1"; }
log_warn() { printf '\033[0;33m⚠ %s\033[0m\n' "$1"; }
log_error() { printf '\033[0;31m✗ %s\033[0m\n' "$1"; }

require_command() {
  local cmd="$1"
  local name="${2:-$1}"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    log_error "Missing required command: $name ($cmd)"
    return 1
  fi
  return 0
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-firmware) SKIP_FIRMWARE=1 ;;
    --skip-backend) SKIP_BACKEND=1 ;;
    --skip-frontend) SKIP_FRONTEND=1 ;;
    --install-only) INSTALL_ONLY=1 ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      log_error "Unknown option: $1"
      usage
      exit 1
      ;;
  esac
  shift
done

log_step "WildCAM ESP32 unified build"
log_info "Project root: $ROOT_DIR"

if [[ $SKIP_FIRMWARE -eq 0 ]]; then
  log_step "Checking firmware toolchain"
  if require_command platformio "PlatformIO"; then
    if [[ $INSTALL_ONLY -eq 0 ]]; then
      log_step "Building ESP32 firmware (PlatformIO)"
      (cd "$FIRMWARE_DIR" && platformio run)
      log_info "Firmware build completed"
    else
      log_info "PlatformIO detected. Install-only flag prevents build execution."
    fi
  else
    log_warn "PlatformIO not found. Skipping firmware build."
  fi
else
  log_info "Firmware build skipped by user flag"
fi

if [[ $SKIP_BACKEND -eq 0 ]]; then
  log_step "Preparing backend environment"
  if require_command python3 "Python 3" && require_command pip3 "pip"; then
    VENV_DIR="$BACKEND_DIR/.venv-build"
    if [[ ! -d "$VENV_DIR" ]]; then
      log_step "Creating backend virtual environment"
      python3 -m venv "$VENV_DIR"
    fi
    # shellcheck disable=SC1090
    source "$VENV_DIR/bin/activate"
    pip install --upgrade pip >/dev/null
    pip install -r "$BACKEND_DIR/requirements.txt"
    if [[ $INSTALL_ONLY -eq 0 ]]; then
      log_step "Performing backend syntax check"
      python -m compileall "$BACKEND_DIR"
      log_info "Backend compile check completed"
    else
      log_info "Dependencies installed; compile step skipped by flag"
    fi
    deactivate
  else
    log_warn "Python 3 toolchain missing. Backend build skipped."
  fi
else
  log_info "Backend build skipped by user flag"
fi

if [[ $SKIP_FRONTEND -eq 0 ]]; then
  log_step "Preparing frontend dashboard"
  if require_command node "Node.js" && require_command npm "npm"; then
    (cd "$FRONTEND_DIR" && npm install)
    if [[ $INSTALL_ONLY -eq 0 ]]; then
      log_step "Building production dashboard bundle"
      (cd "$FRONTEND_DIR" && npm run build)
      log_info "Frontend build completed"
    else
      log_info "Dependencies installed; build step skipped by flag"
    fi
  else
    log_warn "Node.js/npm missing. Frontend build skipped."
  fi
else
  log_info "Frontend build skipped by user flag"
fi

log_step "Build orchestration finished"
