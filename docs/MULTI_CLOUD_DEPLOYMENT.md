# Multi-Cloud Deployment Guide for WildCAM ESP32

## Overview

This guide provides comprehensive instructions for deploying the WildCAM ESP32 platform across multiple cloud providers (AWS, Azure, and Google Cloud Platform) to ensure high availability, global scalability, and disaster resilience.

## Architecture Overview

### Core Components

- **Multi-Cloud Infrastructure**: Simultaneous deployment across AWS, Azure, and GCP
- **Kubernetes Federation**: Unified cluster management across clouds
- **Service Mesh (Istio)**: Cross-cloud communication and traffic management
- **GitOps (ArgoCD)**: Automated, declarative deployments
- **Monitoring Stack**: Unified observability across all clouds

### Geographic Distribution

```
Global Infrastructure
├── AWS (us-west-2, us-east-1, eu-west-1, ap-southeast-1)
├── Azure (West US 2, East US 2, West Europe, Southeast Asia)
└── GCP (us-west1, us-east1, europe-west1, asia-southeast1)
```

## Prerequisites

### Required Tools

1. **Terraform** >= 1.5.0
   ```bash
   curl -fsSL https://apt.releases.hashicorp.com/gpg | sudo apt-key add -
   sudo apt-add-repository "deb [arch=amd64] https://apt.releases.hashicorp.com $(lsb_release -cs) main"
   sudo apt-get update && sudo apt-get install terraform
   ```

2. **kubectl** >= 1.28
   ```bash
   curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
   sudo install -o root -g root -m 0755 kubectl /usr/local/bin/kubectl
   ```

3. **Helm** >= 3.12
   ```bash
   curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash
   ```

4. **ArgoCD CLI** >= 2.8
   ```bash
   curl -sSL -o argocd-linux-amd64 https://github.com/argoproj/argo-cd/releases/latest/download/argocd-linux-amd64
   sudo install -m 555 argocd-linux-amd64 /usr/local/bin/argocd
   ```

5. **Cloud CLIs**
   ```bash
   # AWS CLI
   curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
   unzip awscliv2.zip && sudo ./aws/install
   
   # Azure CLI
   curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash
   
   # GCP CLI
   curl https://sdk.cloud.google.com | bash
   exec -l $SHELL
   gcloud init
   ```

### Cloud Provider Setup

#### AWS Setup

1. **Create IAM User**
   ```bash
   aws iam create-user --user-name wildcam-terraform
   aws iam attach-user-policy --user-name wildcam-terraform \
     --policy-arn arn:aws:iam::aws:policy/AdministratorAccess
   aws iam create-access-key --user-name wildcam-terraform
   ```

2. **Configure Credentials**
   ```bash
   aws configure
   # Enter Access Key ID
   # Enter Secret Access Key
   # Default region: us-west-2
   # Default output format: json
   ```

#### Azure Setup

1. **Login to Azure**
   ```bash
   az login
   az account list --output table
   az account set --subscription "YOUR_SUBSCRIPTION_ID"
   ```

2. **Create Service Principal**
   ```bash
   az ad sp create-for-rbac --name wildcam-terraform --role Contributor
   # Save the output credentials
   ```

#### GCP Setup

1. **Login to GCP**
   ```bash
   gcloud auth login
   gcloud projects list
   gcloud config set project YOUR_PROJECT_ID
   ```

2. **Enable Required APIs**
   ```bash
   gcloud services enable compute.googleapis.com
   gcloud services enable container.googleapis.com
   gcloud services enable sqladmin.googleapis.com
   gcloud services enable storage-api.googleapis.com
   gcloud services enable servicenetworking.googleapis.com
   ```

3. **Create Service Account**
   ```bash
   gcloud iam service-accounts create wildcam-terraform \
     --display-name "WildCAM Terraform Service Account"
   
   gcloud projects add-iam-policy-binding YOUR_PROJECT_ID \
     --member serviceAccount:wildcam-terraform@YOUR_PROJECT_ID.iam.gserviceaccount.com \
     --role roles/owner
   
   gcloud iam service-accounts keys create ~/wildcam-gcp-key.json \
     --iam-account wildcam-terraform@YOUR_PROJECT_ID.iam.gserviceaccount.com
   ```

## Infrastructure Deployment

### Step 1: Initialize Terraform

```bash
cd infrastructure/terraform

# Initialize Terraform
terraform init

# Create terraform.tfvars
cat > terraform.tfvars <<EOF
environment = "production"

# AWS Configuration
aws_region = "us-west-2"

# Azure Configuration
azure_subscription_id = "YOUR_AZURE_SUBSCRIPTION_ID"
azure_region = "West US 2"

# GCP Configuration
gcp_project_id = "YOUR_GCP_PROJECT_ID"
gcp_region = "us-west1"

# Domain Configuration
domain_name = "wildcam.example.com"

# Enable features
enable_multi_region = true
enable_vpn_mesh = true
enable_zero_trust = true
enable_waf = true
enable_prometheus = true
enable_grafana = true
enable_jaeger = true
enable_elk_stack = true
EOF
```

### Step 2: Plan Infrastructure

```bash
# Review planned changes
terraform plan -out=tfplan

# Review outputs
terraform show tfplan
```

### Step 3: Deploy Infrastructure

```bash
# Apply infrastructure changes
terraform apply tfplan

# Save outputs for later use
terraform output -json > outputs.json
```

**Deployment Time**: Approximately 30-45 minutes for all clouds

### Step 4: Configure Kubernetes Access

```bash
# AWS EKS
aws eks update-kubeconfig --name wildcam-production-eks --region us-west-2 --alias wildcam-aws

# Azure AKS
az aks get-credentials --resource-group wildcam-production-rg \
  --name wildcam-production-aks --overwrite-existing --context wildcam-azure

# GCP GKE
gcloud container clusters get-credentials wildcam-production-gke \
  --region us-west1 --project YOUR_PROJECT_ID
kubectl config rename-context gke_YOUR_PROJECT_ID_us-west1_wildcam-production-gke wildcam-gcp

# Verify contexts
kubectl config get-contexts
```

## Service Mesh Deployment

### Install Istio on All Clusters

```bash
# Download Istio
curl -L https://istio.io/downloadIstio | ISTIO_VERSION=1.19.0 sh -
cd istio-1.19.0
export PATH=$PWD/bin:$PATH

# Install on AWS cluster
kubectl config use-context wildcam-aws
istioctl install --set profile=production -y
kubectl label namespace default istio-injection=enabled

# Install on Azure cluster
kubectl config use-context wildcam-azure
istioctl install --set profile=production -y
kubectl label namespace default istio-injection=enabled

# Install on GCP cluster
kubectl config use-context wildcam-gcp
istioctl install --set profile=production -y
kubectl label namespace default istio-injection=enabled
```

### Configure Multi-Cluster Service Mesh

```bash
# Create certificates for cross-cluster communication
mkdir -p certs
cd certs

# Generate root CA
openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 \
  -subj '/O=WildCAM/CN=wildcam-root-ca' \
  -keyout root-key.pem -out root-cert.pem

# Create secrets in each cluster
for CONTEXT in wildcam-aws wildcam-azure wildcam-gcp; do
  kubectl create secret generic cacerts -n istio-system \
    --context=${CONTEXT} \
    --from-file=ca-cert.pem=root-cert.pem \
    --from-file=ca-key.pem=root-key.pem \
    --from-file=root-cert.pem=root-cert.pem \
    --from-file=cert-chain.pem=root-cert.pem
done
```

## GitOps with ArgoCD

### Install ArgoCD

```bash
# Install on each cluster
for CONTEXT in wildcam-aws wildcam-azure wildcam-gcp; do
  kubectl config use-context ${CONTEXT}
  kubectl create namespace argocd
  kubectl apply -n argocd -f https://raw.githubusercontent.com/argoproj/argo-cd/stable/manifests/install.yaml
  
  # Wait for ArgoCD to be ready
  kubectl wait --for=condition=available --timeout=300s deployment/argocd-server -n argocd
done
```

### Configure ArgoCD

```bash
# Get initial admin password (for AWS cluster)
kubectl config use-context wildcam-aws
kubectl -n argocd get secret argocd-initial-admin-secret -o jsonpath="{.data.password}" | base64 -d

# Port forward to access UI
kubectl port-forward svc/argocd-server -n argocd 8080:443 &

# Login via CLI
argocd login localhost:8080 --username admin --password <PASSWORD> --insecure

# Add Git repository
argocd repo add https://github.com/thewriterben/WildCAM_ESP32.git
```

### Deploy Applications

```bash
# Create ArgoCD project
kubectl apply -f - <<EOF
apiVersion: argoproj.io/v1alpha1
kind: AppProject
metadata:
  name: wildcam-multicloud
  namespace: argocd
spec:
  description: WildCAM Multi-Cloud Project
  sourceRepos:
    - https://github.com/thewriterben/WildCAM_ESP32.git
  destinations:
    - namespace: '*'
      server: '*'
  clusterResourceWhitelist:
    - group: '*'
      kind: '*'
EOF

# Deploy to all clouds
kubectl apply -f argocd/applications/wildcam-aws.yaml --context wildcam-aws
kubectl apply -f argocd/applications/wildcam-azure.yaml --context wildcam-azure
kubectl apply -f argocd/applications/wildcam-gcp.yaml --context wildcam-gcp

# Monitor deployments
argocd app list
argocd app get wildcam-aws-production
```

## Monitoring and Observability

### Access Monitoring Dashboards

```bash
# Prometheus (AWS cluster)
kubectl port-forward -n monitoring svc/prometheus-server 9090:80 --context wildcam-aws

# Grafana (AWS cluster)
kubectl port-forward -n monitoring svc/grafana 3000:80 --context wildcam-aws
# Default credentials: admin/admin

# Jaeger (AWS cluster)
kubectl port-forward -n monitoring svc/jaeger-query 16686:16686 --context wildcam-aws
```

### Configure Alerts

```bash
# Create alert rules
kubectl apply -f - <<EOF
apiVersion: v1
kind: ConfigMap
metadata:
  name: alertmanager-config
  namespace: monitoring
data:
  alertmanager.yml: |
    global:
      resolve_timeout: 5m
    
    route:
      group_by: ['alertname', 'cluster']
      group_wait: 10s
      group_interval: 10s
      repeat_interval: 12h
      receiver: 'pagerduty'
    
    receivers:
      - name: 'pagerduty'
        pagerduty_configs:
          - service_key: 'YOUR_PAGERDUTY_KEY'
EOF
```

## Data Replication Setup

### Configure Cross-Cloud Database Replication

**Note**: Database replication requires additional manual configuration using tools like:
- AWS Database Migration Service (DMS)
- PostgreSQL Bi-Directional Replication (BDR)
- Custom replication scripts

Example PostgreSQL logical replication setup:

```sql
-- On primary database (AWS RDS)
CREATE PUBLICATION wildcam_pub FOR ALL TABLES;

-- On replica database (Azure PostgreSQL)
CREATE SUBSCRIPTION wildcam_sub
  CONNECTION 'host=aws-rds-endpoint dbname=wildcam user=replication_user password=xxx'
  PUBLICATION wildcam_pub;

-- On replica database (GCP Cloud SQL)
CREATE SUBSCRIPTION wildcam_sub_gcp
  CONNECTION 'host=aws-rds-endpoint dbname=wildcam user=replication_user password=xxx'
  PUBLICATION wildcam_pub;
```

### Configure Object Storage Replication

```bash
# AWS S3 to Azure Blob
aws s3 sync s3://wildcam-wildlife-data-aws/ \
  https://wildcamdata.blob.core.windows.net/wildlife-images/ \
  --source-region us-west-2

# AWS S3 to GCP Cloud Storage
gsutil -m rsync -r s3://wildcam-wildlife-data-aws/ \
  gs://wildcam-wildlife-data-gcp/
```

## Disaster Recovery Testing

### Perform Failover Test

```bash
# Test AWS to Azure failover
kubectl config use-context wildcam-aws
kubectl scale deployment wildcam-backend --replicas=0

# Verify traffic shifts to Azure
kubectl config use-context wildcam-azure
kubectl get pods -w

# Restore AWS deployment
kubectl config use-context wildcam-aws
kubectl scale deployment wildcam-backend --replicas=5
```

### Test Backup and Restore

```bash
# Trigger manual backup
kubectl create job --from=cronjob/database-backup manual-backup-$(date +%s)

# List backups
aws s3 ls s3://wildcam-backups/

# Test restore (to staging environment)
kubectl apply -f - <<EOF
apiVersion: batch/v1
kind: Job
metadata:
  name: restore-test
spec:
  template:
    spec:
      containers:
      - name: restore
        image: postgres:15
        command: ['pg_restore', '-d', 'wildcam_staging', '/backup/latest.dump']
      restartPolicy: Never
EOF
```

## Cost Optimization

### Monitor Costs

```bash
# AWS Cost Explorer
aws ce get-cost-and-usage --time-period Start=2024-01-01,End=2024-01-31 \
  --granularity MONTHLY --metrics BlendedCost

# Azure Cost Management
az consumption usage list --start-date 2024-01-01 --end-date 2024-01-31

# GCP Billing
gcloud billing accounts list
gcloud billing projects describe YOUR_PROJECT_ID
```

### Implement Cost Savings

1. **Use Reserved Instances** for predictable workloads
2. **Enable Auto-scaling** to match demand
3. **Use Spot/Preemptible Instances** for batch processing
4. **Implement lifecycle policies** for storage
5. **Right-size resources** based on monitoring data

## Security Best Practices

### Enable Encryption

```bash
# Verify encryption at rest
kubectl get storageclass -o yaml | grep encrypted

# Enable encryption in transit (mTLS)
kubectl apply -f - <<EOF
apiVersion: security.istio.io/v1beta1
kind: PeerAuthentication
metadata:
  name: default
  namespace: istio-system
spec:
  mtls:
    mode: STRICT
EOF
```

### Implement Network Policies

```bash
kubectl apply -f - <<EOF
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: deny-all
  namespace: default
spec:
  podSelector: {}
  policyTypes:
  - Ingress
  - Egress
EOF
```

## Troubleshooting

### Common Issues

1. **Cross-cluster communication failure**
   ```bash
   # Verify Istio mesh connectivity
   istioctl proxy-status
   kubectl logs -n istio-system deployment/istiod
   ```

2. **Database replication lag**
   ```sql
   SELECT * FROM pg_stat_replication;
   ```

3. **High latency between regions**
   ```bash
   # Test network latency
   kubectl run -it --rm debug --image=busybox --restart=Never -- sh
   ping azure-endpoint.westus2.cloudapp.azure.com
   ```

## Maintenance Windows

### Rolling Updates

```bash
# Update application version
argocd app set wildcam-aws-production --helm-set backend.image.tag=v2.1.0
argocd app sync wildcam-aws-production

# Monitor rollout
kubectl rollout status deployment/wildcam-backend
```

### Cluster Upgrades

```bash
# AWS EKS
aws eks update-cluster-version --name wildcam-production-eks --kubernetes-version 1.29

# Azure AKS
az aks upgrade --resource-group wildcam-production-rg \
  --name wildcam-production-aks --kubernetes-version 1.29

# GCP GKE
gcloud container clusters upgrade wildcam-production-gke \
  --master --cluster-version 1.29
```

## Support and Documentation

- **Documentation**: https://github.com/thewriterben/WildCAM_ESP32/tree/main/docs
- **Issue Tracker**: https://github.com/thewriterben/WildCAM_ESP32/issues
- **Community**: https://discord.gg/wildcam

## Next Steps

1. Review and customize configuration files
2. Set up monitoring alerts and dashboards
3. Configure backup schedules
4. Perform disaster recovery drills
5. Optimize costs based on usage patterns
6. Scale resources as needed for growth

---

**Last Updated**: 2024-10-14
**Version**: 1.0.0
