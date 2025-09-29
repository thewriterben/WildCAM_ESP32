/**
 * WebSocket Service for real-time communication with WildCAM backend
 */

import io from 'socket.io-client';

class WebSocketService {
  constructor() {
    this.socket = null;
    this.listeners = new Map();
    this.isConnected = false;
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.reconnectDelay = 1000;
  }

  connect(url = 'http://localhost:5000') {
    if (this.socket && this.isConnected) {
      console.log('WebSocket already connected');
      return;
    }

    console.log('Connecting to WebSocket:', url);

    this.socket = io(url, {
      transports: ['websocket', 'polling'],
      upgrade: true,
      rememberUpgrade: true,
      timeout: 20000,
      forceNew: true,
    });

    this.setupEventHandlers();
  }

  setupEventHandlers() {
    if (!this.socket) return;

    this.socket.on('connect', () => {
      console.log('WebSocket connected');
      this.isConnected = true;
      this.reconnectAttempts = 0;
      this.emit('connect');
    });

    this.socket.on('disconnect', (reason) => {
      console.log('WebSocket disconnected:', reason);
      this.isConnected = false;
      this.emit('disconnect', reason);

      // Auto-reconnect logic
      if (reason === 'io server disconnect') {
        // Server-initiated disconnect, don't auto-reconnect
        return;
      }

      this.attemptReconnect();
    });

    this.socket.on('connect_error', (error) => {
      console.error('WebSocket connection error:', error);
      this.isConnected = false;
      this.emit('connect_error', error);
      this.attemptReconnect();
    });

    // Wildlife monitoring specific events
    this.socket.on('new_detection', (detection) => {
      console.log('New wildlife detection received:', detection);
      this.emit('new_detection', detection);
    });

    this.socket.on('device_status_update', (status) => {
      console.log('Device status update:', status);
      this.emit('device_status_update', status);
    });

    this.socket.on('new_alert', (alert) => {
      console.log('New alert received:', alert);
      this.emit('new_alert', alert);
    });

    this.socket.on('system_update', (update) => {
      console.log('System update:', update);
      this.emit('system_update', update);
    });

    // Heartbeat for connection health
    this.socket.on('heartbeat', (data) => {
      this.emit('heartbeat', data);
    });
  }

  attemptReconnect() {
    if (this.reconnectAttempts >= this.maxReconnectAttempts) {
      console.error('Max reconnection attempts reached');
      this.emit('reconnect_failed');
      return;
    }

    this.reconnectAttempts++;
    const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1);

    console.log(`Attempting to reconnect in ${delay}ms (attempt ${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

    setTimeout(() => {
      if (!this.isConnected && this.socket) {
        this.socket.connect();
      }
    }, delay);
  }

  disconnect() {
    if (this.socket) {
      console.log('Disconnecting WebSocket');
      this.socket.disconnect();
      this.socket = null;
      this.isConnected = false;
      this.listeners.clear();
    }
  }

  // Event listener management
  on(event, callback) {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, []);
    }
    this.listeners.get(event).push(callback);
  }

  off(event, callback) {
    if (this.listeners.has(event)) {
      const callbacks = this.listeners.get(event);
      const index = callbacks.indexOf(callback);
      if (index > -1) {
        callbacks.splice(index, 1);
      }
    }
  }

  emit(event, data) {
    if (this.listeners.has(event)) {
      this.listeners.get(event).forEach(callback => {
        try {
          callback(data);
        } catch (error) {
          console.error(`Error in WebSocket event handler for ${event}:`, error);
        }
      });
    }
  }

  // Send data to server
  send(event, data) {
    if (this.socket && this.isConnected) {
      this.socket.emit(event, data);
    } else {
      console.warn('Cannot send data: WebSocket not connected');
    }
  }

  // Convenience methods for specific events
  subscribeToDetections(callback) {
    this.on('new_detection', callback);
  }

  subscribeToDeviceUpdates(callback) {
    this.on('device_status_update', callback);
  }

  subscribeToAlerts(callback) {
    this.on('new_alert', callback);
  }

  subscribeToSystemUpdates(callback) {
    this.on('system_update', callback);
  }

  // Request specific camera data
  requestCameraStatus(deviceId) {
    this.send('request_camera_status', { device_id: deviceId });
  }

  // Request live stream
  requestLiveStream(deviceId) {
    this.send('request_live_stream', { device_id: deviceId });
  }

  // Send camera configuration
  sendCameraConfig(deviceId, config) {
    this.send('camera_config', { device_id: deviceId, config });
  }

  // Connection status
  getConnectionStatus() {
    return {
      connected: this.isConnected,
      reconnectAttempts: this.reconnectAttempts,
      maxReconnectAttempts: this.maxReconnectAttempts,
    };
  }

  // Health check
  ping() {
    if (this.socket && this.isConnected) {
      const startTime = Date.now();
      this.socket.emit('ping', startTime);
      
      const handlePong = (serverTime) => {
        const latency = Date.now() - startTime;
        console.log(`WebSocket latency: ${latency}ms`);
        this.emit('ping_response', { latency, serverTime });
        this.socket.off('pong', handlePong);
      };

      this.socket.on('pong', handlePong);
    }
  }
}

// Create and export singleton instance
const webSocketService = new WebSocketService();
export default webSocketService;