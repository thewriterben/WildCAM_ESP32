#!/bin/bash

# WildCAM Health Check Script
# Comprehensive health monitoring for all system components

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DEPLOYMENT_TYPE="${1:-docker-compose}"
ENVIRONMENT="${2:-production}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Status counters
CHECKS_PASSED=0
CHECKS_FAILED=0
CHECKS_WARNING=0

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((CHECKS_PASSED++))
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
    ((CHECKS_WARNING++))
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((CHECKS_FAILED++))
}

# Health check functions
check_docker_compose_health() {
    log_info "Checking Docker Compose services..."
    
    cd "$PROJECT_ROOT"
    
    # Check if services are running
    if ! docker compose -f docker-compose.prod.yml ps --quiet | grep -q .; then
        log_error "No Docker Compose services are running"
        return 1
    fi
    
    # Check individual service health
    local services=("postgres" "redis" "backend" "frontend" "nginx")
    
    for service in "${services[@]}"; do
        if docker compose -f docker-compose.prod.yml ps "$service" 2>/dev/null | grep -q "Up"; then
            log_success "$service is running"
        else
            log_error "$service is not running"
        fi
    done
    
    # Check service health status
    local healthy_services=$(docker compose -f docker-compose.prod.yml ps --filter "health=healthy" --quiet | wc -l)
    local total_services=$(docker compose -f docker-compose.prod.yml ps --quiet | wc -l)
    
    if [ "$healthy_services" -eq "$total_services" ]; then
        log_success "All services are healthy ($healthy_services/$total_services)"
    else
        log_warning "$healthy_services/$total_services services are healthy"
    fi
}

check_kubernetes_health() {
    log_info "Checking Kubernetes pods..."
    
    local namespace="wildcam-$ENVIRONMENT"
    
    # Check if namespace exists
    if ! kubectl get namespace "$namespace" &>/dev/null; then
        log_error "Namespace $namespace does not exist"
        return 1
    fi
    
    # Check pod status
    local running_pods=$(kubectl get pods -n "$namespace" --field-selector=status.phase=Running --no-headers | wc -l)
    local total_pods=$(kubectl get pods -n "$namespace" --no-headers | wc -l)
    
    if [ "$running_pods" -eq "$total_pods" ] && [ "$total_pods" -gt 0 ]; then
        log_success "All pods are running ($running_pods/$total_pods)"
    else
        log_warning "$running_pods/$total_pods pods are running"
        
        # Show failed pods
        kubectl get pods -n "$namespace" --field-selector=status.phase!=Running --no-headers | while read -r line; do
            local pod_name=$(echo "$line" | awk '{print $1}')
            local status=$(echo "$line" | awk '{print $3}')
            log_error "Pod $pod_name is in $status state"
        done
    fi
    
    # Check deployments
    local deployments=("backend" "frontend" "nginx" "postgres" "redis")
    
    for deployment in "${deployments[@]}"; do
        if kubectl get deployment "$deployment" -n "$namespace" &>/dev/null; then
            local ready=$(kubectl get deployment "$deployment" -n "$namespace" -o jsonpath='{.status.readyReplicas}')
            local desired=$(kubectl get deployment "$deployment" -n "$namespace" -o jsonpath='{.spec.replicas}')
            
            if [ "$ready" = "$desired" ]; then
                log_success "$deployment deployment is ready ($ready/$desired)"
            else
                log_warning "$deployment deployment is not fully ready ($ready/$desired)"
            fi
        else
            log_error "$deployment deployment not found"
        fi
    done
}

check_api_endpoints() {
    log_info "Checking API endpoints..."
    
    local base_url
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            base_url="http://localhost"
            ;;
        "kubernetes")
            base_url="http://localhost"  # Assuming port-forward or ingress
            ;;
    esac
    
    # Health endpoint
    if curl -sf "$base_url/api/health" >/dev/null; then
        log_success "API health endpoint is responding"
    else
        log_error "API health endpoint is not responding"
    fi
    
    # Frontend
    if curl -sf "$base_url/" >/dev/null; then
        log_success "Frontend is responding"
    else
        log_error "Frontend is not responding"
    fi
    
    # Check response times
    local response_time=$(curl -o /dev/null -s -w '%{time_total}' "$base_url/api/health" || echo "999")
    if (( $(echo "$response_time < 1.0" | bc -l) )); then
        log_success "API response time is good (${response_time}s)"
    else
        log_warning "API response time is slow (${response_time}s)"
    fi
}

check_database_health() {
    log_info "Checking database health..."
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            if docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres pg_isready -U wildlife_user -d wildlife_db &>/dev/null; then
                log_success "Database is accepting connections"
            else
                log_error "Database is not accepting connections"
                return 1
            fi
            
            # Check database size
            local db_size=$(docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres psql -U wildlife_user -d wildlife_db -t -c "SELECT pg_size_pretty(pg_database_size('wildlife_db'));" 2>/dev/null | xargs)
            if [ -n "$db_size" ]; then
                log_success "Database size: $db_size"
            else
                log_warning "Could not retrieve database size"
            fi
            
            # Check active connections
            local connections=$(docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres psql -U wildlife_user -d wildlife_db -t -c "SELECT count(*) FROM pg_stat_activity;" 2>/dev/null | xargs)
            if [ -n "$connections" ]; then
                log_success "Active database connections: $connections"
            else
                log_warning "Could not retrieve connection count"
            fi
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            if kubectl exec deployment/postgres -n "$namespace" -- pg_isready -U wildlife_user -d wildlife_db &>/dev/null; then
                log_success "Database is accepting connections"
            else
                log_error "Database is not accepting connections"
            fi
            ;;
    esac
}

check_redis_health() {
    log_info "Checking Redis health..."
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            if docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis redis-cli ping &>/dev/null; then
                log_success "Redis is responding to ping"
            else
                log_error "Redis is not responding"
                return 1
            fi
            
            # Check memory usage
            local memory_info=$(docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis redis-cli info memory | grep used_memory_human | cut -d: -f2 | tr -d '\r')
            if [ -n "$memory_info" ]; then
                log_success "Redis memory usage: $memory_info"
            else
                log_warning "Could not retrieve Redis memory info"
            fi
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            if kubectl exec deployment/redis -n "$namespace" -- redis-cli ping &>/dev/null; then
                log_success "Redis is responding to ping"
            else
                log_error "Redis is not responding"
            fi
            ;;
    esac
}

check_monitoring_health() {
    log_info "Checking monitoring services..."
    
    # Prometheus
    if curl -sf "http://localhost:9090/-/healthy" >/dev/null 2>&1; then
        log_success "Prometheus is healthy"
    else
        log_warning "Prometheus is not accessible (may be expected in some deployments)"
    fi
    
    # Grafana
    if curl -sf "http://localhost:3001/api/health" >/dev/null 2>&1; then
        log_success "Grafana is healthy"
    else
        log_warning "Grafana is not accessible (may be expected in some deployments)"
    fi
}

check_disk_space() {
    log_info "Checking disk space..."
    
    # Check root filesystem
    local disk_usage=$(df / | awk 'NR==2 {print $5}' | sed 's/%//')
    if [ "$disk_usage" -lt 80 ]; then
        log_success "Root filesystem usage: ${disk_usage}%"
    elif [ "$disk_usage" -lt 90 ]; then
        log_warning "Root filesystem usage: ${disk_usage}% (getting high)"
    else
        log_error "Root filesystem usage: ${disk_usage}% (critically high)"
    fi
    
    # Check Docker volumes (if using Docker Compose)
    if [ "$DEPLOYMENT_TYPE" = "docker-compose" ]; then
        local volumes=("postgres_data" "redis_data" "backend_uploads")
        for volume in "${volumes[@]}"; do
            local volume_name="wildcam_esp32_${volume}"
            if docker volume inspect "$volume_name" >/dev/null 2>&1; then
                log_success "Volume $volume exists"
            else
                log_warning "Volume $volume not found"
            fi
        done
    fi
}

check_ssl_certificates() {
    log_info "Checking SSL certificates..."
    
    local cert_path="/etc/ssl/certs/wildcam.crt"
    if [ -f "$cert_path" ]; then
        local expiry_date=$(openssl x509 -enddate -noout -in "$cert_path" | cut -d= -f2)
        local expiry_timestamp=$(date -d "$expiry_date" +%s)
        local current_timestamp=$(date +%s)
        local days_until_expiry=$(( (expiry_timestamp - current_timestamp) / 86400 ))
        
        if [ "$days_until_expiry" -gt 30 ]; then
            log_success "SSL certificate expires in $days_until_expiry days"
        elif [ "$days_until_expiry" -gt 7 ]; then
            log_warning "SSL certificate expires in $days_until_expiry days"
        else
            log_error "SSL certificate expires in $days_until_expiry days (renewal needed)"
        fi
    else
        log_warning "SSL certificate not found at $cert_path"
    fi
}

check_log_rotation() {
    log_info "Checking log rotation..."
    
    local log_dirs=("/var/log/nginx" "./logs")
    for log_dir in "${log_dirs[@]}"; do
        if [ -d "$log_dir" ]; then
            local log_size=$(du -sh "$log_dir" 2>/dev/null | cut -f1)
            local file_count=$(find "$log_dir" -type f -name "*.log*" | wc -l)
            log_success "Log directory $log_dir: $log_size ($file_count files)"
        else
            log_warning "Log directory $log_dir not found"
        fi
    done
}

generate_report() {
    log_info "Generating health check report..."
    
    local total_checks=$((CHECKS_PASSED + CHECKS_FAILED + CHECKS_WARNING))
    local report_file="health-check-$(date +%Y%m%d_%H%M%S).txt"
    
    cat > "$report_file" << EOF
WildCAM Health Check Report
Generated: $(date)
Environment: $ENVIRONMENT
Deployment Type: $DEPLOYMENT_TYPE

SUMMARY:
- Total Checks: $total_checks
- Passed: $CHECKS_PASSED
- Failed: $CHECKS_FAILED  
- Warnings: $CHECKS_WARNING

OVERALL STATUS: $([ $CHECKS_FAILED -eq 0 ] && echo "HEALTHY" || echo "UNHEALTHY")

$([ $CHECKS_FAILED -gt 0 ] && echo "CRITICAL ISSUES FOUND - IMMEDIATE ATTENTION REQUIRED")
$([ $CHECKS_WARNING -gt 0 ] && echo "WARNINGS PRESENT - MONITORING RECOMMENDED")
EOF
    
    echo "Report saved to: $report_file"
}

# Main health check function
main() {
    log_info "Starting WildCAM health check..."
    log_info "Environment: $ENVIRONMENT"
    log_info "Deployment type: $DEPLOYMENT_TYPE"
    echo
    
    # Run health checks based on deployment type
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            check_docker_compose_health
            ;;
        "kubernetes")
            check_kubernetes_health
            ;;
        *)
            log_error "Unknown deployment type: $DEPLOYMENT_TYPE"
            exit 1
            ;;
    esac
    
    echo
    check_api_endpoints
    echo
    check_database_health
    echo
    check_redis_health
    echo
    check_monitoring_health
    echo
    check_disk_space
    echo
    check_ssl_certificates
    echo
    check_log_rotation
    echo
    
    # Generate summary
    echo "=" * 60
    log_info "HEALTH CHECK SUMMARY"
    echo "=" * 60
    
    local total_checks=$((CHECKS_PASSED + CHECKS_FAILED + CHECKS_WARNING))
    
    echo -e "${GREEN}Passed: $CHECKS_PASSED${NC}"
    echo -e "${RED}Failed: $CHECKS_FAILED${NC}"
    echo -e "${YELLOW}Warnings: $CHECKS_WARNING${NC}"
    echo -e "Total: $total_checks"
    
    if [ $CHECKS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}✅ Overall Status: HEALTHY${NC}"
        exit 0
    else
        echo -e "\n${RED}❌ Overall Status: UNHEALTHY${NC}"
        echo -e "${RED}$CHECKS_FAILED critical issue(s) found${NC}"
        exit 1
    fi
}

# Script usage
usage() {
    echo "Usage: $0 [deployment-type] [environment]"
    echo "  deployment-type: docker-compose|kubernetes (default: docker-compose)"
    echo "  environment: development|staging|production (default: production)"
    echo
    echo "Examples:"
    echo "  $0                          # Check docker-compose production"
    echo "  $0 kubernetes staging       # Check kubernetes staging"
    echo "  $0 docker-compose           # Check docker-compose production"
}

# Handle script arguments
if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
    usage
    exit 0
fi

# Run main function
main "$@"