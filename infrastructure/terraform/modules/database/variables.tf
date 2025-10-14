# Variables for Database Replication Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "enable_multi_master" {
  description = "Enable multi-master replication"
  type        = bool
}

variable "backup_retention_days" {
  description = "Backup retention days"
  type        = number
}

variable "backup_window" {
  description = "Backup window"
  type        = string
}

variable "recovery_time_objective" {
  description = "Recovery Time Objective in minutes"
  type        = number
}

variable "recovery_point_objective" {
  description = "Recovery Point Objective in minutes"
  type        = number
}

variable "aws_rds_endpoint" {
  description = "AWS RDS endpoint"
  type        = string
  sensitive   = true
}

variable "azure_postgres_endpoint" {
  description = "Azure PostgreSQL endpoint"
  type        = string
  sensitive   = true
}

variable "gcp_cloudsql_endpoint" {
  description = "GCP Cloud SQL endpoint"
  type        = string
  sensitive   = true
}

variable "tags" {
  description = "Tags to apply to resources"
  type        = map(string)
  default     = {}
}
