# Variables for Security Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "enable_zero_trust" {
  description = "Enable zero-trust security"
  type        = bool
}

variable "domain_name" {
  description = "Domain name for SSL certificates"
  type        = string
}

variable "enable_waf" {
  description = "Enable Web Application Firewall"
  type        = bool
}

variable "enable_ddos_protection" {
  description = "Enable DDoS protection"
  type        = bool
}

variable "encryption_key_rotation_days" {
  description = "Days between key rotation"
  type        = number
}

variable "tags" {
  description = "Tags to apply to resources"
  type        = map(string)
  default     = {}
}
