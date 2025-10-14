# GCP Infrastructure Module for WildCAM ESP32

locals {
  name_prefix = "${var.project_name}-${var.environment}"
}

# VPC Network
resource "google_compute_network" "main" {
  name                    = "${local.name_prefix}-network"
  auto_create_subnetworks = false
  project                 = var.gcp_project_id
}

# Subnet
resource "google_compute_subnetwork" "main" {
  name          = "${local.name_prefix}-subnet"
  ip_cidr_range = var.vpc_subnet_cidr
  region        = var.gcp_region
  network       = google_compute_network.main.id
  project       = var.gcp_project_id

  secondary_ip_range {
    range_name    = "pods"
    ip_cidr_range = "172.16.0.0/16"
  }

  secondary_ip_range {
    range_name    = "services"
    ip_cidr_range = "172.17.0.0/16"
  }
}

# Cloud Router for NAT
resource "google_compute_router" "main" {
  name    = "${local.name_prefix}-router"
  region  = var.gcp_region
  network = google_compute_network.main.id
  project = var.gcp_project_id
}

# Cloud NAT
resource "google_compute_router_nat" "main" {
  name                               = "${local.name_prefix}-nat"
  router                             = google_compute_router.main.name
  region                             = var.gcp_region
  nat_ip_allocate_option            = "AUTO_ONLY"
  source_subnetwork_ip_ranges_to_nat = "ALL_SUBNETWORKS_ALL_IP_RANGES"
  project                            = var.gcp_project_id

  log_config {
    enable = true
    filter = "ERRORS_ONLY"
  }
}

# GKE Cluster
resource "google_container_cluster" "main" {
  name     = "${local.name_prefix}-gke"
  location = var.gcp_region
  project  = var.gcp_project_id

  # We can't create a cluster with no node pool defined, but we want to only use
  # separately managed node pools. So we create the smallest possible default
  # node pool and immediately delete it.
  remove_default_node_pool = true
  initial_node_count       = 1

  network    = google_compute_network.main.name
  subnetwork = google_compute_subnetwork.main.name

  ip_allocation_policy {
    cluster_secondary_range_name  = "pods"
    services_secondary_range_name = "services"
  }

  release_channel {
    channel = "REGULAR"
  }

  addons_config {
    http_load_balancing {
      disabled = false
    }
    horizontal_pod_autoscaling {
      disabled = false
    }
    network_policy_config {
      disabled = false
    }
  }

  network_policy {
    enabled  = true
    provider = "CALICO"
  }

  workload_identity_config {
    workload_pool = "${var.gcp_project_id}.svc.id.goog"
  }

  binary_authorization {
    evaluation_mode = "PROJECT_SINGLETON_POLICY_ENFORCE"
  }

  logging_config {
    enable_components = ["SYSTEM_COMPONENTS", "WORKLOADS"]
  }

  monitoring_config {
    enable_components = ["SYSTEM_COMPONENTS"]
    
    managed_prometheus {
      enabled = true
    }
  }
}

# GKE Node Pools
resource "google_container_node_pool" "default" {
  name       = "default"
  location   = var.gcp_region
  cluster    = google_container_cluster.main.name
  project    = var.gcp_project_id
  node_count = var.gke_node_pools["default"].node_count

  autoscaling {
    min_node_count = var.gke_node_pools["default"].min_count
    max_node_count = var.gke_node_pools["default"].max_count
  }

  node_config {
    machine_type = var.gke_node_pools["default"].machine_type
    disk_size_gb = 100
    disk_type    = "pd-standard"

    oauth_scopes = [
      "https://www.googleapis.com/auth/cloud-platform"
    ]

    metadata = {
      disable-legacy-endpoints = "true"
    }

    workload_metadata_config {
      mode = "GKE_METADATA"
    }

    shielded_instance_config {
      enable_secure_boot          = true
      enable_integrity_monitoring = true
    }
  }

  management {
    auto_repair  = true
    auto_upgrade = true
  }
}

resource "google_container_node_pool" "compute" {
  name       = "compute"
  location   = var.gcp_region
  cluster    = google_container_cluster.main.name
  project    = var.gcp_project_id
  node_count = var.gke_node_pools["compute"].node_count

  autoscaling {
    min_node_count = var.gke_node_pools["compute"].min_count
    max_node_count = var.gke_node_pools["compute"].max_count
  }

  node_config {
    machine_type = var.gke_node_pools["compute"].machine_type
    disk_size_gb = 100
    disk_type    = "pd-ssd"

    oauth_scopes = [
      "https://www.googleapis.com/auth/cloud-platform"
    ]

    metadata = {
      disable-legacy-endpoints = "true"
    }

    workload_metadata_config {
      mode = "GKE_METADATA"
    }

    shielded_instance_config {
      enable_secure_boot          = true
      enable_integrity_monitoring = true
    }
  }

  management {
    auto_repair  = true
    auto_upgrade = true
  }
}

# Cloud Storage Bucket
resource "google_storage_bucket" "wildlife_data" {
  name          = "${local.name_prefix}-wildlife-data-${var.unique_suffix}"
  location      = var.storage_class == "MULTI_REGIONAL" ? "US" : var.gcp_region
  storage_class = var.storage_class
  project       = var.gcp_project_id

  uniform_bucket_level_access = true

  versioning {
    enabled = true
  }

  encryption {
    default_kms_key_name = google_kms_crypto_key.storage.id
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

  labels = var.labels
}

# Cloud SQL PostgreSQL Instance
resource "google_sql_database_instance" "main" {
  name             = "${local.name_prefix}-postgres-${var.unique_suffix}"
  database_version = "POSTGRES_15"
  region           = var.gcp_region
  project          = var.gcp_project_id

  settings {
    tier              = var.cloudsql_tier
    availability_type = "REGIONAL"
    disk_type         = "PD_SSD"
    disk_size         = 100
    disk_autoresize   = true

    backup_configuration {
      enabled                        = var.cloudsql_backup_enabled
      start_time                     = "03:00"
      point_in_time_recovery_enabled = true
      transaction_log_retention_days = 7
      backup_retention_settings {
        retained_backups = 30
      }
    }

    ip_configuration {
      ipv4_enabled    = false
      private_network = google_compute_network.main.id
    }

    database_flags {
      name  = "cloudsql.enable_pgaudit"
      value = "on"
    }
  }

  deletion_protection = var.environment == "production"

  depends_on = [google_service_networking_connection.private_vpc_connection]
}

resource "google_sql_database" "wildcam" {
  name     = "wildcam"
  instance = google_sql_database_instance.main.name
  project  = var.gcp_project_id
}

resource "google_sql_user" "wildcam_admin" {
  name     = "wildcam_admin"
  instance = google_sql_database_instance.main.name
  password = random_password.cloudsql_password.result
  project  = var.gcp_project_id
}

# Private Service Connection for Cloud SQL
resource "google_compute_global_address" "private_ip_address" {
  name          = "${local.name_prefix}-private-ip"
  purpose       = "VPC_PEERING"
  address_type  = "INTERNAL"
  prefix_length = 16
  network       = google_compute_network.main.id
  project       = var.gcp_project_id
}

resource "google_service_networking_connection" "private_vpc_connection" {
  network                 = google_compute_network.main.id
  service                 = "servicenetworking.googleapis.com"
  reserved_peering_ranges = [google_compute_global_address.private_ip_address.name]
}

# Cloud CDN with Load Balancer
resource "google_compute_backend_bucket" "wildlife_cdn" {
  name        = "${local.name_prefix}-cdn-backend"
  bucket_name = google_storage_bucket.wildlife_data.name
  enable_cdn  = true
  project     = var.gcp_project_id

  cdn_policy {
    cache_mode        = "CACHE_ALL_STATIC"
    default_ttl       = 3600
    max_ttl           = 86400
    client_ttl        = 7200
    negative_caching  = true
    serve_while_stale = 86400
  }
}

# KMS Keyring and Keys
resource "google_kms_key_ring" "main" {
  name     = "${local.name_prefix}-keyring"
  location = var.gcp_region
  project  = var.gcp_project_id
}

resource "google_kms_crypto_key" "storage" {
  name            = "storage-key"
  key_ring        = google_kms_key_ring.main.id
  rotation_period = "7776000s" # 90 days

  lifecycle {
    prevent_destroy = false
  }
}

# Secret Manager for PostgreSQL password
resource "google_secret_manager_secret" "postgres_password" {
  secret_id = "${local.name_prefix}-postgres-password"
  project   = var.gcp_project_id

  replication {
    automatic = true
  }

  labels = var.labels
}

resource "google_secret_manager_secret_version" "postgres_password" {
  secret      = google_secret_manager_secret.postgres_password.id
  secret_data = random_password.cloudsql_password.result
}

# Random password for Cloud SQL
resource "random_password" "cloudsql_password" {
  length  = 32
  special = true
}
