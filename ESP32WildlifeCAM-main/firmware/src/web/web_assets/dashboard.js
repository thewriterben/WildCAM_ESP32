/**
 * Wildlife Camera Dashboard JavaScript
 * Main dashboard functionality and UI interactions
 */

class WildlifeDashboard {
    constructor() {
        this.currentTheme = localStorage.getItem('dashboard-theme') || 'light';
        this.currentPage = 0;
        this.pageSize = 20;
        this.totalImages = 0;
        this.charts = {};
        this.updateInterval = null;
        
        this.init();
    }
    
    init() {
        this.setupTheme();
        this.setupEventListeners();
        this.setupWebSocketEvents();
        this.setupLanguageSupport(); // Add language support
        this.startPeriodicUpdates();
        this.loadInitialData();
        
        // Hide loading overlay
        setTimeout(() => {
            document.getElementById('loading-overlay').classList.add('hidden');
        }, 1000);
    }
    
    setupTheme() {
        document.documentElement.setAttribute('data-theme', this.currentTheme);
        this.updateThemeIcon();
    }
    
    updateThemeIcon() {
        const themeIcon = document.querySelector('.theme-icon');
        if (themeIcon) {
            themeIcon.textContent = this.currentTheme === 'dark' ? '☀️' : '🌙';
        }
    }
    
    setupEventListeners() {
        // Theme toggle
        document.getElementById('theme-toggle').addEventListener('click', () => {
            this.toggleTheme();
        });
        
        // Camera controls
        document.getElementById('capture-btn').addEventListener('click', () => {
            this.capturePhoto();
        });
        
        document.getElementById('stream-btn').addEventListener('click', () => {
            this.toggleStream();
        });
        
        // Gallery controls
        document.getElementById('gallery-refresh').addEventListener('click', () => {
            this.refreshGallery();
        });
        
        document.getElementById('download-all').addEventListener('click', () => {
            this.downloadAllImages();
        });
        
        document.getElementById('prev-page').addEventListener('click', () => {
            this.previousPage();
        });
        
        document.getElementById('next-page').addEventListener('click', () => {
            this.nextPage();
        });
        
        // Storage refresh
        document.getElementById('storage-refresh').addEventListener('click', () => {
            this.refreshStorage();
        });
        
        // Settings controls
        document.getElementById('motion-sensitivity').addEventListener('input', (e) => {
            document.querySelector('.range-value').textContent = e.target.value + '%';
            this.updateSetting('motionSensitivity', e.target.value);
        });
        
        document.getElementById('capture-interval').addEventListener('change', (e) => {
            this.updateSetting('captureInterval', e.target.value);
        });
        
        // Chart controls
        document.getElementById('chart-timeframe').addEventListener('change', (e) => {
            this.updateChartTimeframe(e.target.value);
        });
        
        document.querySelectorAll('.chart-tab').forEach(tab => {
            tab.addEventListener('click', (e) => {
                this.switchChart(e.target.dataset.chart);
            });
        });
        
        // Log controls
        document.getElementById('log-filter').addEventListener('change', (e) => {
            this.filterLogs(e.target.value);
        });
        
        document.getElementById('clear-logs').addEventListener('click', () => {
            this.clearLogs();
        });
        
        // Modal controls
        document.querySelector('.modal-close').addEventListener('click', () => {
            this.closeModal();
        });
        
        document.getElementById('image-modal').addEventListener('click', (e) => {
            if (e.target.id === 'image-modal') {
                this.closeModal();
            }
        });
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (e) => {
            this.handleKeyboard(e);
        });
    }
    
    setupWebSocketEvents() {
        // Connection status updates
        window.wsClient.onConnectionStatusChange((status) => {
            this.updateConnectionStatus(status);
        });
        
        // System status updates
        window.addEventListener('systemStatusUpdate', (e) => {
            this.updateSystemStatus(e.detail.data);
        });
        
        // Wildlife detection notifications
        window.addEventListener('wildlifeDetection', (e) => {
            this.handleWildlifeDetection(e.detail.data);
        });
        
        // Power updates
        window.addEventListener('powerUpdate', (e) => {
            this.updatePowerStatus(e.detail.data);
        });
        
        // Storage updates
        window.addEventListener('storageUpdate', (e) => {
            this.updateStorageStatus(e.detail.data);
        });
        
        // Camera status updates
        window.addEventListener('cameraStatusUpdate', (e) => {
            this.updateCameraStatus(e.detail.data);
        });
        
        // Error alerts
        window.addEventListener('errorAlert', (e) => {
            this.showToast('error', e.detail.data.message);
        });
    }
    
    setupLanguageSupport() {
        // Listen for language changes
        if (window.i18n) {
            window.i18n.addObserver((event, data) => {
                if (event === 'languageChanged') {
                    this.onLanguageChange(data.language);
                }
            });
        }
        
        // Handle custom language change events
        document.addEventListener('languageChanged', (e) => {
            if (e.detail && e.detail.language) {
                this.onLanguageChange(e.detail.language);
            }
        });
    }
    
    onLanguageChange(language) {
        console.log(`Dashboard language changed to: ${language}`);
        
        // Update dynamic content that may not be handled by the i18n framework
        this.updateLocalizedContent();
        
        // Update chart labels if charts are loaded
        if (this.charts && Object.keys(this.charts).length > 0) {
            this.updateChartLabels();
        }
        
        // Refresh data to get localized system messages
        this.loadSystemStatus();
        
        // Show language change notification
        const languageInfo = window.i18n ? window.i18n.getCurrentLanguageInfo() : null;
        if (languageInfo) {
            this.showToast('success', `${languageInfo.flag} ${t('ui.language')}: ${languageInfo.nativeName}`);
        }
    }
    
    updateLocalizedContent() {
        // Update document title
        const titleKey = document.title.includes('Dashboard') ? 'ui.dashboard' : 'system.device_name';
        if (window.t) {
            document.title = window.t(titleKey) + ' - Wildlife Camera';
        }
        
        // Update any dynamic content that needs manual translation
        this.updateImageGalleryTexts();
        this.updateChartTooltips();
    }
    
    updateImageGalleryTexts() {
        // Update image gallery pagination and empty state texts
        const emptyState = document.querySelector('.gallery-empty-state');
        if (emptyState && window.t) {
            emptyState.textContent = window.t('storage.no_images');
        }
    }
    
    updateChartLabels() {
        // Update chart labels when language changes
        Object.keys(this.charts).forEach(chartType => {
            const chart = this.charts[chartType];
            if (chart && chart.options && window.t) {
                // Update chart title and axis labels based on chart type
                this.localizeChart(chart, chartType);
            }
        });
    }
    
    updateChartTooltips() {
        // Update chart tooltips with localized text
        if (window.formatDate && window.formatTime) {
            // Charts will use the global formatting functions automatically
        }
    }
    
    localizeChart(chart, chartType) {
        if (!window.t) return;
        
        try {
            switch (chartType) {
                case 'battery':
                    chart.options.plugins.title.text = window.t('power.battery_level');
                    chart.options.scales.y.title.text = window.t('power.percentage');
                    break;
                case 'storage':
                    chart.options.plugins.title.text = window.t('storage.storage_usage');
                    chart.options.scales.y.title.text = window.t('storage.usage_percent');
                    break;
                case 'wildlife':
                    chart.options.plugins.title.text = window.t('wildlife.activity_chart');
                    chart.options.scales.y.title.text = window.t('wildlife.detection_count');
                    break;
            }
            
            chart.update('none'); // Update without animation
        } catch (error) {
            console.warn('Failed to update chart labels:', error);
        }
    }
    
    startPeriodicUpdates() {
        // Update every 30 seconds
        this.updateInterval = setInterval(() => {
            if (window.wsClient.isConnected()) {
                window.wsClient.requestUpdate();
            } else {
                // Fallback to REST API if WebSocket is not connected
                this.loadSystemStatus();
            }
        }, 30000);
    }
    
    async loadInitialData() {
        try {
            await Promise.all([
                this.loadSystemStatus(),
                this.loadImageGallery(),
                this.loadConfiguration()
            ]);
        } catch (error) {
            console.error('Error loading initial data:', error);
            this.showToast('error', 'Failed to load dashboard data');
        }
    }
    
    async loadSystemStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            this.updateSystemStatus(data);
        } catch (error) {
            console.error('Error loading system status:', error);
        }
    }
    
    async loadImageGallery() {
        try {
            const response = await fetch(`/api/images?page=${this.currentPage}&size=${this.pageSize}`);
            const data = await response.json();
            this.updateImageGallery(data);
        } catch (error) {
            console.error('Error loading image gallery:', error);
        }
    }
    
    async loadConfiguration() {
        try {
            const response = await fetch('/api/config');
            const config = await response.json();
            this.updateConfiguration(config);
        } catch (error) {
            console.error('Error loading configuration:', error);
        }
    }
    
    updateConnectionStatus(status) {
        const statusElement = document.getElementById('connection-status');
        const indicator = statusElement.querySelector('.status-indicator');
        const text = statusElement.querySelector('.status-text');
        
        indicator.className = 'status-indicator';
        
        switch (status) {
            case 'connected':
                indicator.classList.add('connected');
                text.textContent = 'Connected';
                break;
            case 'connecting':
                indicator.classList.add('connecting');
                text.textContent = 'Connecting...';
                break;
            case 'disconnected':
                text.textContent = 'Disconnected';
                break;
            case 'error':
                text.textContent = 'Connection Error';
                break;
            case 'failed':
                text.textContent = 'Connection Failed';
                break;
        }
    }
    
    updateSystemStatus(data) {
        // Update system health metrics
        if (data.system) {
            document.getElementById('uptime').textContent = this.formatUptime(data.system.uptime);
            document.getElementById('temperature').textContent = `${data.system.temperature || '--'}°C`;
        }
        
        if (data.memory) {
            const freeKB = Math.round(data.memory.free / 1024);
            const totalKB = Math.round(data.memory.total / 1024);
            document.getElementById('memory').textContent = `${freeKB}KB / ${totalKB}KB`;
        }
        
        if (data.network) {
            document.getElementById('wifi-signal').textContent = 
                data.network.connected ? `${data.network.signal} dBm` : 'Disconnected';
        }
        
        // Update power status
        if (data.battery || data.solar || data.power) {
            this.updatePowerStatus(data);
        }
        
        // Update storage status
        if (data.storage) {
            this.updateStorageStatus(data.storage);
        }
        
        // Update camera status
        if (data.camera || data.captures) {
            this.updateCameraStatus(data);
        }
        
        // Update system status badge
        this.updateSystemStatusBadge(data);
    }
    
    updatePowerStatus(data) {
        const powerData = data.battery || data;
        
        if (powerData.percentage !== undefined) {
            const percentage = powerData.percentage;
            document.getElementById('battery-percentage').textContent = `${percentage}%`;
            document.getElementById('battery-level').style.width = `${percentage}%`;
        }
        
        if (powerData.voltage !== undefined) {
            document.getElementById('battery-voltage').textContent = `${powerData.voltage.toFixed(2)}V`;
        }
        
        if (data.solar && data.solar.voltage !== undefined) {
            document.getElementById('solar-voltage').textContent = `${data.solar.voltage.toFixed(2)}V`;
        }
        
        // Update charging status
        const chargingIcon = document.querySelector('.charging-icon');
        if (chargingIcon) {
            chargingIcon.style.display = powerData.charging ? 'inline' : 'none';
        }
    }
    
    updateStorageStatus(data) {
        const totalGB = (data.total / (1024 * 1024 * 1024)).toFixed(2);
        const usedGB = (data.used / (1024 * 1024 * 1024)).toFixed(2);
        const freeGB = (data.free / (1024 * 1024 * 1024)).toFixed(2);
        const usagePercent = data.total > 0 ? (data.used / data.total) * 100 : 0;
        
        document.getElementById('storage-used-text').textContent = `${usedGB} GB`;
        document.getElementById('storage-free-text').textContent = `${freeGB} GB`;
        document.getElementById('image-count').textContent = data.images || 0;
        document.getElementById('storage-used').style.width = `${usagePercent}%`;
    }
    
    updateCameraStatus(data) {
        const cameraData = data.camera || data;
        const capturesData = data.captures || data;
        
        // Update capture counts
        if (capturesData.daily !== undefined) {
            document.getElementById('daily-captures').textContent = capturesData.daily;
        }
        
        if (capturesData.total !== undefined) {
            document.getElementById('total-captures').textContent = capturesData.total;
        }
        
        // Update wildlife detection info
        if (data.wildlife) {
            const speciesElement = document.getElementById('last-species');
            const confidenceElement = document.getElementById('last-confidence');
            
            if (data.wildlife.lastSpecies && data.wildlife.lastSpecies !== '') {
                speciesElement.textContent = data.wildlife.lastSpecies;
                confidenceElement.textContent = `${Math.round(data.wildlife.lastConfidence * 100)}%`;
            } else {
                speciesElement.textContent = 'No recent activity';
                confidenceElement.textContent = '';
            }
        }
        
        // Update camera ready status
        const cameraStatus = document.getElementById('camera-status');
        if (cameraData.ready !== undefined) {
            const indicator = cameraStatus.querySelector('.status-indicator');
            const text = cameraStatus.querySelector('span:last-child');
            
            if (cameraData.ready) {
                indicator.className = 'status-indicator ready';
                text.textContent = 'Ready';
            } else {
                indicator.className = 'status-indicator';
                text.textContent = 'Not Ready';
            }
        }
    }
    
    updateSystemStatusBadge(data) {
        const badge = document.getElementById('system-status');
        let status = 'good';
        
        // Check various system health indicators
        if (data.battery && data.battery.percentage < 20) {
            status = 'warning';
        }
        
        if (data.storage && data.storage.total > 0) {
            const usagePercent = (data.storage.used / data.storage.total) * 100;
            if (usagePercent > 90) {
                status = 'error';
            }
        }
        
        if (!data.network || !data.network.connected) {
            status = 'warning';
        }
        
        badge.className = `status-badge ${status}`;
        badge.textContent = status.charAt(0).toUpperCase() + status.slice(1);
    }
    
    handleWildlifeDetection(data) {
        // Update last detection display
        document.getElementById('last-species').textContent = data.species;
        document.getElementById('last-confidence').textContent = `${Math.round(data.confidence * 100)}%`;
        
        // Refresh gallery to show new image
        setTimeout(() => {
            this.refreshGallery();
        }, 1000);
        
        // Add to activity log
        this.addLogEntry('info', `Wildlife detected: ${data.species} (${Math.round(data.confidence * 100)}% confidence)`);
    }
    
    updateImageGallery(data) {
        const galleryGrid = document.getElementById('gallery-grid');
        galleryGrid.innerHTML = '';
        
        this.totalImages = data.total || 0;
        
        data.images.forEach(image => {
            const item = this.createGalleryItem(image);
            galleryGrid.appendChild(item);
        });
        
        this.updatePagination();
    }
    
    createGalleryItem(image) {
        const item = document.createElement('div');
        item.className = 'gallery-item';
        
        item.innerHTML = `
            <img src="/api/thumbnail?file=${image.filename}" alt="Wildlife Image" loading="lazy">
            <div class="gallery-item-overlay">
                <div>${image.species || 'Unknown'}</div>
                <div>${this.formatDate(image.timestamp)}</div>
            </div>
        `;
        
        item.addEventListener('click', () => {
            this.showImageModal(image);
        });
        
        return item;
    }
    
    updatePagination() {
        const totalPages = Math.ceil(this.totalImages / this.pageSize);
        const currentPageDisplay = this.totalImages > 0 ? this.currentPage + 1 : 0;
        
        document.getElementById('page-info').textContent = `Page ${currentPageDisplay} of ${totalPages}`;
        document.getElementById('prev-page').disabled = this.currentPage === 0;
        document.getElementById('next-page').disabled = this.currentPage >= totalPages - 1;
    }
    
    updateConfiguration(config) {
        document.getElementById('capture-interval').value = config.captureInterval || 30;
        document.getElementById('motion-sensitivity').value = config.motionSensitivity || 75;
        document.querySelector('.range-value').textContent = `${config.motionSensitivity || 75}%`;
    }
    
    // User actions
    toggleTheme() {
        this.currentTheme = this.currentTheme === 'light' ? 'dark' : 'light';
        document.documentElement.setAttribute('data-theme', this.currentTheme);
        localStorage.setItem('dashboard-theme', this.currentTheme);
        this.updateThemeIcon();
    }
    
    async capturePhoto() {
        const button = document.getElementById('capture-btn');
        const originalText = button.innerHTML;
        
        button.disabled = true;
        button.innerHTML = '<span class="btn-icon">⏳</span>Capturing...';
        
        try {
            const response = await fetch('/api/capture', { method: 'POST' });
            const result = await response.json();
            
            if (result.success) {
                this.showToast('success', 'Photo captured successfully');
                setTimeout(() => this.refreshGallery(), 1000);
            } else {
                this.showToast('error', 'Failed to capture photo');
            }
        } catch (error) {
            console.error('Capture error:', error);
            this.showToast('error', 'Failed to capture photo');
        } finally {
            button.disabled = false;
            button.innerHTML = originalText;
        }
    }
    
    toggleStream() {
        const button = document.getElementById('stream-btn');
        // Placeholder for streaming functionality
        this.showToast('info', 'Streaming feature coming soon');
    }
    
    refreshGallery() {
        this.currentPage = 0;
        this.loadImageGallery();
    }
    
    downloadAllImages() {
        this.showToast('info', 'Download feature coming soon');
    }
    
    previousPage() {
        if (this.currentPage > 0) {
            this.currentPage--;
            this.loadImageGallery();
        }
    }
    
    nextPage() {
        const totalPages = Math.ceil(this.totalImages / this.pageSize);
        if (this.currentPage < totalPages - 1) {
            this.currentPage++;
            this.loadImageGallery();
        }
    }
    
    refreshStorage() {
        fetch('/api/storage')
            .then(response => response.json())
            .then(data => this.updateStorageStatus(data))
            .catch(error => {
                console.error('Storage refresh error:', error);
                this.showToast('error', 'Failed to refresh storage data');
            });
    }
    
    async updateSetting(setting, value) {
        try {
            const config = { [setting]: value };
            const response = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            
            if (response.ok) {
                this.showToast('success', 'Setting updated');
            } else {
                this.showToast('error', 'Failed to update setting');
            }
        } catch (error) {
            console.error('Setting update error:', error);
            this.showToast('error', 'Failed to update setting');
        }
    }
    
    updateChartTimeframe(timeframe) {
        // Placeholder for chart timeframe update
        console.log('Chart timeframe updated:', timeframe);
    }
    
    switchChart(chartType) {
        document.querySelectorAll('.chart-tab').forEach(tab => {
            tab.classList.remove('active');
        });
        document.querySelector(`[data-chart="${chartType}"]`).classList.add('active');
        
        // Placeholder for chart switching
        console.log('Chart switched to:', chartType);
    }
    
    filterLogs(filter) {
        // Placeholder for log filtering
        console.log('Log filter:', filter);
    }
    
    clearLogs() {
        document.getElementById('log-container').innerHTML = '';
        this.showToast('info', 'Logs cleared');
    }
    
    showImageModal(image) {
        const modal = document.getElementById('image-modal');
        const modalImage = document.getElementById('modal-image');
        const modalTitle = document.getElementById('modal-title');
        const modalSpecies = document.getElementById('modal-species');
        const modalConfidence = document.getElementById('modal-confidence');
        const modalTimestamp = document.getElementById('modal-timestamp');
        const modalFilesize = document.getElementById('modal-filesize');
        
        modalImage.src = `/images/${image.filename}`;
        modalTitle.textContent = image.filename;
        modalSpecies.textContent = image.species || 'Unknown';
        modalConfidence.textContent = image.confidence ? `${Math.round(image.confidence * 100)}%` : 'N/A';
        modalTimestamp.textContent = this.formatDate(image.timestamp);
        modalFilesize.textContent = this.formatFileSize(image.size);
        
        modal.classList.add('show');
    }
    
    closeModal() {
        document.getElementById('image-modal').classList.remove('show');
    }
    
    handleKeyboard(e) {
        if (e.key === 'Escape') {
            this.closeModal();
        } else if (e.key === 'r' && (e.ctrlKey || e.metaKey)) {
            e.preventDefault();
            this.refreshGallery();
        } else if (e.key === 'c' && (e.ctrlKey || e.metaKey)) {
            e.preventDefault();
            this.capturePhoto();
        }
    }
    
    // Utility methods
    formatUptime(uptime) {
        const seconds = Math.floor(uptime / 1000);
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        if (days > 0) {
            return `${days}d ${hours}h ${minutes}m`;
        } else if (hours > 0) {
            return `${hours}h ${minutes}m`;
        } else {
            return `${minutes}m`;
        }
    }
    
    formatDate(timestamp) {
        const date = new Date(timestamp);
        return date.toLocaleString();
    }
    
    formatFileSize(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }
    
    showToast(type, message, duration = 5000) {
        const container = document.getElementById('toast-container');
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        toast.textContent = message;
        
        container.appendChild(toast);
        
        // Auto remove after duration
        setTimeout(() => {
            if (toast.parentNode) {
                toast.parentNode.removeChild(toast);
            }
        }, duration);
        
        // Click to dismiss
        toast.addEventListener('click', () => {
            if (toast.parentNode) {
                toast.parentNode.removeChild(toast);
            }
        });
    }
    
    addLogEntry(level, message) {
        const container = document.getElementById('log-container');
        const entry = document.createElement('div');
        entry.className = 'log-entry';
        
        const timestamp = new Date().toLocaleTimeString();
        entry.innerHTML = `
            <span class="log-timestamp">${timestamp}</span>
            <span class="log-level ${level}">${level}</span>
            <span class="log-message">${message}</span>
        `;
        
        container.insertBefore(entry, container.firstChild);
        
        // Limit log entries to prevent memory issues
        while (container.children.length > 100) {
            container.removeChild(container.lastChild);
        }
    }
    
    destroy() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
        }
    }
}

// Initialize dashboard when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.dashboard = new WildlifeDashboard();
});

// Handle page unload
window.addEventListener('beforeunload', () => {
    if (window.dashboard) {
        window.dashboard.destroy();
    }
    if (window.wsClient) {
        window.wsClient.disconnect();
    }
});