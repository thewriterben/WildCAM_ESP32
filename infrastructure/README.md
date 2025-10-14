# WildCAM ESP32 Multi-Cloud Infrastructure

## Overview

This directory contains the infrastructure as code (IaC) for deploying WildCAM ESP32 across multiple cloud providers (AWS, Azure, and Google Cloud Platform) with intelligent traffic routing, automated failover, and comprehensive monitoring.

## Architecture

### High-Level Design

```
┌──────────────────────────────────────────────────────────────┐
│                    Global Traffic Manager                     │
│            (Route53 + CloudFront + Azure FrontDoor)          │
└────────────┬─────────────────┬─────────────────┬────────────┘
             │                 │                 │
    ┌────────▼────────┐ ┌─────▼────────┐ ┌─────▼─────────┐
    │   AWS Region    │ │ Azure Region │ │  GCP Region   │
    │   us-west-2     │ │   westus2    │ │  us-west1     │
    │                 │ │              │ │               │
    │ • EKS Cluster   │ │ • AKS Cluster│ │ • GKE Cluster │
    │ • RDS           │ │ • PostgreSQL │ │ • Cloud SQL   │
    │ • S3            │ │ • Blob Store │ │ • GCS         │
    └─────────────────┘ └──────────────┘ └───────────────┘
```

### Key Features

- **99.99% Uptime**: Multi-cloud redundancy with automatic failover
- **Global Distribution**: 6 regions across 3 cloud providers
- **Intelligent Routing**: Latency-based DNS routing to nearest healthy endpoint
- **Cost Optimization**: Budget tracking and automated resource optimization
- **Security**: End-to-end encryption, HashiCorp Vault, certificate management
- **Observability**: Unified monitoring with Prometheus, Grafana, Jaeger

## Directory Structure

```
infrastructure/
├── terraform/                 # Terraform IaC
│   ├── main.tf               # Main configuration
│   ├── aws.tf                # AWS resources
│   ├── azure.tf              # Azure resources
│   ├── gcp.tf                # GCP resources
│   ├── traffic-management.tf # Load balancing & DNS
│   ├── data-sync.tf          # Data replication
│   ├── monitoring.tf         # Observability stack
│   ├── security.tf           # Security & compliance
│   └── aws/                  # AWS module
│       └── main.tf
├── kubernetes-federation/     # K8s federation configs
│   └── argocd-config.yaml    # ArgoCD GitOps
├── monitoring/               # Monitoring configs
└── scripts/                  # Deployment scripts
    └── deploy-multi-cloud.sh # Main deployment script
```

## Prerequisites

### Required Tools

Install the following tools before deployment:

```bash
# Terraform (>= 1.5.0)
brew install terraform  # macOS
# or
curl -fsSL https://apt.releases.hashicorp.com/gpg | sudo apt-key add -
sudo apt-add-repository "deb [arch=amd64] https://apt.releases.hashicorp.com $(lsb_release -cs) main"
sudo apt-get update && sudo apt-get install terraform

# kubectl (>= 1.28)
brew install kubectl  # macOS
# or
curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
chmod +x kubectl && sudo mv kubectl /usr/local/bin/

# AWS CLI
brew install awscli
# or
pip install awscli

# Azure CLI
brew install azure-cli
# or
curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

# Google Cloud SDK
brew install google-cloud-sdk
# or
curl https://sdk.cloud.google.com | bash

# Helm
brew install helm
# or
curl https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | bash

# ArgoCD CLI (optional but recommended)
brew install argocd
# or
curl -sSL -o /usr/local/bin/argocd https://github.com/argoproj/argo-cd/releases/latest/download/argocd-linux-amd64
chmod +x /usr/local/bin/argocd
```

### Cloud Accounts

You'll need accounts with:
- AWS (with billing enabled)
- Microsoft Azure (subscription required)
- Google Cloud Platform (project with billing enabled)

### Permissions

Ensure you have admin-level permissions on all three cloud providers:
- AWS: AdministratorAccess or equivalent
- Azure: Owner or Contributor role
- GCP: Owner or Editor role

## Quick Start

### 1. Clone Repository

```bash
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/infrastructure
```

### 2. Configure Cloud Credentials

#### AWS
```bash
aws configure
# Enter AWS Access Key ID
# Enter AWS Secret Access Key
# Default region: us-west-2
# Default output format: json
```

#### Azure
```bash
az login
# Follow browser authentication
az account list
az account set --subscription "Your Subscription Name"
```

#### GCP
```bash
gcloud auth login
# Follow browser authentication
gcloud config set project wildcam-production
gcloud auth application-default login
```

### 3. Configure Terraform Backend (Optional but Recommended)

Create an S3 bucket for Terraform state:

```bash
aws s3 mb s3://wildcam-terraform-state --region us-west-2
aws s3api put-bucket-versioning \
  --bucket wildcam-terraform-state \
  --versioning-configuration Status=Enabled
aws dynamodb create-table \
  --table-name wildcam-terraform-locks \
  --attribute-definitions AttributeName=LockID,AttributeType=S \
  --key-schema AttributeName=LockID,KeyType=HASH \
  --billing-mode PAY_PER_REQUEST
```

### 4. Initialize and Deploy

```bash
# Run automated deployment script
./scripts/deploy-multi-cloud.sh

# Or deploy manually:
cd terraform
terraform init
terraform plan -out=tfplan
terraform apply tfplan
```

### 5. Configure kubectl Contexts

```bash
# AWS
aws eks update-kubeconfig --name wildcam-eks-production --region us-west-2 --alias aws-west
aws eks update-kubeconfig --name wildcam-eks-production-secondary --region us-east-1 --alias aws-east

# Azure
az aks get-credentials --resource-group wildcam-rg --name wildcam-aks-production --context azure-west
az aks get-credentials --resource-group wildcam-rg --name wildcam-aks-production-secondary --context azure-east

# GCP
gcloud container clusters get-credentials wildcam-gke-production --region us-west1
kubectl config rename-context <current-name> gcp-west
gcloud container clusters get-credentials wildcam-gke-production-secondary --region us-east1
kubectl config rename-context <current-name> gcp-east
```

### 6. Verify Deployment

```bash
# Check all clusters
for ctx in aws-west aws-east azure-west azure-east gcp-west gcp-east; do
  echo "=== Cluster: $ctx ==="
  kubectl --context $ctx get nodes
  kubectl --context $ctx get pods --all-namespaces
done
```

## Configuration

### Variables

Key variables in `terraform/main.tf`:

```hcl
variable "project_name" {
  default = "wildcam"
}

variable "environment" {
  default = "production"
}

variable "monthly_cost_budget" {
  default = 10000  # USD
}

variable "enable_aws" {
  default = true
}

variable "enable_azure" {
  default = true
}

variable "enable_gcp" {
  default = true
}
```

### Customization

Create a `terraform.tfvars` file:

```hcl
project_name = "my-wildlife-project"
environment  = "production"

# AWS Configuration
aws_region           = "us-west-2"
aws_secondary_region = "us-east-1"

# Azure Configuration
azure_location           = "westus2"
azure_secondary_location = "eastus"

# GCP Configuration
gcp_region           = "us-west1"
gcp_secondary_region = "us-east1"

# Cost Management
monthly_cost_budget = 10000  # USD per month

# Kubernetes
kubernetes_version = "1.28"
node_count        = 3
```

## Cost Management

### Budget Tracking

The infrastructure includes automated cost tracking:
- 80% budget threshold triggers warnings
- 100% budget threshold triggers critical alerts
- Costs are distributed: 33% AWS, 33% Azure, 34% GCP

### View Current Spending

```bash
# Using Terraform outputs
terraform output monthly_spending

# Using cloud CLI
aws ce get-cost-and-usage --time-period Start=2024-01-01,End=2024-01-31
az consumption usage list --start-date 2024-01-01 --end-date 2024-01-31
gcloud billing accounts list
```

### Cost Optimization Tips

1. **Use Spot/Preemptible Instances**: 50-80% cost savings for non-critical workloads
2. **Right-size Resources**: Monitor and adjust instance types based on usage
3. **Enable Auto-Scaling**: Scale down during low traffic periods
4. **Storage Lifecycle**: Move old data to cheaper storage tiers
5. **Reserved Capacity**: Purchase 1-3 year commitments for predictable workloads

## Monitoring

### Access Monitoring Dashboards

```bash
# Port forward Grafana
kubectl --context aws-west port-forward -n monitoring svc/grafana 3000:3000

# Access at http://localhost:3000
# Default credentials: admin / prom-operator
```

### Key Metrics

- **Uptime**: Target 99.99%
- **Latency**: p95 < 500ms, p99 < 1s
- **Error Rate**: < 1%
- **Resource Utilization**: CPU < 70%, Memory < 80%

### Alerts

Alerts are sent to:
- Email: ops@wildcam.org
- Slack: #wildcam-ops
- PagerDuty: For critical alerts

## Security

### Secrets Management

All secrets are managed with HashiCorp Vault:

```bash
# Access Vault UI
kubectl --context aws-west port-forward -n security svc/vault 8200:8200

# Login to Vault
export VAULT_ADDR='http://localhost:8200'
vault login
```

### Certificate Management

Certificates are automatically provisioned and renewed via cert-manager with Let's Encrypt.

### Network Security

- All inter-cluster communication encrypted with Istio mTLS
- Network policies enforce pod-to-pod communication rules
- Falco monitors for runtime security threats

## Disaster Recovery

### Backup Strategy

- **Databases**: Automated daily backups with 30-day retention
- **Object Storage**: Versioning enabled with cross-region replication
- **Kubernetes**: GitOps with ArgoCD (infrastructure as code)

### Recovery Procedures

See [Multi-Cloud Runbooks](../docs/MULTI_CLOUD_RUNBOOKS.md) for detailed procedures:
- Complete cloud provider outage
- Database replication failure
- Data loss recovery
- Security incidents

### RTO and RPO

- **Recovery Time Objective (RTO)**: < 5 minutes
- **Recovery Point Objective (RPO)**: < 1 minute

## Maintenance

### Regular Tasks

- **Weekly**: Kubernetes cluster updates
- **Monthly**: Cost optimization review
- **Quarterly**: Security audits and penetration testing
- **Annually**: Disaster recovery drill

### Updating Infrastructure

```bash
# Update Terraform
cd terraform
terraform plan
terraform apply

# Update Kubernetes applications via ArgoCD
# Changes pushed to git are automatically deployed
git add .
git commit -m "Update application configuration"
git push
```

## Troubleshooting

### Common Issues

#### Terraform Apply Fails

```bash
# Check Terraform state
terraform state list

# Refresh state
terraform refresh

# Import existing resources if needed
terraform import aws_eks_cluster.main wildcam-eks-production
```

#### Kubectl Connection Issues

```bash
# Verify credentials
kubectl config view
kubectl config current-context

# Test connection
kubectl cluster-info
kubectl get nodes
```

#### High Costs

```bash
# Identify expensive resources
terraform cost estimate

# Scale down non-critical workloads
kubectl scale deployment <name> --replicas=1

# Review and delete unused resources
kubectl get pv | grep Released
```

## Documentation

- [Multi-Cloud Deployment Guide](../docs/MULTI_CLOUD_DEPLOYMENT.md)
- [Operational Runbooks](../docs/MULTI_CLOUD_RUNBOOKS.md)
- [AWS Deployment Guide](../docs/AWS_DEPLOYMENT_GUIDE.md)
- [Azure Deployment Guide](../docs/AZURE_DEPLOYMENT_GUIDE.md)

## Support

For issues or questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Email: ops@wildcam.org
- Slack: #wildcam-ops

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## License

This infrastructure code is part of the WildCAM ESP32 project and is licensed under the MIT License. See [LICENSE](../LICENSE) for details.

## Acknowledgments

Built with:
- Terraform by HashiCorp
- Kubernetes
- ArgoCD
- Prometheus & Grafana
- Istio Service Mesh
- HashiCorp Vault
