# Multi-Cloud Traffic Management and Intelligent Routing

# Global Traffic Manager Configuration
module "traffic_management" {
  source = "./modules/traffic-management"
  
  project_name = var.project_name
  environment  = var.environment
  
  # Cloud endpoints
  aws_endpoint   = var.enable_aws ? module.aws_infrastructure[0].cluster_endpoint : null
  azure_endpoint = var.enable_azure ? module.azure_infrastructure[0].cluster_endpoint : null
  gcp_endpoint   = var.enable_gcp ? module.gcp_infrastructure[0].cluster_endpoint : null
  
  # Geographic routing configuration
  enable_geo_routing = true
  routing_policy     = "latency-based"
  
  # Health check configuration
  health_check_interval  = 30
  health_check_timeout   = 10
  health_check_threshold = 3
  
  # Failover configuration
  enable_automatic_failover = true
  failover_threshold        = 3
  recovery_time_objective   = 300 # 5 minutes RTO
  
  # CDN configuration
  enable_cdn            = true
  cdn_cache_ttl         = 3600
  edge_locations        = ["us-west", "us-east", "eu-west", "ap-south", "ap-east"]
  
  tags = local.common_tags
}

# DNS Configuration with Health-Based Routing
resource "aws_route53_zone" "wildlife_monitoring" {
  count = var.enable_aws ? 1 : 0
  name  = "wildcam.org"
  
  tags = merge(local.common_tags, {
    Name = "${var.project_name}-dns-zone"
  })
}

# Health checks for each cloud provider
resource "aws_route53_health_check" "aws_endpoint" {
  count = var.enable_aws ? 1 : 0
  
  type                            = "HTTPS"
  resource_path                   = "/health"
  fqdn                            = module.aws_infrastructure[0].lb_dns_name
  port                            = 443
  request_interval                = 30
  failure_threshold               = 3
  measure_latency                 = true
  enable_sni                      = true
  
  tags = merge(local.common_tags, {
    Name     = "${var.project_name}-aws-health-check"
    Provider = "AWS"
  })
}

resource "aws_route53_health_check" "azure_endpoint" {
  count = var.enable_azure ? 1 : 0
  
  type                            = "HTTPS"
  resource_path                   = "/health"
  fqdn                            = module.azure_infrastructure[0].lb_fqdn
  port                            = 443
  request_interval                = 30
  failure_threshold               = 3
  measure_latency                 = true
  enable_sni                      = true
  
  tags = merge(local.common_tags, {
    Name     = "${var.project_name}-azure-health-check"
    Provider = "Azure"
  })
}

resource "aws_route53_health_check" "gcp_endpoint" {
  count = var.enable_gcp ? 1 : 0
  
  type                            = "HTTPS"
  resource_path                   = "/health"
  fqdn                            = module.gcp_infrastructure[0].lb_ip
  port                            = 443
  request_interval                = 30
  failure_threshold               = 3
  measure_latency                 = true
  enable_sni                      = true
  
  tags = merge(local.common_tags, {
    Name     = "${var.project_name}-gcp-health-check"
    Provider = "GCP"
  })
}

# Latency-based routing records
resource "aws_route53_record" "api_aws" {
  count = var.enable_aws ? 1 : 0
  
  zone_id = aws_route53_zone.wildlife_monitoring[0].zone_id
  name    = "api.wildcam.org"
  type    = "A"
  
  set_identifier  = "AWS-${var.aws_region}"
  health_check_id = aws_route53_health_check.aws_endpoint[0].id
  
  latency_routing_policy {
    region = var.aws_region
  }
  
  alias {
    name                   = module.aws_infrastructure[0].lb_dns_name
    zone_id                = module.aws_infrastructure[0].lb_zone_id
    evaluate_target_health = true
  }
}

resource "aws_route53_record" "api_azure" {
  count = var.enable_azure ? 1 : 0
  
  zone_id = aws_route53_zone.wildlife_monitoring[0].zone_id
  name    = "api.wildcam.org"
  type    = "CNAME"
  ttl     = 60
  
  set_identifier  = "Azure-${var.azure_location}"
  health_check_id = aws_route53_health_check.azure_endpoint[0].id
  
  latency_routing_policy {
    region = var.aws_region # Route53 requires AWS region
  }
  
  records = [module.azure_infrastructure[0].lb_fqdn]
}

resource "aws_route53_record" "api_gcp" {
  count = var.enable_gcp ? 1 : 0
  
  zone_id = aws_route53_zone.wildlife_monitoring[0].zone_id
  name    = "api.wildcam.org"
  type    = "A"
  ttl     = 60
  
  set_identifier  = "GCP-${var.gcp_region}"
  health_check_id = aws_route53_health_check.gcp_endpoint[0].id
  
  latency_routing_policy {
    region = var.aws_region
  }
  
  records = [module.gcp_infrastructure[0].lb_ip]
}

# CloudFront CDN for Global Content Delivery
resource "aws_cloudfront_distribution" "wildlife_cdn" {
  count   = var.enable_aws ? 1 : 0
  enabled = true
  
  # Multiple origins from different clouds
  dynamic "origin" {
    for_each = var.enable_aws ? [1] : []
    content {
      domain_name = module.aws_infrastructure[0].lb_dns_name
      origin_id   = "AWS-Origin"
      
      custom_origin_config {
        http_port              = 80
        https_port             = 443
        origin_protocol_policy = "https-only"
        origin_ssl_protocols   = ["TLSv1.2"]
      }
    }
  }
  
  dynamic "origin" {
    for_each = var.enable_azure ? [1] : []
    content {
      domain_name = module.azure_infrastructure[0].lb_fqdn
      origin_id   = "Azure-Origin"
      
      custom_origin_config {
        http_port              = 80
        https_port             = 443
        origin_protocol_policy = "https-only"
        origin_ssl_protocols   = ["TLSv1.2"]
      }
    }
  }
  
  dynamic "origin" {
    for_each = var.enable_gcp ? [1] : []
    content {
      domain_name = module.gcp_infrastructure[0].lb_ip
      origin_id   = "GCP-Origin"
      
      custom_origin_config {
        http_port              = 80
        https_port             = 443
        origin_protocol_policy = "https-only"
        origin_ssl_protocols   = ["TLSv1.2"]
      }
    }
  }
  
  # Origin groups for failover
  origin_group {
    origin_id = "Multi-Cloud-Group"
    
    failover_criteria {
      status_codes = [500, 502, 503, 504]
    }
    
    member {
      origin_id = "AWS-Origin"
    }
    
    dynamic "member" {
      for_each = var.enable_azure ? [1] : []
      content {
        origin_id = "Azure-Origin"
      }
    }
  }
  
  default_cache_behavior {
    allowed_methods  = ["DELETE", "GET", "HEAD", "OPTIONS", "PATCH", "POST", "PUT"]
    cached_methods   = ["GET", "HEAD"]
    target_origin_id = "Multi-Cloud-Group"
    
    forwarded_values {
      query_string = true
      headers      = ["Authorization", "Origin"]
      
      cookies {
        forward = "all"
      }
    }
    
    viewer_protocol_policy = "redirect-to-https"
    min_ttl                = 0
    default_ttl            = 3600
    max_ttl                = 86400
    compress               = true
  }
  
  # Cache behaviors for static content
  ordered_cache_behavior {
    path_pattern     = "/static/*"
    allowed_methods  = ["GET", "HEAD"]
    cached_methods   = ["GET", "HEAD"]
    target_origin_id = "Multi-Cloud-Group"
    
    forwarded_values {
      query_string = false
      
      cookies {
        forward = "none"
      }
    }
    
    viewer_protocol_policy = "redirect-to-https"
    min_ttl                = 0
    default_ttl            = 86400
    max_ttl                = 31536000
    compress               = true
  }
  
  # Cache behavior for API requests
  ordered_cache_behavior {
    path_pattern     = "/api/*"
    allowed_methods  = ["DELETE", "GET", "HEAD", "OPTIONS", "PATCH", "POST", "PUT"]
    cached_methods   = ["GET", "HEAD"]
    target_origin_id = "Multi-Cloud-Group"
    
    forwarded_values {
      query_string = true
      headers      = ["*"]
      
      cookies {
        forward = "all"
      }
    }
    
    viewer_protocol_policy = "redirect-to-https"
    min_ttl                = 0
    default_ttl            = 0
    max_ttl                = 0
  }
  
  price_class = "PriceClass_All"
  
  restrictions {
    geo_restriction {
      restriction_type = "none"
    }
  }
  
  viewer_certificate {
    cloudfront_default_certificate = true
  }
  
  tags = merge(local.common_tags, {
    Name = "${var.project_name}-cdn"
  })
}

# AWS Global Accelerator for improved performance
resource "aws_globalaccelerator_accelerator" "wildlife_monitoring" {
  count           = var.enable_aws ? 1 : 0
  name            = "${var.project_name}-global-accelerator"
  ip_address_type = "IPV4"
  enabled         = true
  
  attributes {
    flow_logs_enabled   = true
    flow_logs_s3_bucket = module.aws_infrastructure[0].logs_bucket
    flow_logs_s3_prefix = "global-accelerator/"
  }
  
  tags = local.common_tags
}

resource "aws_globalaccelerator_listener" "wildlife_https" {
  count           = var.enable_aws ? 1 : 0
  accelerator_arn = aws_globalaccelerator_accelerator.wildlife_monitoring[0].id
  protocol        = "TCP"
  
  port_range {
    from_port = 443
    to_port   = 443
  }
}

# Endpoint groups for different regions
resource "aws_globalaccelerator_endpoint_group" "aws_primary" {
  count         = var.enable_aws ? 1 : 0
  listener_arn  = aws_globalaccelerator_listener.wildlife_https[0].id
  endpoint_group_region = var.aws_region
  
  health_check_interval_seconds = 30
  health_check_path             = "/health"
  health_check_protocol         = "HTTPS"
  threshold_count               = 3
  traffic_dial_percentage       = 100
  
  endpoint_configuration {
    endpoint_id = module.aws_infrastructure[0].lb_arn
    weight      = 128
  }
}

resource "aws_globalaccelerator_endpoint_group" "aws_secondary" {
  count         = var.enable_aws ? 1 : 0
  listener_arn  = aws_globalaccelerator_listener.wildlife_https[0].id
  endpoint_group_region = var.aws_secondary_region
  
  health_check_interval_seconds = 30
  health_check_path             = "/health"
  health_check_protocol         = "HTTPS"
  threshold_count               = 3
  traffic_dial_percentage       = 100
  
  endpoint_configuration {
    endpoint_id = module.aws_secondary[0].lb_arn
    weight      = 64
  }
}
