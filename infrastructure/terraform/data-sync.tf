# Multi-Cloud Data Synchronization and Replication

# PostgreSQL Multi-Cloud Clustering Configuration
module "database_federation" {
  source = "./modules/database"
  
  project_name = var.project_name
  environment  = var.environment
  
  # Database endpoints
  aws_db_endpoint   = var.enable_aws ? module.aws_infrastructure[0].rds_endpoint : null
  azure_db_endpoint = var.enable_azure ? module.azure_infrastructure[0].postgres_fqdn : null
  gcp_db_endpoint   = var.enable_gcp ? module.gcp_infrastructure[0].cloud_sql_ip : null
  
  # Replication configuration
  enable_logical_replication = true
  replication_mode          = "bidirectional"
  conflict_resolution       = "last-write-wins"
  
  # Data residency
  enable_geo_partitioning = true
  data_retention_days     = 90
  
  tags = local.common_tags
}

# InfluxDB Time-Series Database Clustering
resource "kubernetes_stateful_set" "influxdb_cluster" {
  metadata {
    name      = "${var.project_name}-influxdb-cluster"
    namespace = "monitoring"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "influxdb"
        component = "tsdb"
      }
    }
    
    service_name = "influxdb"
    
    template {
      metadata {
        labels = {
          app       = "influxdb"
          component = "tsdb"
        }
      }
      
      spec {
        container {
          name  = "influxdb"
          image = "influxdb:2.7"
          
          env {
            name  = "INFLUXDB_CLUSTERED_MODE"
            value = "true"
          }
          
          env {
            name  = "INFLUXDB_META_DIR"
            value = "/var/lib/influxdb/meta"
          }
          
          env {
            name  = "INFLUXDB_DATA_DIR"
            value = "/var/lib/influxdb/data"
          }
          
          port {
            container_port = 8086
            name          = "http"
          }
          
          port {
            container_port = 8088
            name          = "rpc"
          }
          
          volume_mount {
            name       = "data"
            mount_path = "/var/lib/influxdb"
          }
        }
        
        affinity {
          pod_anti_affinity {
            required_during_scheduling_ignored_during_execution {
              label_selector {
                match_expressions {
                  key      = "app"
                  operator = "In"
                  values   = ["influxdb"]
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
        name = "data"
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

# Elasticsearch Multi-Cloud Cluster
resource "kubernetes_stateful_set" "elasticsearch_cluster" {
  metadata {
    name      = "${var.project_name}-elasticsearch"
    namespace = "logging"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "elasticsearch"
        component = "search"
      }
    }
    
    service_name = "elasticsearch"
    
    template {
      metadata {
        labels = {
          app       = "elasticsearch"
          component = "search"
        }
      }
      
      spec {
        init_container {
          name    = "increase-vm-max-map"
          image   = "busybox"
          command = ["sysctl", "-w", "vm.max_map_count=262144"]
          
          security_context {
            privileged = true
          }
        }
        
        container {
          name  = "elasticsearch"
          image = "docker.elastic.co/elasticsearch/elasticsearch:8.10.0"
          
          env {
            name  = "cluster.name"
            value = "${var.project_name}-wildlife-search"
          }
          
          env {
            name = "node.name"
            value_from {
              field_ref {
                field_path = "metadata.name"
              }
            }
          }
          
          env {
            name  = "discovery.seed_hosts"
            value = "elasticsearch-0.elasticsearch,elasticsearch-1.elasticsearch,elasticsearch-2.elasticsearch"
          }
          
          env {
            name  = "cluster.initial_master_nodes"
            value = "elasticsearch-0,elasticsearch-1,elasticsearch-2"
          }
          
          env {
            name  = "ES_JAVA_OPTS"
            value = "-Xms2g -Xmx2g"
          }
          
          port {
            container_port = 9200
            name          = "http"
          }
          
          port {
            container_port = 9300
            name          = "transport"
          }
          
          volume_mount {
            name       = "data"
            mount_path = "/usr/share/elasticsearch/data"
          }
          
          resources {
            requests = {
              memory = "4Gi"
              cpu    = "2"
            }
            limits = {
              memory = "4Gi"
              cpu    = "2"
            }
          }
        }
        
        affinity {
          pod_anti_affinity {
            required_during_scheduling_ignored_during_execution {
              label_selector {
                match_expressions {
                  key      = "app"
                  operator = "In"
                  values   = ["elasticsearch"]
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
        name = "data"
      }
      
      spec {
        access_modes = ["ReadWriteOnce"]
        
        resources {
          requests = {
            storage = "200Gi"
          }
        }
      }
    }
  }
}

# Apache Kafka for Real-Time Data Streaming
resource "kubernetes_stateful_set" "kafka_cluster" {
  metadata {
    name      = "${var.project_name}-kafka"
    namespace = "streaming"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "kafka"
        component = "streaming"
      }
    }
    
    service_name = "kafka"
    
    template {
      metadata {
        labels = {
          app       = "kafka"
          component = "streaming"
        }
      }
      
      spec {
        container {
          name  = "kafka"
          image = "confluentinc/cp-kafka:7.5.0"
          
          env {
            name = "KAFKA_BROKER_ID"
            value_from {
              field_ref {
                field_path = "metadata.name"
              }
            }
          }
          
          env {
            name  = "KAFKA_ZOOKEEPER_CONNECT"
            value = "zookeeper:2181"
          }
          
          env {
            name  = "KAFKA_ADVERTISED_LISTENERS"
            value = "PLAINTEXT://$(POD_NAME).kafka:9092"
          }
          
          env {
            name  = "KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR"
            value = "3"
          }
          
          env {
            name  = "KAFKA_DEFAULT_REPLICATION_FACTOR"
            value = "3"
          }
          
          env {
            name  = "KAFKA_MIN_INSYNC_REPLICAS"
            value = "2"
          }
          
          port {
            container_port = 9092
            name          = "kafka"
          }
          
          volume_mount {
            name       = "data"
            mount_path = "/var/lib/kafka/data"
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
        
        affinity {
          pod_anti_affinity {
            required_during_scheduling_ignored_during_execution {
              label_selector {
                match_expressions {
                  key      = "app"
                  operator = "In"
                  values   = ["kafka"]
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
        name = "data"
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

# Object Storage Federation Service
resource "kubernetes_deployment" "storage_federation" {
  metadata {
    name      = "${var.project_name}-storage-federation"
    namespace = "storage"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "storage-federation"
        component = "sync"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "storage-federation"
          component = "sync"
        }
      }
      
      spec {
        container {
          name  = "federation-service"
          image = "rclone/rclone:latest"
          
          command = ["/bin/sh", "-c"]
          
          args = [
            <<-EOT
            rclone sync s3:wildlife-data gs:wildlife-data --multi-thread-streams=8 &&
            rclone sync s3:wildlife-data azure:wildlife-data --multi-thread-streams=8 &&
            rclone sync azure:wildlife-data gs:wildlife-data --multi-thread-streams=8
            EOT
          ]
          
          env {
            name  = "RCLONE_CONFIG"
            value = "/config/rclone.conf"
          }
          
          volume_mount {
            name       = "config"
            mount_path = "/config"
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
          name = "config"
          secret {
            secret_name = "storage-federation-config"
          }
        }
      }
    }
  }
}

# Data Consistency Checker CronJob
resource "kubernetes_cron_job" "data_consistency_check" {
  metadata {
    name      = "${var.project_name}-consistency-check"
    namespace = "monitoring"
  }
  
  spec {
    schedule = "0 */6 * * *" # Every 6 hours
    
    job_template {
      spec {
        template {
          spec {
            container {
              name  = "consistency-checker"
              image = "python:3.11-slim"
              
              command = ["python", "/scripts/check_consistency.py"]
              
              env {
                name  = "AWS_DB_ENDPOINT"
                value = var.enable_aws ? module.aws_infrastructure[0].rds_endpoint : ""
              }
              
              env {
                name  = "AZURE_DB_ENDPOINT"
                value = var.enable_azure ? module.azure_infrastructure[0].postgres_fqdn : ""
              }
              
              env {
                name  = "GCP_DB_ENDPOINT"
                value = var.enable_gcp ? module.gcp_infrastructure[0].cloud_sql_ip : ""
              }
              
              volume_mount {
                name       = "scripts"
                mount_path = "/scripts"
              }
            }
            
            restart_policy = "OnFailure"
            
            volume {
              name = "scripts"
              config_map {
                name = "consistency-check-scripts"
              }
            }
          }
        }
      }
    }
  }
}
