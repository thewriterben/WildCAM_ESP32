# Multi-Cloud Security and Compliance Framework

# HashiCorp Vault for Secrets Management
module "vault" {
  source = "./modules/security/vault"
  
  project_name = var.project_name
  environment  = var.environment
  
  # High availability configuration
  vault_replicas = 3
  enable_ha      = true
  
  # Cloud provider integration
  aws_kms_key_id   = var.enable_aws ? module.aws_infrastructure[0].kms_key_id : null
  azure_key_vault  = var.enable_azure ? module.azure_infrastructure[0].key_vault_id : null
  gcp_kms_key_name = var.enable_gcp ? module.gcp_infrastructure[0].kms_key_name : null
  
  # Auto-unseal configuration
  enable_auto_unseal = true
  seal_type          = "awskms"
  
  tags = local.common_tags
}

# Vault Deployment
resource "kubernetes_stateful_set" "vault" {
  metadata {
    name      = "${var.project_name}-vault"
    namespace = "security"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app       = "vault"
        component = "secrets"
      }
    }
    
    service_name = "vault"
    
    template {
      metadata {
        labels = {
          app       = "vault"
          component = "secrets"
        }
      }
      
      spec {
        service_account_name = "vault"
        
        container {
          name  = "vault"
          image = "hashicorp/vault:1.15"
          
          args = ["server"]
          
          env {
            name = "VAULT_ADDR"
            value = "http://127.0.0.1:8200"
          }
          
          env {
            name = "VAULT_API_ADDR"
            value = "http://$(POD_IP):8200"
          }
          
          env {
            name = "VAULT_CLUSTER_ADDR"
            value = "https://$(POD_IP):8201"
          }
          
          env {
            name = "POD_IP"
            value_from {
              field_ref {
                field_path = "status.podIP"
              }
            }
          }
          
          port {
            container_port = 8200
            name          = "http"
          }
          
          port {
            container_port = 8201
            name          = "https-internal"
          }
          
          volume_mount {
            name       = "vault-config"
            mount_path = "/vault/config"
          }
          
          volume_mount {
            name       = "vault-data"
            mount_path = "/vault/data"
          }
          
          security_context {
            capabilities {
              add = ["IPC_LOCK"]
            }
          }
          
          resources {
            requests = {
              memory = "1Gi"
              cpu    = "500m"
            }
            limits = {
              memory = "2Gi"
              cpu    = "1"
            }
          }
          
          liveness_probe {
            http_get {
              path = "/v1/sys/health?standbyok=true"
              port = 8200
            }
            initial_delay_seconds = 60
            period_seconds        = 5
          }
          
          readiness_probe {
            http_get {
              path = "/v1/sys/health?standbyok=true&sealedcode=204&uninitcode=204"
              port = 8200
            }
            initial_delay_seconds = 5
            period_seconds        = 5
          }
        }
        
        volume {
          name = "vault-config"
          config_map {
            name = "vault-config"
          }
        }
        
        affinity {
          pod_anti_affinity {
            required_during_scheduling_ignored_during_execution {
              label_selector {
                match_expressions {
                  key      = "app"
                  operator = "In"
                  values   = ["vault"]
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
        name = "vault-data"
      }
      
      spec {
        access_modes = ["ReadWriteOnce"]
        
        resources {
          requests = {
            storage = "10Gi"
          }
        }
      }
    }
  }
}

# Vault Configuration
resource "kubernetes_config_map" "vault_config" {
  metadata {
    name      = "vault-config"
    namespace = "security"
  }
  
  data = {
    "vault.hcl" = <<-EOT
      ui = true
      
      listener "tcp" {
        address     = "0.0.0.0:8200"
        tls_disable = 1
      }
      
      storage "raft" {
        path = "/vault/data"
        
        retry_join {
          leader_api_addr = "http://vault-0.vault:8200"
        }
        
        retry_join {
          leader_api_addr = "http://vault-1.vault:8200"
        }
        
        retry_join {
          leader_api_addr = "http://vault-2.vault:8200"
        }
      }
      
      service_registration "kubernetes" {}
      
      seal "awskms" {
        region     = "${var.aws_region}"
        kms_key_id = "${var.enable_aws ? module.aws_infrastructure[0].kms_key_id : ""}"
      }
      
      telemetry {
        prometheus_retention_time = "30s"
        disable_hostname = true
      }
    EOT
  }
}

# External Secrets Operator
resource "kubernetes_deployment" "external_secrets" {
  metadata {
    name      = "${var.project_name}-external-secrets"
    namespace = "security"
  }
  
  spec {
    replicas = 2
    
    selector {
      match_labels = {
        app = "external-secrets"
      }
    }
    
    template {
      metadata {
        labels = {
          app = "external-secrets"
        }
      }
      
      spec {
        service_account_name = "external-secrets"
        
        container {
          name  = "external-secrets"
          image = "ghcr.io/external-secrets/external-secrets:v0.9.0"
          
          args = [
            "--enable-leader-election",
            "--metrics-addr=:8080"
          ]
          
          port {
            container_port = 8080
            name          = "metrics"
          }
          
          resources {
            requests = {
              memory = "256Mi"
              cpu    = "100m"
            }
            limits = {
              memory = "512Mi"
              cpu    = "500m"
            }
          }
        }
      }
    }
  }
}

# Cert-Manager for TLS Certificate Management
resource "kubernetes_deployment" "cert_manager" {
  metadata {
    name      = "${var.project_name}-cert-manager"
    namespace = "cert-manager"
  }
  
  spec {
    replicas = 2
    
    selector {
      match_labels = {
        app = "cert-manager"
      }
    }
    
    template {
      metadata {
        labels = {
          app = "cert-manager"
        }
      }
      
      spec {
        service_account_name = "cert-manager"
        
        container {
          name  = "cert-manager"
          image = "quay.io/jetstack/cert-manager-controller:v1.13.0"
          
          args = [
            "--v=2",
            "--cluster-resource-namespace=$(POD_NAMESPACE)",
            "--leader-election-namespace=kube-system"
          ]
          
          env {
            name = "POD_NAMESPACE"
            value_from {
              field_ref {
                field_path = "metadata.namespace"
              }
            }
          }
          
          resources {
            requests = {
              memory = "256Mi"
              cpu    = "100m"
            }
            limits = {
              memory = "512Mi"
              cpu    = "500m"
            }
          }
        }
      }
    }
  }
}

# Let's Encrypt ClusterIssuer
resource "kubernetes_manifest" "letsencrypt_prod" {
  manifest = {
    apiVersion = "cert-manager.io/v1"
    kind       = "ClusterIssuer"
    
    metadata = {
      name = "letsencrypt-prod"
    }
    
    spec = {
      acme = {
        server = "https://acme-v02.api.letsencrypt.org/directory"
        email  = "ops@wildcam.org"
        
        privateKeySecretRef = {
          name = "letsencrypt-prod"
        }
        
        solvers = [
          {
            http01 = {
              ingress = {
                class = "nginx"
              }
            }
          }
        ]
      }
    }
  }
}

# Falco for Runtime Security
resource "kubernetes_daemonset" "falco" {
  metadata {
    name      = "${var.project_name}-falco"
    namespace = "security"
  }
  
  spec {
    selector {
      match_labels = {
        app       = "falco"
        component = "runtime-security"
      }
    }
    
    template {
      metadata {
        labels = {
          app       = "falco"
          component = "runtime-security"
        }
      }
      
      spec {
        host_network = true
        host_pid     = true
        
        container {
          name  = "falco"
          image = "falcosecurity/falco:0.36.0"
          
          args = [
            "/usr/bin/falco",
            "-K",
            "/var/run/secrets/kubernetes.io/serviceaccount/token",
            "-k",
            "https://kubernetes.default",
            "-pk"
          ]
          
          security_context {
            privileged = true
          }
          
          volume_mount {
            name       = "docker-socket"
            mount_path = "/host/var/run/docker.sock"
          }
          
          volume_mount {
            name       = "dev"
            mount_path = "/host/dev"
          }
          
          volume_mount {
            name       = "proc"
            mount_path = "/host/proc"
            read_only  = true
          }
          
          volume_mount {
            name       = "boot"
            mount_path = "/host/boot"
            read_only  = true
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
          name = "docker-socket"
          host_path {
            path = "/var/run/docker.sock"
          }
        }
        
        volume {
          name = "dev"
          host_path {
            path = "/dev"
          }
        }
        
        volume {
          name = "proc"
          host_path {
            path = "/proc"
          }
        }
        
        volume {
          name = "boot"
          host_path {
            path = "/boot"
          }
        }
      }
    }
  }
}

# OPA (Open Policy Agent) for Policy Enforcement
resource "kubernetes_deployment" "opa" {
  metadata {
    name      = "${var.project_name}-opa"
    namespace = "security"
  }
  
  spec {
    replicas = 3
    
    selector {
      match_labels = {
        app = "opa"
      }
    }
    
    template {
      metadata {
        labels = {
          app = "opa"
        }
      }
      
      spec {
        container {
          name  = "opa"
          image = "openpolicyagent/opa:0.58.0"
          
          args = [
            "run",
            "--server",
            "--addr=0.0.0.0:8181"
          ]
          
          port {
            container_port = 8181
            name          = "http"
          }
          
          liveness_probe {
            http_get {
              path = "/health"
              port = 8181
            }
            initial_delay_seconds = 5
            period_seconds        = 5
          }
          
          readiness_probe {
            http_get {
              path = "/health?plugins"
              port = 8181
            }
            initial_delay_seconds = 5
            period_seconds        = 5
          }
          
          resources {
            requests = {
              memory = "256Mi"
              cpu    = "100m"
            }
            limits = {
              memory = "512Mi"
              cpu    = "500m"
            }
          }
        }
      }
    }
  }
}

# Network Policies for Security
resource "kubernetes_network_policy" "default_deny" {
  metadata {
    name      = "default-deny-all"
    namespace = "default"
  }
  
  spec {
    pod_selector {}
    
    policy_types = ["Ingress", "Egress"]
  }
}

resource "kubernetes_network_policy" "allow_monitoring" {
  metadata {
    name      = "allow-monitoring"
    namespace = "default"
  }
  
  spec {
    pod_selector {
      match_labels = {
        monitoring = "true"
      }
    }
    
    ingress {
      from {
        namespace_selector {
          match_labels = {
            name = "monitoring"
          }
        }
      }
      
      ports {
        protocol = "TCP"
        port     = "9090"
      }
    }
    
    policy_types = ["Ingress"]
  }
}
