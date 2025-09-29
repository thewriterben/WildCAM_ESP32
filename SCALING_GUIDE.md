# 📈 WildCAM ESP32 Scaling Guide

This guide provides detailed instructions for scaling the ESP32 WildCAM system to handle increased load, more cameras, and larger datasets.

## 🎯 Scaling Scenarios

### Small Scale (10-50 cameras)
- **Backend**: 2-3 replicas
- **Database**: Single PostgreSQL instance
- **Storage**: Local or S3
- **Monitoring**: Basic Prometheus + Grafana

### Medium Scale (50-500 cameras)
- **Backend**: 5-10 replicas with load balancing
- **Database**: PostgreSQL with read replicas
- **Storage**: S3 with CDN
- **Monitoring**: Full ELK stack + Prometheus

### Large Scale (500+ cameras)
- **Backend**: Auto-scaling (10-50 replicas)
- **Database**: Partitioned PostgreSQL cluster
- **Storage**: Multi-region S3 with edge caching
- **Monitoring**: Distributed monitoring with federation

## 🔧 Horizontal Scaling

### Kubernetes Auto-scaling

#### Configure Horizontal Pod Autoscaler (HPA)
```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: backend-hpa
  namespace: wildcam
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: backend
  minReplicas: 3
  maxReplicas: 20
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
  - type: Pods
    pods:
      metric:
        name: http_requests_per_second
      target:
        type: AverageValue
        averageValue: "100"
  behavior:
    scaleDown:
      stabilizationWindowSeconds: 300
      policies:
      - type: Percent
        value: 50
        periodSeconds: 60
    scaleUp:
      stabilizationWindowSeconds: 60
      policies:
      - type: Percent
        value: 100
        periodSeconds: 30
```

#### Configure Vertical Pod Autoscaler (VPA)
```yaml
apiVersion: autoscaling.k8s.io/v1
kind: VerticalPodAutoscaler
metadata:
  name: backend-vpa
  namespace: wildcam
spec:
  targetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: backend
  updatePolicy:
    updateMode: "Auto"
  resourcePolicy:
    containerPolicies:
    - containerName: backend
      maxAllowed:
        cpu: 2
        memory: 4Gi
      minAllowed:
        cpu: 100m
        memory: 256Mi
```

### Docker Compose Scaling

```bash
# Scale backend service
docker compose -f docker-compose.prod.yml up -d --scale backend=5

# Scale with specific resource limits
docker compose -f docker-compose.prod.yml up -d --scale backend=5 --scale frontend=3
```

## 🗄️ Database Scaling

### PostgreSQL Read Replicas

#### Master-Slave Configuration
```yaml
# PostgreSQL Master
apiVersion: apps/v1
kind: Deployment
metadata:
  name: postgres-master
spec:
  replicas: 1
  template:
    spec:
      containers:
      - name: postgres
        image: postgres:14
        env:
        - name: POSTGRES_REPLICATION_MODE
          value: master
        - name: POSTGRES_REPLICATION_USER
          value: replicator
        - name: POSTGRES_REPLICATION_PASSWORD
          valueFrom:
            secretKeyRef:
              name: postgres-secrets
              key: replication-password

---
# PostgreSQL Read Replica
apiVersion: apps/v1
kind: Deployment
metadata:
  name: postgres-replica
spec:
  replicas: 2
  template:
    spec:
      containers:
      - name: postgres
        image: postgres:14
        env:
        - name: POSTGRES_REPLICATION_MODE
          value: slave
        - name: POSTGRES_MASTER_SERVICE
          value: postgres-master-service
        - name: POSTGRES_REPLICATION_USER
          value: replicator
        - name: POSTGRES_REPLICATION_PASSWORD
          valueFrom:
            secretKeyRef:
              name: postgres-secrets
              key: replication-password
```

#### Connection Pooling with PgBouncer
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: pgbouncer
spec:
  replicas: 2
  template:
    spec:
      containers:
      - name: pgbouncer
        image: pgbouncer/pgbouncer:latest
        env:
        - name: POOL_MODE
          value: "transaction"
        - name: MAX_CLIENT_CONN
          value: "1000"
        - name: DEFAULT_POOL_SIZE
          value: "25"
        - name: RESERVE_POOL_SIZE
          value: "5"
        volumeMounts:
        - name: pgbouncer-config
          mountPath: /etc/pgbouncer
```

### Database Partitioning

```sql
-- Partition wildlife_detections table by date
CREATE TABLE wildlife_detections_2024_01 PARTITION OF wildlife_detections
FOR VALUES FROM ('2024-01-01') TO ('2024-02-01');

CREATE TABLE wildlife_detections_2024_02 PARTITION OF wildlife_detections
FOR VALUES FROM ('2024-02-01') TO ('2024-03-01');

-- Create indexes for each partition
CREATE INDEX idx_wildlife_detections_2024_01_camera_id 
ON wildlife_detections_2024_01 (camera_id);

CREATE INDEX idx_wildlife_detections_2024_01_timestamp 
ON wildlife_detections_2024_01 (detection_timestamp);
```

## 📦 Storage Scaling

### S3 Configuration with Multiple Regions

```python
# backend/storage/s3_client.py
import boto3
from botocore.config import Config

class MultiRegionS3Client:
    def __init__(self):
        self.regions = ['us-west-2', 'us-east-1', 'eu-west-1']
        self.clients = {}
        for region in self.regions:
            self.clients[region] = boto3.client(
                's3',
                region_name=region,
                config=Config(
                    region_name=region,
                    retries={'max_attempts': 3},
                    max_pool_connections=50
                )
            )
    
    def upload_with_failover(self, file_path, key):
        for region in self.regions:
            try:
                self.clients[region].upload_file(
                    file_path, 
                    f'wildcam-{region}', 
                    key
                )
                return f'wildcam-{region}'
            except Exception as e:
                continue
        raise Exception("All regions failed")
```

### CDN Configuration

```yaml
# CloudFront distribution for image delivery
apiVersion: v1
kind: ConfigMap
metadata:
  name: cdn-config
data:
  cloudfront.yaml: |
    Distribution:
      DistributionConfig:
        CallerReference: wildcam-images-cdn
        DefaultRootObject: index.html
        Origins:
          - Id: s3-origin
            DomainName: wildcam-images.s3.amazonaws.com
            S3OriginConfig:
              OriginAccessIdentity: ""
        DefaultCacheBehavior:
          TargetOriginId: s3-origin
          ViewerProtocolPolicy: redirect-to-https
          Compress: true
          CachePolicyId: 4135ea2d-6df8-44a3-9df3-4b5a84be39ad
        CacheBehaviors:
          - PathPattern: "/api/*"
            TargetOriginId: s3-origin
            ViewerProtocolPolicy: https-only
            CachePolicyId: 4135ea2d-6df8-44a3-9df3-4b5a84be39ad
            TTL:
              DefaultTTL: 3600
              MaxTTL: 86400
```

## 🚀 Performance Optimization

### Backend Optimization

#### Async Processing with Celery
```python
# backend/tasks.py
from celery import Celery
import cv2
import numpy as np

celery_app = Celery('wildcam', broker='redis://redis:6379/0')

@celery_app.task
def process_wildlife_image(image_path, camera_id):
    # Load and process image
    image = cv2.imread(image_path)
    
    # Run AI detection
    detections = run_wildlife_detection(image)
    
    # Store results
    store_detection_results(camera_id, detections)
    
    # Generate thumbnails
    generate_thumbnails(image_path)
    
    return detections

@celery_app.task
def generate_analytics_report(start_date, end_date):
    # Generate heavy analytics in background
    pass
```

#### Caching Strategy
```python
# backend/cache.py
import redis
import json
from functools import wraps

redis_client = redis.Redis(host='redis', port=6379, db=0)

def cache_result(expiration=3600):
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            cache_key = f"{func.__name__}:{hash(str(args) + str(kwargs))}"
            
            # Try to get from cache
            cached = redis_client.get(cache_key)
            if cached:
                return json.loads(cached)
            
            # Execute function and cache result
            result = func(*args, **kwargs)
            redis_client.setex(
                cache_key, 
                expiration, 
                json.dumps(result, default=str)
            )
            return result
        return wrapper
    return decorator

@cache_result(expiration=1800)
def get_camera_statistics(camera_id, days=7):
    # Heavy database query
    return calculate_camera_stats(camera_id, days)
```

### Database Optimization

#### Query Optimization
```sql
-- Create composite indexes for common queries
CREATE INDEX CONCURRENTLY idx_wildlife_detections_camera_date 
ON wildlife_detections (camera_id, detection_timestamp DESC);

CREATE INDEX CONCURRENTLY idx_wildlife_detections_species_confidence 
ON wildlife_detections (species, confidence) 
WHERE confidence > 0.8;

-- Materialized view for analytics
CREATE MATERIALIZED VIEW daily_detection_summary AS
SELECT 
    camera_id,
    DATE(detection_timestamp) as detection_date,
    species,
    COUNT(*) as detection_count,
    AVG(confidence) as avg_confidence
FROM wildlife_detections 
WHERE confidence > 0.7
GROUP BY camera_id, DATE(detection_timestamp), species;

-- Refresh materialized view periodically
CREATE OR REPLACE FUNCTION refresh_daily_summary()
RETURNS void AS $$
BEGIN
    REFRESH MATERIALIZED VIEW CONCURRENTLY daily_detection_summary;
END;
$$ LANGUAGE plpgsql;
```

#### Connection Pool Configuration
```python
# backend/database.py
from sqlalchemy import create_engine
from sqlalchemy.pool import QueuePool

engine = create_engine(
    DATABASE_URL,
    poolclass=QueuePool,
    pool_size=20,
    max_overflow=30,
    pool_pre_ping=True,
    pool_recycle=3600,
    echo=False
)
```

## 📊 Monitoring at Scale

### Prometheus Federation

```yaml
# prometheus-federation.yml
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: 'federate'
    scrape_interval: 15s
    honor_labels: true
    metrics_path: '/federate'
    params:
      'match[]':
        - '{job=~"wildcam-.*"}'
        - '{__name__=~"job:.*"}'
    static_configs:
      - targets:
        - 'prometheus-region1:9090'
        - 'prometheus-region2:9090'
        - 'prometheus-region3:9090'
```

### Custom Metrics for Scaling

```python
# backend/metrics.py
from prometheus_client import Counter, Histogram, Gauge, start_http_server

# Custom metrics
wildlife_detections_total = Counter(
    'wildcam_wildlife_detections_total',
    'Total wildlife detections',
    ['camera_id', 'species']
)

detection_processing_time = Histogram(
    'wildcam_detection_processing_seconds',
    'Time spent processing detections',
    ['camera_id']
)

active_cameras = Gauge(
    'wildcam_active_cameras_total',
    'Number of active cameras'
)

camera_last_seen = Gauge(
    'wildcam_camera_last_seen_timestamp',
    'Timestamp of last camera activity',
    ['camera_id', 'location']
)

queue_size = Gauge(
    'wildcam_processing_queue_size',
    'Number of items in processing queue'
)

def update_camera_metrics():
    # Update metrics based on current system state
    cameras = get_active_cameras()
    active_cameras.set(len(cameras))
    
    for camera in cameras:
        camera_last_seen.labels(
            camera_id=camera.id,
            location=camera.location
        ).set(camera.last_seen.timestamp())

def record_detection(camera_id, species, processing_time):
    wildlife_detections_total.labels(
        camera_id=camera_id,
        species=species
    ).inc()
    
    detection_processing_time.labels(
        camera_id=camera_id
    ).observe(processing_time)
```

## 🔄 Load Testing

### K6 Load Testing Script

```javascript
// loadtest.js
import http from 'k6/http';
import { check, sleep } from 'k6';

export let options = {
  stages: [
    { duration: '2m', target: 10 },   // Ramp up
    { duration: '5m', target: 10 },   // Stay at 10 users
    { duration: '2m', target: 20 },   // Ramp up to 20 users
    { duration: '5m', target: 20 },   // Stay at 20 users
    { duration: '2m', target: 0 },    // Ramp down
  ],
};

export default function() {
  // Test API health endpoint
  let healthResponse = http.get('http://localhost/api/health');
  check(healthResponse, {
    'health status is 200': (r) => r.status === 200,
    'health response time < 500ms': (r) => r.timings.duration < 500,
  });
  
  // Test camera list endpoint
  let camerasResponse = http.get('http://localhost/api/cameras');
  check(camerasResponse, {
    'cameras status is 200': (r) => r.status === 200,
    'cameras response time < 1000ms': (r) => r.timings.duration < 1000,
  });
  
  // Test detection endpoint
  let detectionsResponse = http.get('http://localhost/api/detections?limit=10');
  check(detectionsResponse, {
    'detections status is 200': (r) => r.status === 200,
    'detections response time < 2000ms': (r) => r.timings.duration < 2000,
  });
  
  sleep(1);
}
```

### Running Load Tests

```bash
# Install k6
sudo apt-get install k6

# Run load test
k6 run loadtest.js

# Run with specific VUs and duration
k6 run --vus 50 --duration 10m loadtest.js

# Generate reports
k6 run --out json=loadtest-results.json loadtest.js
```

## 🛡️ Security at Scale

### Rate Limiting Configuration

```yaml
# nginx rate limiting for high traffic
http {
  # Zone definitions for different endpoints
  limit_req_zone $binary_remote_addr zone=api:10m rate=100r/s;
  limit_req_zone $binary_remote_addr zone=auth:10m rate=5r/s;
  limit_req_zone $binary_remote_addr zone=upload:10m rate=1r/s;
  limit_req_zone $camera_id zone=camera:10m rate=10r/s;
  
  # Geographic rate limiting
  geo $limited_country {
    default 0;
    CN 1;
    RU 1;
  }
  
  map $limited_country $limit_key {
    0 "";
    1 $binary_remote_addr;
  }
  
  limit_req_zone $limit_key zone=country:10m rate=1r/s;
  
  server {
    # Apply rate limits
    location /api/ {
      limit_req zone=api burst=200 nodelay;
      limit_req zone=country burst=10 nodelay;
    }
    
    location /api/cameras/upload {
      limit_req zone=upload burst=5 nodelay;
      limit_req zone=camera burst=20 nodelay;
    }
  }
}
```

### DDoS Protection

```yaml
# Kubernetes Network Policy
apiVersion: networking.k8s.io/v1
kind: NetworkPolicy
metadata:
  name: wildcam-network-policy
  namespace: wildcam
spec:
  podSelector: {}
  policyTypes:
  - Ingress
  - Egress
  ingress:
  - from:
    - namespaceSelector:
        matchLabels:
          name: ingress-nginx
    - podSelector:
        matchLabels:
          app: nginx
    ports:
    - protocol: TCP
      port: 5000
  egress:
  - to:
    - podSelector:
        matchLabels:
          app: postgres
    ports:
    - protocol: TCP
      port: 5432
  - to:
    - podSelector:
        matchLabels:
          app: redis
    ports:
    - protocol: TCP
      port: 6379
```

## 📈 Capacity Planning

### Resource Requirements by Scale

| Scale | Cameras | Backend CPU | Backend Memory | Database CPU | Database Memory | Storage/Month |
|-------|---------|-------------|----------------|--------------|-----------------|---------------|
| Small | 10-50 | 2 cores | 4GB | 2 cores | 4GB | 100GB |
| Medium | 50-500 | 8 cores | 16GB | 4 cores | 8GB | 1TB |
| Large | 500+ | 32+ cores | 64GB+ | 16+ cores | 32GB+ | 10TB+ |

### Scaling Checklist

- [ ] **Database**: Configure read replicas and connection pooling
- [ ] **Caching**: Implement Redis cluster for session and data caching
- [ ] **Storage**: Set up multi-region S3 with CDN
- [ ] **Monitoring**: Configure distributed monitoring and alerting
- [ ] **Load Balancing**: Implement proper load balancing with health checks
- [ ] **Auto-scaling**: Configure HPA and VPA for automatic scaling
- [ ] **Security**: Implement rate limiting and DDoS protection
- [ ] **Backup**: Set up automated backups and disaster recovery
- [ ] **Performance**: Optimize queries and implement async processing

## 🚨 Troubleshooting Scale Issues

### Common Scaling Problems

1. **Database Connection Exhaustion**
   ```bash
   # Monitor connection count
   kubectl exec postgres-0 -- psql -U wildlife_user -d wildlife_db -c "SELECT count(*) FROM pg_stat_activity;"
   
   # Implement connection pooling
   kubectl apply -f k8s/pgbouncer.yaml
   ```

2. **High Memory Usage**
   ```bash
   # Check memory usage
   kubectl top pods -n wildcam
   
   # Adjust resource limits
   kubectl patch deployment backend -p '{"spec":{"template":{"spec":{"containers":[{"name":"backend","resources":{"limits":{"memory":"2Gi"}}}]}}}}'
   ```

3. **Slow Query Performance**
   ```sql
   -- Monitor slow queries
   SELECT query, mean_time, calls, rows 
   FROM pg_stat_statements 
   ORDER BY mean_time DESC 
   LIMIT 10;
   
   -- Create missing indexes
   CREATE INDEX CONCURRENTLY idx_missing_index ON table_name (column_name);
   ```

This scaling guide provides comprehensive strategies for growing your WildCAM deployment from a small pilot to a large-scale wildlife monitoring system capable of handling thousands of cameras and processing millions of images.