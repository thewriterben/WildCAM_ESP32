# GCP Infrastructure Configuration

module "gcp_infrastructure" {
  count  = var.enable_gcp ? 1 : 0
  source = "./gcp"
  
  project_name = var.project_name
  environment  = var.environment
  region       = var.gcp_region
  
  # GKE Configuration
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_machine_type = "n2-standard-4"
  
  # Network Configuration
  network_cidr = "10.4.0.0/16"
  subnet_cidr  = "10.4.1.0/24"
  
  # Storage Configuration
  enable_bucket_versioning = true
  storage_class            = "STANDARD"
  
  # Database Configuration
  enable_cloud_sql        = true
  db_tier                 = "db-custom-4-16384"
  db_high_availability    = true
  
  # Cost Management
  monthly_budget = var.monthly_cost_budget * 0.34 # 34% of total budget
  
  labels = local.common_tags
}

module "gcp_secondary" {
  count  = var.enable_gcp ? 1 : 0
  source = "./gcp"
  
  project_name = var.project_name
  environment  = "${var.environment}-secondary"
  region       = var.gcp_secondary_region
  
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_machine_type = "n2-standard-4"
  
  network_cidr = "10.5.0.0/16"
  subnet_cidr  = "10.5.1.0/24"
  
  enable_bucket_versioning = true
  enable_cloud_sql         = true
  db_tier                  = "db-custom-2-8192"
  db_high_availability     = true
  
  labels = merge(local.common_tags, {
    Region = "Secondary"
  })
}

# GCP VPC Network Peering
resource "google_compute_network_peering" "primary_to_secondary" {
  count = var.enable_gcp ? 1 : 0
  
  name         = "${var.project_name}-primary-to-secondary"
  network      = module.gcp_infrastructure[0].network_self_link
  peer_network = module.gcp_secondary[0].network_self_link
  
  export_custom_routes = true
  import_custom_routes = true
}

resource "google_compute_network_peering" "secondary_to_primary" {
  count = var.enable_gcp ? 1 : 0
  
  name         = "${var.project_name}-secondary-to-primary"
  network      = module.gcp_secondary[0].network_self_link
  peer_network = module.gcp_infrastructure[0].network_self_link
  
  export_custom_routes = true
  import_custom_routes = true
}

# GCS Bucket Replication
resource "google_storage_bucket" "wildlife_data_replica" {
  count    = var.enable_gcp ? 1 : 0
  name     = "${var.project_name}-wildlife-data-${var.gcp_secondary_region}"
  location = var.gcp_secondary_region
  
  versioning {
    enabled = true
  }
  
  lifecycle_rule {
    condition {
      age = 90
    }
    action {
      type          = "SetStorageClass"
      storage_class = "NEARLINE"
    }
  }
  
  lifecycle_rule {
    condition {
      age = 365
    }
    action {
      type          = "SetStorageClass"
      storage_class = "COLDLINE"
    }
  }
  
  labels = local.common_tags
}

# GCP Monitoring Alerts
resource "google_monitoring_alert_policy" "cluster_health" {
  count        = var.enable_gcp ? 1 : 0
  display_name = "${var.project_name}-gke-cluster-health"
  combiner     = "OR"
  
  conditions {
    display_name = "GKE Cluster Node Health"
    
    condition_threshold {
      filter          = "resource.type = \"k8s_cluster\" AND metric.type = \"kubernetes.io/node/status\""
      duration        = "300s"
      comparison      = "COMPARISON_LT"
      threshold_value = 1
      
      aggregations {
        alignment_period   = "60s"
        per_series_aligner = "ALIGN_MEAN"
      }
    }
  }
  
  notification_channels = [module.gcp_infrastructure[0].notification_channel_id]
  
  alert_strategy {
    auto_close = "1800s"
  }
}

# GCP Budget Alert
resource "google_billing_budget" "wildlife_monitoring" {
  count           = var.enable_gcp ? 1 : 0
  billing_account = data.google_billing_account.account.id
  display_name    = "${var.project_name}-budget"
  
  budget_filter {
    projects = ["projects/${data.google_project.current.number}"]
  }
  
  amount {
    specified_amount {
      currency_code = "USD"
      units         = tostring(floor(var.monthly_cost_budget * 0.34))
    }
  }
  
  threshold_rules {
    threshold_percent = 0.8
  }
  
  threshold_rules {
    threshold_percent = 1.0
  }
  
  threshold_rules {
    threshold_percent = 1.2
    spend_basis       = "FORECASTED_SPEND"
  }
  
  all_updates_rule {
    monitoring_notification_channels = [module.gcp_infrastructure[0].notification_channel_id]
  }
}

# Cloud Load Balancing for Global Distribution
resource "google_compute_global_address" "wildlife_lb" {
  count = var.enable_gcp ? 1 : 0
  name  = "${var.project_name}-global-ip"
}

resource "google_compute_global_forwarding_rule" "wildlife_https" {
  count                 = var.enable_gcp ? 1 : 0
  name                  = "${var.project_name}-https-forwarding"
  target                = google_compute_target_https_proxy.wildlife[0].self_link
  port_range            = "443"
  ip_address            = google_compute_global_address.wildlife_lb[0].address
  load_balancing_scheme = "EXTERNAL"
}

resource "google_compute_target_https_proxy" "wildlife" {
  count   = var.enable_gcp ? 1 : 0
  name    = "${var.project_name}-https-proxy"
  url_map = google_compute_url_map.wildlife[0].self_link
  ssl_certificates = [
    module.gcp_infrastructure[0].ssl_certificate_id
  ]
}

resource "google_compute_url_map" "wildlife" {
  count           = var.enable_gcp ? 1 : 0
  name            = "${var.project_name}-url-map"
  default_service = google_compute_backend_service.wildlife[0].self_link
}

resource "google_compute_backend_service" "wildlife" {
  count                 = var.enable_gcp ? 1 : 0
  name                  = "${var.project_name}-backend-service"
  protocol              = "HTTP"
  port_name             = "http"
  timeout_sec           = 30
  enable_cdn            = true
  load_balancing_scheme = "EXTERNAL"
  
  backend {
    group           = module.gcp_infrastructure[0].instance_group
    balancing_mode  = "UTILIZATION"
    capacity_scaler = 1.0
  }
  
  health_checks = [module.gcp_infrastructure[0].health_check_id]
  
  cdn_policy {
    cache_mode        = "CACHE_ALL_STATIC"
    default_ttl       = 3600
    client_ttl        = 7200
    max_ttl           = 86400
    negative_caching  = true
    serve_while_stale = 86400
  }
}

data "google_billing_account" "account" {
  display_name = "Wildlife Monitoring Account"
  open         = true
}

data "google_project" "current" {
  project_id = "${var.project_name}-${var.environment}"
}
