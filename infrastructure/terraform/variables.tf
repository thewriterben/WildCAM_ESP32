# Variables for Multi-Cloud WildCAM ESP32 Infrastructure

# Global Variables
variable "environment" {
  description = "Environment name (dev, staging, production)"
  type        = string
  validation {
    condition     = contains(["dev", "staging", "production"], var.environment)
    error_message = "Environment must be dev, staging, or production."
  }
}

variable "enable_multi_region" {
  description = "Enable multi-region deployment for high availability"
  type        = bool
  default     = true
}

variable "domain_name" {
  description = "Primary domain name for the platform"
  type        = string
  default     = "wildcam.example.com"
}

# AWS Configuration
variable "aws_region" {
  description = "Primary AWS region"
  type        = string
  default     = "us-west-2"
}

variable "aws_vpc_cidr" {
  description = "CIDR block for AWS VPC"
  type        = string
  default     = "10.0.0.0/16"
}

variable "aws_availability_zones" {
  description = "Availability zones for AWS deployment"
  type        = list(string)
  default     = ["us-west-2a", "us-west-2b", "us-west-2c"]
}

variable "eks_cluster_version" {
  description = "Kubernetes version for EKS cluster"
  type        = string
  default     = "1.28"
}

variable "eks_node_groups" {
  description = "EKS node group configurations"
  type = map(object({
    instance_types = list(string)
    desired_size   = number
    min_size       = number
    max_size       = number
  }))
  default = {
    general = {
      instance_types = ["t3.medium", "t3a.medium"]
      desired_size   = 3
      min_size       = 2
      max_size       = 10
    }
    compute = {
      instance_types = ["c5.xlarge", "c5a.xlarge"]
      desired_size   = 2
      min_size       = 1
      max_size       = 20
    }
  }
}

variable "rds_instance_class" {
  description = "RDS instance class"
  type        = string
  default     = "db.r6g.large"
}

variable "rds_backup_retention" {
  description = "RDS backup retention period in days"
  type        = number
  default     = 30
}

variable "s3_lifecycle_rules" {
  description = "S3 lifecycle rules for cost optimization"
  type = list(object({
    id      = string
    enabled = bool
    transitions = list(object({
      days          = number
      storage_class = string
    }))
  }))
  default = [
    {
      id      = "archive-old-data"
      enabled = true
      transitions = [
        {
          days          = 90
          storage_class = "GLACIER"
        },
        {
          days          = 365
          storage_class = "DEEP_ARCHIVE"
        }
      ]
    }
  ]
}

variable "enable_versioning" {
  description = "Enable S3 bucket versioning"
  type        = bool
  default     = true
}

# Azure Configuration
variable "azure_subscription_id" {
  description = "Azure subscription ID"
  type        = string
  sensitive   = true
}

variable "azure_region" {
  description = "Primary Azure region"
  type        = string
  default     = "West US 2"
}

variable "azure_vnet_address_space" {
  description = "Address space for Azure VNet"
  type        = list(string)
  default     = ["10.1.0.0/16"]
}

variable "aks_kubernetes_version" {
  description = "Kubernetes version for AKS cluster"
  type        = string
  default     = "1.28"
}

variable "aks_node_pools" {
  description = "AKS node pool configurations"
  type = map(object({
    vm_size     = string
    node_count  = number
    min_count   = number
    max_count   = number
  }))
  default = {
    system = {
      vm_size    = "Standard_D4s_v3"
      node_count = 3
      min_count  = 2
      max_count  = 5
    }
    user = {
      vm_size    = "Standard_D8s_v3"
      node_count = 2
      min_count  = 1
      max_count  = 10
    }
  }
}

variable "postgres_sku_name" {
  description = "Azure PostgreSQL SKU name"
  type        = string
  default     = "GP_Standard_D4s_v3"
}

variable "postgres_backup_retention" {
  description = "PostgreSQL backup retention in days"
  type        = number
  default     = 30
}

variable "storage_replication_type" {
  description = "Azure storage replication type"
  type        = string
  default     = "GRS"  # Geo-redundant storage
}

# GCP Configuration
variable "gcp_project_id" {
  description = "GCP project ID"
  type        = string
}

variable "gcp_region" {
  description = "Primary GCP region"
  type        = string
  default     = "us-west1"
}

variable "gcp_vpc_subnet_cidr" {
  description = "CIDR block for GCP VPC subnet"
  type        = string
  default     = "10.2.0.0/16"
}

variable "gke_cluster_version" {
  description = "Kubernetes version for GKE cluster"
  type        = string
  default     = "1.28"
}

variable "gke_node_pools" {
  description = "GKE node pool configurations"
  type = map(object({
    machine_type = string
    node_count   = number
    min_count    = number
    max_count    = number
  }))
  default = {
    default = {
      machine_type = "n2-standard-4"
      node_count   = 3
      min_count    = 2
      max_count    = 10
    }
    compute = {
      machine_type = "c2-standard-8"
      node_count   = 2
      min_count    = 1
      max_count    = 20
    }
  }
}

variable "cloudsql_tier" {
  description = "Cloud SQL tier"
  type        = string
  default     = "db-n1-standard-4"
}

variable "cloudsql_backup_enabled" {
  description = "Enable Cloud SQL backups"
  type        = bool
  default     = true
}

variable "storage_class" {
  description = "GCS storage class"
  type        = string
  default     = "MULTI_REGIONAL"
}

# Networking
variable "enable_vpn_mesh" {
  description = "Enable VPN mesh networking between clouds"
  type        = bool
  default     = true
}

# Security
variable "enable_zero_trust" {
  description = "Enable zero-trust security architecture"
  type        = bool
  default     = true
}

variable "enable_waf" {
  description = "Enable Web Application Firewall"
  type        = bool
  default     = true
}

variable "enable_ddos_protection" {
  description = "Enable DDoS protection"
  type        = bool
  default     = true
}

variable "encryption_key_rotation_days" {
  description = "Days between encryption key rotation"
  type        = number
  default     = 90
}

# Monitoring
variable "enable_prometheus" {
  description = "Enable Prometheus monitoring"
  type        = bool
  default     = true
}

variable "enable_grafana" {
  description = "Enable Grafana dashboards"
  type        = bool
  default     = true
}

variable "enable_jaeger" {
  description = "Enable Jaeger distributed tracing"
  type        = bool
  default     = true
}

variable "enable_elk_stack" {
  description = "Enable ELK stack for log aggregation"
  type        = bool
  default     = true
}

variable "pagerduty_api_key" {
  description = "PagerDuty API key for alerting"
  type        = string
  sensitive   = true
  default     = ""
}

variable "alert_email" {
  description = "Email address for alerts"
  type        = string
  default     = "alerts@wildcam.example.com"
}

# Database
variable "enable_multi_master" {
  description = "Enable multi-master database replication"
  type        = bool
  default     = true
}

variable "backup_retention_days" {
  description = "Database backup retention in days"
  type        = number
  default     = 30
}

variable "backup_window" {
  description = "Preferred backup window"
  type        = string
  default     = "03:00-04:00"
}
