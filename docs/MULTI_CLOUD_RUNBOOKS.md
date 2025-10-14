# Multi-Cloud Operational Runbooks

## Overview
This document contains step-by-step procedures for common operational tasks in the WildCAM multi-cloud deployment.

## Table of Contents
- [Emergency Procedures](#emergency-procedures)
- [Routine Maintenance](#routine-maintenance)
- [Failover Procedures](#failover-procedures)
- [Scaling Operations](#scaling-operations)
- [Disaster Recovery](#disaster-recovery)
- [Security Incidents](#security-incidents)

---

## Emergency Procedures

### 🚨 Complete Cloud Provider Outage

**Symptoms:**
- All health checks failing for one cloud provider
- 5xx errors from specific provider
- Alerts: "CloudProviderDown"

**Impact:** Reduced capacity but service continues from other providers

**Procedure:**

1. **Verify the Outage**
   ```bash
   # Check health status
   kubectl get healthchecks -n monitoring
   
   # Check provider status pages
   # AWS: https://status.aws.amazon.com/
   # Azure: https://status.azure.com/
   # GCP: https://status.cloud.google.com/
   ```

2. **Confirm Automatic Failover**
   ```bash
   # Check traffic routing
   kubectl logs -n ingress-nginx ingress-nginx-controller | grep failover
   
   # Verify traffic distribution
   kubectl top pods -n wildcam --sort-by=cpu
   ```

3. **Scale Remaining Providers**
   ```bash
   # If needed, temporarily increase capacity
   kubectl scale deployment wildcam-backend --replicas=10 -n wildcam
   
   # On AWS (if Azure/GCP down)
   kubectl --context aws-west scale deployment wildcam-backend --replicas=15
   kubectl --context aws-east scale deployment wildcam-backend --replicas=15
   ```

4. **Monitor Performance**
   ```bash
   # Watch response times
   kubectl top pods -n wildcam --watch
   
   # Check error rates
   kubectl logs -f deployment/wildcam-backend -n wildcam | grep -i error
   ```

5. **Communication**
   - Post status update to status page
   - Notify stakeholders via Slack/email
   - Update incident timeline

6. **Recovery Actions**
   Once provider recovers:
   ```bash
   # Verify health
   kubectl get healthchecks -n monitoring
   
   # Gradual traffic restoration (10% increments)
   kubectl annotate service wildcam-backend \
     traffic-policy='{"aws": 33, "azure": 33, "gcp": 34}'
   
   # Monitor for 15 minutes at each step
   watch -n 60 'kubectl top pods -n wildcam'
   ```

**Prevention:**
- Weekly chaos engineering tests
- Automated failover drills
- Provider diversity (avoid single points of failure)

---

### 🔥 Database Replication Failure

**Symptoms:**
- Replication lag > 60 seconds
- Inconsistent data across regions
- Alerts: "DatabaseReplicationLag"

**Impact:** Potential data inconsistency between clouds

**Procedure:**

1. **Identify Failed Replication**
   ```bash
   # Check replication status
   kubectl exec -it postgres-primary-0 -n database -- \
     psql -U postgres -c "
       SELECT 
         application_name,
         state,
         sync_state,
         write_lag,
         flush_lag,
         replay_lag
       FROM pg_stat_replication;
     "
   ```

2. **Check Replica Health**
   ```bash
   # AWS RDS
   aws rds describe-db-instances \
     --db-instance-identifier wildcam-db-west \
     --query 'DBInstances[0].DBInstanceStatus'
   
   # Azure PostgreSQL
   az postgres flexible-server show \
     --name wildcam-db-west \
     --resource-group wildcam-rg \
     --query state
   
   # GCP Cloud SQL
   gcloud sql instances describe wildcam-db-west \
     --format='value(state)'
   ```

3. **Restart Replication**
   ```bash
   # On primary
   kubectl exec -it postgres-primary-0 -n database -- \
     psql -U postgres -c "
       SELECT pg_drop_replication_slot('replica_slot_aws');
       SELECT pg_create_physical_replication_slot('replica_slot_aws');
     "
   
   # On replica
   kubectl exec -it postgres-replica-aws-0 -n database -- \
     psql -U postgres -c "
       SELECT pg_wal_replay_resume();
     "
   ```

4. **Verify Replication Recovery**
   ```bash
   # Monitor lag
   watch -n 5 'kubectl exec -it postgres-primary-0 -n database -- \
     psql -U postgres -c "SELECT write_lag FROM pg_stat_replication;"'
   ```

5. **Data Consistency Check**
   ```bash
   # Run consistency checker
   kubectl create job --from=cronjob/data-consistency-check \
     manual-consistency-check-$(date +%s) -n monitoring
   
   # Check results
   kubectl logs job/manual-consistency-check-<timestamp> -n monitoring
   ```

**Prevention:**
- Monitor replication lag continuously
- Automated consistency checks every 6 hours
- Network bandwidth monitoring

---

## Routine Maintenance

### 📅 Weekly Kubernetes Cluster Updates

**Timing:** Saturdays 2:00 AM UTC (lowest traffic)

**Procedure:**

1. **Pre-Update Checks**
   ```bash
   # Backup critical data
   ./scripts/backup-all-clusters.sh
   
   # Check cluster health
   kubectl get nodes --all-namespaces
   kubectl get pods --all-namespaces | grep -v Running
   
   # Review pending updates
   kubectl get nodes -o wide
   ```

2. **Update Process (Rolling)**
   ```bash
   # Start with secondary regions
   # AWS Secondary (us-east-1)
   eksctl upgrade cluster --name wildcam-eks-east --approve
   eksctl upgrade nodegroup --cluster wildcam-eks-east --name workers
   
   # Wait 30 minutes and monitor
   sleep 1800
   
   # Azure Secondary (eastus)
   az aks upgrade \
     --resource-group wildcam-rg \
     --name wildcam-aks-east \
     --kubernetes-version 1.28.3 \
     --yes
   
   # Wait 30 minutes
   sleep 1800
   
   # GCP Secondary (us-east1)
   gcloud container clusters upgrade wildcam-gke-east \
     --master --cluster-version 1.28.3 \
     --region us-east1
   
   gcloud container clusters upgrade wildcam-gke-east \
     --node-pool=default-pool \
     --region us-east1
   ```

3. **Validation**
   ```bash
   # Verify all pods running
   kubectl get pods --all-namespaces | grep -v Running | grep -v Completed
   
   # Check application health
   curl -s https://api.wildcam.org/health | jq
   
   # Run smoke tests
   ./scripts/smoke-tests.sh
   ```

4. **Update Primary Regions**
   Repeat steps 2-3 for primary regions

5. **Post-Update Tasks**
   ```bash
   # Update documentation
   echo "Clusters updated to 1.28.3 on $(date)" >> CHANGELOG.md
   
   # Notify team
   ./scripts/send-slack-notification.sh "Kubernetes clusters updated successfully"
   ```

---

### 💰 Monthly Cost Optimization Review

**Timing:** First Monday of each month

**Procedure:**

1. **Generate Cost Reports**
   ```bash
   # AWS Cost Report
   aws ce get-cost-and-usage \
     --time-period Start=2024-01-01,End=2024-01-31 \
     --granularity MONTHLY \
     --metrics BlendedCost \
     --group-by Type=SERVICE
   
   # Azure Cost Analysis
   az consumption usage list \
     --start-date 2024-01-01 \
     --end-date 2024-01-31 \
     --output table
   
   # GCP Billing Report
   gcloud billing accounts list
   # Access: https://console.cloud.google.com/billing/
   ```

2. **Identify Cost Savings Opportunities**
   ```bash
   # Find underutilized resources
   kubectl top nodes
   kubectl top pods --all-namespaces --sort-by=cpu
   
   # Check for unused persistent volumes
   kubectl get pv | grep Released
   
   # Review spot/preemptible instance usage
   kubectl get nodes -l instance-type=spot
   ```

3. **Implement Optimizations**
   ```bash
   # Scale down over-provisioned services
   kubectl scale deployment low-priority-service --replicas=2
   
   # Delete unused resources
   kubectl delete pvc unused-pvc-name -n storage
   
   # Enable cluster autoscaler
   kubectl apply -f k8s/autoscaling/cluster-autoscaler.yaml
   ```

4. **Update Budget Alerts**
   ```bash
   # Adjust thresholds if needed
   terraform apply -var="monthly_cost_budget=9500"
   ```

5. **Document Findings**
   - Create cost optimization report
   - Share with stakeholders
   - Update cost projections

---

## Failover Procedures

### 🔄 Manual Failover from AWS to Azure

**When to Use:**
- Planned AWS maintenance
- AWS performance degradation
- Cost optimization

**Procedure:**

1. **Pre-Failover Verification**
   ```bash
   # Verify Azure cluster health
   kubectl --context azure-west get nodes
   kubectl --context azure-west get pods -n wildcam
   
   # Check Azure database replication
   az postgres flexible-server replica list \
     --name wildcam-db-west \
     --resource-group wildcam-rg
   
   # Verify storage sync
   az storage blob list \
     --account-name wildcamstorage \
     --container-name wildlife-data
   ```

2. **Increase Azure Capacity**
   ```bash
   # Scale up before shifting traffic
   kubectl --context azure-west scale deployment wildcam-backend \
     --replicas=20 -n wildcam
   
   # Wait for pods to be ready
   kubectl --context azure-west wait --for=condition=ready pod \
     -l app=wildcam-backend -n wildcam --timeout=300s
   ```

3. **Update DNS/Traffic Routing**
   ```bash
   # Update Route53 weights
   aws route53 change-resource-record-sets \
     --hosted-zone-id Z1234567890ABC \
     --change-batch file://route53-azure-primary.json
   
   # Or update load balancer weights
   kubectl annotate service wildcam-backend \
     traffic-policy='{"aws": 0, "azure": 100, "gcp": 0}'
   ```

4. **Monitor Traffic Shift**
   ```bash
   # Watch request distribution
   kubectl logs -f deployment/wildcam-backend -n wildcam | \
     grep -o 'provider: [a-z]*' | sort | uniq -c
   
   # Monitor response times
   kubectl top pods -n wildcam --sort-by=cpu
   ```

5. **Scale Down AWS**
   ```bash
   # After 30 minutes of stable Azure traffic
   kubectl --context aws-west scale deployment wildcam-backend \
     --replicas=5 -n wildcam
   ```

6. **Verification**
   ```bash
   # Run health checks
   curl -s https://api.wildcam.org/health | jq '.provider'
   
   # Verify data consistency
   kubectl create job manual-consistency-check -n monitoring \
     --from=cronjob/data-consistency-check
   ```

---

### ↩️ Rollback from Azure to AWS

**Procedure:**

1. **Scale Up AWS**
   ```bash
   kubectl --context aws-west scale deployment wildcam-backend \
     --replicas=20 -n wildcam
   ```

2. **Shift Traffic Back**
   ```bash
   kubectl annotate service wildcam-backend \
     traffic-policy='{"aws": 100, "azure": 0, "gcp": 0}'
   ```

3. **Monitor and Scale Down Azure**
   ```bash
   # After stability
   kubectl --context azure-west scale deployment wildcam-backend \
     --replicas=5 -n wildcam
   ```

---

## Scaling Operations

### 📈 Scale Up for Wildlife Migration Season

**Scenario:** Expecting 5x traffic increase during wildlife migration monitoring

**Timing:** 2 weeks before peak season

**Procedure:**

1. **Capacity Planning**
   ```bash
   # Current capacity
   kubectl top nodes
   kubectl get hpa -n wildcam
   
   # Calculate target capacity (5x increase)
   # Current: 15 replicas → Target: 75 replicas
   ```

2. **Increase Node Count**
   ```bash
   # AWS
   eksctl scale nodegroup --cluster wildcam-eks-west \
     --nodes-min 10 --nodes-max 30 --name workers
   
   # Azure
   az aks nodepool scale \
     --resource-group wildcam-rg \
     --cluster-name wildcam-aks-west \
     --name agentpool \
     --node-count 15
   
   # GCP
   gcloud container clusters resize wildcam-gke-west \
     --num-nodes 15 --region us-west1
   ```

3. **Update HPA Limits**
   ```bash
   kubectl patch hpa wildcam-backend -n wildcam --patch '
   {
     "spec": {
       "minReplicas": 20,
       "maxReplicas": 100
     }
   }'
   ```

4. **Increase Database Capacity**
   ```bash
   # AWS RDS
   aws rds modify-db-instance \
     --db-instance-identifier wildcam-db-west \
     --db-instance-class db.r6g.2xlarge \
     --apply-immediately
   
   # Azure PostgreSQL
   az postgres flexible-server update \
     --name wildcam-db-west \
     --resource-group wildcam-rg \
     --sku-name GP_Standard_D8s_v3
   
   # GCP Cloud SQL
   gcloud sql instances patch wildcam-db-west \
     --tier db-custom-8-32768
   ```

5. **Pre-warm Caches**
   ```bash
   # Load frequently accessed data
   ./scripts/cache-warmup.sh
   
   # Pre-generate thumbnails
   kubectl create job thumbnail-batch -n processing \
     --image=wildcam/thumbnail-generator:latest
   ```

6. **Load Testing**
   ```bash
   # Run load tests
   k6 run --vus 1000 --duration 30m load-test.js
   
   # Monitor results
   kubectl logs -f deployment/load-test -n testing
   ```

---

### 📉 Scale Down After Season

**Procedure:**

1. **Monitor Traffic Patterns**
   ```bash
   # Verify traffic decrease
   kubectl top pods -n wildcam --sort-by=cpu
   ```

2. **Gradual Scale Down**
   ```bash
   # Reduce HPA limits
   kubectl patch hpa wildcam-backend -n wildcam --patch '
   {
     "spec": {
       "minReplicas": 5,
       "maxReplicas": 20
     }
   }'
   
   # Wait 24 hours and monitor
   ```

3. **Reduce Node Count**
   ```bash
   # After verifying stability
   eksctl scale nodegroup --cluster wildcam-eks-west \
     --nodes-min 3 --nodes-max 10 --name workers
   ```

4. **Downgrade Database**
   ```bash
   aws rds modify-db-instance \
     --db-instance-identifier wildcam-db-west \
     --db-instance-class db.r6g.xlarge \
     --apply-immediately
   ```

---

## Disaster Recovery

### 💾 Complete Data Loss Recovery

**Scenario:** Catastrophic data loss in primary database

**Procedure:**

1. **Assess Damage**
   ```bash
   # Check database status
   kubectl exec -it postgres-primary-0 -n database -- psql -U postgres -l
   
   # Check last backup
   aws s3 ls s3://wildcam-backups/ --recursive | sort | tail -n 10
   ```

2. **Stop Application Traffic**
   ```bash
   # Put maintenance page up
   kubectl scale deployment wildcam-backend --replicas=0 -n wildcam
   kubectl apply -f k8s/maintenance-page.yaml
   ```

3. **Restore from Backup**
   ```bash
   # AWS RDS restore
   aws rds restore-db-instance-from-db-snapshot \
     --db-instance-identifier wildcam-db-restored \
     --db-snapshot-identifier wildcam-db-snapshot-2024-01-15-06-00
   
   # Wait for restoration
   aws rds wait db-instance-available \
     --db-instance-identifier wildcam-db-restored
   ```

4. **Update Connection Strings**
   ```bash
   # Update Kubernetes secrets
   kubectl create secret generic postgres-credentials \
     --from-literal=host=wildcam-db-restored.cluster-xyz.us-west-2.rds.amazonaws.com \
     --from-literal=database=wildlifedb \
     --from-literal=username=admin \
     --from-literal=password=secure_password \
     --dry-run=client -o yaml | kubectl apply -f -
   ```

5. **Verify Data Integrity**
   ```bash
   # Run data validation scripts
   kubectl exec -it postgres-primary-0 -n database -- \
     psql -U postgres -d wildlifedb -f /scripts/validate-data.sql
   ```

6. **Restore Application Service**
   ```bash
   # Scale back up
   kubectl scale deployment wildcam-backend --replicas=10 -n wildcam
   
   # Remove maintenance page
   kubectl delete -f k8s/maintenance-page.yaml
   ```

7. **Re-establish Replication**
   ```bash
   # Configure replication to other clouds
   ./scripts/setup-replication.sh --source wildcam-db-restored
   ```

---

## Security Incidents

### 🔒 Compromised Credentials

**Symptoms:**
- Unauthorized API access
- Suspicious database queries
- Alerts from security monitoring

**Procedure:**

1. **Immediate Actions**
   ```bash
   # Rotate all credentials
   vault write -force auth/token/revoke-orphan
   
   # Revoke compromised AWS keys
   aws iam update-access-key \
     --access-key-id AKIA... \
     --status Inactive \
     --user-name compromised-user
   
   # Rotate Kubernetes secrets
   kubectl delete secret api-credentials -n wildcam
   kubectl create secret generic api-credentials \
     --from-literal=api-key=$(openssl rand -hex 32)
   ```

2. **Audit Access Logs**
   ```bash
   # AWS CloudTrail
   aws cloudtrail lookup-events \
     --lookup-attributes AttributeKey=Username,AttributeValue=compromised-user \
     --max-results 50
   
   # Kubernetes audit logs
   kubectl logs -n kube-system kube-apiserver | \
     grep compromised-user
   ```

3. **Assess Impact**
   ```bash
   # Check for data exfiltration
   kubectl logs deployment/wildcam-backend -n wildcam | \
     grep -E "download|export" | tail -n 100
   
   # Review database access
   kubectl exec -it postgres-primary-0 -n database -- \
     psql -U postgres -c "
       SELECT usename, application_name, client_addr, query_start, query
       FROM pg_stat_activity
       WHERE usename = 'compromised_user'
       ORDER BY query_start DESC;
     "
   ```

4. **Remediation**
   ```bash
   # Update all affected systems
   ansible-playbook playbooks/rotate-credentials.yml
   
   # Deploy updated secrets
   kubectl apply -k k8s/overlays/production/secrets/
   
   # Restart affected pods
   kubectl rollout restart deployment wildcam-backend -n wildcam
   ```

5. **Documentation**
   - Document incident timeline
   - List affected systems
   - Actions taken
   - Lessons learned

---

## Appendix

### Useful Commands

```bash
# Quick health check all clusters
for ctx in aws-west aws-east azure-west azure-east gcp-west gcp-east; do
  echo "=== $ctx ==="
  kubectl --context $ctx get nodes
  kubectl --context $ctx get pods -n wildcam | grep -v Running
done

# Cost estimation
terraform cost estimate

# Database backup
pg_dump -h postgres-primary.wildcam.org -U postgres wildlifedb > backup.sql

# Restore database
psql -h postgres-primary.wildcam.org -U postgres wildlifedb < backup.sql
```

### Emergency Contacts

- **On-Call Engineer**: +1-555-WILDLIFE
- **Database Admin**: dba@wildcam.org
- **Security Team**: security@wildcam.org
- **Cloud Architects**: architects@wildcam.org

### Escalation Path

1. Level 1: On-Call Engineer (0-15 minutes)
2. Level 2: Senior SRE (15-30 minutes)
3. Level 3: Principal Engineer (30-60 minutes)
4. Level 4: CTO (>60 minutes or critical impact)
