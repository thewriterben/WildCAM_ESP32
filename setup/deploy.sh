#!/bin/bash

# ESP32 Wildlife Camera System Deployment Script
# This script sets up the complete wildlife monitoring system

set -e

echo "🦌 ESP32 Wildlife Camera System Deployment"
echo "=========================================="

# Check prerequisites
echo "📋 Checking prerequisites..."

if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker first."
    exit 1
fi

if ! command -v docker-compose &> /dev/null && ! docker compose version &> /dev/null; then
    echo "❌ Docker Compose is not installed. Please install Docker Compose first."
    exit 1
fi

echo "✅ Prerequisites check passed"

# Create environment file if it doesn't exist
if [ ! -f .env ]; then
    echo "📝 Creating environment configuration..."
    cat > .env << EOF
# JWT and Security
JWT_SECRET_KEY=$(openssl rand -hex 32)
SECRET_KEY=$(openssl rand -hex 32)

# Database
POSTGRES_PASSWORD=secure_$(openssl rand -hex 8)

# AWS S3 (optional)
# AWS_ACCESS_KEY_ID=your_access_key
# AWS_SECRET_ACCESS_KEY=your_secret_key
# S3_BUCKET=wildlife-captures

# InfluxDB (optional)
INFLUXDB_USERNAME=admin
INFLUXDB_PASSWORD=wildcam_$(openssl rand -hex 8)
INFLUXDB_TOKEN=wildcam-token-$(openssl rand -hex 16)

# Application URLs
REACT_APP_API_URL=http://localhost
REACT_APP_WEBSOCKET_URL=http://localhost

# Environment
FLASK_ENV=production
NODE_ENV=production
EOF
    echo "✅ Environment file created (.env)"
    echo "⚠️  Please review and update the .env file with your specific configuration"
fi

# Create necessary directories
echo "📁 Creating directory structure..."
mkdir -p logs
mkdir -p data/uploads
mkdir -p data/postgres
mkdir -p data/redis
mkdir -p data/influxdb
mkdir -p mosquitto/config
mkdir -p mosquitto/data
mkdir -p mosquitto/logs

# Create mosquitto configuration
if [ ! -f mosquitto/config/mosquitto.conf ]; then
    cat > mosquitto/config/mosquitto.conf << EOF
# Mosquitto MQTT Broker Configuration
listener 1883
allow_anonymous true
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

# WebSocket support
listener 9001
protocol websockets
EOF
    echo "✅ MQTT broker configuration created"
fi

# Set permissions
chmod 755 mosquitto/config
chmod 755 mosquitto/data
chmod 755 mosquitto/logs

echo "📦 Building and starting services..."

# Build and start services
if docker compose version &> /dev/null; then
    COMPOSE_CMD="docker compose"
else
    COMPOSE_CMD="docker-compose"
fi

# Pull base images
echo "⬇️  Pulling base images..."
$COMPOSE_CMD pull postgres redis nginx eclipse-mosquitto influxdb

# Build custom images
echo "🔨 Building application images..."
$COMPOSE_CMD build --no-cache

# Start services
echo "🚀 Starting services..."
$COMPOSE_CMD up -d

# Wait for services to be healthy
echo "⏳ Waiting for services to start..."
sleep 30

# Check service health
echo "🏥 Checking service health..."
services=("postgres" "redis" "backend" "frontend" "nginx")

for service in "${services[@]}"; do
    if $COMPOSE_CMD ps $service | grep -q "Up"; then
        echo "✅ $service is running"
    else
        echo "❌ $service failed to start"
        echo "📋 Service logs:"
        $COMPOSE_CMD logs $service
    fi
done

# Display status
echo ""
echo "🎉 Deployment completed!"
echo "======================="
echo ""
echo "📊 Dashboard: http://localhost"
echo "🔧 Backend API: http://localhost/api"
echo "📈 InfluxDB: http://localhost:8086"
echo "💾 Database: localhost:5432"
echo "📨 MQTT: localhost:1883"
echo ""
echo "🔐 Default login credentials:"
echo "   Username: admin"
echo "   Password: admin123"
echo ""
echo "📝 Important files:"
echo "   - Configuration: .env"
echo "   - Logs: docker-compose logs [service]"
echo "   - Data: ./data/ directory"
echo ""
echo "🛠️  Management commands:"
echo "   - Stop: $COMPOSE_CMD down"
echo "   - Restart: $COMPOSE_CMD restart"
echo "   - Logs: $COMPOSE_CMD logs -f"
echo "   - Status: $COMPOSE_CMD ps"
echo ""
echo "⚠️  Remember to:"
echo "   - Update default passwords"
echo "   - Configure AWS S3 if using cloud storage"
echo "   - Set up SSL certificates for production"
echo "   - Configure firewall rules"
echo ""
echo "📚 For more information, see the README.md file"