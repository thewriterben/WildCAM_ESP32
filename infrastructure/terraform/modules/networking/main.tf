# Cross-Cloud Networking Module for WildCAM ESP32
# Establishes VPN mesh networking between AWS, Azure, and GCP

locals {
  name_prefix = "${var.project_name}-${var.environment}"
}

# AWS VPN Gateway
resource "aws_vpn_gateway" "main" {
  count  = var.enable_vpn_mesh ? 1 : 0
  vpc_id = var.aws_vpc_id

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-vpn-gateway"
  })
}

# Customer Gateways for Azure and GCP
resource "aws_customer_gateway" "azure" {
  count      = var.enable_vpn_mesh ? 1 : 0
  bgp_asn    = 65000
  ip_address = "1.1.1.1" # Placeholder - will be replaced with actual Azure VPN gateway IP
  type       = "ipsec.1"

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-cgw-azure"
  })
}

resource "aws_customer_gateway" "gcp" {
  count      = var.enable_vpn_mesh ? 1 : 0
  bgp_asn    = 65001
  ip_address = "2.2.2.2" # Placeholder - will be replaced with actual GCP VPN gateway IP
  type       = "ipsec.1"

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-cgw-gcp"
  })
}

# VPN Connections
resource "aws_vpn_connection" "azure" {
  count               = var.enable_vpn_mesh ? 1 : 0
  vpn_gateway_id      = aws_vpn_gateway.main[0].id
  customer_gateway_id = aws_customer_gateway.azure[0].id
  type                = "ipsec.1"
  static_routes_only  = false

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-vpn-azure"
  })
}

resource "aws_vpn_connection" "gcp" {
  count               = var.enable_vpn_mesh ? 1 : 0
  vpn_gateway_id      = aws_vpn_gateway.main[0].id
  customer_gateway_id = aws_customer_gateway.gcp[0].id
  type                = "ipsec.1"
  static_routes_only  = false

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-vpn-gcp"
  })
}

# Output VPN configuration for documentation
output "vpn_mesh_enabled" {
  description = "Whether VPN mesh is enabled"
  value       = var.enable_vpn_mesh
}

output "vpn_configuration" {
  description = "VPN configuration details for manual setup"
  value = var.enable_vpn_mesh ? {
    aws_vpn_gateway_id = aws_vpn_gateway.main[0].id
    message            = "VPN mesh networking configured. Additional manual configuration required for Azure VPN Gateway and GCP Cloud VPN."
  } : null
}
