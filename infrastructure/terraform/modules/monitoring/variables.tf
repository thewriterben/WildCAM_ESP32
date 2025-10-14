# Variables for Monitoring Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "enable_prometheus" {
  description = "Enable Prometheus"
  type        = bool
}

variable "enable_grafana" {
  description = "Enable Grafana"
  type        = bool
}

variable "enable_jaeger" {
  description = "Enable Jaeger"
  type        = bool
}

variable "enable_elk_stack" {
  description = "Enable ELK stack"
  type        = bool
}

variable "pagerduty_api_key" {
  description = "PagerDuty API key"
  type        = string
  sensitive   = true
}

variable "alert_email" {
  description = "Alert email address"
  type        = string
}

variable "aws_cloudwatch_enabled" {
  description = "Enable AWS CloudWatch"
  type        = bool
  default     = true
}

variable "azure_monitor_enabled" {
  description = "Enable Azure Monitor"
  type        = bool
  default     = true
}

variable "gcp_operations_enabled" {
  description = "Enable GCP Operations"
  type        = bool
  default     = true
}

variable "tags" {
  description = "Tags to apply to resources"
  type        = map(string)
  default     = {}
}
