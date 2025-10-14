# Variables for GCP Infrastructure Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "gcp_project_id" {
  description = "GCP project ID"
  type        = string
}

variable "gcp_region" {
  description = "GCP region"
  type        = string
}

variable "unique_suffix" {
  description = "Unique suffix for resource naming"
  type        = string
}

variable "vpc_subnet_cidr" {
  description = "VPC subnet CIDR"
  type        = string
}

variable "gke_cluster_version" {
  description = "GKE cluster version"
  type        = string
}

variable "gke_node_pools" {
  description = "GKE node pool configurations"
  type = map(object({
    machine_type = string
    node_count   = number
    min_count    = number
    max_count    = number
  }))
}

variable "cloudsql_tier" {
  description = "Cloud SQL tier"
  type        = string
}

variable "cloudsql_backup_enabled" {
  description = "Enable Cloud SQL backups"
  type        = bool
}

variable "storage_class" {
  description = "GCS storage class"
  type        = string
}

variable "labels" {
  description = "Labels to apply to resources"
  type        = map(string)
  default     = {}
}
