# Multi-stage Dockerfile for NVIDIA Jetson Edge Gateway

# Stage 1: Base image with JetPack dependencies
# This assumes JetPack is already installed on the host Jetson device
# For containerized deployment, use NVIDIA's L4T base images

FROM nvcr.io/nvidia/l4t-base:r35.1.0

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PYTHONUNBUFFERED=1
ENV CUDA_HOME=/usr/local/cuda
ENV LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
ENV PATH=$CUDA_HOME/bin:$PATH

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3-pip \
    python3-dev \
    python3-opencv \
    libopencv-dev \
    git \
    wget \
    curl \
    nano \
    htop \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy requirements first for layer caching
COPY requirements.txt .

# Install Python dependencies
RUN pip3 install --no-cache-dir --upgrade pip && \
    pip3 install --no-cache-dir -r requirements.txt

# Copy application code
COPY . .

# Create necessary directories
RUN mkdir -p \
    /app/data/detections \
    /app/logs \
    /app/models \
    /app/config

# Expose ports
# Gateway API
EXPOSE 8000
# ESP32 HTTP API
EXPOSE 8080
# ESP32 WebSocket
EXPOSE 8081

# Set default configuration path
ENV CONFIG_PATH=/app/config/gateway_config.yaml

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD curl -f http://localhost:8000/api/health || exit 1

# Run the gateway service
CMD ["python3", "main.py", "--config", "/app/config/gateway_config.yaml"]
