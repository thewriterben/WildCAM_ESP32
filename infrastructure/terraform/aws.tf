# AWS Infrastructure Configuration

module "aws_infrastructure" {
  count  = var.enable_aws ? 1 : 0
  source = "./aws"
  
  project_name = var.project_name
  environment  = var.environment
  region       = var.aws_region
  
  # EKS Configuration
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_instance_type = "t3.xlarge"
  
  # Network Configuration
  vpc_cidr           = "10.0.0.0/16"
  availability_zones = ["${var.aws_region}a", "${var.aws_region}b", "${var.aws_region}c"]
  
  # Storage Configuration
  enable_s3_versioning = true
  s3_lifecycle_rules   = true
  
  # Database Configuration
  enable_rds           = true
  db_instance_class    = "db.r6g.xlarge"
  db_multi_az          = true
  
  # Cost Management
  enable_cost_anomaly_detection = true
  monthly_budget                = var.monthly_cost_budget * 0.33 # 33% of total budget
  
  tags = local.common_tags
}

module "aws_secondary" {
  count    = var.enable_aws ? 1 : 0
  source   = "./aws"
  providers = {
    aws = aws.secondary
  }
  
  project_name = var.project_name
  environment  = "${var.environment}-secondary"
  region       = var.aws_secondary_region
  
  kubernetes_version = var.kubernetes_version
  node_count        = var.node_count
  node_instance_type = "t3.xlarge"
  
  vpc_cidr           = "10.1.0.0/16"
  availability_zones = ["${var.aws_secondary_region}a", "${var.aws_secondary_region}b"]
  
  enable_s3_versioning = true
  enable_rds           = true
  db_instance_class    = "db.r6g.large"
  db_multi_az          = true
  
  tags = merge(local.common_tags, {
    Region = "Secondary"
  })
}

# AWS Cross-Region VPC Peering
resource "aws_vpc_peering_connection" "cross_region" {
  count = var.enable_aws ? 1 : 0
  
  vpc_id        = module.aws_infrastructure[0].vpc_id
  peer_vpc_id   = module.aws_secondary[0].vpc_id
  peer_region   = var.aws_secondary_region
  auto_accept   = false
  
  tags = merge(local.common_tags, {
    Name = "${var.project_name}-cross-region-peering"
  })
}

# S3 Cross-Region Replication
resource "aws_s3_bucket_replication_configuration" "wildlife_data" {
  count = var.enable_aws ? 1 : 0
  
  bucket = module.aws_infrastructure[0].s3_bucket_id
  role   = module.aws_infrastructure[0].replication_role_arn
  
  rule {
    id     = "replicate-all"
    status = "Enabled"
    
    filter {
      prefix = ""
    }
    
    destination {
      bucket        = module.aws_secondary[0].s3_bucket_arn
      storage_class = "STANDARD_IA"
      
      replication_time {
        status = "Enabled"
        time {
          minutes = 15
        }
      }
      
      metrics {
        status = "Enabled"
        event_threshold {
          minutes = 15
        }
      }
    }
    
    delete_marker_replication {
      status = "Enabled"
    }
  }
}

# AWS CloudWatch Alarms for Multi-Cloud Health
resource "aws_cloudwatch_metric_alarm" "cluster_health" {
  count = var.enable_aws ? 1 : 0
  
  alarm_name          = "${var.project_name}-aws-cluster-health"
  comparison_operator = "LessThanThreshold"
  evaluation_periods  = 2
  metric_name         = "cluster_failed_node_count"
  namespace           = "AWS/EKS"
  period              = 300
  statistic           = "Average"
  threshold           = 1
  alarm_description   = "Monitor EKS cluster health"
  treat_missing_data  = "notBreaching"
  
  alarm_actions = [module.aws_infrastructure[0].sns_topic_arn]
}

# Cost Optimization - Reserved Instances Recommendations
resource "aws_ce_cost_category" "wildlife_monitoring" {
  count = var.enable_aws ? 1 : 0
  
  name         = "WildlifeMonitoringCosts"
  rule_version = "CostCategoryExpression.v1"
  
  rule {
    value = "Production"
    rule {
      dimension {
        key           = "LINKED_ACCOUNT"
        values        = [data.aws_caller_identity.current.account_id]
        match_options = ["EQUALS"]
      }
    }
  }
}

data "aws_caller_identity" "current" {}
