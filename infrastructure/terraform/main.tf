# Multi-Cloud Deployment for WildCAM ESP32 Platform
# Orchestrates deployment across AWS, Azure, and Google Cloud Platform

terraform {
  required_version = ">= 1.5.0"
  
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
    azurerm = {
      source  = "hashicorp/azurerm"
      version = "~> 3.0"
    }
    google = {
      source  = "hashicorp/google"
      version = "~> 5.0"
    }
    kubernetes = {
      source  = "hashicorp/kubernetes"
      version = "~> 2.23"
    }
    helm = {
      source  = "hashicorp/helm"
      version = "~> 2.11"
    }
    random = {
      source  = "hashicorp/random"
      version = "~> 3.5"
    }
  }

  # Backend configuration for state management
  backend "s3" {
    # This will be overridden by backend-config files
    bucket         = "wildcam-terraform-state"
    key            = "global/terraform.tfstate"
    region         = "us-west-2"
    encrypt        = true
    dynamodb_table = "wildcam-terraform-locks"
  }
}

# Provider configurations
provider "aws" {
  region = var.aws_region
  
  default_tags {
    tags = {
      Project     = "WildCAM-ESP32"
      ManagedBy   = "Terraform"
      Environment = var.environment
      Platform    = "Multi-Cloud"
    }
  }
}

provider "azurerm" {
  features {
    resource_group {
      prevent_deletion_if_contains_resources = false
    }
  }
  subscription_id = var.azure_subscription_id
}

provider "google" {
  project = var.gcp_project_id
  region  = var.gcp_region
}

# Generate random suffix for unique resource naming
resource "random_id" "suffix" {
  byte_length = 4
}

# Data sources for existing resources
data "aws_caller_identity" "current" {}
data "azurerm_client_config" "current" {}
data "google_project" "current" {
  project_id = var.gcp_project_id
}

# Local variables
locals {
  project_name    = "wildcam-esp32"
  unique_suffix   = random_id.suffix.hex
  common_tags = {
    Project     = "WildCAM-ESP32"
    Environment = var.environment
    ManagedBy   = "Terraform"
  }
}

# AWS Infrastructure Module
module "aws_infrastructure" {
  source = "./modules/aws"
  
  environment       = var.environment
  project_name      = local.project_name
  aws_region        = var.aws_region
  unique_suffix     = local.unique_suffix
  enable_multi_region = var.enable_multi_region
  
  # Networking
  vpc_cidr          = var.aws_vpc_cidr
  availability_zones = var.aws_availability_zones
  
  # EKS Configuration
  eks_cluster_version = var.eks_cluster_version
  eks_node_groups     = var.eks_node_groups
  
  # Database
  rds_instance_class  = var.rds_instance_class
  rds_backup_retention = var.rds_backup_retention
  
  # Storage
  s3_lifecycle_rules  = var.s3_lifecycle_rules
  enable_versioning   = var.enable_versioning
  
  tags = local.common_tags
}

# Azure Infrastructure Module
module "azure_infrastructure" {
  source = "./modules/azure"
  
  environment       = var.environment
  project_name      = local.project_name
  azure_region      = var.azure_region
  unique_suffix     = local.unique_suffix
  
  # Networking
  vnet_address_space = var.azure_vnet_address_space
  
  # AKS Configuration
  aks_kubernetes_version = var.aks_kubernetes_version
  aks_node_pools        = var.aks_node_pools
  
  # Database
  postgres_sku_name     = var.postgres_sku_name
  postgres_backup_retention = var.postgres_backup_retention
  
  # Storage
  storage_replication_type = var.storage_replication_type
  
  tags = local.common_tags
}

# GCP Infrastructure Module
module "gcp_infrastructure" {
  source = "./modules/gcp"
  
  environment    = var.environment
  project_name   = local.project_name
  gcp_project_id = var.gcp_project_id
  gcp_region     = var.gcp_region
  unique_suffix  = local.unique_suffix
  
  # Networking
  vpc_subnet_cidr = var.gcp_vpc_subnet_cidr
  
  # GKE Configuration
  gke_cluster_version = var.gke_cluster_version
  gke_node_pools     = var.gke_node_pools
  
  # Database
  cloudsql_tier      = var.cloudsql_tier
  cloudsql_backup_enabled = var.cloudsql_backup_enabled
  
  # Storage
  storage_class      = var.storage_class
  
  labels = local.common_tags
}

# Cross-Cloud Networking Module
module "networking" {
  source = "./modules/networking"
  
  environment   = var.environment
  project_name  = local.project_name
  
  # VPN Configuration
  enable_vpn_mesh = var.enable_vpn_mesh
  
  # AWS Networking
  aws_vpc_id    = module.aws_infrastructure.vpc_id
  aws_region    = var.aws_region
  
  # Azure Networking
  azure_vnet_id = module.azure_infrastructure.vnet_id
  azure_region  = var.azure_region
  
  # GCP Networking
  gcp_network_name = module.gcp_infrastructure.network_name
  gcp_region       = var.gcp_region
  
  tags = local.common_tags
}

# Security Module
module "security" {
  source = "./modules/security"
  
  environment  = var.environment
  project_name = local.project_name
  
  # Zero-Trust Configuration
  enable_zero_trust = var.enable_zero_trust
  
  # Certificate Management
  domain_name = var.domain_name
  
  # WAF Configuration
  enable_waf = var.enable_waf
  
  # DDoS Protection
  enable_ddos_protection = var.enable_ddos_protection
  
  # Encryption
  encryption_key_rotation_days = var.encryption_key_rotation_days
  
  tags = local.common_tags
}

# Monitoring Module
module "monitoring" {
  source = "./modules/monitoring"
  
  environment  = var.environment
  project_name = local.project_name
  
  # Prometheus/Grafana
  enable_prometheus = var.enable_prometheus
  enable_grafana    = var.enable_grafana
  
  # Distributed Tracing
  enable_jaeger = var.enable_jaeger
  
  # Log Aggregation
  enable_elk_stack = var.enable_elk_stack
  
  # Alerting
  pagerduty_api_key = var.pagerduty_api_key
  alert_email       = var.alert_email
  
  # Cloud-specific monitoring
  aws_cloudwatch_enabled   = true
  azure_monitor_enabled    = true
  gcp_operations_enabled   = true
  
  tags = local.common_tags
}

# Database Replication Module
module "database_replication" {
  source = "./modules/database"
  
  environment  = var.environment
  project_name = local.project_name
  
  # Multi-master replication
  enable_multi_master = var.enable_multi_master
  
  # Backup configuration
  backup_retention_days = var.backup_retention_days
  backup_window        = var.backup_window
  
  # RTO/RPO targets
  recovery_time_objective  = 15  # minutes
  recovery_point_objective = 5   # minutes
  
  # Database endpoints
  aws_rds_endpoint   = module.aws_infrastructure.rds_endpoint
  azure_postgres_endpoint = module.azure_infrastructure.postgres_endpoint
  gcp_cloudsql_endpoint = module.gcp_infrastructure.cloudsql_endpoint
  
  tags = local.common_tags
}

# Outputs
output "aws_resources" {
  description = "AWS infrastructure resources"
  value = {
    vpc_id           = module.aws_infrastructure.vpc_id
    eks_cluster_name = module.aws_infrastructure.eks_cluster_name
    s3_bucket_name   = module.aws_infrastructure.s3_bucket_name
    rds_endpoint     = module.aws_infrastructure.rds_endpoint
    cloudfront_domain = module.aws_infrastructure.cloudfront_domain
  }
  sensitive = true
}

output "azure_resources" {
  description = "Azure infrastructure resources"
  value = {
    resource_group_name = module.azure_infrastructure.resource_group_name
    aks_cluster_name    = module.azure_infrastructure.aks_cluster_name
    storage_account_name = module.azure_infrastructure.storage_account_name
    postgres_endpoint   = module.azure_infrastructure.postgres_endpoint
    cdn_endpoint        = module.azure_infrastructure.cdn_endpoint
  }
  sensitive = true
}

output "gcp_resources" {
  description = "GCP infrastructure resources"
  value = {
    network_name      = module.gcp_infrastructure.network_name
    gke_cluster_name  = module.gcp_infrastructure.gke_cluster_name
    storage_bucket    = module.gcp_infrastructure.storage_bucket
    cloudsql_endpoint = module.gcp_infrastructure.cloudsql_endpoint
    cdn_backend       = module.gcp_infrastructure.cdn_backend
  }
  sensitive = true
}

output "monitoring_endpoints" {
  description = "Monitoring and observability endpoints"
  value = {
    prometheus_url = module.monitoring.prometheus_url
    grafana_url    = module.monitoring.grafana_url
    jaeger_url     = module.monitoring.jaeger_url
    kibana_url     = module.monitoring.kibana_url
  }
}
