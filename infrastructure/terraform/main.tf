# Multi-Cloud Deployment Architecture for WildCAM ESP32
# Main Terraform Configuration

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
  }
  
  # Remote state configuration for multi-cloud coordination
  backend "s3" {
    bucket         = "wildcam-terraform-state"
    key            = "multi-cloud/terraform.tfstate"
    region         = "us-west-2"
    encrypt        = true
    dynamodb_table = "wildcam-terraform-locks"
  }
}

# Variables
variable "project_name" {
  description = "Project name for resource naming"
  type        = string
  default     = "wildcam"
}

variable "environment" {
  description = "Environment (production, staging, development)"
  type        = string
  default     = "production"
}

variable "aws_region" {
  description = "AWS primary region"
  type        = string
  default     = "us-west-2"
}

variable "aws_secondary_region" {
  description = "AWS secondary region for HA"
  type        = string
  default     = "us-east-1"
}

variable "azure_location" {
  description = "Azure primary location"
  type        = string
  default     = "westus2"
}

variable "azure_secondary_location" {
  description = "Azure secondary location for HA"
  type        = string
  default     = "eastus"
}

variable "gcp_region" {
  description = "GCP primary region"
  type        = string
  default     = "us-west1"
}

variable "gcp_secondary_region" {
  description = "GCP secondary region for HA"
  type        = string
  default     = "us-east1"
}

variable "enable_multi_cloud" {
  description = "Enable multi-cloud deployment"
  type        = bool
  default     = true
}

variable "enable_aws" {
  description = "Deploy to AWS"
  type        = bool
  default     = true
}

variable "enable_azure" {
  description = "Deploy to Azure"
  type        = bool
  default     = true
}

variable "enable_gcp" {
  description = "Deploy to GCP"
  type        = bool
  default     = true
}

variable "kubernetes_version" {
  description = "Kubernetes version"
  type        = string
  default     = "1.28"
}

variable "node_count" {
  description = "Default node count per cluster"
  type        = number
  default     = 3
}

variable "monthly_cost_budget" {
  description = "Monthly cost budget in USD"
  type        = number
  default     = 10000
}

# Common tags for all resources
locals {
  common_tags = {
    Project     = var.project_name
    Environment = var.environment
    ManagedBy   = "Terraform"
    Purpose     = "Wildlife Monitoring"
  }
}

# AWS Provider Configuration
provider "aws" {
  region = var.aws_region
  
  default_tags {
    tags = local.common_tags
  }
}

provider "aws" {
  alias  = "secondary"
  region = var.aws_secondary_region
  
  default_tags {
    tags = merge(local.common_tags, {
      Region = "Secondary"
    })
  }
}

# Azure Provider Configuration
provider "azurerm" {
  features {
    resource_group {
      prevent_deletion_if_contains_resources = false
    }
  }
}

# GCP Provider Configuration
provider "google" {
  project = "${var.project_name}-${var.environment}"
  region  = var.gcp_region
}

# Outputs
output "aws_cluster_endpoint" {
  description = "AWS EKS cluster endpoint"
  value       = var.enable_aws ? module.aws_infrastructure[0].cluster_endpoint : null
  sensitive   = true
}

output "azure_cluster_endpoint" {
  description = "Azure AKS cluster endpoint"
  value       = var.enable_azure ? module.azure_infrastructure[0].cluster_endpoint : null
  sensitive   = true
}

output "gcp_cluster_endpoint" {
  description = "GCP GKE cluster endpoint"
  value       = var.enable_gcp ? module.gcp_infrastructure[0].cluster_endpoint : null
  sensitive   = true
}

output "global_load_balancer_ip" {
  description = "Global load balancer IP address"
  value       = module.traffic_management.global_lb_ip
}

output "cdn_domain" {
  description = "CDN domain name"
  value       = module.traffic_management.cdn_domain
}
