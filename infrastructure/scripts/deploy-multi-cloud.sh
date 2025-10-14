#!/bin/bash
# Multi-Cloud Deployment Script for WildCAM ESP32

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="${PROJECT_NAME:-wildcam}"
ENVIRONMENT="${ENVIRONMENT:-production}"
TERRAFORM_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../terraform" && pwd)"

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_prerequisites() {
    log_info "Checking prerequisites..."
    
    local missing_tools=()
    
    command -v terraform >/dev/null 2>&1 || missing_tools+=("terraform")
    command -v kubectl >/dev/null 2>&1 || missing_tools+=("kubectl")
    command -v aws >/dev/null 2>&1 || missing_tools+=("aws")
    command -v az >/dev/null 2>&1 || missing_tools+=("az")
    command -v gcloud >/dev/null 2>&1 || missing_tools+=("gcloud")
    command -v helm >/dev/null 2>&1 || missing_tools+=("helm")
    
    if [ ${#missing_tools[@]} -ne 0 ]; then
        log_error "Missing required tools: ${missing_tools[*]}"
        log_info "Please install missing tools and try again"
        exit 1
    fi
    
    log_info "All prerequisites satisfied"
}

verify_cloud_credentials() {
    log_info "Verifying cloud credentials..."
    
    # Check AWS
    if ! aws sts get-caller-identity >/dev/null 2>&1; then
        log_error "AWS credentials not configured"
        exit 1
    fi
    log_info "✓ AWS credentials valid"
    
    # Check Azure
    if ! az account show >/dev/null 2>&1; then
        log_error "Azure credentials not configured"
        exit 1
    fi
    log_info "✓ Azure credentials valid"
    
    # Check GCP
    if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q .; then
        log_error "GCP credentials not configured"
        exit 1
    fi
    log_info "✓ GCP credentials valid"
}

initialize_terraform() {
    log_info "Initializing Terraform..."
    cd "$TERRAFORM_DIR"
    
    terraform init -upgrade
    
    log_info "Terraform initialized successfully"
}

plan_infrastructure() {
    log_info "Planning infrastructure changes..."
    cd "$TERRAFORM_DIR"
    
    terraform plan \
        -var="project_name=$PROJECT_NAME" \
        -var="environment=$ENVIRONMENT" \
        -out=tfplan
    
    log_info "Review the plan above"
    read -p "Do you want to continue? (yes/no): " -r
    if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
        log_warn "Deployment cancelled by user"
        exit 0
    fi
}

apply_infrastructure() {
    log_info "Applying infrastructure changes..."
    cd "$TERRAFORM_DIR"
    
    terraform apply tfplan
    
    log_info "Infrastructure deployed successfully"
}

print_access_info() {
    log_info "====================================="
    log_info "Multi-Cloud Deployment Complete!"
    log_info "====================================="
    echo ""
    log_info "Next Steps:"
    log_info "1. Configure kubectl contexts for each cluster"
    log_info "2. Install ArgoCD for GitOps deployment"
    log_info "3. Configure DNS records"
    log_info "4. Set up SSL certificates"
    log_info "5. Configure monitoring alerts"
}

main() {
    log_info "Starting WildCAM Multi-Cloud Deployment"
    log_info "Project: $PROJECT_NAME"
    log_info "Environment: $ENVIRONMENT"
    echo ""
    
    check_prerequisites
    verify_cloud_credentials
    initialize_terraform
    plan_infrastructure
    apply_infrastructure
    print_access_info
    
    log_info "Deployment completed successfully!"
}

# Run main function
main "$@"
