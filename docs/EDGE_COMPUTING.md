# Edge Computing Integration for WildCAM ESP32

## Overview

The WildCAM ESP32 platform extends beyond cloud infrastructure to support edge computing deployments at remote field stations. This enables low-latency processing, offline operation, and reduced bandwidth costs for wildlife monitoring in areas with limited connectivity.

## Architecture

```
                    Cloud (AWS/Azure/GCP)
                    ┌─────────────────┐
                    │  Central Cloud  │
                    │   Kubernetes    │
                    └────────┬────────┘
                             │
                ┌────────────┼────────────┐
                │            │            │
           ┌────▼───┐   ┌───▼────┐   ┌──▼─────┐
           │Regional │   │Regional│   │Regional│
           │  Hub    │   │  Hub   │   │  Hub   │
           └────┬────┘   └────┬───┘   └───┬────┘
                │             │            │
        ┌───────┴─────┬───────┴────┬───────┴───────┐
        │             │            │               │
   ┌────▼────┐   ┌───▼────┐  ┌────▼────┐    ┌────▼────┐
   │  Field  │   │ Field  │  │  Field  │    │  Field  │
   │Station 1│   │Station2│  │Station3│    │Station 4│
   │  (K3s)  │   │  (K3s) │  │  (K3s) │    │  (K3s)  │
   └────┬────┘   └────┬───┘  └────┬───┘    └────┬────┘
        │             │           │             │
    ┌───▼──┐      ┌──▼───┐   ┌──▼───┐      ┌──▼───┐
    │ESP32 │      │ESP32 │   │ESP32 │      │ESP32 │
    │Camera│      │Camera│   │Camera│      │Camera│
    └──────┘      └──────┘   └──────┘      └──────┘
```

## K3s Edge Cluster Setup

### Hardware Requirements

**Minimum Configuration (Single Node)**:
- CPU: 4 cores (ARM64 or x86_64)
- RAM: 8GB
- Storage: 128GB SSD
- Network: 4G/5G or Satellite connectivity
- Power: Solar + Battery backup

**Recommended Configuration (3 Node Cluster)**:
- CPU: 8 cores per node
- RAM: 16GB per node
- Storage: 256GB SSD per node
- Network: Redundant connectivity
- Power: Hybrid solar/grid with UPS

### Installation

#### 1. Install K3s on Master Node

```bash
# Install K3s server
curl -sfL https://get.k3s.io | sh -s - server \
  --write-kubeconfig-mode 644 \
  --disable traefik \
  --disable servicelb \
  --node-name edge-master \
  --node-label location=field-station \
  --node-label region=amazon-rainforest \
  --cluster-cidr=10.42.0.0/16 \
  --service-cidr=10.43.0.0/16

# Get node token for workers
sudo cat /var/lib/rancher/k3s/server/node-token
```

#### 2. Install K3s on Worker Nodes

```bash
# Join worker nodes to cluster
curl -sfL https://get.k3s.io | K3S_URL=https://master-ip:6443 \
  K3S_TOKEN=<node-token> sh -s - agent \
  --node-name edge-worker-1 \
  --node-label location=field-station
```

#### 3. Configure kubectl Access

```bash
# Copy kubeconfig from master
scp root@edge-master:/etc/rancher/k3s/k3s.yaml ~/.kube/config-edge

# Update server address
sed -i 's/127.0.0.1/edge-master-ip/g' ~/.kube/config-edge

# Set context
export KUBECONFIG=~/.kube/config-edge
kubectl config use-context default
```

### Edge Application Deployment

#### Deploy WildCAM Edge Stack

```yaml
# wildcam-edge-deployment.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: wildcam-edge
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: wildcam-edge-processor
  namespace: wildcam-edge
spec:
  replicas: 2
  selector:
    matchLabels:
      app: edge-processor
  template:
    metadata:
      labels:
        app: edge-processor
    spec:
      containers:
      - name: processor
        image: wildcam/edge-processor:latest
        resources:
          requests:
            cpu: 500m
            memory: 1Gi
          limits:
            cpu: 2000m
            memory: 4Gi
        env:
        - name: EDGE_MODE
          value: "true"
        - name: CLOUD_SYNC_INTERVAL
          value: "300"  # 5 minutes
        - name: OFFLINE_BUFFER_SIZE
          value: "1000"  # images
        volumeMounts:
        - name: data-buffer
          mountPath: /data/buffer
        - name: ai-models
          mountPath: /models
      volumes:
      - name: data-buffer
        persistentVolumeClaim:
          claimName: edge-buffer-pvc
      - name: ai-models
        persistentVolumeClaim:
          claimName: edge-models-pvc
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: edge-buffer-pvc
  namespace: wildcam-edge
spec:
  accessModes:
  - ReadWriteOnce
  resources:
    requests:
      storage: 100Gi
  storageClassName: local-path
---
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: edge-models-pvc
  namespace: wildcam-edge
spec:
  accessModes:
  - ReadWriteOnce
  resources:
    requests:
      storage: 10Gi
  storageClassName: local-path
```

Apply the configuration:
```bash
kubectl apply -f wildcam-edge-deployment.yaml
```

## Edge AI Model Management

### Model Synchronization

**Architecture**:
1. Models trained in cloud (AWS SageMaker, Azure ML, Vertex AI)
2. Optimized for edge deployment (TensorFlow Lite, ONNX)
3. Distributed to edge clusters via GitOps
4. Automatic model updates with version control

**Deployment Script**:
```bash
#!/bin/bash
# sync-edge-models.sh

CLOUD_BUCKET="s3://wildcam-models/edge"
EDGE_PATH="/models"
MODEL_VERSION="v2.1.0"

# Download latest models
aws s3 sync ${CLOUD_BUCKET}/${MODEL_VERSION}/ ${EDGE_PATH}/

# Verify model checksums
sha256sum -c ${EDGE_PATH}/checksums.txt

# Update model registry
kubectl create configmap edge-models \
  --from-literal=version=${MODEL_VERSION} \
  --from-literal=path=${EDGE_PATH} \
  --namespace wildcam-edge \
  --dry-run=client -o yaml | kubectl apply -f -

# Restart processors to load new models
kubectl rollout restart deployment/wildcam-edge-processor -n wildcam-edge
```

### Supported Models

- **Species Detection**: YOLO-tiny optimized for 150+ species
- **Behavior Classification**: MobileNet for activity recognition
- **Image Quality**: Lightweight blur/exposure detection
- **Anomaly Detection**: One-class SVM for unusual events

## Offline-First Architecture

### Data Buffering

```yaml
# edge-buffer-config.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: edge-buffer-config
  namespace: wildcam-edge
data:
  buffer-size: "1000"  # Maximum images to buffer
  buffer-priority: "detection-first"  # Prioritize detections
  compression: "jpeg-85"  # JPEG quality 85%
  retry-interval: "300"  # 5 minutes between sync attempts
  max-retry-age: "604800"  # 7 days
```

### Sync Strategy

**Priority Levels**:
1. **Critical**: Endangered species detections (sync immediately)
2. **High**: New species detections (sync within 5 minutes)
3. **Medium**: Regular wildlife images (sync within 1 hour)
4. **Low**: Calibration/test images (sync when bandwidth available)

**Implementation**:
```python
# edge_sync_manager.py
import asyncio
from enum import Enum

class SyncPriority(Enum):
    CRITICAL = 1
    HIGH = 2
    MEDIUM = 3
    LOW = 4

class EdgeSyncManager:
    def __init__(self, buffer_size=1000):
        self.buffer = []
        self.buffer_size = buffer_size
        self.online = False
        
    async def queue_upload(self, image_data, priority=SyncPriority.MEDIUM):
        """Queue image for upload with priority"""
        self.buffer.append({
            'data': image_data,
            'priority': priority,
            'timestamp': time.time()
        })
        
        # Sort by priority
        self.buffer.sort(key=lambda x: (x['priority'].value, x['timestamp']))
        
        # Enforce buffer size
        if len(self.buffer) > self.buffer_size:
            self.buffer = self.buffer[:self.buffer_size]
        
        # Attempt sync if online
        if self.online:
            await self.sync_buffer()
    
    async def sync_buffer(self):
        """Sync buffered data to cloud"""
        while self.buffer and self.online:
            item = self.buffer[0]
            try:
                await self.upload_to_cloud(item['data'])
                self.buffer.pop(0)
            except Exception as e:
                logger.error(f"Upload failed: {e}")
                self.online = False
                break
```

## Network Resilience

### Connectivity Monitoring

```bash
#!/bin/bash
# monitor-connectivity.sh

CLOUD_ENDPOINT="https://api.wildcam.example.com/health"
PING_INTERVAL=60  # seconds

while true; do
    if curl -s --max-time 5 "$CLOUD_ENDPOINT" > /dev/null 2>&1; then
        echo "$(date): Cloud connection: UP"
        kubectl label node edge-master connectivity=online --overwrite
    else
        echo "$(date): Cloud connection: DOWN"
        kubectl label node edge-master connectivity=offline --overwrite
    fi
    sleep $PING_INTERVAL
done
```

### Bandwidth Optimization

**Techniques**:
- Image compression (JPEG 85% quality, ~70% size reduction)
- Delta sync (only changed files)
- Scheduled sync windows (off-peak hours)
- Adaptive bitrate based on connection speed
- Deduplication (hash-based)

**Configuration**:
```yaml
# bandwidth-optimizer-config.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: bandwidth-optimizer
  namespace: wildcam-edge
data:
  compression-quality: "85"
  max-upload-rate: "1048576"  # 1MB/s
  sync-window-start: "02:00"
  sync-window-end: "06:00"
  adaptive-quality: "true"
  deduplication: "true"
```

## Power Management

### Solar Power Integration

**Monitoring**:
```bash
# Install power monitoring
kubectl apply -f - <<EOF
apiVersion: v1
kind: ConfigMap
metadata:
  name: power-config
  namespace: wildcam-edge
data:
  battery-min: "20"  # Minimum battery percentage
  solar-threshold: "50"  # Minimum solar wattage
  low-power-mode: "true"
---
apiVersion: v1
kind: Pod
metadata:
  name: power-monitor
  namespace: wildcam-edge
spec:
  containers:
  - name: monitor
    image: wildcam/power-monitor:latest
    volumeMounts:
    - name: battery-status
      mountPath: /sys/class/power_supply
  volumes:
  - name: battery-status
    hostPath:
      path: /sys/class/power_supply
EOF
```

**Power-Aware Scheduling**:
```yaml
# power-aware-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: wildcam-edge-processor
  namespace: wildcam-edge
spec:
  template:
    spec:
      affinity:
        nodeAffinity:
          requiredDuringSchedulingIgnoredDuringExecution:
            nodeSelectorTerms:
            - matchExpressions:
              - key: battery-level
                operator: Gt
                values:
                - "30"
```

## Security at the Edge

### Device Authentication

```bash
# Generate device certificates
openssl req -x509 -newkey rsa:4096 \
  -keyout edge-device.key \
  -out edge-device.crt \
  -days 365 -nodes \
  -subj "/CN=edge-station-001/O=WildCAM/C=US"

# Create Kubernetes secret
kubectl create secret tls edge-device-cert \
  --cert=edge-device.crt \
  --key=edge-device.key \
  --namespace wildcam-edge
```

### Encrypted Communication

All edge-to-cloud communication uses:
- **TLS 1.3**: End-to-end encryption
- **mTLS**: Mutual authentication
- **VPN**: IPsec tunnel for additional security

## Monitoring and Alerts

### Edge Metrics

```yaml
# prometheus-edge-config.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: prometheus-edge-config
  namespace: monitoring
data:
  prometheus.yml: |
    global:
      scrape_interval: 60s
      external_labels:
        cluster: 'edge'
        location: 'field-station-001'
    
    scrape_configs:
    - job_name: 'edge-nodes'
      static_configs:
      - targets: ['localhost:9100']
    
    - job_name: 'edge-applications'
      kubernetes_sd_configs:
      - role: pod
        namespaces:
          names:
          - wildcam-edge
```

### Alert Rules

```yaml
# edge-alerts.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: edge-alert-rules
  namespace: monitoring
data:
  alerts.yml: |
    groups:
    - name: edge-alerts
      rules:
      - alert: EdgeOffline
        expr: up{job="edge-nodes"} == 0
        for: 10m
        annotations:
          summary: "Edge node offline"
      
      - alert: LowBattery
        expr: battery_percent < 20
        annotations:
          summary: "Battery level critical"
      
      - alert: BufferFull
        expr: edge_buffer_size > 900
        annotations:
          summary: "Edge buffer nearly full"
```

## Deployment Checklist

### Pre-Deployment
- [ ] Hardware provisioned and tested
- [ ] Network connectivity verified (4G/5G/Satellite)
- [ ] Solar panels and batteries installed
- [ ] K3s installed and configured
- [ ] Edge applications deployed
- [ ] AI models synced
- [ ] Certificates generated

### Post-Deployment
- [ ] Connectivity monitoring active
- [ ] Power monitoring configured
- [ ] First sync completed successfully
- [ ] ESP32 cameras connected
- [ ] Metrics collection verified
- [ ] Alert rules tested
- [ ] Documentation updated

## Troubleshooting

### Common Issues

**Issue: K3s won't start**
```bash
# Check logs
sudo journalctl -u k3s -f

# Reset K3s
sudo systemctl stop k3s
sudo rm -rf /var/lib/rancher/k3s
sudo systemctl start k3s
```

**Issue: Edge offline for extended period**
```bash
# Check network connectivity
ping -c 4 8.8.8.8

# Check VPN tunnel
ipsec status

# Force sync attempt
kubectl exec -it -n wildcam-edge deployment/wildcam-edge-processor -- \
  python3 /app/force_sync.py
```

**Issue: Models not loading**
```bash
# Verify model files
ls -lh /models/

# Check model registry
kubectl get configmap edge-models -n wildcam-edge -o yaml

# Re-sync models
./sync-edge-models.sh
```

## Future Enhancements

- **5G Integration**: Native 5G modem support
- **StarLink Support**: Direct satellite connectivity
- **Edge Federation**: Multi-edge cluster coordination
- **Local AI Training**: Federated learning at edge
- **Mesh Networking**: Edge-to-edge direct communication

---

**Last Updated**: 2024-10-14
**Version**: 1.0.0
**Maintained By**: Edge Computing Team
