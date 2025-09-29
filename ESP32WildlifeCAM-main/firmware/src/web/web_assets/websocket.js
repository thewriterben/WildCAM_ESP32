/**
 * WebSocket Client for Wildlife Camera Dashboard
 * Handles real-time communication with the ESP32 server
 */

class WebSocketClient {
    constructor() {
        this.socket = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 1000; // Start with 1 second
        this.messageHandlers = new Map();
        this.connectionStatusCallbacks = [];
        this.isConnecting = false;
        
        // Message types from server
        this.MESSAGE_TYPES = {
            SYSTEM_STATUS: 0,
            WILDLIFE_DETECTION: 1,
            POWER_UPDATE: 2,
            STORAGE_UPDATE: 3,
            CAMERA_STATUS: 4,
            ERROR_ALERT: 5,
            HEARTBEAT: 6
        };
        
        this.init();
    }
    
    init() {
        this.connect();
        
        // Register default message handlers
        this.onMessage('system_status', this.handleSystemStatus.bind(this));
        this.onMessage('wildlife_detection', this.handleWildlifeDetection.bind(this));
        this.onMessage('power_update', this.handlePowerUpdate.bind(this));
        this.onMessage('storage_update', this.handleStorageUpdate.bind(this));
        this.onMessage('camera_status', this.handleCameraStatus.bind(this));
        this.onMessage('error_alert', this.handleErrorAlert.bind(this));
        this.onMessage('heartbeat', this.handleHeartbeat.bind(this));
    }
    
    connect() {
        if (this.isConnecting || this.isConnected()) {
            return;
        }
        
        this.isConnecting = true;
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        console.log('Connecting to WebSocket:', wsUrl);
        this.updateConnectionStatus('connecting');
        
        try {
            this.socket = new WebSocket(wsUrl);
            this.setupEventHandlers();
        } catch (error) {
            console.error('WebSocket connection error:', error);
            this.handleConnectionError();
        }
    }
    
    setupEventHandlers() {
        this.socket.onopen = (event) => {
            console.log('WebSocket connected');
            this.isConnecting = false;
            this.reconnectAttempts = 0;
            this.reconnectDelay = 1000;
            this.updateConnectionStatus('connected');
            
            // Request initial data
            this.send({ action: 'request_update' });
        };
        
        this.socket.onmessage = (event) => {
            try {
                const message = JSON.parse(event.data);
                this.handleMessage(message);
            } catch (error) {
                console.error('Error parsing WebSocket message:', error);
            }
        };
        
        this.socket.onclose = (event) => {
            console.log('WebSocket connection closed:', event.code, event.reason);
            this.isConnecting = false;
            this.socket = null;
            this.updateConnectionStatus('disconnected');
            
            // Attempt reconnection if not closed intentionally
            if (event.code !== 1000) {
                this.scheduleReconnect();
            }
        };
        
        this.socket.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.handleConnectionError();
        };
    }
    
    handleMessage(message) {
        const messageType = this.getMessageTypeName(message.type);
        
        if (this.messageHandlers.has(messageType)) {
            const handlers = this.messageHandlers.get(messageType);
            handlers.forEach(handler => {
                try {
                    handler(message.data, message.timestamp);
                } catch (error) {
                    console.error(`Error in message handler for ${messageType}:`, error);
                }
            });
        }
    }
    
    getMessageTypeName(type) {
        return Object.keys(this.MESSAGE_TYPES).find(key => 
            this.MESSAGE_TYPES[key] === type
        )?.toLowerCase() || 'unknown';
    }
    
    handleConnectionError() {
        this.isConnecting = false;
        this.socket = null;
        this.updateConnectionStatus('error');
        this.scheduleReconnect();
    }
    
    scheduleReconnect() {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            console.log('Max reconnect attempts reached');
            this.updateConnectionStatus('failed');
            return;
        }
        
        this.reconnectAttempts++;
        const delay = Math.min(this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1), 30000);
        
        console.log(`Reconnecting in ${delay}ms (attempt ${this.reconnectAttempts})`);
        
        setTimeout(() => {
            this.connect();
        }, delay);
    }
    
    send(data) {
        if (this.isConnected()) {
            try {
                this.socket.send(JSON.stringify(data));
                return true;
            } catch (error) {
                console.error('Error sending WebSocket message:', error);
                return false;
            }
        } else {
            console.warn('Cannot send message: WebSocket not connected');
            return false;
        }
    }
    
    isConnected() {
        return this.socket && this.socket.readyState === WebSocket.OPEN;
    }
    
    disconnect() {
        if (this.socket) {
            this.socket.close(1000, 'Client disconnect');
        }
    }
    
    onMessage(messageType, handler) {
        if (!this.messageHandlers.has(messageType)) {
            this.messageHandlers.set(messageType, []);
        }
        this.messageHandlers.get(messageType).push(handler);
    }
    
    offMessage(messageType, handler) {
        if (this.messageHandlers.has(messageType)) {
            const handlers = this.messageHandlers.get(messageType);
            const index = handlers.indexOf(handler);
            if (index > -1) {
                handlers.splice(index, 1);
            }
        }
    }
    
    onConnectionStatusChange(callback) {
        this.connectionStatusCallbacks.push(callback);
    }
    
    updateConnectionStatus(status) {
        this.connectionStatusCallbacks.forEach(callback => {
            try {
                callback(status);
            } catch (error) {
                console.error('Error in connection status callback:', error);
            }
        });
    }
    
    // Default message handlers
    handleSystemStatus(data, timestamp) {
        console.log('System status update:', data);
        
        // Dispatch custom event for components to listen to
        window.dispatchEvent(new CustomEvent('systemStatusUpdate', {
            detail: { data, timestamp }
        }));
    }
    
    handleWildlifeDetection(data, timestamp) {
        console.log('Wildlife detection:', data);
        
        // Show toast notification
        if (window.dashboard && window.dashboard.showToast) {
            window.dashboard.showToast('success', 
                `Wildlife detected: ${data.species} (${Math.round(data.confidence * 100)}% confidence)`
            );
        }
        
        // Dispatch custom event
        window.dispatchEvent(new CustomEvent('wildlifeDetection', {
            detail: { data, timestamp }
        }));
    }
    
    handlePowerUpdate(data, timestamp) {
        console.log('Power update:', data);
        
        // Check for low battery warning
        if (data.battery && data.battery.percentage < 20) {
            if (window.dashboard && window.dashboard.showToast) {
                window.dashboard.showToast('warning', 
                    `Low battery: ${data.battery.percentage}%`
                );
            }
        }
        
        window.dispatchEvent(new CustomEvent('powerUpdate', {
            detail: { data, timestamp }
        }));
    }
    
    handleStorageUpdate(data, timestamp) {
        console.log('Storage update:', data);
        
        // Check for low storage warning
        const usagePercent = data.total > 0 ? (data.used / data.total) * 100 : 0;
        if (usagePercent > 90) {
            if (window.dashboard && window.dashboard.showToast) {
                window.dashboard.showToast('warning', 
                    `Storage almost full: ${Math.round(usagePercent)}%`
                );
            }
        }
        
        window.dispatchEvent(new CustomEvent('storageUpdate', {
            detail: { data, timestamp }
        }));
    }
    
    handleCameraStatus(data, timestamp) {
        console.log('Camera status:', data);
        
        window.dispatchEvent(new CustomEvent('cameraStatusUpdate', {
            detail: { data, timestamp }
        }));
    }
    
    handleErrorAlert(data, timestamp) {
        console.error('System error alert:', data);
        
        if (window.dashboard && window.dashboard.showToast) {
            window.dashboard.showToast('error', data.message);
        }
        
        window.dispatchEvent(new CustomEvent('errorAlert', {
            detail: { data, timestamp }
        }));
    }
    
    handleHeartbeat(data, timestamp) {
        // Silent heartbeat - just update last seen time
        this.lastHeartbeat = timestamp;
    }
    
    // Utility methods for common operations
    requestUpdate() {
        return this.send({ action: 'request_update' });
    }
    
    ping() {
        return this.send({ action: 'ping' });
    }
    
    getConnectionInfo() {
        return {
            connected: this.isConnected(),
            reconnectAttempts: this.reconnectAttempts,
            lastHeartbeat: this.lastHeartbeat
        };
    }
}

// Global WebSocket client instance
window.wsClient = new WebSocketClient();

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = WebSocketClient;
}