#!/bin/bash

# WildCAM Backup and Restore Script
# Comprehensive backup and restore functionality for the ESP32 WildCAM system

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DEPLOYMENT_TYPE="${DEPLOYMENT_TYPE:-docker-compose}"
ENVIRONMENT="${ENVIRONMENT:-production}"
BACKUP_DIR="${BACKUP_DIR:-$PROJECT_ROOT/backups}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create backup directory
create_backup_dir() {
    local backup_timestamp=$(date +%Y%m%d_%H%M%S)
    local backup_path="$BACKUP_DIR/$backup_timestamp"
    mkdir -p "$backup_path"
    echo "$backup_path"
}

# Database backup functions
backup_postgres() {
    local backup_path="$1"
    log_info "Backing up PostgreSQL database..."
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            # Create database dump
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres \
                pg_dump -U wildlife_user -d wildlife_db --clean --if-exists > "$backup_path/database.sql"
            
            # Create globals dump (users, roles, etc.)
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres \
                pg_dumpall -U wildlife_user --globals-only > "$backup_path/globals.sql"
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            kubectl exec deployment/postgres -n "$namespace" -- \
                pg_dump -U wildlife_user -d wildlife_db --clean --if-exists > "$backup_path/database.sql"
            
            kubectl exec deployment/postgres -n "$namespace" -- \
                pg_dumpall -U wildlife_user --globals-only > "$backup_path/globals.sql"
            ;;
    esac
    
    # Compress database backup
    gzip "$backup_path/database.sql"
    gzip "$backup_path/globals.sql"
    
    log_success "Database backup completed: $backup_path/database.sql.gz"
}

restore_postgres() {
    local backup_path="$1"
    log_info "Restoring PostgreSQL database..."
    
    if [ ! -f "$backup_path/database.sql.gz" ]; then
        log_error "Database backup file not found: $backup_path/database.sql.gz"
        return 1
    fi
    
    # Decompress backup
    gunzip -c "$backup_path/database.sql.gz" > "/tmp/database_restore.sql"
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            # Stop backend to prevent new connections
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" stop backend
            
            # Restore database
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T postgres \
                psql -U wildlife_user -d wildlife_db < "/tmp/database_restore.sql"
            
            # Restart backend
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" start backend
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            
            # Scale down backend
            kubectl scale deployment backend --replicas=0 -n "$namespace"
            
            # Restore database
            kubectl exec deployment/postgres -n "$namespace" -- \
                psql -U wildlife_user -d wildlife_db < "/tmp/database_restore.sql"
            
            # Scale up backend
            kubectl scale deployment backend --replicas=3 -n "$namespace"
            ;;
    esac
    
    # Clean up temporary file
    rm -f "/tmp/database_restore.sql"
    
    log_success "Database restore completed"
}

# Redis backup functions
backup_redis() {
    local backup_path="$1"
    log_info "Backing up Redis data..."
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            # Trigger Redis save
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis redis-cli BGSAVE
            
            # Wait for background save to complete
            while [ "$(docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis redis-cli LASTSAVE)" = "$(docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis redis-cli LASTSAVE)" ]; do
                sleep 1
            done
            
            # Copy Redis dump file
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" exec -T redis cat /data/dump.rdb > "$backup_path/redis_dump.rdb"
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            kubectl exec deployment/redis -n "$namespace" -- redis-cli BGSAVE
            
            # Wait for background save to complete
            sleep 5
            
            kubectl exec deployment/redis -n "$namespace" -- cat /data/dump.rdb > "$backup_path/redis_dump.rdb"
            ;;
    esac
    
    # Compress Redis backup
    gzip "$backup_path/redis_dump.rdb"
    
    log_success "Redis backup completed: $backup_path/redis_dump.rdb.gz"
}

restore_redis() {
    local backup_path="$1"
    log_info "Restoring Redis data..."
    
    if [ ! -f "$backup_path/redis_dump.rdb.gz" ]; then
        log_error "Redis backup file not found: $backup_path/redis_dump.rdb.gz"
        return 1
    fi
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            # Stop Redis
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" stop redis
            
            # Restore Redis dump file
            gunzip -c "$backup_path/redis_dump.rdb.gz" | docker run --rm -i -v wildcam_esp32_redis_data:/data alpine sh -c 'cat > /data/dump.rdb'
            
            # Start Redis
            docker compose -f "$PROJECT_ROOT/docker-compose.prod.yml" start redis
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            
            # Scale down Redis
            kubectl scale deployment redis --replicas=0 -n "$namespace"
            
            # Copy backup file to pod
            gunzip -c "$backup_path/redis_dump.rdb.gz" > "/tmp/redis_restore.rdb"
            kubectl cp "/tmp/redis_restore.rdb" "$namespace/redis-pod:/data/dump.rdb"
            
            # Scale up Redis
            kubectl scale deployment redis --replicas=1 -n "$namespace"
            
            # Clean up
            rm -f "/tmp/redis_restore.rdb"
            ;;
    esac
    
    log_success "Redis restore completed"
}

# File storage backup functions
backup_uploads() {
    local backup_path="$1"
    log_info "Backing up file uploads..."
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            # Create tar archive of uploads volume
            docker run --rm -v wildcam_esp32_backend_uploads:/data -v "$backup_path:/backup" \
                alpine tar czf /backup/uploads.tar.gz -C /data .
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            # Find a backend pod to access the uploads PVC
            local backend_pod=$(kubectl get pods -n "$namespace" -l app=backend -o jsonpath='{.items[0].metadata.name}')
            
            kubectl exec "$backend_pod" -n "$namespace" -- tar czf /tmp/uploads.tar.gz -C /app/uploads .
            kubectl cp "$namespace/$backend_pod:/tmp/uploads.tar.gz" "$backup_path/uploads.tar.gz"
            kubectl exec "$backend_pod" -n "$namespace" -- rm /tmp/uploads.tar.gz
            ;;
    esac
    
    log_success "Uploads backup completed: $backup_path/uploads.tar.gz"
}

restore_uploads() {
    local backup_path="$1"
    log_info "Restoring file uploads..."
    
    if [ ! -f "$backup_path/uploads.tar.gz" ]; then
        log_error "Uploads backup file not found: $backup_path/uploads.tar.gz"
        return 1
    fi
    
    case $DEPLOYMENT_TYPE in
        "docker-compose")
            docker run --rm -v wildcam_esp32_backend_uploads:/data -v "$backup_path:/backup" \
                alpine sh -c 'cd /data && tar xzf /backup/uploads.tar.gz'
            ;;
        "kubernetes")
            local namespace="wildcam-$ENVIRONMENT"
            local backend_pod=$(kubectl get pods -n "$namespace" -l app=backend -o jsonpath='{.items[0].metadata.name}')
            
            kubectl cp "$backup_path/uploads.tar.gz" "$namespace/$backend_pod:/tmp/uploads.tar.gz"
            kubectl exec "$backend_pod" -n "$namespace" -- sh -c 'cd /app/uploads && tar xzf /tmp/uploads.tar.gz'
            kubectl exec "$backend_pod" -n "$namespace" -- rm /tmp/uploads.tar.gz
            ;;
    esac
    
    log_success "Uploads restore completed"
}

# Configuration backup functions
backup_configs() {
    local backup_path="$1"
    log_info "Backing up configuration files..."
    
    # Create configs directory
    mkdir -p "$backup_path/configs"
    
    # Copy important configuration files
    cp "$PROJECT_ROOT/.env.production" "$backup_path/configs/" 2>/dev/null || log_warning ".env.production not found"
    cp "$PROJECT_ROOT/docker-compose.prod.yml" "$backup_path/configs/"
    
    # Copy monitoring configs
    if [ -d "$PROJECT_ROOT/monitoring" ]; then
        cp -r "$PROJECT_ROOT/monitoring" "$backup_path/configs/"
    fi
    
    # Copy nginx configs
    if [ -d "$PROJECT_ROOT/nginx" ]; then
        cp -r "$PROJECT_ROOT/nginx" "$backup_path/configs/"
    fi
    
    # Copy Kubernetes configs
    if [ -d "$PROJECT_ROOT/k8s" ]; then
        cp -r "$PROJECT_ROOT/k8s" "$backup_path/configs/"
    fi
    
    # Create backup metadata
    cat > "$backup_path/backup_metadata.json" << EOF
{
    "timestamp": "$(date -Iseconds)",
    "deployment_type": "$DEPLOYMENT_TYPE",
    "environment": "$ENVIRONMENT",
    "backup_version": "1.0",
    "components": [
        "database",
        "redis",
        "uploads",
        "configs"
    ]
}
EOF
    
    log_success "Configuration backup completed"
}

# Full backup function
full_backup() {
    log_info "Starting full system backup..."
    
    local backup_path=$(create_backup_dir)
    log_info "Backup location: $backup_path"
    
    # Backup all components
    backup_postgres "$backup_path"
    backup_redis "$backup_path"
    backup_uploads "$backup_path"
    backup_configs "$backup_path"
    
    # Create backup summary
    local backup_size=$(du -sh "$backup_path" | cut -f1)
    
    cat > "$backup_path/README.txt" << EOF
WildCAM System Backup
====================

Created: $(date)
Environment: $ENVIRONMENT
Deployment Type: $DEPLOYMENT_TYPE
Backup Size: $backup_size

Contents:
- database.sql.gz      PostgreSQL database dump
- globals.sql.gz       PostgreSQL global objects
- redis_dump.rdb.gz    Redis data dump
- uploads.tar.gz       File uploads archive
- configs/             Configuration files
- backup_metadata.json Backup metadata

Restore Command:
    $0 restore "$backup_path"

EOF
    
    log_success "Full backup completed successfully!"
    log_info "Backup location: $backup_path"
    log_info "Backup size: $backup_size"
}

# Full restore function
full_restore() {
    local backup_path="$1"
    
    if [ ! -d "$backup_path" ]; then
        log_error "Backup directory not found: $backup_path"
        return 1
    fi
    
    if [ ! -f "$backup_path/backup_metadata.json" ]; then
        log_error "Invalid backup: metadata file not found"
        return 1
    fi
    
    log_info "Starting full system restore from: $backup_path"
    log_warning "This will overwrite existing data. Are you sure? (yes/no)"
    read -r confirmation
    
    if [ "$confirmation" != "yes" ]; then
        log_info "Restore cancelled"
        return 0
    fi
    
    # Restore all components
    restore_postgres "$backup_path"
    restore_redis "$backup_path"
    restore_uploads "$backup_path"
    
    log_success "Full restore completed successfully!"
    log_info "Please restart the application to ensure all changes take effect"
}

# List available backups
list_backups() {
    log_info "Available backups in $BACKUP_DIR:"
    
    if [ ! -d "$BACKUP_DIR" ]; then
        log_warning "Backup directory does not exist: $BACKUP_DIR"
        return 0
    fi
    
    local backup_count=0
    for backup_dir in "$BACKUP_DIR"/*; do
        if [ -d "$backup_dir" ] && [ -f "$backup_dir/backup_metadata.json" ]; then
            local backup_name=$(basename "$backup_dir")
            local backup_size=$(du -sh "$backup_dir" | cut -f1)
            local backup_date=$(date -d "${backup_name:0:8}" +"%Y-%m-%d" 2>/dev/null || echo "Unknown")
            
            echo "  $backup_name ($backup_size) - $backup_date"
            ((backup_count++))
        fi
    done
    
    if [ $backup_count -eq 0 ]; then
        log_info "No backups found"
    else
        log_info "Total backups: $backup_count"
    fi
}

# Cleanup old backups
cleanup_backups() {
    local keep_days="${1:-30}"
    log_info "Cleaning up backups older than $keep_days days..."
    
    if [ ! -d "$BACKUP_DIR" ]; then
        log_warning "Backup directory does not exist: $BACKUP_DIR"
        return 0
    fi
    
    local deleted_count=0
    find "$BACKUP_DIR" -type d -name "????????_??????" -mtime +$keep_days | while read -r backup_dir; do
        log_info "Removing old backup: $(basename "$backup_dir")"
        rm -rf "$backup_dir"
        ((deleted_count++))
    done
    
    log_success "Cleanup completed. Removed $deleted_count old backups"
}

# Automated backup with retention
automated_backup() {
    local retention_days="${1:-30}"
    
    log_info "Starting automated backup with $retention_days days retention..."
    
    # Perform full backup
    full_backup
    
    # Cleanup old backups
    cleanup_backups "$retention_days"
    
    log_success "Automated backup completed"
}

# Main function
main() {
    local command="${1:-}"
    
    case "$command" in
        "backup"|"full-backup")
            full_backup
            ;;
        "restore"|"full-restore")
            if [ -z "${2:-}" ]; then
                log_error "Backup path required for restore"
                log_info "Usage: $0 restore <backup_path>"
                list_backups
                exit 1
            fi
            full_restore "$2"
            ;;
        "list"|"list-backups")
            list_backups
            ;;
        "cleanup")
            cleanup_backups "${2:-30}"
            ;;
        "auto"|"automated")
            automated_backup "${2:-30}"
            ;;
        "backup-db"|"database")
            local backup_path=$(create_backup_dir)
            backup_postgres "$backup_path"
            backup_configs "$backup_path"
            ;;
        "backup-files"|"uploads")
            local backup_path=$(create_backup_dir)
            backup_uploads "$backup_path"
            ;;
        *)
            usage
            exit 1
            ;;
    esac
}

# Usage information
usage() {
    echo "WildCAM Backup and Restore Tool"
    echo
    echo "Usage: $0 <command> [options]"
    echo
    echo "Commands:"
    echo "  backup, full-backup     Create a complete system backup"
    echo "  restore <path>          Restore from backup directory"
    echo "  list, list-backups      List available backups"
    echo "  cleanup [days]          Remove backups older than specified days (default: 30)"
    echo "  auto [days]             Automated backup with cleanup (default: 30 days retention)"
    echo "  backup-db               Backup only database"
    echo "  backup-files            Backup only file uploads"
    echo
    echo "Environment Variables:"
    echo "  DEPLOYMENT_TYPE         docker-compose|kubernetes (default: docker-compose)"
    echo "  ENVIRONMENT            development|staging|production (default: production)"
    echo "  BACKUP_DIR             Backup directory path (default: ./backups)"
    echo
    echo "Examples:"
    echo "  $0 backup                           # Full backup"
    echo "  $0 restore backups/20240115_143022  # Restore from specific backup"
    echo "  $0 list                             # List available backups"
    echo "  $0 auto 7                           # Automated backup, keep 7 days"
    echo "  DEPLOYMENT_TYPE=kubernetes $0 backup # Backup Kubernetes deployment"
}

# Handle script arguments
if [ $# -eq 0 ] || [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
    usage
    exit 0
fi

# Run main function
main "$@"