# Dockerfile for Raspberry Pi LoRa Gateway
FROM python:3.10-slim-bullseye

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3-dev \
    libgpiod2 \
    gcc \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Copy shared components
COPY edge/shared /app/edge/shared

# Copy Raspberry Pi components
COPY edge/raspberry_pi /app/edge/raspberry_pi

# Install Python dependencies
RUN pip install --no-cache-dir -r /app/edge/raspberry_pi/requirements.txt

# Create data directories
RUN mkdir -p /data /var/log/wildcam

# Set environment variables
ENV PYTHONUNBUFFERED=1

# Expose ports
EXPOSE 5000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=20s --retries=3 \
    CMD python3 -c "import requests; requests.get('http://localhost:5000/health')"

# Run gateway service
CMD ["python3", "-m", "edge.raspberry_pi.gateway.main", "--config", "/etc/wildcam/rpi", "--env", "production"]
