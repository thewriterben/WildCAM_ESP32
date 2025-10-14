# Monitoring and Observability Module for Multi-Cloud WildCAM ESP32

locals {
  name_prefix = "${var.project_name}-${var.environment}"
}

# Placeholder outputs for monitoring endpoints
# In production, these would deploy actual Prometheus, Grafana, Jaeger, and ELK stack

output "prometheus_url" {
  description = "Prometheus monitoring URL"
  value       = var.enable_prometheus ? "https://prometheus.${var.project_name}.example.com" : null
}

output "grafana_url" {
  description = "Grafana dashboards URL"
  value       = var.enable_grafana ? "https://grafana.${var.project_name}.example.com" : null
}

output "jaeger_url" {
  description = "Jaeger tracing URL"
  value       = var.enable_jaeger ? "https://jaeger.${var.project_name}.example.com" : null
}

output "kibana_url" {
  description = "Kibana logs URL"
  value       = var.enable_elk_stack ? "https://kibana.${var.project_name}.example.com" : null
}

output "monitoring_config" {
  description = "Monitoring configuration summary"
  value = {
    prometheus_enabled = var.enable_prometheus
    grafana_enabled    = var.enable_grafana
    jaeger_enabled     = var.enable_jaeger
    elk_stack_enabled  = var.enable_elk_stack
    pagerduty_integrated = var.pagerduty_api_key != ""
    alert_email        = var.alert_email
    aws_cloudwatch     = var.aws_cloudwatch_enabled
    azure_monitor      = var.azure_monitor_enabled
    gcp_operations     = var.gcp_operations_enabled
  }
}
