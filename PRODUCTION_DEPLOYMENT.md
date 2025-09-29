# 🚀 Production Deployment Guide

This comprehensive guide covers the deployment and maintenance of the ESP32 WildCAM system in production environments.

## 📋 Prerequisites

### System Requirements
- **Docker**: 20.10+ with Docker Compose v2
- **Kubernetes**: 1.24+ (optional, for K8s deployment)
- **Hardware**: 4GB+ RAM, 50GB+ storage, 2+ CPU cores
- **OS**: Ubuntu 20.04+ LTS, RHEL 8+, or similar

### Security Requirements
- SSL/TLS certificates (Let's Encrypt recommended)
- Firewall configured (only necessary ports open)
- Secret management system (HashiCorp Vault, AWS Secrets Manager, etc.)
- Regular security updates scheduled

## 🛠 Deployment Options

### Option 1: Docker Compose (Recommended for smaller deployments)

#### 1. Environment Setup
```bash
# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32

# Create production environment file
cp .env.example .env.prod
nano .env.prod  # Configure your production settings
```

#### 2. Production Environment Variables
```bash
# Security
JWT_SECRET_KEY=your-super-secure-jwt-key-here
SECRET_KEY=your-super-secure-secret-key-here
POSTGRES_PASSWORD=your-secure-postgres-password
REDIS_PASSWORD=your-secure-redis-password

# Database
POSTGRES_USER=wildlife_user
POSTGRES_DB=wildlife_db

# Application URLs
REACT_APP_API_URL=https://api.your-domain.com
REACT_APP_WEBSOCKET_URL=wss://api.your-domain.com

# Monitoring
GRAFANA_ADMIN_USER=admin
GRAFANA_ADMIN_PASSWORD=your-secure-grafana-password

# Optional: Cloud Storage
AWS_ACCESS_KEY_ID=your-aws-access-key
AWS_SECRET_ACCESS_KEY=your-aws-secret-key
S3_BUCKET=your-wildlife-bucket

# Optional: InfluxDB
INFLUXDB_USERNAME=admin
INFLUXDB_PASSWORD=your-influxdb-password
INFLUXDB_TOKEN=your-influxdb-token
```

#### 3. Deploy Production Stack
```bash
# Deploy with production configuration
docker compose -f docker-compose.prod.yml --env-file .env.prod up -d

# Check service status
docker compose -f docker-compose.prod.yml ps

# View logs
docker compose -f docker-compose.prod.yml logs -f
```

### Option 2: Kubernetes (Recommended for larger deployments)

#### 1. Deploy Base Infrastructure
```bash
# Apply base configurations
kubectl apply -k k8s/base/

# Deploy to production environment
kubectl apply -k k8s/overlays/production/
```

#### 2. Configure Secrets
```bash
# Create production secrets
kubectl create secret generic wildcam-secrets \
  --from-literal=postgres-password=your-secure-password \
  --from-literal=jwt-secret=your-jwt-secret \
  --from-literal=secret-key=your-secret-key \
  --namespace=wildcam-prod
```

#### 3. Set up Ingress and TLS
```bash
# Install cert-manager for automatic SSL
kubectl apply -f https://github.com/cert-manager/cert-manager/releases/download/v1.13.0/cert-manager.yaml

# Configure your domain in k8s/overlays/production/ingress.yaml
kubectl apply -f k8s/overlays/production/ingress.yaml
```

## 📊 Monitoring Setup

### Prometheus + Grafana

Access monitoring dashboards:
- **Grafana**: https://your-domain.com/grafana (admin/your-password)
- **Prometheus**: https://your-domain.com/prometheus

### Key Metrics to Monitor
- API response times and error rates
- Database connection pool status
- Active camera count and last-seen timestamps
- Wildlife detection success/failure rates
- System resource usage (CPU, memory, disk)
- Container health and restart counts

### Alerting Configuration

Alerts are configured for:
- Service downtime (>2 minutes)
- High CPU usage (>80% for 5 minutes)
- High memory usage (>85% for 5 minutes)
- High API latency (95th percentile >2 seconds)
- High error rate (>5% for 5 minutes)
- Camera offline (>5 minutes without data)
- Database connection failures

## 🔐 Security Hardening

### Container Security
- All containers run as non-root users
- Read-only filesystems where possible
- Security contexts with no-new-privileges
- Resource limits and quotas configured
- Regular base image updates

### Network Security
```bash
# Configure firewall (UFW example)
ufw enable
ufw allow 22/tcp    # SSH
ufw allow 80/tcp    # HTTP
ufw allow 443/tcp   # HTTPS
ufw allow 1883/tcp  # MQTT (ESP32 communication)
ufw deny 5432/tcp   # Block direct database access
ufw deny 6379/tcp   # Block direct Redis access
```

### SSL/TLS Configuration
```bash
# Install certbot for Let's Encrypt
sudo apt install certbot

# Generate certificates
sudo certbot certonly --standalone -d your-domain.com

# Configure automatic renewal
sudo crontab -e
# Add: 0 12 * * * /usr/bin/certbot renew --quiet
```

## 🔄 CI/CD Pipeline

The GitHub Actions workflow provides:

### Continuous Integration
- Automated testing (backend Python, frontend React)
- Security scanning with Trivy
- Code quality checks (linting, formatting)
- Dependency vulnerability scanning

### Continuous Deployment
- Multi-platform Docker image builds (amd64, arm64)
- Automated deployments to staging and production
- Rolling updates with health checks
- Rollback capabilities

### Pipeline Secrets Required
```bash
# GitHub repository secrets
STAGING_HOST=staging.your-domain.com
STAGING_USER=deploy
STAGING_SSH_KEY=your-staging-ssh-private-key

PRODUCTION_HOST=your-domain.com
PRODUCTION_USER=deploy
PRODUCTION_SSH_KEY=your-production-ssh-private-key
```

## 📈 Scaling Guidelines

### Horizontal Scaling (Kubernetes)
```yaml
# Adjust replicas in k8s/overlays/production/replica-count.yaml
backend: 5 replicas    # API servers
frontend: 3 replicas   # Static file servers
nginx: 3 replicas      # Load balancers
```

### Vertical Scaling
```yaml
# Adjust resources in k8s/overlays/production/resource-limits.yaml
backend:
  requests: 1Gi RAM, 500m CPU
  limits: 2Gi RAM, 1000m CPU
```

### Database Scaling
- Enable PostgreSQL read replicas for read-heavy workloads
- Configure connection pooling (PgBouncer)
- Consider partitioning for large wildlife_detections table
- Set up automated backups with point-in-time recovery

## 🔧 Maintenance Tasks

### Daily
- Check service health status
- Review error logs and alerts
- Monitor resource usage trends
- Verify backup completion

### Weekly
- Update container images (security patches)
- Review and rotate logs
- Test backup restoration procedure
- Update SSL certificates if needed

### Monthly
- Performance analysis and optimization
- Capacity planning review
- Security audit and updates
- Disaster recovery testing

## 🚨 Incident Response

### Service Recovery
```bash
# Check service status
docker compose -f docker-compose.prod.yml ps

# Restart specific service
docker compose -f docker-compose.prod.yml restart backend

# View detailed logs
docker compose -f docker-compose.prod.yml logs --tail=100 backend

# Scale service replicas (K8s)
kubectl scale deployment backend --replicas=5 -n wildcam-prod
```

### Database Recovery
```bash
# Database backup
docker compose -f docker-compose.prod.yml exec postgres pg_dump -U wildlife_user wildlife_db > backup.sql

# Database restore
docker compose -f docker-compose.prod.yml exec -T postgres psql -U wildlife_user wildlife_db < backup.sql
```

### Emergency Procedures
1. **Complete System Failure**: Deploy from backup infrastructure
2. **Database Corruption**: Restore from latest backup
3. **Security Breach**: Rotate all secrets, review access logs
4. **DDoS Attack**: Enable rate limiting, contact hosting provider

## 📞 Support and Monitoring

### Log Aggregation (ELK Stack)
- **Elasticsearch**: Log storage and indexing
- **Logstash**: Log processing and enrichment  
- **Kibana**: Log visualization and analysis

### Performance Monitoring
- Response time percentiles (P50, P95, P99)
- Error rate by endpoint
- Database query performance
- Cache hit rates

### Business Metrics
- Active camera count
- Wildlife detection accuracy
- Data ingestion rates
- User engagement metrics

## 🌍 Multi-Environment Setup

### Development
```bash
kubectl apply -k k8s/overlays/development/
```

### Staging
```bash
kubectl apply -k k8s/overlays/staging/
```

### Production
```bash
kubectl apply -k k8s/overlays/production/
```

Each environment has isolated:
- Namespaces and resource quotas
- Secrets and configurations
- Database instances
- Monitoring and alerting

---

## 📚 Additional Resources

- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Docker Compose Reference](https://docs.docker.com/compose/)
- [Prometheus Monitoring](https://prometheus.io/docs/)
- [Grafana Dashboards](https://grafana.com/docs/)
- [ESP32 Camera Integration](./docs/ESP32_INTEGRATION.md)

For technical support, please refer to the troubleshooting section or contact the development team.