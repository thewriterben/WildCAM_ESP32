# Variables for Networking Module

variable "environment" {
  description = "Environment name"
  type        = string
}

variable "project_name" {
  description = "Project name"
  type        = string
}

variable "enable_vpn_mesh" {
  description = "Enable VPN mesh networking"
  type        = bool
}

variable "aws_vpc_id" {
  description = "AWS VPC ID"
  type        = string
}

variable "aws_region" {
  description = "AWS region"
  type        = string
}

variable "azure_vnet_id" {
  description = "Azure VNet ID"
  type        = string
}

variable "azure_region" {
  description = "Azure region"
  type        = string
}

variable "gcp_network_name" {
  description = "GCP network name"
  type        = string
}

variable "gcp_region" {
  description = "GCP region"
  type        = string
}

variable "tags" {
  description = "Tags to apply to resources"
  type        = map(string)
  default     = {}
}
