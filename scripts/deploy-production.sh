#!/bin/bash

# ESP32 Wildlife Camera Production Deployment Script
# This script automates the production deployment process

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENVIRONMENT="${1:-production}"
DEPLOY_TYPE="${2:-docker-compose}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking prerequisites..."
    
    # Check Docker
    if ! command -v docker &> /dev/null; then
        log_error "Docker is not installed"
        exit 1
    fi
    
    # Check environment file
    if [ ! -f "$PROJECT_ROOT/.env.$ENVIRONMENT" ]; then
        log_error "Environment file .env.$ENVIRONMENT not found"
        log_info "Please create it from .env.example"
        exit 1
    fi
    
    # Check deployment type specific requirements
    case $DEPLOY_TYPE in
        "docker-compose")
            if ! docker compose version &> /dev/null; then
                log_error "Docker Compose is not available"
                exit 1
            fi
            ;;
        "kubernetes")
            if ! command -v kubectl &> /dev/null; then
                log_error "kubectl is not installed"
                exit 1
            fi
            if ! kubectl cluster-info &> /dev/null; then
                log_error "No Kubernetes cluster connection"
                exit 1
            fi
            ;;
        *)
            log_error "Unknown deployment type: $DEPLOY_TYPE"
            exit 1
            ;;
    esac
    
    log_info "Prerequisites check passed"
}

# Backup existing deployment
backup_deployment() {
    log_info "Creating backup of existing deployment..."
    
    BACKUP_DIR="$PROJECT_ROOT/backups/$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$BACKUP_DIR"
    
    case $DEPLOY_TYPE in
        "docker-compose")
            # Backup database
            if docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" ps postgres | grep -q "Up"; then
                log_info "Backing up database..."
                docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres \
                    pg_dump -U wildlife_user wildlife_db > "$BACKUP_DIR/database_backup.sql"
            fi
            
            # Backup volumes
            docker run --rm -v wildcam_esp32_backend_uploads:/data -v "$BACKUP_DIR:/backup" \
                alpine tar czf /backup/uploads_backup.tar.gz -C /data .
            ;;
        "kubernetes")
            # Backup using kubectl
            kubectl get all -n "wildcam-$ENVIRONMENT" -o yaml > "$BACKUP_DIR/k8s_resources.yaml"
            ;;
    esac
    
    log_info "Backup created in $BACKUP_DIR"
}

# Deploy with Docker Compose
deploy_docker_compose() {
    log_info "Deploying with Docker Compose..."
    
    cd "$PROJECT_ROOT"
    
    # Pull latest images
    log_info "Pulling latest images..."
    docker compose -f docker-compose.prod.yml --env-file ".env.$ENVIRONMENT" pull
    
    # Deploy services
    log_info "Starting services..."
    docker compose -f docker-compose.prod.yml --env-file ".env.$ENVIRONMENT" up -d
    
    # Wait for services to be healthy
    log_info "Waiting for services to be healthy..."
    timeout 300 bash -c '
        while ! docker compose -f docker-compose.prod.yml ps | grep -q "healthy"; do
            echo "Waiting for services to be healthy..."
            sleep 10
        done
    ' || {
        log_error "Services failed to become healthy within 5 minutes"
        return 1
    }
    
    log_info "Docker Compose deployment completed"
}

# Deploy with Kubernetes
deploy_kubernetes() {
    log_info "Deploying with Kubernetes..."
    
    cd "$PROJECT_ROOT"
    
    # Apply configurations
    case $ENVIRONMENT in
        "development")
            kubectl apply -k k8s/overlays/development/
            ;;
        "staging")
            kubectl apply -k k8s/overlays/staging/
            ;;
        "production")
            kubectl apply -k k8s/overlays/production/
            ;;
        *)
            log_error "Unknown environment: $ENVIRONMENT"
            return 1
            ;;
    esac
    
    # Wait for rollout to complete
    log_info "Waiting for rollout to complete..."
    kubectl rollout status deployment/backend -n "wildcam-$ENVIRONMENT" --timeout=300s
    kubectl rollout status deployment/frontend -n "wildcam-$ENVIRONMENT" --timeout=300s
    kubectl rollout status deployment/nginx -n "wildcam-$ENVIRONMENT" --timeout=300s
    
    log_info "Kubernetes deployment completed"
}

# Health check
health_check() {
    log_info "Performing health checks..."
    
    case $DEPLOY_TYPE in
        "docker-compose")
            # Check API health
            if curl -f http://localhost/api/health &> /dev/null; then
                log_info "API health check passed"
            else
                log_error "API health check failed"
                return 1
            fi
            
            # Check frontend
            if curl -f http://localhost &> /dev/null; then
                log_info "Frontend health check passed"
            else
                log_error "Frontend health check failed"
                return 1
            fi
            ;;
        "kubernetes")
            # Check pod health
            if kubectl get pods -n "wildcam-$ENVIRONMENT" | grep -v "Running\|Completed"; then
                log_error "Some pods are not in Running state"
                return 1
            fi
            log_info "All pods are healthy"
            ;;
    esac
    
    log_info "Health checks passed"
}

# Post-deployment tasks
post_deployment() {
    log_info "Running post-deployment tasks..."
    
    # Update monitoring dashboards
    case $DEPLOY_TYPE in
        "docker-compose")
            # Restart Grafana to pick up new dashboards
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" restart grafana
            ;;
        "kubernetes")
            # Update Grafana configmaps
            kubectl rollout restart deployment/grafana -n monitoring
            ;;
    esac
    
    # Send deployment notification (webhook, Slack, etc.)
    if [ -n "${WEBHOOK_URL:-}" ]; then
        curl -X POST "$WEBHOOK_URL" \
            -H "Content-Type: application/json" \
            -d "{\"text\":\"WildCAM deployment to $ENVIRONMENT completed successfully\"}"
    fi
    
    log_info "Post-deployment tasks completed"
}

# Rollback function
rollback() {
    log_warn "Rolling back deployment..."
    
    case $DEPLOY_TYPE in
        "docker-compose")
            # Stop current deployment
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" down
            
            # Restore from backup
            LATEST_BACKUP=$(ls -1t "$PROJECT_ROOT/backups" | head -n1)
            if [ -n "$LATEST_BACKUP" ]; then
                log_info "Restoring from backup: $LATEST_BACKUP"
                # Restore database
                docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" up -d postgres
                sleep 30
                docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres \
                    psql -U wildlife_user wildlife_db < "$PROJECT_ROOT/backups/$LATEST_BACKUP/database_backup.sql"
            fi
            ;;
        "kubernetes")
            # Rollback deployment
            kubectl rollout undo deployment/backend -n "wildcam-$ENVIRONMENT"
            kubectl rollout undo deployment/frontend -n "wildcam-$ENVIRONMENT"
            kubectl rollout undo deployment/nginx -n "wildcam-$ENVIRONMENT"
            ;;
    esac
    
    log_info "Rollback completed"
}

# Main deployment function
main() {
    log_info "Starting WildCAM production deployment"
    log_info "Environment: $ENVIRONMENT"
    log_info "Deployment type: $DEPLOY_TYPE"
    
    # Set trap for cleanup on exit
    trap 'log_error "Deployment failed"; rollback; exit 1' ERR
    
    check_prerequisites
    backup_deployment
    
    case $DEPLOY_TYPE in
        "docker-compose")
            deploy_docker_compose
            ;;
        "kubernetes")
            deploy_kubernetes
            ;;
    esac
    
    health_check
    post_deployment
    
    log_info "🎉 WildCAM deployment completed successfully!"
    log_info "Access your deployment at:"
    
    case $DEPLOY_TYPE in
        "docker-compose")
            log_info "  Application: http://localhost"
            log_info "  Grafana: http://localhost:3001"
            log_info "  Prometheus: http://localhost:9090"
            ;;
        "kubernetes")
            INGRESS_IP=$(kubectl get ingress wildcam-ingress -n "wildcam-$ENVIRONMENT" -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
            log_info "  Application: https://$INGRESS_IP"
            log_info "  Monitoring: Check your configured ingress domains"
            ;;
    esac
}

# Script usage
usage() {
    echo "Usage: $0 [environment] [deployment-type]"
    echo "  environment: development|staging|production (default: production)"
    echo "  deployment-type: docker-compose|kubernetes (default: docker-compose)"
    echo
    echo "Examples:"
    echo "  $0                                    # Deploy to production with docker-compose"
    echo "  $0 staging                           # Deploy to staging with docker-compose"
    echo "  $0 production kubernetes             # Deploy to production with kubernetes"
}

# Handle script arguments
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    usage
    exit 0
fi

# Run main function
main "$@"