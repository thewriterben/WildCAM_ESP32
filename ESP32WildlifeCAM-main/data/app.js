/**
 * ESP32 Wildlife CAM - Web Application JavaScript
 * Handles user interface interactions and communication with ESP32 device
 */

class WildlifeCAM {
    constructor() {
        this.baseUrl = window.location.origin;
        this.websocket = null;
        this.isStreaming = false;
        this.currentSection = 'live';
        
        // Initialize when DOM is ready
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', () => this.init());
        } else {
            this.init();
        }
    }

    init() {
        console.log('Initializing ESP32 Wildlife CAM interface...');
        
        this.setupNavigation();
        this.setupLiveView();
        this.setupSettings();
        this.setupSystemInfo();
        this.setupAnalytics(); // Add analytics setup
        this.connectWebSocket();
        this.loadGallery();
        this.startStatusUpdates();
        
        console.log('Wildlife CAM interface initialized');
    }

    setupNavigation() {
        const navButtons = document.querySelectorAll('.nav-btn');
        const sections = document.querySelectorAll('.section');

        navButtons.forEach(button => {
            button.addEventListener('click', () => {
                const sectionId = button.dataset.section;
                
                // Update active button
                navButtons.forEach(btn => btn.classList.remove('active'));
                button.classList.add('active');
                
                // Update active section
                sections.forEach(section => section.classList.remove('active'));
                document.getElementById(`${sectionId}-section`).classList.add('active');
                
                this.currentSection = sectionId;
                this.onSectionChange(sectionId);
            });
        });
    }

    onSectionChange(sectionId) {
        switch(sectionId) {
            case 'live':
                this.refreshLiveView();
                break;
            case 'gallery':
                this.loadGallery();
                break;
            case 'analytics':
                this.loadAnalytics();
                break;
            case 'system':
                this.refreshSystemInfo();
                break;
        }
    }

    setupLiveView() {
        // Start/Stop streaming
        const startStreamBtn = document.getElementById('start-stream');
        if (startStreamBtn) {
            startStreamBtn.addEventListener('click', () => this.toggleStream());
        }

        // Capture photo
        const captureBtn = document.getElementById('capture-btn');
        if (captureBtn) {
            captureBtn.addEventListener('click', () => this.capturePhoto());
        }

        // Test motion detection
        const triggerBtn = document.getElementById('trigger-detection');
        if (triggerBtn) {
            triggerBtn.addEventListener('click', () => this.triggerMotionTest());
        }

        // Motion sensitivity slider
        const sensitivitySlider = document.getElementById('motion-sensitivity');
        const sensitivityValue = document.getElementById('sensitivity-value');
        if (sensitivitySlider && sensitivityValue) {
            sensitivitySlider.addEventListener('input', (e) => {
                sensitivityValue.textContent = e.target.value;
                this.updateSetting('motion_sensitivity', parseInt(e.target.value));
            });
        }

        // Checkboxes
        const nightMode = document.getElementById('night-mode');
        if (nightMode) {
            nightMode.addEventListener('change', (e) => {
                this.updateSetting('night_mode', e.target.checked);
            });
        }

        const flashEnabled = document.getElementById('flash-enabled');
        if (flashEnabled) {
            flashEnabled.addEventListener('change', (e) => {
                this.updateSetting('flash_enabled', e.target.checked);
            });
        }
    }

    setupSettings() {
        // Save settings button
        const saveBtn = document.getElementById('save-settings');
        if (saveBtn) {
            saveBtn.addEventListener('click', () => this.saveSettings());
        }

        // Reset settings button
        const resetBtn = document.getElementById('reset-settings');
        if (resetBtn) {
            resetBtn.addEventListener('click', () => this.resetSettings());
        }

        // PIR sensitivity slider
        const pirSlider = document.getElementById('pir-sensitivity');
        const pirValue = document.getElementById('pir-value');
        if (pirSlider && pirValue) {
            pirSlider.addEventListener('input', (e) => {
                pirValue.textContent = e.target.value;
            });
        }
    }

    setupSystemInfo() {
        // System action buttons
        const restartBtn = document.getElementById('restart-system');
        if (restartBtn) {
            restartBtn.addEventListener('click', () => this.restartSystem());
        }

        const updateBtn = document.getElementById('update-firmware');
        if (updateBtn) {
            updateBtn.addEventListener('click', () => this.updateFirmware());
        }

        const downloadLogsBtn = document.getElementById('download-logs');
        if (downloadLogsBtn) {
            downloadLogsBtn.addEventListener('click', () => this.downloadLogs());
        }

        const factoryResetBtn = document.getElementById('factory-reset');
        if (factoryResetBtn) {
            factoryResetBtn.addEventListener('click', () => this.factoryReset());
        }
    }

    connectWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        try {
            this.websocket = new WebSocket(wsUrl);
            
            this.websocket.onopen = () => {
                console.log('WebSocket connected');
                this.updateConnectionStatus(true);
            };
            
            this.websocket.onmessage = (event) => {
                this.handleWebSocketMessage(event.data);
            };
            
            this.websocket.onclose = () => {
                console.log('WebSocket disconnected');
                this.updateConnectionStatus(false);
                // Attempt to reconnect after 5 seconds
                setTimeout(() => this.connectWebSocket(), 5000);
            };
            
            this.websocket.onerror = (error) => {
                console.error('WebSocket error:', error);
                this.updateConnectionStatus(false);
            };
        } catch (error) {
            console.error('Failed to connect WebSocket:', error);
            this.updateConnectionStatus(false);
        }
    }

    handleWebSocketMessage(data) {
        try {
            const message = JSON.parse(data);
            
            switch(message.type) {
                case 'status_update':
                    this.updateSystemStatus(message.data);
                    break;
                case 'new_capture':
                    this.handleNewCapture(message.data);
                    break;
                case 'motion_event':
                    this.handleMotionEvent(message.data);
                    break;
                case 'system_alert':
                    this.showAlert(message.data.message, message.data.type);
                    break;
                default:
                    console.log('Unknown message type:', message.type);
            }
        } catch (error) {
            console.error('Error parsing WebSocket message:', error);
        }
    }

    async toggleStream() {
        const button = document.getElementById('start-stream');
        const liveImage = document.getElementById('live-image');
        const placeholder = document.querySelector('.feed-placeholder');
        
        if (!this.isStreaming) {
            try {
                button.textContent = 'Starting...';
                button.disabled = true;
                
                const response = await this.apiCall('/api/stream/start', 'POST');
                
                if (response.success) {
                    this.isStreaming = true;
                    placeholder.style.display = 'none';
                    liveImage.style.display = 'block';
                    button.textContent = 'Stop Stream';
                    button.classList.remove('btn-primary');
                    button.classList.add('btn-danger');
                    
                    // Start image refresh
                    this.startImageRefresh();
                } else {
                    throw new Error(response.message || 'Failed to start stream');
                }
            } catch (error) {
                this.showAlert(`Failed to start stream: ${error.message}`, 'error');
                button.textContent = 'Start Live Stream';
            } finally {
                button.disabled = false;
            }
        } else {
            try {
                await this.apiCall('/api/stream/stop', 'POST');
                this.stopStream();
            } catch (error) {
                this.showAlert(`Failed to stop stream: ${error.message}`, 'error');
            }
        }
    }

    startImageRefresh() {
        if (this.imageRefreshInterval) {
            clearInterval(this.imageRefreshInterval);
        }
        
        this.imageRefreshInterval = setInterval(() => {
            if (this.isStreaming) {
                const liveImage = document.getElementById('live-image');
                if (liveImage) {
                    // Add timestamp to prevent caching
                    liveImage.src = `/api/stream/current?t=${Date.now()}`;
                }
            }
        }, 1000); // Refresh every second
    }

    stopStream() {
        this.isStreaming = false;
        
        if (this.imageRefreshInterval) {
            clearInterval(this.imageRefreshInterval);
        }
        
        const button = document.getElementById('start-stream');
        const liveImage = document.getElementById('live-image');
        const placeholder = document.querySelector('.feed-placeholder');
        
        button.textContent = 'Start Live Stream';
        button.classList.remove('btn-danger');
        button.classList.add('btn-primary');
        
        liveImage.style.display = 'none';
        placeholder.style.display = 'block';
    }

    async capturePhoto() {
        const button = document.getElementById('capture-btn');
        const originalText = button.textContent;
        
        try {
            button.textContent = 'Capturing...';
            button.disabled = true;
            
            const response = await this.apiCall('/api/camera/capture', 'POST');
            
            if (response.success) {
                this.showAlert('Photo captured successfully!', 'success');
                this.addActivityItem('Manual photo capture');
                
                // Refresh gallery if visible
                if (this.currentSection === 'gallery') {
                    this.loadGallery();
                }
            } else {
                throw new Error(response.message || 'Capture failed');
            }
        } catch (error) {
            this.showAlert(`Capture failed: ${error.message}`, 'error');
        } finally {
            button.textContent = originalText;
            button.disabled = false;
        }
    }

    async triggerMotionTest() {
        const button = document.getElementById('trigger-detection');
        const originalText = button.textContent;
        
        try {
            button.textContent = 'Testing...';
            button.disabled = true;
            
            const response = await this.apiCall('/api/motion/test', 'POST');
            
            if (response.success) {
                this.showAlert('Motion detection test completed', 'info');
                this.addActivityItem('Motion detection test');
            } else {
                throw new Error(response.message || 'Test failed');
            }
        } catch (error) {
            this.showAlert(`Test failed: ${error.message}`, 'error');
        } finally {
            button.textContent = originalText;
            button.disabled = false;
        }
    }

    async loadGallery() {
        const gallery = document.getElementById('image-gallery');
        if (!gallery) return;
        
        try {
            gallery.innerHTML = '<div class="loading">Loading images...</div>';
            
            const response = await this.apiCall('/api/gallery/list');
            
            if (response.success && response.images) {
                this.renderGallery(response.images);
            } else {
                gallery.innerHTML = '<div class="no-images">No images found</div>';
            }
        } catch (error) {
            gallery.innerHTML = '<div class="error">Failed to load gallery</div>';
            console.error('Gallery load error:', error);
        }
    }

    renderGallery(images) {
        const gallery = document.getElementById('image-gallery');
        
        if (images.length === 0) {
            gallery.innerHTML = '<div class="no-images">No images found</div>';
            return;
        }
        
        gallery.innerHTML = images.map(image => `
            <div class="image-item" data-image="${image.filename}">
                <img src="/api/gallery/thumbnail/${image.filename}" 
                     alt="Wildlife capture" 
                     loading="lazy"
                     onclick="window.wildlifeCAM.viewImage('${image.filename}')">
                <div class="image-info">
                    <span class="image-date">${this.formatDate(image.timestamp)}</span>
                    <span class="image-species">${image.species || 'Unknown'}</span>
                </div>
            </div>
        `).join('');
    }

    viewImage(filename) {
        // Open image in full size (could be a modal or new tab)
        window.open(`/api/gallery/full/${filename}`, '_blank');
    }

    // Enhanced Analytics Setup
    setupAnalytics() {
        this.charts = new Map();
        this.currentAnalyticsTab = 'overview';
        this.analyticsData = {};
        
        // Setup analytics navigation
        document.querySelectorAll('.analytics-tab').forEach(tab => {
            tab.addEventListener('click', (e) => {
                this.switchAnalyticsTab(e.target.dataset.tab);
            });
        });
        
        // Setup analytics controls
        const timeRangeSelect = document.getElementById('analytics-timerange');
        if (timeRangeSelect) {
            timeRangeSelect.addEventListener('change', () => {
                this.loadAnalyticsData();
            });
        }
        
        const refreshBtn = document.getElementById('refresh-analytics');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', () => {
                this.loadAnalyticsData();
            });
        }
        
        const exportBtn = document.getElementById('export-analytics');
        if (exportBtn) {
            exportBtn.addEventListener('click', () => {
                this.exportAnalyticsData();
            });
        }
        
        // Load initial analytics data
        this.loadAnalyticsData();
    }
    
    switchAnalyticsTab(tabName) {
        // Update tab buttons
        document.querySelectorAll('.analytics-tab').forEach(tab => {
            tab.classList.remove('active');
        });
        document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
        
        // Update tab content
        document.querySelectorAll('.analytics-tab-content').forEach(content => {
            content.classList.remove('active');
        });
        document.getElementById(`${tabName}-tab`).classList.add('active');
        
        this.currentAnalyticsTab = tabName;
        
        // Load specific analytics data for the tab
        this.loadTabAnalytics(tabName);
    }
    
    async loadAnalyticsData() {
        const timeRange = document.getElementById('analytics-timerange')?.value || 'week';
        
        try {
            // Load summary data for overview
            const summaryResponse = await this.apiCall('/api/analytics/summary');
            if (summaryResponse) {
                this.analyticsData.summary = summaryResponse;
                this.updateOverviewMetrics(summaryResponse);
            }
            
            // Load current tab specific data
            await this.loadTabAnalytics(this.currentAnalyticsTab);
            
        } catch (error) {
            console.error('Analytics load error:', error);
            this.showAlert('Failed to load analytics data', 'error');
        }
    }
    
    async loadTabAnalytics(tabName) {
        const timeRange = document.getElementById('analytics-timerange')?.value || 'week';
        
        try {
            switch (tabName) {
                case 'overview':
                    await this.loadOverviewAnalytics(timeRange);
                    break;
                case 'wildlife':
                    await this.loadWildlifeAnalytics(timeRange);
                    break;
                case 'system':
                    await this.loadSystemAnalytics(timeRange);
                    break;
                case 'historical':
                    await this.loadHistoricalAnalytics(timeRange);
                    break;
                case 'performance':
                    await this.loadPerformanceAnalytics(timeRange);
                    break;
            }
        } catch (error) {
            console.error(`Failed to load ${tabName} analytics:`, error);
        }
    }
    
    async loadOverviewAnalytics(timeRange) {
        const [wildlifeResponse, systemResponse] = await Promise.all([
            this.apiCall(`/api/analytics/wildlife?timeRange=${timeRange}`),
            this.apiCall(`/api/analytics/system?timeRange=${timeRange}`)
        ]);
        
        if (wildlifeResponse) {
            this.renderOverviewActivityChart(wildlifeResponse.dailyActivity);
            this.renderOverviewSpeciesChart(wildlifeResponse.speciesDistribution);
        }
    }
    
    async loadWildlifeAnalytics(timeRange) {
        const response = await this.apiCall(`/api/analytics/wildlife?timeRange=${timeRange}`);
        if (response) {
            this.renderSpeciesFrequencyChart(response.speciesDistribution);
            this.renderConfidenceDistributionChart(response.confidenceDistribution);
            this.renderHourlyHeatmapChart(response.hourlyActivity);
            this.renderMotionSuccessChart(response.motionStats);
            this.updateSpeciesTable(response.speciesDistribution);
        }
    }
    
    async loadSystemAnalytics(timeRange) {
        const response = await this.apiCall(`/api/analytics/system?timeRange=${timeRange}`);
        if (response) {
            this.renderBatteryTrendChart(response.batteryHistory);
            this.renderPowerConsumptionChart(response.battery);
            this.renderStorageUsageChart(response.storage);
            this.renderNetworkPerformanceChart(response.network);
            this.renderTemperatureTrendChart(response.temperatureHistory);
            this.renderSystemLoadChart(response.performance);
        }
    }
    
    async loadHistoricalAnalytics(timeRange) {
        const response = await this.apiCall(`/api/analytics/historical?timeRange=${timeRange}`);
        if (response) {
            this.updateTrendCards(response.trends);
            this.renderMonthlyComparisonChart(response.monthlyComparison);
            this.renderSeasonalPatternsChart(response.seasonalPatterns);
            this.updateMilestones(response.milestones);
        }
    }
    
    async loadPerformanceAnalytics(timeRange) {
        const response = await this.apiCall(`/api/analytics/performance?timeRange=${timeRange}`);
        if (response) {
            this.updatePerformanceScores(response.healthScores);
            this.renderResponseTimeChart(response.metrics);
            this.renderMemoryUtilizationChart(response.metrics);
            this.updateErrorMetrics(response.errorAnalysis);
        }
    }
    
    updateOverviewMetrics(data) {
        // Update metric cards with real data
        this.updateElement('total-captures', data.totalCaptures);
        this.updateElement('species-count', data.speciesCount);
        this.updateElement('peak-activity', data.peakActivity);
        this.updateElement('detection-accuracy', `${data.accuracy}%`);
        this.updateElement('system-health', `${data.batteryHealth}%`);
        this.updateElement('system-uptime', this.formatUptime(data.uptime));
        
        // Update change indicators
        this.updateElement('captures-change', data.trends?.capturesChange || 'N/A');
        this.updateElement('species-change', `${data.trends?.newSpecies || 0} new this week`);
        this.updateElement('activity-pattern', data.lastDetection || 'No recent activity');
        this.updateElement('accuracy-trend', data.trends?.accuracyChange || 'N/A');
        this.updateElement('health-status', data.systemStatus || 'Unknown');
        this.updateElement('uptime-change', data.trends?.uptimeChange || 'N/A');
    }
    
    // Chart rendering methods
    renderOverviewActivityChart(activityData) {
        const canvas = document.getElementById('overview-activity-chart');
        if (!canvas || !activityData) return;
        
        this.destroyChart('overview-activity');
        
        const ctx = canvas.getContext('2d');
        this.charts.set('overview-activity', new Chart(ctx, {
            type: 'line',
            data: {
                labels: activityData.map(d => d.date),
                datasets: [{
                    label: 'Daily Detections',
                    data: activityData.map(d => d.detections),
                    borderColor: '#228b22',
                    backgroundColor: 'rgba(34, 139, 34, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
        }));
    }
    
    renderOverviewSpeciesChart(speciesData) {
        const canvas = document.getElementById('overview-species-chart');
        if (!canvas || !speciesData) return;
        
        this.destroyChart('overview-species');
        
        const ctx = canvas.getContext('2d');
        this.charts.set('overview-species', new Chart(ctx, {
            type: 'pie',
            data: {
                labels: speciesData.map(s => s.name),
                datasets: [{
                    data: speciesData.map(s => s.count),
                    backgroundColor: [
                        '#228b22', '#32cd32', '#ffa500', '#2196f3', 
                        '#ff9800', '#9c27b0', '#f44336', '#795548'
                    ]
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'bottom'
                    }
                }
            }
        }));
    }
    
    renderSpeciesFrequencyChart(speciesData) {
        const canvas = document.getElementById('species-frequency-chart');
        if (!canvas || !speciesData) return;
        
        this.destroyChart('species-frequency');
        
        const ctx = canvas.getContext('2d');
        this.charts.set('species-frequency', new Chart(ctx, {
            type: 'bar',
            data: {
                labels: speciesData.map(s => s.name),
                datasets: [{
                    label: 'Detections',
                    data: speciesData.map(s => s.count),
                    backgroundColor: '#228b22'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    }
                }
            }
        }));
    }
    
    renderConfidenceDistributionChart(confidenceData) {
        const canvas = document.getElementById('confidence-distribution-chart');
        if (!canvas || !confidenceData) return;
        
        this.destroyChart('confidence-distribution');
        
        const ctx = canvas.getContext('2d');
        this.charts.set('confidence-distribution', new Chart(ctx, {
            type: 'bar',
            data: {
                labels: confidenceData.map(c => c.range),
                datasets: [{
                    label: 'Count',
                    data: confidenceData.map(c => c.count),
                    backgroundColor: '#2196f3'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    }
                }
            }
        }));
    }
    
    updateSpeciesTable(speciesData) {
        const tbody = document.getElementById('species-table-body');
        if (!tbody || !speciesData) return;
        
        tbody.innerHTML = '';
        speciesData.forEach(species => {
            const row = tbody.insertRow();
            row.innerHTML = `
                <td>${species.name}</td>
                <td>${species.count}</td>
                <td>${(species.avgConfidence * 100).toFixed(1)}%</td>
                <td>${species.activityPattern || 'Unknown'}</td>
                <td>2 hours ago</td>
            `;
        });
    }
    
    updatePerformanceScores(scores) {
        this.updateElement('overall-score', scores.overall);
        this.updateElement('camera-status', `${scores.camera}%`);
        this.updateElement('storage-status', `${scores.storage}%`);
        this.updateElement('battery-status', `${scores.battery}%`);
        this.updateElement('network-status', `${scores.network}%`);
    }
    
    updateErrorMetrics(errorData) {
        this.updateElement('total-errors', errorData.totalErrors);
        this.updateElement('critical-errors', errorData.criticalErrors);
        this.updateElement('recovery-rate', `${errorData.recoveryRate}%`);
    }
    
    async exportAnalyticsData() {
        const timeRange = document.getElementById('analytics-timerange')?.value || 'week';
        
        try {
            const response = await fetch(`${this.baseUrl}/api/analytics/export?format=json&timeRange=${timeRange}`);
            const data = await response.json();
            
            // Create and download the file
            const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `wildlife-analytics-${timeRange}-${new Date().toISOString().split('T')[0]}.json`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
            
            this.showAlert('Analytics data exported successfully', 'success');
        } catch (error) {
            console.error('Export error:', error);
            this.showAlert('Failed to export analytics data', 'error');
        }
    }
    
    // Helper methods
    updateElement(id, value) {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = value;
        }
    }
    
    destroyChart(chartKey) {
        if (this.charts.has(chartKey)) {
            this.charts.get(chartKey).destroy();
            this.charts.delete(chartKey);
        }
    }
    
    formatUptime(milliseconds) {
        const seconds = Math.floor(milliseconds / 1000);
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        return `${days}d ${hours}h ${minutes}m`;
    }

    async loadAnalytics() {
        // Keep existing method for backward compatibility
        return this.loadAnalyticsData();
    }

    updateAnalyticsDisplay(data) {
        // Keep existing method for backward compatibility
        return this.updateOverviewMetrics(data);
    }

    renderCharts(data) {
        // Keep existing method for backward compatibility but enhance it
        if (data.dailyActivity) {
            this.renderOverviewActivityChart(data.dailyActivity);
        }
        if (data.speciesDistribution) {
            this.renderOverviewSpeciesChart(data.speciesDistribution);
        }
    }

    renderActivityChart(activityData) {
        const canvas = document.getElementById('activity-chart');
        if (!canvas) return;
        
        const ctx = canvas.getContext('2d');
        // Simple bar chart implementation
        // In a real implementation, you'd use Chart.js or similar
        ctx.fillStyle = '#4CAF50';
        ctx.fillRect(10, 10, 50, 100);
        ctx.fillText('Activity Chart', 10, 130);
    }

    renderSpeciesChart(speciesData) {
        const canvas = document.getElementById('species-chart');
        if (!canvas) return;
        
        const ctx = canvas.getContext('2d');
        // Simple pie chart implementation
        ctx.fillStyle = '#2196F3';
        ctx.beginPath();
        ctx.arc(100, 100, 50, 0, Math.PI);
        ctx.fill();
        ctx.fillText('Species Chart', 10, 180);
    }

    async saveSettings() {
        const settings = this.collectSettings();
        
        try {
            const response = await this.apiCall('/api/settings/update', 'POST', settings);
            
            if (response.success) {
                this.showAlert('Settings saved successfully!', 'success');
            } else {
                throw new Error(response.message || 'Save failed');
            }
        } catch (error) {
            this.showAlert(`Failed to save settings: ${error.message}`, 'error');
        }
    }

    collectSettings() {
        return {
            imageQuality: document.getElementById('image-quality')?.value,
            captureResolution: document.getElementById('capture-resolution')?.value,
            pirSensitivity: document.getElementById('pir-sensitivity')?.value,
            triggerDelay: document.getElementById('trigger-delay')?.value,
            burstMode: document.getElementById('burst-mode')?.checked,
            sleepMode: document.getElementById('sleep-mode')?.value,
            lowBatteryThreshold: document.getElementById('low-battery-threshold')?.value,
        };
    }

    async resetSettings() {
        if (!confirm('Are you sure you want to reset all settings to defaults?')) {
            return;
        }
        
        try {
            const response = await this.apiCall('/api/settings/reset', 'POST');
            
            if (response.success) {
                this.showAlert('Settings reset to defaults', 'info');
                location.reload(); // Reload to show default values
            } else {
                throw new Error(response.message || 'Reset failed');
            }
        } catch (error) {
            this.showAlert(`Failed to reset settings: ${error.message}`, 'error');
        }
    }

    async updateSetting(key, value) {
        try {
            await this.apiCall('/api/settings/update', 'POST', { [key]: value });
        } catch (error) {
            console.error(`Failed to update ${key}:`, error);
        }
    }

    refreshSystemInfo() {
        this.updateSystemStatus();
    }

    async updateSystemStatus(data = null) {
        if (!data) {
            try {
                const response = await this.apiCall('/api/system/status');
                data = response.data;
            } catch (error) {
                console.error('Failed to fetch system status:', error);
                return;
            }
        }
        
        // Update status indicators
        const systemStatus = document.getElementById('system-status');
        const batteryLevel = document.getElementById('battery-level');
        const uptime = document.getElementById('uptime');
        
        if (systemStatus) {
            systemStatus.textContent = data.online ? 'Online' : 'Offline';
            systemStatus.className = `status-value ${data.online ? 'online' : 'offline'}`;
        }
        
        if (batteryLevel) {
            batteryLevel.textContent = `${data.batteryPercent || 0}%`;
        }
        
        if (uptime) {
            uptime.textContent = data.uptime || 'Unknown';
        }
        
        // Update battery indicator
        const batteryFill = document.querySelector('.battery-fill');
        if (batteryFill) {
            batteryFill.style.width = `${data.batteryPercent || 0}%`;
        }
    }

    updateConnectionStatus(connected) {
        const systemStatus = document.getElementById('system-status');
        if (systemStatus) {
            systemStatus.textContent = connected ? 'Online' : 'Offline';
            systemStatus.className = `status-value ${connected ? 'online' : 'offline'}`;
        }
    }

    handleNewCapture(data) {
        this.addActivityItem(`New capture: ${data.species || 'Unknown species'}`);
        
        // Update counters
        const totalCaptures = document.querySelector('.metric-value');
        if (totalCaptures) {
            const current = parseInt(totalCaptures.textContent.replace(',', '')) || 0;
            totalCaptures.textContent = (current + 1).toLocaleString();
        }
    }

    handleMotionEvent(data) {
        this.addActivityItem(`Motion detected: ${data.trigger || 'PIR sensor'}`);
    }

    addActivityItem(event) {
        const activityLog = document.getElementById('activity-log');
        if (!activityLog) return;
        
        const timestamp = new Date().toLocaleString();
        const newItem = document.createElement('div');
        newItem.className = 'activity-item';
        newItem.innerHTML = `
            <span class="timestamp">${timestamp}</span>
            <span class="event">${event}</span>
        `;
        
        activityLog.insertBefore(newItem, activityLog.firstChild);
        
        // Keep only last 20 items
        while (activityLog.children.length > 20) {
            activityLog.removeChild(activityLog.lastChild);
        }
    }

    showAlert(message, type = 'info') {
        // Create alert element
        const alert = document.createElement('div');
        alert.className = `alert alert-${type}`;
        alert.textContent = message;
        
        // Add to page
        document.body.appendChild(alert);
        
        // Auto-remove after 5 seconds
        setTimeout(() => {
            if (alert.parentNode) {
                alert.parentNode.removeChild(alert);
            }
        }, 5000);
    }

    async restartSystem() {
        if (!confirm('Are you sure you want to restart the system?')) {
            return;
        }
        
        try {
            await this.apiCall('/api/system/restart', 'POST');
            this.showAlert('System restart initiated', 'info');
        } catch (error) {
            this.showAlert(`Restart failed: ${error.message}`, 'error');
        }
    }

    async updateFirmware() {
        this.showAlert('Firmware update feature coming soon', 'info');
    }

    async downloadLogs() {
        try {
            const response = await fetch('/api/system/logs');
            const blob = await response.blob();
            
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `wildlife_cam_logs_${Date.now()}.txt`;
            a.click();
            
            window.URL.revokeObjectURL(url);
        } catch (error) {
            this.showAlert(`Download failed: ${error.message}`, 'error');
        }
    }

    async factoryReset() {
        if (!confirm('Are you sure you want to perform a factory reset? This will erase all data and settings.')) {
            return;
        }
        
        if (!confirm('This action cannot be undone. Continue?')) {
            return;
        }
        
        try {
            await this.apiCall('/api/system/factory-reset', 'POST');
            this.showAlert('Factory reset initiated', 'warning');
        } catch (error) {
            this.showAlert(`Factory reset failed: ${error.message}`, 'error');
        }
    }

    startStatusUpdates() {
        // Update status every 30 seconds
        setInterval(() => {
            this.updateSystemStatus();
        }, 30000);
    }

    formatDate(timestamp) {
        const date = new Date(timestamp);
        return date.toLocaleDateString() + ' ' + date.toLocaleTimeString();
    }

    async apiCall(endpoint, method = 'GET', data = null) {
        const options = {
            method,
            headers: {
                'Content-Type': 'application/json',
            }
        };
        
        if (data) {
            options.body = JSON.stringify(data);
        }
        
        const response = await fetch(endpoint, options);
        
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
        return await response.json();
    }
}

// Initialize the application
window.wildlifeCAM = new WildlifeCAM();