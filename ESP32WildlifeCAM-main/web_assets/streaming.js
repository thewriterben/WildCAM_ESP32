/**
 * ESP32 Wildlife Camera - Streaming Dashboard JavaScript
 * Handles WebSocket communication, stream control, and UI updates
 */

class WildlifeStreamDashboard {
    constructor() {
        this.ws = null;
        this.streamActive = false;
        this.streamStartTime = 0;
        this.statusUpdateInterval = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 10;
        this.reconnectDelay = 3000;
        
        this.eventListeners = new Map();
        
        this.init();
    }

    /**
     * Initialize the dashboard
     */
    init() {
        console.log('Initializing Wildlife Stream Dashboard');
        this.initWebSocket();
        this.bindEventListeners();
        this.updateUI();
        this.refreshStatus();
    }

    /**
     * Initialize WebSocket connection
     */
    initWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        this.ws = new WebSocket(wsUrl);
        
        this.ws.onopen = () => {
            console.log('WebSocket connected');
            this.reconnectAttempts = 0;
            this.showNotification('Connected to camera', 'success');
            this.refreshStatus();
        };
        
        this.ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                this.handleWebSocketMessage(data);
            } catch (e) {
                console.error('Failed to parse WebSocket message:', e);
            }
        };
        
        this.ws.onclose = () => {
            console.log('WebSocket disconnected');
            this.showNotification('Disconnected from camera', 'warning');
            this.scheduleReconnect();
        };
        
        this.ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.showNotification('Connection error', 'danger');
        };
    }

    /**
     * Schedule WebSocket reconnection
     */
    scheduleReconnect() {
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
            this.reconnectAttempts++;
            console.log(`Reconnecting in ${this.reconnectDelay}ms (attempt ${this.reconnectAttempts})`);
            setTimeout(() => this.initWebSocket(), this.reconnectDelay);
            this.reconnectDelay = Math.min(this.reconnectDelay * 1.5, 30000); // Max 30 seconds
        } else {
            this.showNotification('Failed to reconnect. Please refresh the page.', 'danger');
        }
    }

    /**
     * Bind event listeners to UI elements
     */
    bindEventListeners() {
        // Stream control buttons
        document.getElementById('startStreamBtn')?.addEventListener('click', () => this.startStream());
        document.getElementById('stopStreamBtn')?.addEventListener('click', () => this.stopStream());
        
        // Settings controls
        document.getElementById('frameRate')?.addEventListener('change', () => this.updateStreamConfig());
        document.getElementById('quality')?.addEventListener('change', () => this.updateStreamConfig());
        document.getElementById('frameSize')?.addEventListener('change', () => this.updateStreamConfig());
        document.getElementById('motionOnly')?.addEventListener('change', () => this.updateStreamConfig());
        
        // Refresh button
        document.querySelector('[onclick="refreshStatus()"]')?.addEventListener('click', () => this.refreshStatus());
    }

    /**
     * Handle incoming WebSocket messages
     */
    handleWebSocketMessage(data) {
        switch (data.type) {
            case 'STREAM_STATUS':
                this.updateStreamStatus(data.data);
                break;
            case 'SYSTEM_STATUS':
                this.updateSystemStatus(data.data);
                break;
            case 'WILDLIFE_DETECTION':
                this.handleWildlifeDetection(data.data);
                break;
            case 'POWER_UPDATE':
                this.updatePowerStatus(data.data);
                break;
            case 'ERROR_ALERT':
                this.showNotification(data.data.message, 'danger');
                break;
            default:
                console.log('Unknown message type:', data.type);
        }
    }

    /**
     * Start streaming
     */
    async startStream() {
        try {
            this.setLoading(true);
            
            const config = this.getStreamConfig();
            const params = new URLSearchParams(config);
            
            const response = await fetch(`/api/stream/start?${params}`, { method: 'POST' });
            const data = await response.json();
            
            if (data.success) {
                this.streamActive = true;
                this.streamStartTime = Date.now();
                this.updateUI();
                this.startStreamViewer();
                this.startStatusUpdates();
                this.showNotification('Stream started successfully', 'success');
            } else {
                throw new Error(data.error || 'Unknown error');
            }
        } catch (error) {
            console.error('Error starting stream:', error);
            this.showNotification(`Failed to start stream: ${error.message}`, 'danger');
        } finally {
            this.setLoading(false);
        }
    }

    /**
     * Stop streaming
     */
    async stopStream() {
        try {
            this.setLoading(true);
            
            const response = await fetch('/api/stream/stop', { method: 'POST' });
            const data = await response.json();
            
            if (data.success) {
                this.streamActive = false;
                this.updateUI();
                this.stopStreamViewer();
                this.stopStatusUpdates();
                this.showNotification('Stream stopped', 'info');
            } else {
                throw new Error(data.error || 'Unknown error');
            }
        } catch (error) {
            console.error('Error stopping stream:', error);
            this.showNotification(`Failed to stop stream: ${error.message}`, 'danger');
        } finally {
            this.setLoading(false);
        }
    }

    /**
     * Update stream configuration
     */
    async updateStreamConfig() {
        if (!this.streamActive) return;
        
        try {
            const config = this.getStreamConfig();
            const params = new URLSearchParams(config);
            
            const response = await fetch(`/api/stream/config?${params}`, { method: 'POST' });
            const data = await response.json();
            
            if (!data.success) {
                throw new Error(data.error || 'Configuration update failed');
            }
        } catch (error) {
            console.error('Error updating config:', error);
            this.showNotification(`Configuration error: ${error.message}`, 'warning');
        }
    }

    /**
     * Get current stream configuration from UI
     */
    getStreamConfig() {
        return {
            fps: document.getElementById('frameRate')?.value || '5',
            quality: document.getElementById('quality')?.value || 'medium',
            frameSize: document.getElementById('frameSize')?.value || 'vga',
            motionOnly: document.getElementById('motionOnly')?.checked || false
        };
    }

    /**
     * Start stream viewer
     */
    startStreamViewer() {
        const viewer = document.getElementById('streamViewer');
        if (viewer) {
            viewer.innerHTML = `
                <img id="streamImg" 
                     src="/api/stream" 
                     style="max-width: 100%; max-height: 100%;" 
                     onload="this.style.opacity=1"
                     onerror="this.src='/api/stream'" />
                <div class="stream-overlay">
                    <span id="streamInfo">Live</span>
                </div>
            `;
        }
    }

    /**
     * Stop stream viewer
     */
    stopStreamViewer() {
        const viewer = document.getElementById('streamViewer');
        if (viewer) {
            viewer.innerHTML = '<div>Stream not active</div>';
        }
    }

    /**
     * Start periodic status updates
     */
    startStatusUpdates() {
        if (this.statusUpdateInterval) clearInterval(this.statusUpdateInterval);
        this.statusUpdateInterval = setInterval(() => this.updateStreamStats(), 2000);
    }

    /**
     * Stop periodic status updates
     */
    stopStatusUpdates() {
        if (this.statusUpdateInterval) {
            clearInterval(this.statusUpdateInterval);
            this.statusUpdateInterval = null;
        }
    }

    /**
     * Update stream statistics
     */
    async updateStreamStats() {
        try {
            const response = await fetch('/api/stream/stats');
            const data = await response.json();
            
            this.updateStreamStatus(data.status);
            this.updateStreamStatistics(data.stats);
        } catch (error) {
            console.error('Error fetching stream stats:', error);
        }
    }

    /**
     * Update stream status display
     */
    updateStreamStatus(status) {
        this.updateElement('streamStatus', status.streaming ? 'Online' : 'Offline');
        this.updateElement('viewerCount', status.clients || 0);
        this.updateElement('currentFPS', `${status.fps || 0} FPS`);
        this.updateElement('currentQuality', status.quality || 'Medium');
        
        // Update stream indicator
        const indicator = document.getElementById('streamIndicator');
        if (indicator) {
            indicator.className = `status-indicator ${status.streaming ? 'status-online' : 'status-offline'}`;
        }
        
        // Update duration
        if (status.streaming && status.duration) {
            this.updateElement('streamDuration', this.formatDuration(status.duration));
        }
        
        // Update motion status
        if (status.motionTriggered !== undefined) {
            this.updateElement('motionStatus', status.motionTriggered ? 'Yes' : 'No');
            const motionElement = document.getElementById('motionStatus');
            if (motionElement) {
                motionElement.className = status.motionTriggered ? 'text-success' : 'text-muted';
            }
        }
    }

    /**
     * Update stream statistics
     */
    updateStreamStatistics(stats) {
        this.updateElement('totalFrames', stats.totalFrames || 0);
        this.updateElement('dataTransferred', this.formatBytes(stats.totalBytes || 0));
        this.updateElement('droppedFrames', stats.droppedFrames || 0);
    }

    /**
     * Update system status
     */
    updateSystemStatus(system) {
        if (system.battery) {
            this.updateElement('batteryLevel', `${system.battery.percentage}%`);
            this.updateBatteryIndicator(system.battery.percentage);
        }
        
        if (system.power) {
            const powerStates = ['Critical', 'Low', 'Good', 'Normal'];
            this.updateElement('powerState', powerStates[system.power.state] || 'Unknown');
        }
        
        if (system.memory) {
            this.updateElement('memoryFree', this.formatBytes(system.memory.free));
        }
        
        if (system.network) {
            this.updateNetworkStatus(system.network);
        }
    }

    /**
     * Update battery indicator
     */
    updateBatteryIndicator(percentage) {
        // Create or update battery indicator if it exists
        const batteryElement = document.getElementById('batteryLevel');
        if (batteryElement && batteryElement.parentElement) {
            let indicator = batteryElement.parentElement.querySelector('.battery-icon');
            if (!indicator) {
                indicator = document.createElement('div');
                indicator.className = 'battery-icon';
                indicator.innerHTML = '<div class="battery-level"></div>';
                batteryElement.parentElement.appendChild(indicator);
            }
            
            const level = indicator.querySelector('.battery-level');
            if (level) {
                level.style.width = `${percentage}%`;
                level.className = `battery-level ${percentage < 20 ? 'low' : percentage < 50 ? 'medium' : ''}`;
            }
        }
    }

    /**
     * Handle wildlife detection events
     */
    handleWildlifeDetection(data) {
        const message = `${data.species} detected (${data.confidence.toFixed(1)}% confidence)`;
        this.showNotification(message, 'info');
        
        // Trigger stream if motion-only mode is active
        if (!this.streamActive && document.getElementById('motionOnly')?.checked) {
            this.startStream();
        }
    }

    /**
     * Update power status
     */
    updatePowerStatus(data) {
        if (data.battery) {
            this.updateElement('batteryLevel', `${data.battery.percentage}%`);
        }
        
        if (data.charging !== undefined) {
            const chargingElement = document.getElementById('chargingStatus');
            if (chargingElement) {
                chargingElement.textContent = data.charging ? 'Charging' : 'Not Charging';
                chargingElement.className = data.charging ? 'text-success' : 'text-muted';
            }
        }
    }

    /**
     * Update network status
     */
    updateNetworkStatus(network) {
        if (network.connected !== undefined) {
            const wifiElement = document.getElementById('wifiStatus');
            if (wifiElement) {
                wifiElement.textContent = network.connected ? 'Connected' : 'Disconnected';
                wifiElement.className = network.connected ? 'text-success' : 'text-danger';
            }
        }
        
        if (network.signal !== undefined) {
            const signalElement = document.getElementById('signalStrength');
            if (signalElement) {
                signalElement.textContent = `${network.signal} dBm`;
            }
        }
    }

    /**
     * Update UI based on stream state
     */
    updateUI() {
        const startBtn = document.getElementById('startStreamBtn');
        const stopBtn = document.getElementById('stopStreamBtn');
        
        if (startBtn) startBtn.disabled = this.streamActive;
        if (stopBtn) stopBtn.disabled = !this.streamActive;
    }

    /**
     * Refresh status manually
     */
    async refreshStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            this.updateSystemStatus(data);
        } catch (error) {
            console.error('Error fetching status:', error);
            this.showNotification('Failed to refresh status', 'warning');
        }
    }

    /**
     * Set loading state
     */
    setLoading(loading) {
        const buttons = document.querySelectorAll('.btn');
        buttons.forEach(btn => {
            if (loading) {
                btn.classList.add('loading');
                btn.disabled = true;
            } else {
                btn.classList.remove('loading');
                this.updateUI(); // Restore proper button states
            }
        });
    }

    /**
     * Show notification
     */
    showNotification(message, type = 'info') {
        // Simple notification system - could be enhanced with a proper notification library
        const notification = document.createElement('div');
        notification.className = `alert alert-${type}`;
        notification.textContent = message;
        notification.style.position = 'fixed';
        notification.style.top = '20px';
        notification.style.right = '20px';
        notification.style.zIndex = '9999';
        notification.style.maxWidth = '300px';
        
        document.body.appendChild(notification);
        
        setTimeout(() => {
            notification.remove();
        }, 5000);
    }

    /**
     * Update element text content safely
     */
    updateElement(id, content) {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = content;
        }
    }

    /**
     * Format bytes to human readable string
     */
    formatBytes(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
    }

    /**
     * Format duration to MM:SS
     */
    formatDuration(seconds) {
        const mins = Math.floor(seconds / 60);
        const secs = seconds % 60;
        return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }

    /**
     * Add event listener
     */
    addEventListener(event, callback) {
        if (!this.eventListeners.has(event)) {
            this.eventListeners.set(event, []);
        }
        this.eventListeners.get(event).push(callback);
    }

    /**
     * Emit event
     */
    emit(event, data) {
        const listeners = this.eventListeners.get(event);
        if (listeners) {
            listeners.forEach(callback => callback(data));
        }
    }
}

// Initialize dashboard when DOM is ready
let dashboard = null;

document.addEventListener('DOMContentLoaded', function() {
    dashboard = new WildlifeStreamDashboard();
});

// Legacy function compatibility
function startStream() {
    if (dashboard) dashboard.startStream();
}

function stopStream() {
    if (dashboard) dashboard.stopStream();
}

function updateStreamConfig() {
    if (dashboard) dashboard.updateStreamConfig();
}

function refreshStatus() {
    if (dashboard) dashboard.refreshStatus();
}