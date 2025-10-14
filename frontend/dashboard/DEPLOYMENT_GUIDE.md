# 🚀 WildCAM Dashboard - Deployment Guide

**Complete guide for deploying the advanced user dashboard to production**

## 📋 Table of Contents

- [Prerequisites](#-prerequisites)
- [Local Development](#-local-development)
- [Docker Deployment](#-docker-deployment)
- [Cloud Deployment](#-cloud-deployment)
- [CI/CD Pipeline](#-cicd-pipeline)
- [Performance Optimization](#-performance-optimization)
- [Monitoring & Logging](#-monitoring--logging)
- [Troubleshooting](#-troubleshooting)

## 📦 Prerequisites

### System Requirements

- **Node.js**: 16.x or higher
- **npm**: 8.x or higher (or yarn 1.22+)
- **Docker**: 20.10+ (for containerized deployment)
- **Git**: Latest version

### Backend Requirements

- Backend API server running and accessible
- PostgreSQL database configured
- WebSocket server enabled

### Network Requirements

- HTTPS certificate (for production)
- DNS configuration
- Firewall rules configured
- CORS properly set up on backend

## 💻 Local Development

### Quick Start

```bash
# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/frontend/dashboard

# Install dependencies
npm install

# Configure environment
cp .env.example .env
nano .env  # Edit with your settings

# Start development server
npm start
```

Dashboard will be available at `http://localhost:3000`

### Development Configuration

Edit `.env`:

```bash
REACT_APP_API_URL=http://localhost:5000
REACT_APP_WS_URL=ws://localhost:5000
REACT_APP_DEBUG=true
REACT_APP_MOCK_DATA=false
```

### Hot Reload

The development server supports hot module replacement (HMR):
- Changes to React components reload instantly
- CSS changes apply without page refresh
- State is preserved during most changes

### Development Tools

```bash
# Run linter
npm run lint

# Format code
npm run format

# Run tests
npm test

# Check bundle size
npm run analyze
```

## 🐳 Docker Deployment

### Development Container

Build and run the development container:

```bash
# Build image
docker build -t wildcam-dashboard:dev -f Dockerfile .

# Run container
docker run -d \
  --name wildcam-dashboard \
  -p 3000:3000 \
  -v $(pwd)/src:/app/src \
  -e REACT_APP_API_URL=http://backend:5000 \
  wildcam-dashboard:dev

# View logs
docker logs -f wildcam-dashboard

# Stop container
docker stop wildcam-dashboard
```

### Production Container

Build optimized production image:

```bash
# Build production image
docker build -t wildcam-dashboard:prod -f Dockerfile.prod .

# Run production container
docker run -d \
  --name wildcam-dashboard-prod \
  -p 80:80 \
  -e REACT_APP_API_URL=https://api.example.com \
  wildcam-dashboard:prod
```

### Docker Compose

Use `docker-compose.yml` for complete stack:

```yaml
version: '3.8'

services:
  dashboard:
    build:
      context: ./frontend/dashboard
      dockerfile: Dockerfile.prod
    ports:
      - "80:80"
    environment:
      - REACT_APP_API_URL=http://backend:5000
      - REACT_APP_WS_URL=ws://backend:5000
    depends_on:
      - backend
    networks:
      - wildcam-network
    restart: unless-stopped

  backend:
    build: ./backend
    ports:
      - "5000:5000"
    environment:
      - DATABASE_URL=postgresql://user:pass@postgres:5432/wildcam
    depends_on:
      - postgres
    networks:
      - wildcam-network
    restart: unless-stopped

  postgres:
    image: postgres:14-alpine
    environment:
      - POSTGRES_DB=wildcam
      - POSTGRES_USER=user
      - POSTGRES_PASSWORD=secure_password
    volumes:
      - postgres-data:/var/lib/postgresql/data
    networks:
      - wildcam-network
    restart: unless-stopped

networks:
  wildcam-network:
    driver: bridge

volumes:
  postgres-data:
```

Deploy with Docker Compose:

```bash
# Start all services
docker-compose up -d

# View logs
docker-compose logs -f dashboard

# Scale services
docker-compose up -d --scale backend=3

# Stop all services
docker-compose down

# Stop and remove volumes
docker-compose down -v
```

## ☁️ Cloud Deployment

### AWS Deployment

#### Using AWS Amplify

```bash
# Install Amplify CLI
npm install -g @aws-amplify/cli

# Initialize Amplify
amplify init

# Add hosting
amplify add hosting

# Deploy
amplify publish
```

#### Using AWS S3 + CloudFront

```bash
# Build production bundle
npm run build

# Install AWS CLI
pip install awscli

# Configure AWS credentials
aws configure

# Create S3 bucket
aws s3 mb s3://wildcam-dashboard

# Enable static website hosting
aws s3 website s3://wildcam-dashboard \
  --index-document index.html \
  --error-document index.html

# Upload build files
aws s3 sync build/ s3://wildcam-dashboard --delete

# Create CloudFront distribution
aws cloudfront create-distribution \
  --origin-domain-name wildcam-dashboard.s3.amazonaws.com \
  --default-root-object index.html
```

#### Using Elastic Beanstalk

Create `.ebextensions/01_nginx.config`:

```yaml
files:
  "/etc/nginx/conf.d/proxy.conf":
    mode: "000644"
    owner: root
    group: root
    content: |
      client_max_body_size 20M;
      
container_commands:
  01_reload_nginx:
    command: "service nginx reload"
```

Deploy:

```bash
# Install EB CLI
pip install awsebcli

# Initialize EB
eb init -p docker wildcam-dashboard

# Create environment
eb create production

# Deploy
eb deploy
```

### Google Cloud Platform

#### Using Firebase Hosting

```bash
# Install Firebase CLI
npm install -g firebase-tools

# Login
firebase login

# Initialize project
firebase init hosting

# Build
npm run build

# Deploy
firebase deploy
```

#### Using Google Cloud Run

```bash
# Build and push image
gcloud builds submit --tag gcr.io/PROJECT_ID/wildcam-dashboard

# Deploy to Cloud Run
gcloud run deploy wildcam-dashboard \
  --image gcr.io/PROJECT_ID/wildcam-dashboard \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated
```

### Microsoft Azure

#### Using Azure Static Web Apps

```bash
# Install Azure CLI
curl -sL https://aka.ms/InstallAzureCLIDeb | sudo bash

# Login
az login

# Create resource group
az group create --name wildcam-rg --location eastus

# Create static web app
az staticwebapp create \
  --name wildcam-dashboard \
  --resource-group wildcam-rg \
  --source https://github.com/user/WildCAM_ESP32 \
  --location "East US 2" \
  --branch main \
  --app-location "/frontend/dashboard" \
  --output-location "build"
```

### DigitalOcean

#### Using App Platform

```bash
# Install doctl
snap install doctl

# Authenticate
doctl auth init

# Create app spec
cat > app.yaml << EOF
name: wildcam-dashboard
services:
- name: web
  github:
    repo: thewriterben/WildCAM_ESP32
    branch: main
    deploy_on_push: true
  build_command: cd frontend/dashboard && npm run build
  run_command: npm start
  envs:
  - key: REACT_APP_API_URL
    value: https://api.wildcam.example.com
  http_port: 3000
  instance_count: 1
  instance_size_slug: basic-xxs
EOF

# Create app
doctl apps create --spec app.yaml
```

## 🔄 CI/CD Pipeline

### GitHub Actions

Create `.github/workflows/deploy.yml`:

```yaml
name: Deploy Dashboard

on:
  push:
    branches: [main]
    paths:
      - 'frontend/dashboard/**'
  pull_request:
    branches: [main]
    paths:
      - 'frontend/dashboard/**'

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Node.js
        uses: actions/setup-node@v3
        with:
          node-version: '18'
          cache: 'npm'
          cache-dependency-path: frontend/dashboard/package-lock.json
      
      - name: Install dependencies
        working-directory: frontend/dashboard
        run: npm ci
      
      - name: Run linter
        working-directory: frontend/dashboard
        run: npm run lint
      
      - name: Run tests
        working-directory: frontend/dashboard
        run: npm test -- --coverage --watchAll=false
      
      - name: Build
        working-directory: frontend/dashboard
        run: npm run build
        env:
          REACT_APP_API_URL: ${{ secrets.API_URL }}
      
      - name: Upload build artifacts
        uses: actions/upload-artifact@v3
        with:
          name: build
          path: frontend/dashboard/build

  deploy:
    needs: test
    runs-on: ubuntu-latest
    if: github.ref == 'refs/heads/main'
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Download build artifacts
        uses: actions/download-artifact@v3
        with:
          name: build
          path: frontend/dashboard/build
      
      - name: Deploy to production
        env:
          AWS_ACCESS_KEY_ID: ${{ secrets.AWS_ACCESS_KEY_ID }}
          AWS_SECRET_ACCESS_KEY: ${{ secrets.AWS_SECRET_ACCESS_KEY }}
        run: |
          aws s3 sync frontend/dashboard/build/ s3://wildcam-dashboard --delete
          aws cloudfront create-invalidation --distribution-id ${{ secrets.CLOUDFRONT_DIST_ID }} --paths "/*"
```

### GitLab CI

Create `.gitlab-ci.yml`:

```yaml
image: node:18

stages:
  - test
  - build
  - deploy

variables:
  DASHBOARD_DIR: frontend/dashboard

cache:
  key: ${CI_COMMIT_REF_SLUG}
  paths:
    - ${DASHBOARD_DIR}/node_modules/

test:
  stage: test
  script:
    - cd ${DASHBOARD_DIR}
    - npm ci
    - npm run lint
    - npm test -- --coverage --watchAll=false
  coverage: '/Lines\s*:\s*(\d+\.\d+)%/'

build:
  stage: build
  script:
    - cd ${DASHBOARD_DIR}
    - npm ci
    - npm run build
  artifacts:
    paths:
      - ${DASHBOARD_DIR}/build/
    expire_in: 1 day
  only:
    - main

deploy_staging:
  stage: deploy
  script:
    - cd ${DASHBOARD_DIR}
    - apt-get update -y && apt-get install -y rsync
    - rsync -avz --delete build/ $STAGING_SERVER:/var/www/dashboard/
  environment:
    name: staging
    url: https://staging.wildcam.example.com
  only:
    - main

deploy_production:
  stage: deploy
  script:
    - cd ${DASHBOARD_DIR}
    - apt-get update -y && apt-get install -y rsync
    - rsync -avz --delete build/ $PROD_SERVER:/var/www/dashboard/
  environment:
    name: production
    url: https://dashboard.wildcam.example.com
  when: manual
  only:
    - main
```

## ⚡ Performance Optimization

### Build Optimization

#### Analyze Bundle Size

```bash
# Install analyzer
npm install --save-dev source-map-explorer

# Add script to package.json
"analyze": "source-map-explorer 'build/static/js/*.js'"

# Build and analyze
npm run build
npm run analyze
```

#### Code Splitting

```javascript
import { lazy, Suspense } from 'react';

// Lazy load components
const SpeciesAnalytics = lazy(() => import('./components/SpeciesAnalytics'));
const CameraManagement = lazy(() => import('./components/CameraManagement'));

function App() {
  return (
    <Suspense fallback={<CircularProgress />}>
      <Routes>
        <Route path="/analytics" element={<SpeciesAnalytics />} />
        <Route path="/devices" element={<CameraManagement />} />
      </Routes>
    </Suspense>
  );
}
```

#### Tree Shaking

Ensure proper imports for tree shaking:

```javascript
// ❌ Bad - imports entire library
import _ from 'lodash';

// ✅ Good - imports only needed function
import debounce from 'lodash/debounce';

// ❌ Bad - imports all icons
import * as Icons from '@mui/icons-material';

// ✅ Good - imports specific icons
import { Dashboard, Settings } from '@mui/icons-material';
```

### Runtime Optimization

#### Service Worker

Enable service worker in `src/index.js`:

```javascript
import * as serviceWorkerRegistration from './serviceWorkerRegistration';

// Enable service worker for offline support
serviceWorkerRegistration.register({
  onUpdate: (registration) => {
    const waitingServiceWorker = registration.waiting;
    
    if (waitingServiceWorker) {
      waitingServiceWorker.addEventListener('statechange', (event) => {
        if (event.target.state === 'activated') {
          window.location.reload();
        }
      });
      waitingServiceWorker.postMessage({ type: 'SKIP_WAITING' });
    }
  },
});
```

#### Image Optimization

```bash
# Install imagemin
npm install --save-dev imagemin imagemin-webp

# Create optimization script
node scripts/optimize-images.js
```

### Caching Strategy

#### Nginx Caching

```nginx
server {
    listen 80;
    server_name dashboard.wildcam.example.com;
    root /var/www/dashboard;
    index index.html;

    # Enable gzip
    gzip on;
    gzip_types text/plain text/css application/json application/javascript text/xml application/xml application/xml+rss text/javascript;
    gzip_min_length 1000;

    # Cache static assets
    location /static/ {
        expires 1y;
        add_header Cache-Control "public, immutable";
        access_log off;
    }

    # Cache fonts
    location ~* \.(woff|woff2|ttf|otf)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
        access_log off;
    }

    # Don't cache HTML
    location / {
        try_files $uri $uri/ /index.html;
        add_header Cache-Control "no-cache";
    }
}
```

## 📊 Monitoring & Logging

### Application Monitoring

#### Sentry Integration

```javascript
// src/index.js
import * as Sentry from '@sentry/react';
import { BrowserTracing } from '@sentry/tracing';

Sentry.init({
  dsn: process.env.REACT_APP_SENTRY_DSN,
  integrations: [
    new BrowserTracing(),
  ],
  tracesSampleRate: 1.0,
  environment: process.env.NODE_ENV,
});
```

#### Google Analytics

```javascript
// src/utils/analytics.js
import ReactGA from 'react-ga4';

export const initGA = () => {
  ReactGA.initialize(process.env.REACT_APP_GA_ID);
};

export const logPageView = () => {
  ReactGA.send({ hitType: 'pageview', page: window.location.pathname });
};

export const logEvent = (category, action, label) => {
  ReactGA.event({
    category,
    action,
    label,
  });
};
```

### Infrastructure Monitoring

#### Prometheus Metrics

Create `nginx.conf` with metrics endpoint:

```nginx
server {
    listen 9090;
    
    location /metrics {
        stub_status;
        access_log off;
    }
}
```

#### Health Check Endpoint

Create `public/health.json`:

```json
{
  "status": "healthy",
  "version": "3.0.0",
  "timestamp": "2025-10-14T00:00:00Z"
}
```

### Log Aggregation

#### CloudWatch Logs

```bash
# Install CloudWatch agent
sudo wget https://s3.amazonaws.com/amazoncloudwatch-agent/amazon_linux/amd64/latest/amazon-cloudwatch-agent.rpm

# Install agent
sudo rpm -U ./amazon-cloudwatch-agent.rpm

# Configure agent
sudo /opt/aws/amazon-cloudwatch-agent/bin/amazon-cloudwatch-agent-ctl \
  -a fetch-config \
  -m ec2 \
  -s \
  -c file:/opt/aws/amazon-cloudwatch-agent/etc/config.json
```

## 🔧 Troubleshooting

### Common Issues

#### Build Fails

**Problem**: `npm run build` fails with memory error

**Solution**:
```bash
# Increase Node memory limit
NODE_OPTIONS=--max_old_space_size=4096 npm run build
```

#### Deployment Fails

**Problem**: Docker image too large

**Solution**:
```dockerfile
# Use multi-stage build
FROM node:18-alpine AS builder
WORKDIR /app
COPY package*.json ./
RUN npm ci --only=production
COPY . .
RUN npm run build

FROM nginx:alpine
COPY --from=builder /app/build /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf
```

#### CORS Errors

**Problem**: API requests blocked by CORS

**Solution**:
1. Configure backend CORS headers
2. Use proxy in development (`package.json`):
```json
{
  "proxy": "http://localhost:5000"
}
```

#### WebSocket Connection Fails

**Problem**: WebSocket cannot connect

**Solution**:
1. Check WebSocket URL protocol (ws:// or wss://)
2. Verify firewall allows WebSocket connections
3. Configure nginx for WebSocket:
```nginx
location /ws/ {
    proxy_pass http://backend:5000;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
    proxy_set_header Host $host;
    proxy_cache_bypass $http_upgrade;
}
```

### Debug Mode

Enable debug logging:

```bash
# Set in .env
REACT_APP_DEBUG=true

# Or in localStorage
localStorage.setItem('debug', 'wildcam:*');
```

### Performance Issues

Check performance metrics:

```javascript
// In browser console
performance.getEntriesByType('navigation');
performance.getEntriesByType('resource');
```

---

## 📚 Additional Resources

- [Create React App Deployment](https://create-react-app.dev/docs/deployment/)
- [Docker Best Practices](https://docs.docker.com/develop/dev-best-practices/)
- [AWS Deployment Guide](https://aws.amazon.com/getting-started/hands-on/deploy-react-app-cicd-amplify/)
- [Nginx Configuration](https://nginx.org/en/docs/)

---

**Deployment Guide Version**: 3.0.0  
**Last Updated**: 2025-10-14  
**Maintained by**: WildCAM ESP32 Team
