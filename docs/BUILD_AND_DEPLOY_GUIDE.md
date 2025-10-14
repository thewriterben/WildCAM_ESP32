# WildCAM ESP32 Build and Deployment Guide

This guide consolidates the steps required to build the ESP32 firmware, provision the backend services, and compile the web dashboard for the WildCAM platform. Follow the sections below to assemble the full system on a development workstation before deploying it to the field.

## 1. Prerequisites

| Component | Requirements |
|-----------|--------------|
| Firmware | [PlatformIO CLI](https://platformio.org/install) with ESP32 toolchains |
| Backend | Python 3.10+, `pip`, PostgreSQL (or SQLite for local testing), Redis (optional but recommended) |
| Frontend | Node.js 18+ with `npm` |
| Containerized deployment | Docker and Docker Compose (optional shortcut via `setup/deploy.sh`) |

Create a working directory and clone the repository:

```bash
mkdir -p ~/wildcam && cd ~/wildcam
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32
```

## 2. Firmware Build (ESP32)

1. Install PlatformIO either as a VS Code extension or via `pip install platformio`.
2. Connect your ESP32-CAM (or ESP32-S3) board over USB.
3. Build the firmware:

   ```bash
   cd firmware
   platformio run
   ```

4. Upload the firmware to the board (replace `/dev/ttyUSB0` with your serial port):

   ```bash
   platformio run -t upload --upload-port /dev/ttyUSB0
   ```

5. Monitor serial logs to confirm boot and sensor initialization:

   ```bash
   platformio device monitor
   ```

The firmware sources live in `firmware/` with hardware abstraction, sensor drivers, networking, and power-management modules grouped into dedicated folders for clarity. See [`firmware/platformio.ini`](../firmware/platformio.ini) and [`firmware/main.cpp`](../firmware/main.cpp) for entry points.

## 3. Backend Application (Python / Flask)

1. Create a virtual environment and install dependencies:

   ```bash
   cd backend
   python3 -m venv .venv
   source .venv/bin/activate
   pip install --upgrade pip
   pip install -r requirements.txt
   ```

2. Configure environment variables via `.env` (copy `.env.example` if available) and set up the database connection. PostgreSQL is recommended for production; SQLite works for local prototyping.
3. Apply database migrations and start the API:

   ```bash
   flask db upgrade  # or `alembic upgrade head` if you prefer Alembic directly
   flask run --host 0.0.0.0 --port 5000
   ```

4. Launch the Celery workers if you plan to process asynchronous ML alerts:

   ```bash
   celery -A tasks.alert_tasks worker --loglevel=info
   ```

The backend exposes REST endpoints, alert workflows, and machine-learning services defined inside `backend/api`, `backend/services`, and `backend/tasks`. Refer to [backend/README_ALERT_SYSTEM.md](../backend/README_ALERT_SYSTEM.md) for detailed module descriptions.

## 4. Frontend Dashboard (React)

1. Install Node.js (LTS) and `npm`.
2. Install dependencies and build the production bundle:

   ```bash
   cd frontend/dashboard
   npm install
   npm run build
   ```

3. For local development with live reload, run `npm start`. The development server proxies API calls to the Flask backend configured at `http://localhost:5000` in `package.json`.【F:frontend/dashboard/package.json†L1-L40】

The compiled dashboard output lives in `frontend/dashboard/build/`, ready to be served via Nginx or the provided Docker image.

## 5. Unified Build Script

To streamline the above steps you can use the helper script `scripts/build_system.sh`:

```bash
./scripts/build_system.sh            # Run full build
./scripts/build_system.sh --install-only   # Install dependencies without compiling
./scripts/build_system.sh --skip-frontend  # Skip a particular component
```

The script validates prerequisites, provisions a backend virtual environment, installs dashboard dependencies, and triggers the PlatformIO build when the respective toolchains are available.【F:scripts/build_system.sh†L1-L134】

## 6. Containerized Deployment (Optional)

If Docker is available, `setup/deploy.sh` provisions PostgreSQL, Redis, Mosquitto, InfluxDB, the backend, and the React dashboard automatically. Review the generated `.env` file and run:

```bash
./setup/deploy.sh
```

## 7. Next Steps

- Configure satellite or mesh networking modules if you are operating in remote regions.
- Integrate the mobile application (`frontend/mobile`) for field technicians.
- Enable advanced alert analytics by connecting the backend to the ML pipelines in `backend/ml` and the research modules in `backend/research_platform.py`.

With the firmware flashed, backend running, and dashboard built, the WildCAM system is ready for end-to-end testing in a lab setting before deployment to conservation sites.
