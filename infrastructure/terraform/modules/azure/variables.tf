# Variables for Azure Infrastructure Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "azure_region" {
  description = "Azure region"
  type        = string
}

variable "unique_suffix" {
  description = "Unique suffix for resource naming"
  type        = string
}

variable "vnet_address_space" {
  description = "VNet address space"
  type        = list(string)
}

variable "aks_kubernetes_version" {
  description = "AKS Kubernetes version"
  type        = string
}

variable "aks_node_pools" {
  description = "AKS node pool configurations"
  type = map(object({
    vm_size    = string
    node_count = number
    min_count  = number
    max_count  = number
  }))
}

variable "postgres_sku_name" {
  description = "PostgreSQL SKU name"
  type        = string
}

variable "postgres_backup_retention" {
  description = "PostgreSQL backup retention days"
  type        = number
}

variable "storage_replication_type" {
  description = "Storage replication type"
  type        = string
}

variable "tags" {
  description = "Tags to apply to resources"
  type        = map(string)
  default     = {}
}
