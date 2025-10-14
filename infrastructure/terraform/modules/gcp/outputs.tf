# Outputs for GCP Infrastructure Module

output "network_name" {
  description = "VPC network name"
  value       = google_compute_network.main.name
}

output "network_id" {
  description = "VPC network ID"
  value       = google_compute_network.main.id
}

output "subnet_id" {
  description = "Subnet ID"
  value       = google_compute_subnetwork.main.id
}

output "gke_cluster_name" {
  description = "GKE cluster name"
  value       = google_container_cluster.main.name
}

output "gke_cluster_endpoint" {
  description = "GKE cluster endpoint"
  value       = google_container_cluster.main.endpoint
  sensitive   = true
}

output "gke_cluster_ca_certificate" {
  description = "GKE cluster CA certificate"
  value       = google_container_cluster.main.master_auth[0].cluster_ca_certificate
  sensitive   = true
}

output "storage_bucket" {
  description = "Cloud Storage bucket name"
  value       = google_storage_bucket.wildlife_data.name
}

output "storage_bucket_url" {
  description = "Cloud Storage bucket URL"
  value       = google_storage_bucket.wildlife_data.url
}

output "cloudsql_endpoint" {
  description = "Cloud SQL instance connection name"
  value       = google_sql_database_instance.main.connection_name
  sensitive   = true
}

output "cloudsql_instance_name" {
  description = "Cloud SQL instance name"
  value       = google_sql_database_instance.main.name
}

output "cloudsql_database_name" {
  description = "Cloud SQL database name"
  value       = google_sql_database.wildcam.name
}

output "cdn_backend" {
  description = "CDN backend bucket name"
  value       = google_compute_backend_bucket.wildlife_cdn.name
}

output "kms_keyring_id" {
  description = "KMS keyring ID"
  value       = google_kms_key_ring.main.id
}
