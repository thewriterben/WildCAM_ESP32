# Multi-Cloud Cost Optimization Guide for WildCAM ESP32

## Overview

This guide provides strategies and tools for optimizing costs across AWS, Azure, and Google Cloud Platform while maintaining high availability and performance for the WildCAM ESP32 wildlife monitoring platform.

## Current Cost Baseline

### Monthly Infrastructure Costs (Production)

```
AWS: $15,000/month
├── EKS Control Plane: $219
├── EC2 Instances: $6,000
├── RDS PostgreSQL: $3,000
├── S3 Storage: $1,500
├── Data Transfer: $2,000
├── CloudFront CDN: $1,000
├── CloudWatch: $500
├── KMS & Secrets: $200
├── VPN Gateway: $300
└── Other Services: $281

Azure: $12,000/month
├── AKS Control Plane: $0 (Free)
├── Virtual Machines: $5,000
├── PostgreSQL Flexible: $2,500
├── Blob Storage: $1,000
├── Data Transfer: $1,500
├── CDN: $800
├── Log Analytics: $400
├── Key Vault: $100
├── VPN Gateway: $400
└── Other Services: $300

GCP: $13,000/month
├── GKE Control Plane: $219
├── Compute Engine: $5,500
├── Cloud SQL: $3,000
├── Cloud Storage: $1,000
├── Data Transfer: $1,500
├── Cloud CDN: $700
├── Operations Suite: $400
├── KMS & Secrets: $150
├── VPN Gateway: $300
└── Other Services: $231

TOTAL: $40,000/month ($480,000/year)
```

## Cost Optimization Strategies

### 1. Compute Optimization

#### Use Reserved Instances / Savings Plans

**AWS**:
```bash
# Analyze usage patterns
aws ce get-reservation-coverage \
  --time-period Start=2024-01-01,End=2024-10-14 \
  --granularity MONTHLY

# Purchase Compute Savings Plan (1 year, All Upfront)
# Expected Savings: 40% = $2,400/month
```

**Azure**:
```bash
# Review reservation recommendations
az consumption reservation recommendation list \
  --resource-group wildcam-production-rg

# Purchase VM reservations (1 year)
# Expected Savings: 40% = $2,000/month
```

**GCP**:
```bash
# Analyze commitment recommendations
gcloud compute commitments describe-resources \
  --region us-west1

# Purchase committed use discount (1 year)
# Expected Savings: 37% = $2,035/month
```

**Total Compute Savings**: ~$6,435/month ($77,220/year)

#### Leverage Spot/Preemptible Instances

Use for batch processing, ML training, and non-critical workloads:

**AWS Spot Instances**:
```yaml
# eks-spot-nodegroup.yaml
apiVersion: eksctl.io/v1alpha5
kind: ClusterConfig
metadata:
  name: wildcam-production-eks
nodeGroups:
  - name: batch-processing
    instancesDistribution:
      maxPrice: 0.5
      instanceTypes:
        - c5.2xlarge
        - c5a.2xlarge
        - c5n.2xlarge
      onDemandBaseCapacity: 0
      onDemandPercentageAboveBaseCapacity: 0
      spotInstancePools: 3
    minSize: 0
    maxSize: 20
    desiredCapacity: 5
    labels:
      workload: batch
```

**Expected Savings**: 60-70% on batch workloads = $1,200/month

#### Auto-Scaling Configuration

Optimize for actual usage patterns:

```yaml
# hpa-optimized.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: backend-hpa
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: wildcam-backend
  minReplicas: 3  # Reduced from 5 during off-peak
  maxReplicas: 20
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 75  # Increased from 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 85  # Increased from 80
  behavior:
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 50
        periodSeconds: 60
```

**Expected Savings**: 20% during off-peak hours = $1,000/month

### 2. Storage Optimization

#### Implement Lifecycle Policies

**AWS S3**:
```bash
aws s3api put-bucket-lifecycle-configuration \
  --bucket wildcam-wildlife-data-aws \
  --lifecycle-configuration '{
    "Rules": [
      {
        "Id": "images-lifecycle",
        "Status": "Enabled",
        "Filter": {"Prefix": "images/"},
        "Transitions": [
          {
            "Days": 30,
            "StorageClass": "INTELLIGENT_TIERING"
          },
          {
            "Days": 90,
            "StorageClass": "GLACIER"
          },
          {
            "Days": 365,
            "StorageClass": "DEEP_ARCHIVE"
          }
        ]
      }
    ]
  }'
```

**Storage Cost Comparison**:
```
Standard: $0.023/GB
Intelligent Tiering: $0.021/GB (auto-optimize)
Glacier: $0.004/GB (90-day retrieval)
Deep Archive: $0.00099/GB (12-hour retrieval)

100TB Storage:
- All Standard: $2,300/month
- Optimized: ~$800/month
- Savings: $1,500/month
```

**Total Storage Savings**: ~$4,500/month across all clouds

#### Delete Unnecessary Data

```bash
# Identify unused snapshots
aws ec2 describe-snapshots \
  --owner-ids self \
  --query 'Snapshots[?StartTime<=`2024-01-01`]' \
  --output table

# Delete old EBS snapshots (>90 days, not tagged)
aws ec2 describe-snapshots \
  --owner-ids self \
  --filters "Name=tag:Keep,Values=true" \
  --query 'Snapshots[?StartTime<=`2024-07-01`].SnapshotId' \
  --output text | \
xargs -n 1 aws ec2 delete-snapshot --snapshot-id

# Expected Savings: $200/month
```

### 3. Network Optimization

#### Reduce Cross-Region Data Transfer

**Strategy**: Intelligent routing to minimize data egress

```python
# intelligent_router.py
def route_request(user_location, cloud_providers):
    """Route to nearest cloud with data"""
    distances = {
        'aws': calculate_distance(user_location, 'us-west-2'),
        'azure': calculate_distance(user_location, 'westus2'),
        'gcp': calculate_distance(user_location, 'us-west1')
    }
    return min(distances, key=distances.get)
```

**Implement CDN Caching**:
```
Before: Direct S3 access
- Data Transfer: $0.09/GB out
- 50TB/month = $4,500

After: CloudFront CDN
- CDN Transfer: $0.085/GB (cheaper)
- Cache Hit Ratio: 85%
- Effective Transfer: 7.5TB from S3
- Cost: $675 + $638 (CDN) = $1,313
- Savings: $3,187/month
```

**Total Network Savings**: ~$6,000/month across all clouds

#### Optimize VPN Architecture

```
Current: Full mesh VPN between all clouds
- AWS-Azure VPN: $300/month
- AWS-GCP VPN: $300/month
- Azure-GCP VPN: $300/month
- Total: $900/month

Optimized: Hub-and-spoke with primary in AWS
- AWS as hub: $600/month
- Savings: $300/month
```

### 4. Database Optimization

#### Right-Size Database Instances

**Analysis**:
```sql
-- Check actual resource usage
SELECT 
    datname,
    avg(numbackends) as avg_connections,
    max(numbackends) as max_connections
FROM pg_stat_database
WHERE datname = 'wildcam'
GROUP BY datname;

-- Current: db.r6g.xlarge (4 vCPU, 32GB RAM)
-- Avg Usage: 40% CPU, 50% Memory
-- Recommendation: db.r6g.large (2 vCPU, 16GB RAM)
-- Savings: 50% = $1,500/month
```

#### Implement Read Replicas Efficiently

```
Current Setup:
- 1 Primary + 2 Read Replicas = $4,500/month

Optimized:
- 1 Primary + 1 Read Replica = $3,000/month
- Use connection pooling (PgBouncer)
- Implement query caching (Redis)
- Savings: $1,500/month
```

**Total Database Savings**: ~$4,500/month across all clouds

### 5. Monitoring and Observability

#### Optimize Log Retention

```bash
# Current: 90 days retention for all logs
# Optimized: Tiered retention

# Critical logs: 90 days
# Application logs: 30 days
# Debug logs: 7 days

# AWS CloudWatch
aws logs put-retention-policy \
  --log-group-name /aws/eks/wildcam-production/application \
  --retention-in-days 30

# Expected Savings: $300/month
```

#### Reduce Metrics Resolution

```yaml
# prometheus-config.yaml
global:
  scrape_interval: 60s  # Increased from 15s
  evaluation_interval: 60s

# Storage reduction: 75%
# Savings: $400/month
```

**Total Monitoring Savings**: ~$1,200/month across all clouds

### 6. Tag-Based Cost Allocation

Implement comprehensive tagging:

```bash
# Tag all resources for cost tracking
aws resourcegroupstaggingapi tag-resources \
  --resource-arn-list $(aws ec2 describe-instances --query 'Reservations[].Instances[].InstanceId' --output text) \
  --tags \
    Project=WildCAM \
    Environment=Production \
    CostCenter=Engineering \
    Owner=cloud-team@wildcam.com
```

**Tag Strategy**:
```
Required Tags:
- Project: WildCAM
- Environment: (dev/staging/production)
- CostCenter: (Engineering/Research/Operations)
- Owner: (team email)
- Application: (backend/frontend/database)

Optional Tags:
- Criticality: (high/medium/low)
- AutoShutdown: (yes/no)
- BackupPolicy: (daily/weekly/none)
```

### 7. Automated Cost Anomaly Detection

**AWS Cost Anomaly Detection**:
```bash
# Create cost anomaly monitor
aws ce create-anomaly-monitor \
  --anomaly-monitor '{
    "MonitorName": "WildCAM-Cost-Monitor",
    "MonitorType": "DIMENSIONAL",
    "MonitorDimension": "SERVICE"
  }'

# Create alert subscription
aws ce create-anomaly-subscription \
  --anomaly-subscription '{
    "SubscriptionName": "WildCAM-Alerts",
    "Threshold": 1000,
    "Frequency": "DAILY",
    "MonitorArnList": ["arn:aws:ce::123456789012:anomalymonitor/..."],
    "Subscribers": [
      {
        "Type": "EMAIL",
        "Address": "cloud-costs@wildcam.com"
      }
    ]
  }'
```

## Cost Optimization Roadmap

### Immediate (0-30 days) - $10,000/month savings

- [x] Implement auto-scaling policies
- [x] Configure storage lifecycle rules
- [ ] Right-size database instances
- [ ] Enable CDN caching
- [ ] Optimize log retention

### Short-term (1-3 months) - $8,000/month additional

- [ ] Purchase reserved instances (1 year)
- [ ] Implement spot instances for batch
- [ ] Consolidate VPN architecture
- [ ] Reduce monitoring metrics
- [ ] Optimize read replica configuration

### Medium-term (3-6 months) - $5,000/month additional

- [ ] Migrate to ARM-based instances (Graviton, Ampere)
- [ ] Implement cross-region data replication optimization
- [ ] Consolidate backup storage
- [ ] Implement resource scheduling (dev/staging)
- [ ] Optimize Kubernetes cluster configurations

### Long-term (6-12 months) - $4,000/month additional

- [ ] Purchase 3-year reserved instances
- [ ] Implement FinOps culture and training
- [ ] Develop custom cost dashboards
- [ ] Negotiate enterprise discounts
- [ ] Evaluate alternative cloud providers for specific workloads

**Total Potential Savings**: $27,000/month ($324,000/year)
**Optimized Monthly Cost**: $13,000/month (67.5% reduction)

## Cost Monitoring Tools

### Cloud-Native Tools

**AWS Cost Explorer**:
```bash
# Generate monthly cost report
aws ce get-cost-and-usage \
  --time-period Start=2024-10-01,End=2024-10-31 \
  --granularity MONTHLY \
  --metrics BlendedCost UsageQuantity \
  --group-by Type=TAG,Key=Environment
```

**Azure Cost Management**:
```bash
# View cost analysis
az consumption usage list \
  --start-date 2024-10-01 \
  --end-date 2024-10-31 \
  --top 10
```

**GCP Billing**:
```bash
# Export billing data to BigQuery
gcloud billing accounts list
gcloud beta billing accounts get-iam-policy BILLING_ACCOUNT_ID
```

### Third-Party Tools

- **CloudHealth**: Multi-cloud cost management
- **Spot.io**: Automated spot instance management
- **Kubecost**: Kubernetes cost allocation
- **Harness**: Cloud cost optimization and governance

## Best Practices

### 1. Regular Cost Reviews
- Weekly: Review cost anomalies
- Monthly: Department cost allocation
- Quarterly: Optimization opportunities
- Annually: Reserved instance renewals

### 2. Showback/Chargeback
```yaml
# kubecost-values.yaml
kubecostProductConfigs:
  clusterName: wildcam-production
  productKey:
    key: <your-key>
  pricing:
    costModel:
      description: "WildCAM Cost Model"
      CPU: 0.031611
      RAM: 0.004237
      storage: 0.00005479
```

### 3. Cost Awareness Training
- Monthly FinOps training for engineers
- Cost impact reviews in design docs
- Budget alerts to team leads
- Cost optimization KPIs

### 4. Continuous Optimization
```bash
# Automated weekly cost optimization checks
#!/bin/bash
# cost-optimization-check.sh

echo "Running cost optimization checks..."

# Check for idle resources
idle_instances=$(aws ec2 describe-instances \
  --filters "Name=instance-state-name,Values=running" \
  --query 'Reservations[].Instances[?CpuOptions.CoreCount<=2 && LaunchTime<=`2024-10-07`]' \
  --output json | jq length)

echo "Found $idle_instances potentially idle instances"

# Check for unattached volumes
unattached=$(aws ec2 describe-volumes \
  --filters "Name=status,Values=available" \
  --query 'Volumes[*].[VolumeId,Size]' \
  --output table)

echo "Unattached volumes:"
echo "$unattached"

# Check for old snapshots
old_snapshots=$(aws ec2 describe-snapshots \
  --owner-ids self \
  --query 'Snapshots[?StartTime<=`2024-01-01`]' | jq length)

echo "Found $old_snapshots old snapshots (>9 months)"
```

## Cost Allocation by Workload

```
Backend Services: $18,000/month (45%)
├── Compute: $10,000
├── Database: $6,000
└── Storage: $2,000

Frontend Services: $6,000/month (15%)
├── Compute: $4,000
├── CDN: $1,500
└── Storage: $500

Data Pipeline: $8,000/month (20%)
├── Batch Processing: $5,000
├── Message Queue: $2,000
└── ETL Jobs: $1,000

Monitoring: $3,000/month (7.5%)
├── Metrics: $1,500
├── Logs: $1,000
└── Tracing: $500

Network: $3,000/month (7.5%)
├── Data Transfer: $2,000
├── Load Balancers: $600
└── VPN: $400

Other: $2,000/month (5%)
```

## Success Metrics

Track these KPIs monthly:

- **Cost per transaction**: Target < $0.001
- **Cost per active user**: Target < $0.50
- **Infrastructure efficiency**: Target > 80%
- **Waste percentage**: Target < 10%
- **Budget variance**: Target ± 5%

---

**Last Updated**: 2024-10-14
**Version**: 1.0.0
**Maintained By**: FinOps Team
