# Security Module for Multi-Cloud WildCAM ESP32
# Implements zero-trust architecture, WAF, DDoS protection, and certificate management

locals {
  name_prefix = "${var.project_name}-${var.environment}"
}

# AWS Certificate Manager - SSL/TLS Certificates
resource "aws_acm_certificate" "main" {
  count             = var.enable_zero_trust ? 1 : 0
  domain_name       = var.domain_name
  validation_method = "DNS"

  subject_alternative_names = [
    "*.${var.domain_name}"
  ]

  lifecycle {
    create_before_destroy = true
  }

  tags = merge(var.tags, {
    Name = "${local.name_prefix}-certificate"
  })
}

# AWS WAF Web ACL
resource "aws_wafv2_web_acl" "main" {
  count       = var.enable_waf ? 1 : 0
  name        = "${local.name_prefix}-waf"
  description = "WAF for WildCAM ESP32"
  scope       = "REGIONAL"

  default_action {
    allow {}
  }

  # Rate limiting rule
  rule {
    name     = "RateLimitRule"
    priority = 1

    action {
      block {}
    }

    statement {
      rate_based_statement {
        limit              = 2000
        aggregate_key_type = "IP"
      }
    }

    visibility_config {
      cloudwatch_metrics_enabled = true
      metric_name                = "RateLimitRule"
      sampled_requests_enabled   = true
    }
  }

  # AWS Managed Rules - Common Rule Set
  rule {
    name     = "AWSManagedRulesCommonRuleSet"
    priority = 2

    override_action {
      none {}
    }

    statement {
      managed_rule_group_statement {
        name        = "AWSManagedRulesCommonRuleSet"
        vendor_name = "AWS"
      }
    }

    visibility_config {
      cloudwatch_metrics_enabled = true
      metric_name                = "AWSManagedRulesCommonRuleSet"
      sampled_requests_enabled   = true
    }
  }

  # SQL Injection Protection
  rule {
    name     = "AWSManagedRulesSQLiRuleSet"
    priority = 3

    override_action {
      none {}
    }

    statement {
      managed_rule_group_statement {
        name        = "AWSManagedRulesSQLiRuleSet"
        vendor_name = "AWS"
      }
    }

    visibility_config {
      cloudwatch_metrics_enabled = true
      metric_name                = "AWSManagedRulesSQLiRuleSet"
      sampled_requests_enabled   = true
    }
  }

  visibility_config {
    cloudwatch_metrics_enabled = true
    metric_name                = "${local.name_prefix}-waf"
    sampled_requests_enabled   = true
  }

  tags = var.tags
}

# AWS Shield Advanced (DDoS Protection)
# Note: Shield Advanced is a paid service with significant cost
# For production, consider enabling this resource

# AWS Secrets Manager for credential rotation
resource "aws_secretsmanager_secret" "api_keys" {
  count                   = var.enable_zero_trust ? 1 : 0
  name                    = "${local.name_prefix}-api-keys"
  description             = "API keys for WildCAM ESP32 platform"
  recovery_window_in_days = 7

  rotation_rules {
    automatically_after_days = var.encryption_key_rotation_days
  }

  tags = var.tags
}

# IAM Role for Secrets Rotation Lambda
resource "aws_iam_role" "secrets_rotation" {
  count = var.enable_zero_trust ? 1 : 0
  name  = "${local.name_prefix}-secrets-rotation-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Action = "sts:AssumeRole"
      Effect = "Allow"
      Principal = {
        Service = "lambda.amazonaws.com"
      }
    }]
  })

  tags = var.tags
}

# Output security configurations
output "security_enabled" {
  description = "Security features enabled"
  value = {
    zero_trust        = var.enable_zero_trust
    waf_enabled       = var.enable_waf
    ddos_protection   = var.enable_ddos_protection
    certificate_arn   = var.enable_zero_trust && length(aws_acm_certificate.main) > 0 ? aws_acm_certificate.main[0].arn : null
    waf_acl_arn       = var.enable_waf && length(aws_wafv2_web_acl.main) > 0 ? aws_wafv2_web_acl.main[0].arn : null
  }
}
