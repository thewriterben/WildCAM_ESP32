#!/bin/bash

# ESP32 Wildlife CAM - Branch Creation and Management Script
# Creates and manages development branches for different development phases

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BRANCH_PREFIX="feature"
MAIN_BRANCH="main"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Help function
show_help() {
    cat << EOF
ESP32 Wildlife CAM - Branch Management Script

USAGE:
    $0 [COMMAND] [OPTIONS]

COMMANDS:
    create <branch_name>     Create new feature branch
    list                     List all project branches
    switch <branch_name>     Switch to existing branch
    merge <branch_name>      Merge branch to main
    cleanup                  Clean up merged branches
    phase <phase_number>     Create phase-specific branch
    release <version>        Create release branch

PHASE BRANCHES:
    phase 1    Foundation (hardware abstraction, GPIO, camera)
    phase 2    Power management and motion detection
    phase 3    Storage and communication systems
    phase 4    AI integration and optimization
    phase 5    Field testing and validation
    phase 6    Production preparation

OPTIONS:
    -h, --help              Show this help message
    -v, --verbose           Enable verbose output
    -f, --force             Force operation (use with caution)
    --dry-run               Show what would be done without executing

EXAMPLES:
    $0 create camera-driver-improvement
    $0 phase 2
    $0 merge feature/camera-driver-improvement
    $0 release v2.1.0
    $0 cleanup --dry-run

EOF
}

# Validate git repository
check_git_repo() {
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        log_error "Not in a git repository"
        exit 1
    fi
}

# Get current branch
get_current_branch() {
    git rev-parse --abbrev-ref HEAD
}

# Check if branch exists
branch_exists() {
    local branch_name="$1"
    git show-ref --verify --quiet "refs/heads/$branch_name"
}

# Check if remote branch exists
remote_branch_exists() {
    local branch_name="$1"
    git show-ref --verify --quiet "refs/remotes/origin/$branch_name"
}

# Create new feature branch
create_branch() {
    local branch_name="$1"
    local full_branch_name="${BRANCH_PREFIX}/${branch_name}"
    
    log_info "Creating new branch: $full_branch_name"
    
    # Ensure we're on main and up to date
    git checkout "$MAIN_BRANCH"
    git pull origin "$MAIN_BRANCH"
    
    # Create and switch to new branch
    if branch_exists "$full_branch_name"; then
        log_warning "Branch $full_branch_name already exists"
        if [[ "${FORCE:-false}" == "true" ]]; then
            log_warning "Force deleting existing branch"
            git branch -D "$full_branch_name"
        else
            log_error "Use --force to overwrite existing branch"
            return 1
        fi
    fi
    
    git checkout -b "$full_branch_name"
    
    # Create initial commit with branch marker
    cat > ".branch_info" << EOF
Branch: $full_branch_name
Created: $(date -u +"%Y-%m-%d %H:%M:%S UTC")
Creator: $(git config user.name) <$(git config user.email)>
Purpose: ESP32 Wildlife CAM development branch
EOF
    
    git add ".branch_info"
    git commit -m "Create branch: $full_branch_name

Initial branch setup for ESP32 Wildlife CAM development.
Branch created on $(date -u +"%Y-%m-%d %H:%M:%S UTC")"
    
    log_success "Branch $full_branch_name created and checked out"
    log_info "Branch info saved to .branch_info"
}

# Create phase-specific branch
create_phase_branch() {
    local phase_number="$1"
    local phase_name
    local branch_name
    
    case "$phase_number" in
        1)
            phase_name="foundation"
            branch_name="phase1-foundation-hardware"
            ;;
        2)
            phase_name="power-motion"
            branch_name="phase2-power-motion-detection"
            ;;
        3)
            phase_name="storage-comm"
            branch_name="phase3-storage-communication"
            ;;
        4)
            phase_name="ai-optimization"
            branch_name="phase4-ai-optimization"
            ;;
        5)
            phase_name="field-testing"
            branch_name="phase5-field-testing-validation"
            ;;
        6)
            phase_name="production"
            branch_name="phase6-production-preparation"
            ;;
        *)
            log_error "Invalid phase number: $phase_number"
            log_info "Valid phases: 1-6"
            return 1
            ;;
    esac
    
    log_info "Creating Phase $phase_number branch: $phase_name"
    
    # Create phase branch with specific initial files
    create_branch "$branch_name"
    
    # Create phase-specific directory structure
    mkdir -p "phase${phase_number}"
    cat > "phase${phase_number}/README.md" << EOF
# Phase $phase_number: $phase_name

## Phase Objectives
$(get_phase_objectives "$phase_number")

## Implementation Checklist
$(get_phase_checklist "$phase_number")

## Testing Requirements
$(get_phase_testing "$phase_number")

## Documentation Requirements
$(get_phase_documentation "$phase_number")
EOF
    
    git add "phase${phase_number}/"
    git commit -m "Add Phase $phase_number structure: $phase_name

Initialize development structure for Phase $phase_number of ESP32 Wildlife CAM.
Focus: $phase_name"
    
    log_success "Phase $phase_number branch created with initial structure"
}

# Get phase objectives
get_phase_objectives() {
    local phase="$1"
    case "$phase" in
        1)
            echo "- Hardware abstraction layer implementation"
            echo "- GPIO conflict resolution"
            echo "- Camera driver development"
            echo "- Basic system initialization"
            ;;
        2)
            echo "- Power management system"
            echo "- Motion detection implementation"
            echo "- Battery monitoring and solar charging"
            echo "- Low-power mode optimization"
            ;;
        3)
            echo "- Storage management system"
            echo "- Data compression and optimization"
            echo "- Communication protocols (WiFi/LoRa)"
            echo "- Remote monitoring capabilities"
            ;;
        4)
            echo "- AI species recognition integration"
            echo "- Performance optimization"
            echo "- Advanced image processing"
            echo "- Machine learning model deployment"
            ;;
        5)
            echo "- Comprehensive field testing"
            echo "- Real-world validation scenarios"
            echo "- Performance benchmarking"
            echo "- Issue identification and resolution"
            ;;
        6)
            echo "- Production firmware finalization"
            echo "- Manufacturing procedures"
            echo "- Quality assurance processes"
            echo "- Documentation completion"
            ;;
    esac
}

# Get phase checklist
get_phase_checklist() {
    local phase="$1"
    case "$phase" in
        1)
            echo "- [ ] Board detection and identification"
            echo "- [ ] GPIO pin mapping and conflict resolution"
            echo "- [ ] Camera initialization and configuration"
            echo "- [ ] Basic image capture functionality"
            echo "- [ ] Error handling and recovery"
            ;;
        2)
            echo "- [ ] Battery voltage monitoring"
            echo "- [ ] Solar panel charging control"
            echo "- [ ] PIR motion sensor integration"
            echo "- [ ] Power consumption optimization"
            echo "- [ ] Sleep mode implementation"
            ;;
        3)
            echo "- [ ] SD card storage management"
            echo "- [ ] Image compression and storage"
            echo "- [ ] WiFi connectivity (optional)"
            echo "- [ ] Data transmission protocols"
            echo "- [ ] Remote configuration capabilities"
            ;;
        4)
            echo "- [ ] Species recognition model integration"
            echo "- [ ] Image preprocessing optimization"
            echo "- [ ] Real-time inference implementation"
            echo "- [ ] Model update mechanisms"
            echo "- [ ] Performance profiling and optimization"
            ;;
        5)
            echo "- [ ] Urban deployment testing"
            echo "- [ ] Trail monitoring validation"
            echo "- [ ] Wilderness deployment testing"
            echo "- [ ] Weather stress testing"
            echo "- [ ] Long-term reliability validation"
            ;;
        6)
            echo "- [ ] Production firmware compilation"
            echo "- [ ] Manufacturing test procedures"
            echo "- [ ] Quality control validation"
            echo "- [ ] User documentation completion"
            echo "- [ ] Support and maintenance procedures"
            ;;
    esac
}

# Get phase testing requirements
get_phase_testing() {
    local phase="$1"
    case "$phase" in
        1)
            echo "- Hardware compatibility testing"
            echo "- GPIO functionality validation"
            echo "- Camera image quality assessment"
            echo "- System stability testing"
            ;;
        2)
            echo "- Power consumption measurement"
            echo "- Battery life validation"
            echo "- Motion detection accuracy testing"
            echo "- Solar charging efficiency testing"
            ;;
        3)
            echo "- Storage performance testing"
            echo "- Data integrity validation"
            echo "- Communication range testing"
            echo "- Network reliability assessment"
            ;;
        4)
            echo "- AI model accuracy validation"
            echo "- Performance benchmarking"
            echo "- Resource utilization testing"
            echo "- Edge case handling validation"
            ;;
        5)
            echo "- Multi-environment field testing"
            echo "- Long-term deployment validation"
            echo "- Weather resistance testing"
            echo "- Wildlife interaction assessment"
            ;;
        6)
            echo "- Production line testing"
            echo "- Quality assurance validation"
            echo "- Customer acceptance testing"
            echo "- Support procedure validation"
            ;;
    esac
}

# Get phase documentation requirements
get_phase_documentation() {
    local phase="$1"
    case "$phase" in
        1)
            echo "- Hardware compatibility matrix"
            echo "- GPIO pin assignment documentation"
            echo "- Camera configuration guide"
            echo "- Troubleshooting procedures"
            ;;
        2)
            echo "- Power management documentation"
            echo "- Battery life optimization guide"
            echo "- Motion detection configuration"
            echo "- Power consumption analysis"
            ;;
        3)
            echo "- Storage management procedures"
            echo "- Communication protocol documentation"
            echo "- Remote monitoring setup guide"
            echo "- Data format specifications"
            ;;
        4)
            echo "- AI model integration guide"
            echo "- Performance optimization procedures"
            echo "- Species recognition accuracy metrics"
            echo "- Model update procedures"
            ;;
        5)
            echo "- Field testing procedures"
            echo "- Deployment guidelines"
            echo "- Performance validation metrics"
            echo "- Issue resolution documentation"
            ;;
        6)
            echo "- Production procedures"
            echo "- Quality control documentation"
            echo "- User manuals and guides"
            echo "- Support and maintenance procedures"
            ;;
    esac
}

# List all branches
list_branches() {
    log_info "ESP32 Wildlife CAM Project Branches:"
    echo ""
    
    local current_branch
    current_branch=$(get_current_branch)
    
    echo "Local branches:"
    git branch | while read -r marker branch; do
        if [[ "$marker" == "*" ]]; then
            echo -e "  ${GREEN}* $branch${NC} (current)"
        else
            echo "    $branch"
        fi
    done
    
    echo ""
    echo "Remote branches:"
    if git ls-remote --heads origin > /dev/null 2>&1; then
        git branch -r | grep -v HEAD | sed 's|origin/||' | sort | while read -r branch; do
            echo "    origin/$branch"
        done
    else
        echo "    (no remote repository configured)"
    fi
    
    echo ""
    log_info "Current branch: $current_branch"
}

# Switch to branch
switch_branch() {
    local branch_name="$1"
    
    log_info "Switching to branch: $branch_name"
    
    # Check if local branch exists
    if branch_exists "$branch_name"; then
        git checkout "$branch_name"
        log_success "Switched to local branch: $branch_name"
    # Check if remote branch exists
    elif remote_branch_exists "$branch_name"; then
        git checkout -b "$branch_name" "origin/$branch_name"
        log_success "Created local branch from remote: $branch_name"
    else
        log_error "Branch not found: $branch_name"
        log_info "Available branches:"
        git branch -a | grep -E "(^\*|^\s)" | sed 's/^../  /'
        return 1
    fi
}

# Merge branch to main
merge_branch() {
    local branch_name="$1"
    local current_branch
    current_branch=$(get_current_branch)
    
    log_info "Merging branch $branch_name to $MAIN_BRANCH"
    
    # Ensure branch exists
    if ! branch_exists "$branch_name"; then
        log_error "Branch not found: $branch_name"
        return 1
    fi
    
    # Switch to main and update
    git checkout "$MAIN_BRANCH"
    git pull origin "$MAIN_BRANCH"
    
    # Merge the branch
    if git merge --no-ff "$branch_name" -m "Merge branch '$branch_name' into $MAIN_BRANCH

$(git log --oneline "$MAIN_BRANCH..$branch_name" | head -5)"; then
        log_success "Successfully merged $branch_name to $MAIN_BRANCH"
        
        # Push to remote
        if git remote | grep -q origin; then
            git push origin "$MAIN_BRANCH"
            log_success "Pushed merged changes to remote"
        fi
        
        # Ask about branch cleanup
        if [[ "${INTERACTIVE:-true}" == "true" ]]; then
            read -p "Delete merged branch $branch_name? (y/N): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                git branch -d "$branch_name"
                log_success "Deleted merged branch: $branch_name"
            fi
        fi
    else
        log_error "Merge failed - please resolve conflicts manually"
        return 1
    fi
}

# Clean up merged branches
cleanup_branches() {
    local dry_run="${DRY_RUN:-false}"
    
    log_info "Cleaning up merged branches"
    
    if [[ "$dry_run" == "true" ]]; then
        log_info "DRY RUN - showing branches that would be deleted:"
    fi
    
    # Get merged branches (excluding main and current)
    local merged_branches
    merged_branches=$(git branch --merged "$MAIN_BRANCH" | grep -v "\\*\\|$MAIN_BRANCH" | xargs -n 1)
    
    if [[ -z "$merged_branches" ]]; then
        log_info "No merged branches to clean up"
        return 0
    fi
    
    echo "Merged branches:"
    for branch in $merged_branches; do
        if [[ "$dry_run" == "true" ]]; then
            echo "  Would delete: $branch"
        else
            echo "  Deleting: $branch"
            git branch -d "$branch"
        fi
    done
    
    if [[ "$dry_run" != "true" ]]; then
        log_success "Cleanup completed"
    fi
}

# Create release branch
create_release_branch() {
    local version="$1"
    local release_branch="release/${version}"
    
    log_info "Creating release branch: $release_branch"
    
    # Ensure we're on main and up to date
    git checkout "$MAIN_BRANCH"
    git pull origin "$MAIN_BRANCH"
    
    # Create release branch
    git checkout -b "$release_branch"
    
    # Update version information
    echo "$version" > VERSION
    
    # Create release notes template
    cat > "RELEASE_NOTES_${version}.md" << EOF
# ESP32 Wildlife CAM Release $version

## New Features
- [ ] Feature 1
- [ ] Feature 2

## Bug Fixes
- [ ] Fix 1
- [ ] Fix 2

## Improvements
- [ ] Improvement 1
- [ ] Improvement 2

## Breaking Changes
- [ ] None

## Migration Guide
- [ ] No migration required

## Testing
- [ ] All tests passing
- [ ] Field testing completed
- [ ] Performance validation completed

## Documentation
- [ ] User documentation updated
- [ ] API documentation updated
- [ ] Installation guide updated
EOF
    
    git add VERSION "RELEASE_NOTES_${version}.md"
    git commit -m "Prepare release $version

- Update version to $version
- Add release notes template
- Ready for release testing and validation"
    
    log_success "Release branch $release_branch created"
    log_info "Edit RELEASE_NOTES_${version}.md to document changes"
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -f|--force)
                FORCE=true
                shift
                ;;
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            --non-interactive)
                INTERACTIVE=false
                shift
                ;;
            create)
                COMMAND="create"
                BRANCH_NAME="$2"
                shift 2
                ;;
            list)
                COMMAND="list"
                shift
                ;;
            switch)
                COMMAND="switch"
                BRANCH_NAME="$2"
                shift 2
                ;;
            merge)
                COMMAND="merge"
                BRANCH_NAME="$2"
                shift 2
                ;;
            cleanup)
                COMMAND="cleanup"
                shift
                ;;
            phase)
                COMMAND="phase"
                PHASE_NUMBER="$2"
                shift 2
                ;;
            release)
                COMMAND="release"
                RELEASE_VERSION="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Main execution
main() {
    cd "$PROJECT_ROOT"
    check_git_repo
    
    # Set defaults
    VERBOSE=${VERBOSE:-false}
    FORCE=${FORCE:-false}
    DRY_RUN=${DRY_RUN:-false}
    INTERACTIVE=${INTERACTIVE:-true}
    
    case "${COMMAND:-}" in
        create)
            if [[ -z "${BRANCH_NAME:-}" ]]; then
                log_error "Branch name required for create command"
                exit 1
            fi
            create_branch "$BRANCH_NAME"
            ;;
        list)
            list_branches
            ;;
        switch)
            if [[ -z "${BRANCH_NAME:-}" ]]; then
                log_error "Branch name required for switch command"
                exit 1
            fi
            switch_branch "$BRANCH_NAME"
            ;;
        merge)
            if [[ -z "${BRANCH_NAME:-}" ]]; then
                log_error "Branch name required for merge command"
                exit 1
            fi
            merge_branch "$BRANCH_NAME"
            ;;
        cleanup)
            cleanup_branches
            ;;
        phase)
            if [[ -z "${PHASE_NUMBER:-}" ]]; then
                log_error "Phase number required for phase command"
                exit 1
            fi
            create_phase_branch "$PHASE_NUMBER"
            ;;
        release)
            if [[ -z "${RELEASE_VERSION:-}" ]]; then
                log_error "Release version required for release command"
                exit 1
            fi
            create_release_branch "$RELEASE_VERSION"
            ;;
        *)
            log_error "No command specified"
            show_help
            exit 1
            ;;
    esac
}

# Execute if run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    parse_args "$@"
    main
fi