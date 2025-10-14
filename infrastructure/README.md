# WildCAM ESP32 Multi-Cloud Infrastructure

This directory contains Infrastructure as Code (IaC) for deploying the WildCAM ESP32 platform across multiple cloud providers (AWS, Azure, and Google Cloud Platform).

## Directory Structure

```
infrastructure/
├── terraform/              # Terraform configurations
│   ├── main.tf            # Root Terraform configuration
│   ├── variables.tf       # Variable definitions
│   ├── modules/           # Reusable Terraform modules
│   │   ├── aws/          # AWS infrastructure
│   │   ├── azure/        # Azure infrastructure
│   │   ├── gcp/          # GCP infrastructure
│   │   ├── networking/   # Cross-cloud networking
│   │   ├── security/     # Security configurations
│   │   ├── monitoring/   # Monitoring setup
│   │   └── database/     # Database replication
│   └── environments/      # Environment-specific configs
│       ├── dev/
│       ├── staging/
│       └── production/
├── pulumi/                # Pulumi alternative (type-safe IaC)
├── ansible/               # Configuration management
└── policies/              # OPA policies for compliance

```

## Quick Start

### Prerequisites

1. **Install Required Tools**:
   - Terraform >= 1.5.0
   - AWS CLI >= 2.0
   - Azure CLI >= 2.0
   - Google Cloud SDK >= 400.0
   - kubectl >= 1.28
   - helm >= 3.12

2. **Configure Cloud Credentials**:
   ```bash
   # AWS
   aws configure
   
   # Azure
   az login
   az account set --subscription "YOUR_SUBSCRIPTION_ID"
   
   # GCP
   gcloud auth login
   gcloud config set project YOUR_PROJECT_ID
   ```

### Deployment Steps

1. **Initialize Terraform**:
   ```bash
   cd terraform
   terraform init
   ```

2. **Configure Variables**:
   ```bash
   cp environments/production/terraform.tfvars.example terraform.tfvars
   # Edit terraform.tfvars with your values
   ```

3. **Plan Deployment**:
   ```bash
   terraform plan -out=tfplan
   ```

4. **Apply Infrastructure**:
   ```bash
   terraform apply tfplan
   ```

## Infrastructure Modules

### AWS Module
Provisions:
- VPC with public/private subnets across 3 AZs
- EKS cluster with auto-scaling node groups
- RDS PostgreSQL with Multi-AZ deployment
- S3 buckets with lifecycle policies
- CloudFront CDN
- KMS encryption keys

### Azure Module
Provisions:
- Virtual Network with subnets
- AKS cluster with multiple node pools
- PostgreSQL Flexible Server
- Storage Account with geo-replication
- CDN Profile and Endpoint
- Key Vault for secrets

### GCP Module
Provisions:
- VPC Network with custom subnets
- GKE cluster with auto-scaling
- Cloud SQL PostgreSQL
- Cloud Storage buckets
- Cloud CDN
- KMS encryption keys

### Networking Module
Configures:
- VPN mesh between clouds
- Cross-cloud connectivity
- DNS management
- Load balancing

### Security Module
Implements:
- Zero-trust architecture
- WAF rules
- DDoS protection
- SSL/TLS certificates
- Secret rotation

### Monitoring Module
Deploys:
- Prometheus for metrics
- Grafana for visualization
- Jaeger for distributed tracing
- ELK stack for logs
- PagerDuty integration

### Database Module
Configures:
- Multi-master replication
- Automated backups
- Point-in-time recovery
- Cross-cloud synchronization

## Environment Management

### Development
```bash
terraform workspace select dev
terraform apply -var-file=environments/dev/terraform.tfvars
```

### Staging
```bash
terraform workspace select staging
terraform apply -var-file=environments/staging/terraform.tfvars
```

### Production
```bash
terraform workspace select production
terraform apply -var-file=environments/production/terraform.tfvars
```

## State Management

Terraform state is stored in:
- **Backend**: AWS S3
- **State Locking**: DynamoDB
- **Encryption**: AES-256

Configure backend:
```hcl
terraform {
  backend "s3" {
    bucket         = "wildcam-terraform-state"
    key            = "global/terraform.tfstate"
    region         = "us-west-2"
    encrypt        = true
    dynamodb_table = "wildcam-terraform-locks"
  }
}
```

## Security Best Practices

1. **Never commit secrets** - Use environment variables or secret managers
2. **Enable encryption** - All data encrypted at rest and in transit
3. **Use least privilege** - IAM roles with minimum required permissions
4. **Enable audit logging** - CloudTrail, Azure Monitor, Cloud Audit Logs
5. **Regular updates** - Keep Terraform and providers updated
6. **State file protection** - Encrypt and restrict access to state files

## Cost Optimization

### Monitoring Costs
```bash
# AWS
aws ce get-cost-and-usage --time-period Start=2024-10-01,End=2024-10-31 \
  --granularity MONTHLY --metrics BlendedCost

# Azure
az consumption usage list --start-date 2024-10-01 --end-date 2024-10-31

# GCP
gcloud billing accounts list
gcloud billing projects describe YOUR_PROJECT_ID
```

### Cost-Saving Features
- Auto-scaling based on demand
- Spot/Preemptible instances for batch jobs
- Storage lifecycle policies
- Reserved instances for predictable workloads
- Right-sizing based on metrics

## Troubleshooting

### Common Issues

1. **Terraform state lock**:
   ```bash
   terraform force-unlock LOCK_ID
   ```

2. **Provider authentication errors**:
   ```bash
   # Verify credentials
   aws sts get-caller-identity
   az account show
   gcloud auth list
   ```

3. **Resource quota exceeded**:
   ```bash
   # AWS
   aws service-quotas list-service-quotas --service-code vpc
   
   # Azure
   az vm list-usage --location westus2
   
   # GCP
   gcloud compute project-info describe --project=YOUR_PROJECT_ID
   ```

### Getting Help

- **Documentation**: [Multi-Cloud Deployment Guide](../docs/MULTI_CLOUD_DEPLOYMENT.md)
- **Disaster Recovery**: [DR Plan](../docs/DISASTER_RECOVERY.md)
- **Issues**: https://github.com/thewriterben/WildCAM_ESP32/issues
- **Support**: team@wildcam.example.com

## Maintenance

### Regular Tasks

- **Weekly**: Review cost reports
- **Monthly**: Update Terraform providers
- **Quarterly**: DR drills and security audits
- **Annually**: Architecture review

### Updating Infrastructure

```bash
# Update providers
terraform init -upgrade

# Test changes
terraform plan

# Apply updates
terraform apply
```

## Compliance

Infrastructure meets:
- **GDPR**: Data residency and privacy controls
- **CCPA**: California privacy regulations
- **SOC 2**: Security and availability controls
- **ISO 27001**: Information security standards

## Contributing

When modifying infrastructure:

1. Create a feature branch
2. Make changes and test locally
3. Run `terraform fmt` and `terraform validate`
4. Submit pull request with detailed description
5. Wait for peer review and CI checks
6. Deploy to dev → staging → production

## Resources

- [Terraform Documentation](https://www.terraform.io/docs)
- [AWS Best Practices](https://aws.amazon.com/architecture/well-architected/)
- [Azure Architecture Center](https://docs.microsoft.com/en-us/azure/architecture/)
- [GCP Architecture Framework](https://cloud.google.com/architecture/framework)

---

**Maintained by**: Cloud Architecture Team
**Last Updated**: 2024-10-14
**Version**: 1.0.0
