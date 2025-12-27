# Dockerfile for NVIDIA Jetson Edge Gateway
# Base image with CUDA and TensorRT support
FROM nvcr.io/nvidia/l4t-pytorch:r35.2.1-pth2.0-py3

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3-pip \
    libopencv-dev \
    python3-opencv \
    && rm -rf /var/lib/apt/lists/*

# Copy shared components
COPY edge/shared /app/edge/shared

# Copy Jetson components
COPY edge/jetson /app/edge/jetson

# Install Python dependencies
RUN pip3 install --no-cache-dir -r /app/edge/jetson/requirements.txt

# Create data directories
RUN mkdir -p /data /models /var/log/wildcam

# Set environment variables
ENV PYTHONUNBUFFERED=1
ENV CUDA_VISIBLE_DEVICES=0

# Expose ports
EXPOSE 8080 8081

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD python3 -c "import requests; requests.get('http://localhost:8080/health')"

# Run main service
CMD ["python3", "-m", "edge.jetson.services.main", "--config", "/etc/wildcam/jetson", "--env", "production"]
