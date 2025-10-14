# Azure Infrastructure Configuration

module "azure_infrastructure" {
  count  = var.enable_azure ? 1 : 0
  source = "./azure"
  
  project_name = var.project_name
  environment  = var.environment
  location     = var.azure_location
  
  # AKS Configuration
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_vm_size      = "Standard_D4s_v3"
  
  # Network Configuration
  vnet_address_space = ["10.2.0.0/16"]
  subnet_prefixes    = ["10.2.1.0/24", "10.2.2.0/24", "10.2.3.0/24"]
  
  # Storage Configuration
  enable_blob_versioning = true
  enable_soft_delete     = true
  
  # Database Configuration
  enable_postgres           = true
  postgres_sku_name        = "GP_Standard_D4s_v3"
  postgres_high_availability = true
  
  # Cost Management
  monthly_budget = var.monthly_cost_budget * 0.33 # 33% of total budget
  
  tags = local.common_tags
}

module "azure_secondary" {
  count  = var.enable_azure ? 1 : 0
  source = "./azure"
  
  project_name = var.project_name
  environment  = "${var.environment}-secondary"
  location     = var.azure_secondary_location
  
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_vm_size      = "Standard_D4s_v3"
  
  vnet_address_space = ["10.3.0.0/16"]
  subnet_prefixes    = ["10.3.1.0/24", "10.3.2.0/24"]
  
  enable_blob_versioning = true
  enable_postgres        = true
  postgres_sku_name     = "GP_Standard_D2s_v3"
  
  tags = merge(local.common_tags, {
    Region = "Secondary"
  })
}

# Azure Virtual Network Peering
resource "azurerm_virtual_network_peering" "primary_to_secondary" {
  count = var.enable_azure ? 1 : 0
  
  name                      = "${var.project_name}-primary-to-secondary"
  resource_group_name       = module.azure_infrastructure[0].resource_group_name
  virtual_network_name      = module.azure_infrastructure[0].vnet_name
  remote_virtual_network_id = module.azure_secondary[0].vnet_id
  
  allow_virtual_network_access = true
  allow_forwarded_traffic      = true
  allow_gateway_transit        = false
}

resource "azurerm_virtual_network_peering" "secondary_to_primary" {
  count = var.enable_azure ? 1 : 0
  
  name                      = "${var.project_name}-secondary-to-primary"
  resource_group_name       = module.azure_secondary[0].resource_group_name
  virtual_network_name      = module.azure_secondary[0].vnet_name
  remote_virtual_network_id = module.azure_infrastructure[0].vnet_id
  
  allow_virtual_network_access = true
  allow_forwarded_traffic      = true
  allow_gateway_transit        = false
}

# Azure Blob Storage Replication
resource "azurerm_storage_account_network_rules" "wildlife_data" {
  count = var.enable_azure ? 1 : 0
  
  storage_account_id = module.azure_infrastructure[0].storage_account_id
  
  default_action             = "Deny"
  ip_rules                   = []
  virtual_network_subnet_ids = module.azure_infrastructure[0].subnet_ids
  bypass                     = ["AzureServices"]
}

# Azure Monitor Alerts
resource "azurerm_monitor_metric_alert" "cluster_health" {
  count = var.enable_azure ? 1 : 0
  
  name                = "${var.project_name}-azure-cluster-health"
  resource_group_name = module.azure_infrastructure[0].resource_group_name
  scopes              = [module.azure_infrastructure[0].aks_cluster_id]
  description         = "Alert when AKS cluster health degrades"
  
  criteria {
    metric_namespace = "Microsoft.ContainerService/managedClusters"
    metric_name      = "kube_node_status_condition"
    aggregation      = "Average"
    operator         = "LessThan"
    threshold        = 1
  }
  
  action {
    action_group_id = module.azure_infrastructure[0].action_group_id
  }
}

# Azure Cost Management Budget
resource "azurerm_consumption_budget_resource_group" "wildlife_monitoring" {
  count = var.enable_azure ? 1 : 0
  
  name              = "${var.project_name}-budget"
  resource_group_id = module.azure_infrastructure[0].resource_group_id
  
  amount     = var.monthly_cost_budget * 0.33
  time_grain = "Monthly"
  
  time_period {
    start_date = formatdate("YYYY-MM-01'T'00:00:00'Z'", timestamp())
  }
  
  notification {
    enabled   = true
    threshold = 80.0
    operator  = "GreaterThan"
    
    contact_emails = [
      "ops@wildcam.org",
    ]
  }
  
  notification {
    enabled   = true
    threshold = 100.0
    operator  = "GreaterThan"
    
    contact_emails = [
      "ops@wildcam.org",
      "admin@wildcam.org",
    ]
  }
}

# Azure Front Door for Global Load Balancing
resource "azurerm_frontdoor" "wildlife_monitoring" {
  count               = var.enable_azure ? 1 : 0
  name                = "${var.project_name}-frontdoor"
  resource_group_name = module.azure_infrastructure[0].resource_group_name
  
  routing_rule {
    name               = "wildlifeRoutingRule"
    accepted_protocols = ["Http", "Https"]
    patterns_to_match  = ["/*"]
    frontend_endpoints = ["${var.project_name}FrontendEndpoint"]
    
    forwarding_configuration {
      forwarding_protocol = "HttpsOnly"
      backend_pool_name   = "wildlifeBackendPool"
    }
  }
  
  backend_pool_load_balancing {
    name = "wildlifeLoadBalancingSettings"
  }
  
  backend_pool_health_probe {
    name     = "wildlifeHealthProbeSetting"
    protocol = "Https"
    path     = "/health"
  }
  
  backend_pool {
    name = "wildlifeBackendPool"
    
    backend {
      host_header = module.azure_infrastructure[0].ingress_ip
      address     = module.azure_infrastructure[0].ingress_ip
      http_port   = 80
      https_port  = 443
      priority    = 1
      weight      = 50
    }
    
    load_balancing_name = "wildlifeLoadBalancingSettings"
    health_probe_name   = "wildlifeHealthProbeSetting"
  }
  
  frontend_endpoint {
    name      = "${var.project_name}FrontendEndpoint"
    host_name = "${var.project_name}-${var.environment}.azurefd.net"
  }
}
