#!/bin/bash
# Prerequisites Check Script for Multi-Cloud Deployment
# Verifies all required tools and credentials are configured

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
CHECKS_PASSED=0
CHECKS_FAILED=0
CHECKS_WARNED=0

# Helper functions
check_passed() {
    echo -e "${GREEN}✓${NC} $1"
    ((CHECKS_PASSED++))
}

check_failed() {
    echo -e "${RED}✗${NC} $1"
    ((CHECKS_FAILED++))
}

check_warned() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((CHECKS_WARNED++))
}

check_version() {
    local tool=$1
    local required_version=$2
    local current_version=$3
    
    if [ -z "$current_version" ]; then
        check_failed "$tool not found"
        return 1
    fi
    
    check_passed "$tool $current_version (required: $required_version)"
    return 0
}

echo "================================================"
echo "Multi-Cloud Deployment Prerequisites Check"
echo "================================================"
echo ""

# Check Terraform
echo "Checking Terraform..."
if command -v terraform &> /dev/null; then
    TERRAFORM_VERSION=$(terraform version -json | grep -oP '"terraform_version":"\K[^"]+' 2>/dev/null || terraform version | head -n1 | cut -d'v' -f2)
    check_version "Terraform" ">= 1.5.0" "$TERRAFORM_VERSION"
else
    check_failed "Terraform not found. Install from: https://www.terraform.io/downloads"
fi

# Check kubectl
echo "Checking kubectl..."
if command -v kubectl &> /dev/null; then
    KUBECTL_VERSION=$(kubectl version --client -o json 2>/dev/null | grep -oP '"gitVersion":"v\K[^"]+' || kubectl version --client --short 2>/dev/null | cut -d'v' -f2)
    check_version "kubectl" ">= 1.28.0" "$KUBECTL_VERSION"
else
    check_failed "kubectl not found. Install from: https://kubernetes.io/docs/tasks/tools/"
fi

# Check Helm
echo "Checking Helm..."
if command -v helm &> /dev/null; then
    HELM_VERSION=$(helm version --short | cut -d'v' -f2 | cut -d'+' -f1)
    check_version "Helm" ">= 3.12.0" "$HELM_VERSION"
else
    check_failed "Helm not found. Install from: https://helm.sh/docs/intro/install/"
fi

# Check AWS CLI
echo "Checking AWS CLI..."
if command -v aws &> /dev/null; then
    AWS_VERSION=$(aws --version 2>&1 | cut -d' ' -f1 | cut -d'/' -f2)
    check_version "AWS CLI" ">= 2.0" "$AWS_VERSION"
    
    # Check AWS credentials
    if aws sts get-caller-identity &> /dev/null; then
        AWS_ACCOUNT=$(aws sts get-caller-identity --query Account --output text)
        check_passed "AWS credentials configured (Account: $AWS_ACCOUNT)"
    else
        check_warned "AWS credentials not configured. Run: aws configure"
    fi
else
    check_failed "AWS CLI not found. Install from: https://aws.amazon.com/cli/"
fi

# Check Azure CLI
echo "Checking Azure CLI..."
if command -v az &> /dev/null; then
    AZURE_VERSION=$(az version --output json 2>/dev/null | grep -oP '"azure-cli": "\K[^"]+' || az version | grep "azure-cli" | awk '{print $2}')
    check_version "Azure CLI" ">= 2.0" "$AZURE_VERSION"
    
    # Check Azure credentials
    if az account show &> /dev/null; then
        AZURE_SUBSCRIPTION=$(az account show --query name --output tsv)
        check_passed "Azure credentials configured (Subscription: $AZURE_SUBSCRIPTION)"
    else
        check_warned "Azure credentials not configured. Run: az login"
    fi
else
    check_failed "Azure CLI not found. Install from: https://docs.microsoft.com/en-us/cli/azure/install-azure-cli"
fi

# Check Google Cloud SDK
echo "Checking Google Cloud SDK..."
if command -v gcloud &> /dev/null; then
    GCLOUD_VERSION=$(gcloud version --format="value(version)" 2>/dev/null | head -n1)
    check_version "gcloud" ">= 400.0.0" "$GCLOUD_VERSION"
    
    # Check GCP credentials
    if gcloud auth list --filter=status:ACTIVE --format="value(account)" &> /dev/null; then
        GCP_ACCOUNT=$(gcloud auth list --filter=status:ACTIVE --format="value(account)" | head -n1)
        GCP_PROJECT=$(gcloud config get-value project 2>/dev/null)
        if [ -n "$GCP_ACCOUNT" ]; then
            check_passed "GCP credentials configured (Account: $GCP_ACCOUNT, Project: ${GCP_PROJECT:-not set})"
        else
            check_warned "GCP credentials not configured. Run: gcloud auth login"
        fi
    else
        check_warned "GCP credentials not configured. Run: gcloud auth login"
    fi
else
    check_failed "gcloud SDK not found. Install from: https://cloud.google.com/sdk/docs/install"
fi

# Check Docker
echo "Checking Docker..."
if command -v docker &> /dev/null; then
    DOCKER_VERSION=$(docker version --format '{{.Server.Version}}' 2>/dev/null || docker --version | cut -d' ' -f3 | tr -d ',')
    check_version "Docker" ">= 20.10" "$DOCKER_VERSION"
    
    # Check Docker daemon
    if docker ps &> /dev/null; then
        check_passed "Docker daemon is running"
    else
        check_warned "Docker daemon is not running. Start Docker."
    fi
else
    check_warned "Docker not found (optional, but recommended)"
fi

# Check Git
echo "Checking Git..."
if command -v git &> /dev/null; then
    GIT_VERSION=$(git --version | cut -d' ' -f3)
    check_version "Git" ">= 2.0" "$GIT_VERSION"
else
    check_failed "Git not found. Install from: https://git-scm.com/downloads"
fi

# Check jq (useful for JSON parsing)
echo "Checking jq..."
if command -v jq &> /dev/null; then
    JQ_VERSION=$(jq --version | cut -d'-' -f2)
    check_version "jq" ">= 1.6" "$JQ_VERSION"
else
    check_warned "jq not found (optional, but recommended for JSON parsing)"
fi

# Check disk space
echo "Checking disk space..."
AVAILABLE_SPACE=$(df -BG . | awk 'NR==2 {print $4}' | sed 's/G//')
if [ "$AVAILABLE_SPACE" -gt 10 ]; then
    check_passed "Disk space: ${AVAILABLE_SPACE}GB available"
else
    check_warned "Low disk space: ${AVAILABLE_SPACE}GB available (10GB recommended)"
fi

# Check internet connectivity
echo "Checking internet connectivity..."
if ping -c 1 8.8.8.8 &> /dev/null; then
    check_passed "Internet connectivity: OK"
else
    check_failed "No internet connectivity detected"
fi

# Summary
echo ""
echo "================================================"
echo "Prerequisites Check Summary"
echo "================================================"
echo -e "${GREEN}Passed:${NC} $CHECKS_PASSED"
echo -e "${YELLOW}Warnings:${NC} $CHECKS_WARNED"
echo -e "${RED}Failed:${NC} $CHECKS_FAILED"
echo ""

if [ $CHECKS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All critical prerequisites met!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. cd infrastructure/terraform"
    echo "2. cp environments/production/terraform.tfvars.example terraform.tfvars"
    echo "3. Edit terraform.tfvars with your configuration"
    echo "4. terraform init"
    echo "5. terraform plan"
    echo "6. terraform apply"
    echo ""
    echo "For detailed instructions, see: MULTI_CLOUD_QUICKSTART.md"
    exit 0
else
    echo -e "${RED}✗ Some critical prerequisites are missing${NC}"
    echo ""
    echo "Please install the missing tools and configure credentials before proceeding."
    echo "See MULTI_CLOUD_QUICKSTART.md for installation instructions."
    exit 1
fi
