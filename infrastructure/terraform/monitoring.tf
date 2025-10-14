# Multi-Cloud Monitoring and Observability Stack

# OpenTelemetry Collector for Distributed Tracing
module "observability" {
  source = "./modules/monitoring"
  
  project_name = var.project_name
  environment  = var.environment
  
  # Cloud endpoints for metrics collection
  aws_metrics_endpoint   = var.enable_aws ? module.aws_infrastructure[0].cloudwatch_endpoint : null
  azure_metrics_endpoint = var.enable_azure ? module.azure_infrastructure[0].monitor_endpoint : null
  gcp_metrics_endpoint   = var.enable_gcp ? module.gcp_infrastructure[0].monitoring_endpoint : null
  
  # Alerting configuration
  alert_email       = "ops@wildcam.org"
  pagerduty_api_key = var.pagerduty_api_key
  slack_webhook_url = var.slack_webhook_url
  
  tags = local.common_tags
}

variable "pagerduty_api_key" {
  description = "PagerDuty API key for critical alerts"
  type        = string
  sensitive   = true
  default     = ""
}

variable "slack_webhook_url" {
  description = "Slack webhook URL for notifications"
  type        = string
  sensitive   = true
  default     = ""
}

# Prometheus Federation for Multi-Cloud Metrics
resource "kubernetes_deployment" "prometheus_federation" {
  metadata {
    name      = "${var.project_name}-prometheus-federation"
    namespace = "monitoring"
  }
  
  spec {
    replicas = 2
    
    selector {
      match_labels = {
        app       = "prometheus"
        component = "federation"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "prometheus"
          component = "federation"
        }
      }
      
      spec {
        container {
          name  = "prometheus"
          image = "prom/prometheus:v2.48.0"
          
          args = [
            "--config.file=/etc/prometheus/prometheus.yml",
            "--storage.tsdb.path=/prometheus",
            "--storage.tsdb.retention.time=30d",
            "--web.enable-lifecycle",
            "--web.enable-admin-api"
          ]
          
          port {
            container_port = 9090
            name          = "web"
          }
          
          volume_mount {
            name       = "config"
            mount_path = "/etc/prometheus"
          }
          
          volume_mount {
            name       = "storage"
            mount_path = "/prometheus"
          }
          
          resources {
            requests = {
              memory = "4Gi"
              cpu    = "2"
            }
            limits = {
              memory = "8Gi"
              cpu    = "4"
            }
          }
          
          liveness_probe {
            http_get {
              path = "/-/healthy"
              port = 9090
            }
            initial_delay_seconds = 30
            period_seconds        = 10
          }
          
          readiness_probe {
            http_get {
              path = "/-/ready"
              port = 9090
            }
            initial_delay_seconds = 5
            period_seconds        = 5
          }
        }
        
        volume {
          name = "config"
          config_map {
            name = "prometheus-federation-config"
          }
        }
        
        volume {
          name = "storage"
          persistent_volume_claim {
            claim_name = "prometheus-storage"
          }
        }
      }
    }
  }
}

# Grafana for Unified Visualization
resource "kubernetes_deployment" "grafana" {
  metadata {
    name      = "${var.project_name}-grafana"
    namespace = "monitoring"
  }
  
  spec {
    replicas = 2
    
    selector {
      match_labels = {
        app       = "grafana"
        component = "visualization"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "grafana"
          component = "visualization"
        }
      }
      
      spec {
        container {
          name  = "grafana"
          image = "grafana/grafana:10.2.0"
          
          env {
            name  = "GF_SECURITY_ADMIN_PASSWORD"
            value_from {
              secret_key_ref {
                name = "grafana-admin"
                key  = "password"
              }
            }
          }
          
          env {
            name  = "GF_INSTALL_PLUGINS"
            value = "grafana-piechart-panel,grafana-worldmap-panel,grafana-clock-panel"
          }
          
          port {
            container_port = 3000
            name          = "http"
          }
          
          volume_mount {
            name       = "storage"
            mount_path = "/var/lib/grafana"
          }
          
          volume_mount {
            name       = "dashboards"
            mount_path = "/etc/grafana/provisioning/dashboards"
          }
          
          volume_mount {
            name       = "datasources"
            mount_path = "/etc/grafana/provisioning/datasources"
          }
          
          resources {
            requests = {
              memory = "512Mi"
              cpu    = "500m"
            }
            limits = {
              memory = "1Gi"
              cpu    = "1"
            }
          }
        }
        
        volume {
          name = "storage"
          persistent_volume_claim {
            claim_name = "grafana-storage"
          }
        }
        
        volume {
          name = "dashboards"
          config_map {
            name = "grafana-dashboards"
          }
        }
        
        volume {
          name = "datasources"
          config_map {
            name = "grafana-datasources"
          }
        }
      }
    }
  }
}

# Jaeger for Distributed Tracing
resource "kubernetes_deployment" "jaeger_collector" {
  metadata {
    name      = "${var.project_name}-jaeger-collector"
    namespace = "tracing"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "jaeger"
        component = "collector"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "jaeger"
          component = "collector"
        }
      }
      
      spec {
        container {
          name  = "jaeger-collector"
          image = "jaegertracing/jaeger-collector:1.51"
          
          env {
            name  = "SPAN_STORAGE_TYPE"
            value = "elasticsearch"
          }
          
          env {
            name  = "ES_SERVER_URLS"
            value = "http://elasticsearch:9200"
          }
          
          port {
            container_port = 14250
            name          = "grpc"
          }
          
          port {
            container_port = 14268
            name          = "http"
          }
          
          port {
            container_port = 9411
            name          = "zipkin"
          }
          
          resources {
            requests = {
              memory = "1Gi"
              cpu    = "1"
            }
            limits = {
              memory = "2Gi"
              cpu    = "2"
            }
          }
        }
      }
    }
  }
}

# Loki for Log Aggregation
resource "kubernetes_stateful_set" "loki" {
  metadata {
    name      = "${var.project_name}-loki"
    namespace = "logging"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "loki"
        component = "logging"
      }
    }
    
    service_name = "loki"
    
    template {
      metadata {
        labels = {
          app       = "loki"
          component = "logging"
        }
      }
      
      spec {
        container {
          name  = "loki"
          image = "grafana/loki:2.9.0"
          
          args = [
            "-config.file=/etc/loki/loki.yaml"
          ]
          
          port {
            container_port = 3100
            name          = "http"
          }
          
          port {
            container_port = 9095
            name          = "grpc"
          }
          
          volume_mount {
            name       = "config"
            mount_path = "/etc/loki"
          }
          
          volume_mount {
            name       = "storage"
            mount_path = "/loki"
          }
          
          resources {
            requests = {
              memory = "2Gi"
              cpu    = "1"
            }
            limits = {
              memory = "4Gi"
              cpu    = "2"
            }
          }
        }
        
        volume {
          name = "config"
          config_map {
            name = "loki-config"
          }
        }
        
        affinity {
          pod_anti_affinity {
            required_during_scheduling_ignored_during_execution {
              label_selector {
                match_expressions {
                  key      = "app"
                  operator = "In"
                  values   = ["loki"]
                }
              }
              topology_key = "kubernetes.io/hostname"
            }
          }
        }
      }
    }
    
    volume_claim_template {
      metadata {
        name = "storage"
      }
      
      spec {
        access_modes = ["ReadWriteOnce"]
        
        resources {
          requests = {
            storage = "100Gi"
          }
        }
      }
    }
  }
}

# Cost Monitoring Dashboard
resource "kubernetes_config_map" "cost_dashboard" {
  metadata {
    name      = "cost-monitoring-dashboard"
    namespace = "monitoring"
  }
  
  data = {
    "cost-dashboard.json" = jsonencode({
      title = "Multi-Cloud Cost Monitoring"
      panels = [
        {
          title = "Total Monthly Spend"
          type  = "graph"
          targets = [
            {
              expr = "sum(aws_cost_total + azure_cost_total + gcp_cost_total)"
            }
          ]
        },
        {
          title = "Cost by Cloud Provider"
          type  = "piechart"
          targets = [
            { expr = "aws_cost_total", legendFormat = "AWS" },
            { expr = "azure_cost_total", legendFormat = "Azure" },
            { expr = "gcp_cost_total", legendFormat = "GCP" }
          ]
        },
        {
          title = "Budget vs Actual"
          type  = "gauge"
          targets = [
            {
              expr = "(sum(aws_cost_total + azure_cost_total + gcp_cost_total) / ${var.monthly_cost_budget}) * 100"
            }
          ]
        }
      ]
    })
  }
}

# SLA Monitoring
resource "kubernetes_config_map" "sla_monitoring" {
  metadata {
    name      = "sla-monitoring-config"
    namespace = "monitoring"
  }
  
  data = {
    "rules.yaml" = <<-EOT
      groups:
      - name: sla_monitoring
        interval: 30s
        rules:
        - alert: SLAViolation99.99Uptime
          expr: (sum(up{job="kubernetes-apiservers"}) / count(up{job="kubernetes-apiservers"})) < 0.9999
          for: 5m
          labels:
            severity: critical
            sla: "99.99%"
          annotations:
            summary: "SLA violation: Uptime below 99.99%"
            description: "Current uptime: {{ $value | humanizePercentage }}"
        
        - alert: LatencyThresholdExceeded
          expr: histogram_quantile(0.95, rate(http_request_duration_seconds_bucket[5m])) > 0.5
          for: 5m
          labels:
            severity: warning
          annotations:
            summary: "95th percentile latency exceeds 500ms"
            description: "Current p95 latency: {{ $value | humanizeDuration }}"
        
        - alert: ErrorRateTooHigh
          expr: (sum(rate(http_requests_total{status=~"5.."}[5m])) / sum(rate(http_requests_total[5m]))) > 0.01
          for: 5m
          labels:
            severity: warning
          annotations:
            summary: "Error rate exceeds 1%"
            description: "Current error rate: {{ $value | humanizePercentage }}"
    EOT
  }
}

# Chaos Engineering Integration
resource "kubernetes_deployment" "chaos_mesh" {
  metadata {
    name      = "${var.project_name}-chaos-mesh"
    namespace = "chaos-testing"
  }
  
  spec {
    replicas = 1
    
    selector {
      match_labels = {
        app       = "chaos-mesh"
        component = "controller"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "chaos-mesh"
          component = "controller"
        }
      }
      
      spec {
        container {
          name  = "chaos-controller-manager"
          image = "pingcap/chaos-mesh:v2.6.0"
          
          command = ["/usr/local/bin/chaos-controller-manager"]
          
          env {
            name = "NAMESPACE"
            value_from {
              field_ref {
                field_path = "metadata.namespace"
              }
            }
          }
          
          resources {
            requests = {
              memory = "512Mi"
              cpu    = "500m"
            }
            limits = {
              memory = "1Gi"
              cpu    = "1"
            }
          }
        }
      }
    }
  }
}
