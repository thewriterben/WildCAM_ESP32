# WildCAM ESP32 Multi-Cloud Architecture

## System Overview

The WildCAM ESP32 platform employs a sophisticated multi-cloud architecture designed for maximum availability, global scalability, and disaster resilience. The system is deployed across AWS, Microsoft Azure, and Google Cloud Platform in an active-active configuration.

## High-Level Architecture

```
                                    ┌─────────────────────┐
                                    │   Global CDN/DNS    │
                                    │   (CloudFlare)      │
                                    └──────────┬──────────┘
                                               │
                        ┌──────────────────────┼──────────────────────┐
                        │                      │                      │
                   ┌────▼─────┐         ┌─────▼────┐          ┌─────▼────┐
                   │   AWS    │         │  Azure   │          │   GCP    │
                   │ us-west-2│         │westus2   │          │us-west1  │
                   └────┬─────┘         └─────┬────┘          └─────┬────┘
                        │                     │                      │
                   ┌────▼─────────────────────▼──────────────────────▼────┐
                   │           VPN Mesh / Service Mesh (Istio)            │
                   └────┬─────────────────────┬──────────────────────┬────┘
                        │                     │                      │
            ┌───────────▼──────────┐  ┌──────▼───────────┐  ┌──────▼──────────┐
            │    EKS Cluster       │  │   AKS Cluster    │  │  GKE Cluster    │
            │  ┌─────────────┐     │  │ ┌──────────────┐ │  │ ┌─────────────┐ │
            │  │ Backend Pods│     │  │ │ Backend Pods │ │  │ │Backend Pods │ │
            │  │ Frontend Pods│    │  │ │Frontend Pods │ │  │ │Frontend Pods│ │
            │  └─────────────┘     │  │ └──────────────┘ │  │ └─────────────┘ │
            └───────────┬──────────┘  └──────┬───────────┘  └──────┬──────────┘
                        │                     │                      │
            ┌───────────▼──────────┐  ┌──────▼───────────┐  ┌──────▼──────────┐
            │  RDS PostgreSQL      │  │Azure PostgreSQL  │  │Cloud SQL Postgres│
            │  Multi-AZ            │  │ Flexible Server  │  │   Regional      │
            └───────────┬──────────┘  └──────┬───────────┘  └──────┬──────────┘
                        │                     │                      │
                        └─────────────────────┼──────────────────────┘
                                              │
                                    ┌─────────▼──────────┐
                                    │ Database Replication│
                                    │   (BDR/Kafka)      │
                                    └────────────────────┘
```

## Component Architecture

### 1. Edge Layer

**Global Traffic Management**
- CloudFlare DNS with geo-routing
- DDoS protection and WAF at edge
- SSL/TLS termination
- Global load balancing (40% AWS, 30% Azure, 30% GCP)

**Regional CDN**
- AWS CloudFront (Americas, Europe)
- Azure CDN (Asia-Pacific)
- GCP Cloud CDN (Global)

### 2. Kubernetes Layer

**AWS EKS Configuration**
```yaml
Cluster Version: 1.28
Node Groups:
  - General Purpose: 3-20 nodes (m5.xlarge)
  - Compute Optimized: 2-30 nodes (c5.2xlarge)
Networking: AWS VPC CNI
Service Mesh: Istio 1.19
Auto-scaling: Cluster Autoscaler + HPA
```

**Azure AKS Configuration**
```yaml
Cluster Version: 1.28
Node Pools:
  - System Pool: 3-10 nodes (Standard_D8s_v3)
  - User Pool: 2-20 nodes (Standard_D16s_v3)
Networking: Azure CNI + Calico
Service Mesh: Istio 1.19
Auto-scaling: Cluster Autoscaler + HPA
```

**GCP GKE Configuration**
```yaml
Cluster Version: 1.28
Node Pools:
  - Default: 3-20 nodes (n2-standard-8)
  - Compute: 2-30 nodes (c2-standard-16)
Networking: VPC-native + Calico
Service Mesh: Istio 1.19
Auto-scaling: Cluster Autoscaler + HPA
```

### 3. Application Layer

**Backend Services**
- **Language**: Python 3.11 (FastAPI)
- **Replicas**: 3-20 per cluster (auto-scaled)
- **Resources**: 1-4 CPU, 2-8GB RAM
- **Health Checks**: Liveness, Readiness, Startup probes
- **Metrics**: Prometheus instrumentation

**Frontend Services**
- **Framework**: React 18 with Next.js
- **Replicas**: 3-15 per cluster (auto-scaled)
- **Resources**: 250m-1 CPU, 512MB-2GB RAM
- **Static Assets**: Served via CDN
- **SSR**: Server-side rendering for SEO

**API Gateway**
- **Technology**: Istio Virtual Services
- **Features**: 
  - Rate limiting (2000 req/s per IP)
  - Circuit breaking
  - Retry policies
  - Timeout management
  - mTLS enforcement

### 4. Data Layer

**Primary Databases**
```
AWS RDS PostgreSQL 15
├── Instance: db.r6g.xlarge
├── Multi-AZ: Enabled
├── Replicas: 2 read replicas
├── Backup: 30-day retention
├── PITR: Enabled
└── Encryption: KMS

Azure PostgreSQL Flexible Server
├── SKU: GP_Standard_D8s_v3
├── High Availability: Zone-redundant
├── Replicas: Enabled
├── Backup: 30-day retention
├── PITR: Enabled
└── Encryption: Customer-managed keys

GCP Cloud SQL PostgreSQL
├── Tier: db-n1-standard-8
├── High Availability: Regional
├── Replicas: Enabled
├── Backup: 30-day retention
├── PITR: Enabled
└── Encryption: CMEK
```

**Caching Layer**
- **Technology**: Redis Cluster
- **Deployment**: 1 master + 3 replicas per cloud
- **Use Cases**: 
  - Session management
  - API response caching
  - Real-time leaderboards
  - Rate limiting counters

**Message Queue**
- **Technology**: Apache Kafka
- **Deployment**: 3-node cluster per cloud
- **Topics**:
  - wildlife-detections
  - image-processing
  - alerts-notifications
  - cross-cloud-replication
- **Retention**: 7 days

### 5. Storage Layer

**Object Storage**
```
AWS S3
├── Bucket: wildcam-wildlife-data-aws
├── Replication: Cross-region to us-east-1
├── Versioning: Enabled (90-day retention)
├── Lifecycle: Hot → Glacier (90d) → Deep Archive (1y)
└── Encryption: SSE-KMS

Azure Blob Storage
├── Account: wildcamdataazure
├── Replication: GRS (Geo-redundant)
├── Versioning: Enabled
├── Lifecycle: Hot → Cool (90d) → Archive (1y)
└── Encryption: Customer-managed keys

GCP Cloud Storage
├── Bucket: wildcam-wildlife-data-gcp
├── Class: Multi-regional
├── Versioning: Enabled
├── Lifecycle: Standard → Nearline (90d) → Coldline (1y)
└── Encryption: CMEK
```

**Total Storage Capacity**: 
- Images: 50TB per cloud (150TB total)
- Videos: 100TB per cloud (300TB total)
- Backups: 25TB per cloud (75TB total)

### 6. Security Layer

**Identity and Access Management**
```
AWS IAM
├── Service Accounts: 15+
├── Roles: Least privilege
├── MFA: Required for humans
└── Federation: OIDC with GitHub

Azure AD
├── Service Principals: 15+
├── RBAC: Granular permissions
├── MFA: Required
└── Federation: OIDC

GCP IAM
├── Service Accounts: 15+
├── Roles: Custom roles
├── MFA: Required
└── Workload Identity: Enabled
```

**Secrets Management**
- AWS Secrets Manager
- Azure Key Vault
- GCP Secret Manager
- HashiCorp Vault (cross-cloud)

**Network Security**
```
Defense in Depth:
1. WAF (CloudFlare, AWS WAF, Azure Front Door)
2. DDoS Protection (Cloud-native + CloudFlare)
3. Network Policies (Calico)
4. Service Mesh mTLS (Istio)
5. Pod Security Policies
6. Image Scanning (Trivy, Snyk)
```

### 7. Monitoring and Observability

**Metrics**
- **Prometheus**: Per-cluster deployment
- **Thanos**: Cross-cluster aggregation
- **Grafana**: Unified dashboards
- **Metrics Exported**:
  - Application metrics (latency, throughput)
  - Infrastructure metrics (CPU, memory, disk)
  - Business metrics (detections, uploads)

**Logging**
- **Stack**: ELK (Elasticsearch, Logstash, Kibana)
- **Deployment**: Centralized per region
- **Log Sources**:
  - Application logs
  - Kubernetes events
  - Audit logs
  - Network flow logs
- **Retention**: 90 days hot, 1 year cold

**Tracing**
- **Technology**: Jaeger
- **Sampling**: 1% of requests
- **Backends**: Elasticsearch
- **Features**:
  - Service dependency graph
  - Performance profiling
  - Error tracking

**Alerting**
- **PagerDuty**: Critical alerts
- **Slack**: Warning alerts
- **Email**: Informational alerts
- **Alert Rules**:
  - High error rate (> 1%)
  - High latency (> 500ms p99)
  - Database replication lag (> 30s)
  - Pod crashes (> 3 in 5 min)
  - Resource exhaustion

## Data Flow

### Wildlife Detection Event Flow

```
1. ESP32 Camera captures image
   └─> Edge processing (TensorFlow Lite)
       └─> Species detected (confidence > 0.7)
           └─> Upload to nearest cloud endpoint
               └─> Load balancer routes to healthy backend
                   └─> Store image in object storage
                   └─> Write metadata to database
                   └─> Publish to Kafka topic
                       └─> Trigger ML pipeline
                       └─> Send to research platforms
                       └─> Update dashboards
```

### Cross-Cloud Replication Flow

```
1. Data written to AWS RDS
   └─> PostgreSQL logical replication
       ├─> Replicate to Azure PostgreSQL
       └─> Replicate to GCP Cloud SQL
   
2. Object uploaded to AWS S3
   └─> S3 Event triggers Lambda
       └─> Copy to Azure Blob (via DataSync)
       └─> Copy to GCP Storage (via Transfer Service)
```

## Disaster Recovery

### RTO/RPO Targets

| Scenario | RTO | RPO | Strategy |
|----------|-----|-----|----------|
| Single pod failure | < 30s | 0 | K8s auto-restart |
| Node failure | < 2m | 0 | Auto-scaling |
| AZ failure | < 5m | 0 | Multi-AZ deployment |
| Region failure | < 15m | < 5m | Cross-region failover |
| Cloud provider outage | < 2m | 0 | Multi-cloud active-active |
| Database corruption | < 15m | < 5m | PITR restore |
| Ransomware attack | < 1h | < 24h | Immutable backups |

### Backup Strategy

```
Database Backups:
├── Continuous WAL archiving
├── Daily snapshots (30-day retention)
├── Weekly snapshots (12-week retention)
└── Monthly snapshots (7-year retention)

Object Storage:
├── Versioning (90-day retention)
├── Cross-region replication
├── Cross-cloud replication (24h delay)
└── Immutable object lock (compliance)

Configuration:
├── Git version control
├── Terraform state (S3 + DynamoDB)
└── Secrets backup (encrypted vault)
```

## Performance Optimization

### Caching Strategy

```
L1: Browser Cache (24h for static assets)
├─> L2: CDN Cache (1h for images, 5m for API)
    ├─> L3: Redis Cache (5m for hot data)
        └─> L4: Database query cache
```

### Geographic Distribution

```
Traffic Routing:
Americas → AWS (us-west-2, us-east-1)
Europe → Azure (West Europe)
Asia-Pacific → GCP (asia-southeast1)
```

### Auto-Scaling Policies

```
Application Pods:
├── Target CPU: 70%
├── Target Memory: 80%
├── Scale up: +50% (max 30s)
└── Scale down: -25% (max 5m cooldown)

Cluster Nodes:
├── Pending pods: > 30s
├── Node utilization: > 80%
└── Scale down: Unused > 10m
```

## Cost Management

### Monthly Budget (Production)

```
AWS: $15,000/month
├── EKS: $4,000
├── EC2: $6,000
├── RDS: $3,000
├── S3: $1,500
└── Other: $500

Azure: $12,000/month
├── AKS: $3,500
├── VMs: $5,000
├── PostgreSQL: $2,500
└── Storage: $1,000

GCP: $13,000/month
├── GKE: $3,500
├── Compute: $5,500
├── Cloud SQL: $3,000
└── Storage: $1,000

Total: ~$40,000/month
```

### Cost Optimization

- Reserved Instances: 30% savings on compute
- Spot Instances: 60% savings on batch jobs
- Storage lifecycle: 50% savings on old data
- Auto-scaling: 40% savings on off-peak
- Cross-region data transfer: Optimized routing

## Compliance and Governance

### Regulatory Compliance

- **GDPR**: EU data residency, right to deletion
- **CCPA**: California privacy controls
- **SOC 2 Type II**: Security controls audit
- **ISO 27001**: Information security

### Access Controls

```
Production Access:
├── Engineers: Read-only (default)
├── SRE Team: Break-glass access
├── Automated Systems: Service accounts
└── Audit: All access logged
```

## Future Enhancements

1. **Edge Computing**: Deploy K3s clusters at field stations
2. **5G Integration**: Direct cellular upload from cameras
3. **AI/ML Pipeline**: Real-time species classification
4. **Blockchain**: Immutable wildlife records
5. **Quantum-Safe Encryption**: Post-quantum cryptography
6. **Serverless Functions**: AWS Lambda, Azure Functions, Cloud Functions

---

**Document Version**: 1.0.0
**Last Updated**: 2024-10-14
**Maintained By**: Cloud Architecture Team
