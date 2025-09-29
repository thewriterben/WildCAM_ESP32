/**
 * Simple Charts Library for Wildlife Camera Dashboard
 * Lightweight charting for real-time system metrics
 */

class SimpleChart {
    constructor(canvasId, options = {}) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.data = [];
        this.maxDataPoints = options.maxDataPoints || 50;
        this.options = {
            type: options.type || 'line',
            colors: options.colors || ['#3498db', '#27ae60', '#f39c12', '#e74c3c'],
            backgroundColor: options.backgroundColor || 'transparent',
            gridColor: options.gridColor || '#dee2e6',
            textColor: options.textColor || '#6c757d',
            lineWidth: options.lineWidth || 2,
            pointRadius: options.pointRadius || 3,
            padding: options.padding || 20,
            showGrid: options.showGrid !== false,
            showLabels: options.showLabels !== false,
            animate: options.animate !== false,
            ...options
        };
        
        this.setupCanvas();
        this.setupResponsive();
    }
    
    setupCanvas() {
        // Set canvas size for high DPI displays
        const rect = this.canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        
        this.canvas.width = rect.width * dpr;
        this.canvas.height = rect.height * dpr;
        
        this.ctx.scale(dpr, dpr);
        this.canvas.style.width = rect.width + 'px';
        this.canvas.style.height = rect.height + 'px';
        
        this.width = rect.width;
        this.height = rect.height;
        this.plotWidth = this.width - (this.options.padding * 2);
        this.plotHeight = this.height - (this.options.padding * 2);
    }
    
    setupResponsive() {
        const resizeObserver = new ResizeObserver(() => {
            this.setupCanvas();
            this.render();
        });
        resizeObserver.observe(this.canvas);
    }
    
    addData(value, label = null) {
        const timestamp = Date.now();
        this.data.push({
            value: value,
            label: label,
            timestamp: timestamp
        });
        
        // Limit data points
        if (this.data.length > this.maxDataPoints) {
            this.data.shift();
        }
        
        if (this.options.animate) {
            this.animateToNewData();
        } else {
            this.render();
        }
    }
    
    addMultiData(values, label = null) {
        const timestamp = Date.now();
        this.data.push({
            values: values,
            label: label,
            timestamp: timestamp
        });
        
        if (this.data.length > this.maxDataPoints) {
            this.data.shift();
        }
        
        if (this.options.animate) {
            this.animateToNewData();
        } else {
            this.render();
        }
    }
    
    setData(data) {
        this.data = data.slice(-this.maxDataPoints);
        this.render();
    }
    
    clear() {
        this.data = [];
        this.ctx.clearRect(0, 0, this.width, this.height);
    }
    
    render() {
        this.ctx.clearRect(0, 0, this.width, this.height);
        
        if (this.data.length === 0) {
            this.drawNoData();
            return;
        }
        
        if (this.options.showGrid) {
            this.drawGrid();
        }
        
        switch (this.options.type) {
            case 'line':
                this.drawLineChart();
                break;
            case 'bar':
                this.drawBarChart();
                break;
            case 'area':
                this.drawAreaChart();
                break;
        }
        
        if (this.options.showLabels) {
            this.drawLabels();
        }
    }
    
    drawGrid() {
        this.ctx.strokeStyle = this.options.gridColor;
        this.ctx.lineWidth = 1;
        this.ctx.setLineDash([2, 2]);
        
        const gridLines = 5;
        
        // Horizontal grid lines
        for (let i = 0; i <= gridLines; i++) {
            const y = this.options.padding + (this.plotHeight / gridLines) * i;
            this.ctx.beginPath();
            this.ctx.moveTo(this.options.padding, y);
            this.ctx.lineTo(this.options.padding + this.plotWidth, y);
            this.ctx.stroke();
        }
        
        // Vertical grid lines
        for (let i = 0; i <= gridLines; i++) {
            const x = this.options.padding + (this.plotWidth / gridLines) * i;
            this.ctx.beginPath();
            this.ctx.moveTo(x, this.options.padding);
            this.ctx.lineTo(x, this.options.padding + this.plotHeight);
            this.ctx.stroke();
        }
        
        this.ctx.setLineDash([]);
    }
    
    drawLineChart() {
        const isMultiSeries = this.data.length > 0 && Array.isArray(this.data[0].values);
        
        if (isMultiSeries) {
            this.drawMultiLineChart();
        } else {
            this.drawSingleLineChart();
        }
    }
    
    drawSingleLineChart() {
        const values = this.data.map(d => d.value);
        const minValue = Math.min(...values);
        const maxValue = Math.max(...values);
        const range = maxValue - minValue || 1;
        
        this.ctx.strokeStyle = this.options.colors[0];
        this.ctx.lineWidth = this.options.lineWidth;
        this.ctx.beginPath();
        
        this.data.forEach((point, index) => {
            const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
            const y = this.options.padding + this.plotHeight - 
                     ((point.value - minValue) / range) * this.plotHeight;
            
            if (index === 0) {
                this.ctx.moveTo(x, y);
            } else {
                this.ctx.lineTo(x, y);
            }
        });
        
        this.ctx.stroke();
        
        // Draw points
        this.ctx.fillStyle = this.options.colors[0];
        this.data.forEach((point, index) => {
            const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
            const y = this.options.padding + this.plotHeight - 
                     ((point.value - minValue) / range) * this.plotHeight;
            
            this.ctx.beginPath();
            this.ctx.arc(x, y, this.options.pointRadius, 0, 2 * Math.PI);
            this.ctx.fill();
        });
    }
    
    drawMultiLineChart() {
        const seriesCount = this.data[0].values.length;
        const allValues = this.data.flatMap(d => d.values);
        const minValue = Math.min(...allValues);
        const maxValue = Math.max(...allValues);
        const range = maxValue - minValue || 1;
        
        for (let series = 0; series < seriesCount; series++) {
            this.ctx.strokeStyle = this.options.colors[series % this.options.colors.length];
            this.ctx.lineWidth = this.options.lineWidth;
            this.ctx.beginPath();
            
            this.data.forEach((point, index) => {
                const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
                const y = this.options.padding + this.plotHeight - 
                         ((point.values[series] - minValue) / range) * this.plotHeight;
                
                if (index === 0) {
                    this.ctx.moveTo(x, y);
                } else {
                    this.ctx.lineTo(x, y);
                }
            });
            
            this.ctx.stroke();
            
            // Draw points
            this.ctx.fillStyle = this.options.colors[series % this.options.colors.length];
            this.data.forEach((point, index) => {
                const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
                const y = this.options.padding + this.plotHeight - 
                         ((point.values[series] - minValue) / range) * this.plotHeight;
                
                this.ctx.beginPath();
                this.ctx.arc(x, y, this.options.pointRadius, 0, 2 * Math.PI);
                this.ctx.fill();
            });
        }
    }
    
    drawAreaChart() {
        const values = this.data.map(d => d.value);
        const minValue = Math.min(...values);
        const maxValue = Math.max(...values);
        const range = maxValue - minValue || 1;
        
        // Create gradient
        const gradient = this.ctx.createLinearGradient(0, this.options.padding, 0, this.options.padding + this.plotHeight);
        gradient.addColorStop(0, this.options.colors[0] + '80'); // 50% opacity
        gradient.addColorStop(1, this.options.colors[0] + '20'); // 12.5% opacity
        
        this.ctx.fillStyle = gradient;
        this.ctx.beginPath();
        
        // Start from bottom left
        this.ctx.moveTo(this.options.padding, this.options.padding + this.plotHeight);
        
        // Draw the area
        this.data.forEach((point, index) => {
            const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
            const y = this.options.padding + this.plotHeight - 
                     ((point.value - minValue) / range) * this.plotHeight;
            this.ctx.lineTo(x, y);
        });
        
        // Close the area at bottom right
        this.ctx.lineTo(this.options.padding + this.plotWidth, this.options.padding + this.plotHeight);
        this.ctx.closePath();
        this.ctx.fill();
        
        // Draw the line on top
        this.ctx.strokeStyle = this.options.colors[0];
        this.ctx.lineWidth = this.options.lineWidth;
        this.ctx.beginPath();
        
        this.data.forEach((point, index) => {
            const x = this.options.padding + (this.plotWidth / (this.data.length - 1)) * index;
            const y = this.options.padding + this.plotHeight - 
                     ((point.value - minValue) / range) * this.plotHeight;
            
            if (index === 0) {
                this.ctx.moveTo(x, y);
            } else {
                this.ctx.lineTo(x, y);
            }
        });
        
        this.ctx.stroke();
    }
    
    drawBarChart() {
        const values = this.data.map(d => d.value);
        const minValue = Math.min(...values);
        const maxValue = Math.max(...values);
        const range = maxValue - minValue || 1;
        const barWidth = this.plotWidth / this.data.length * 0.8;
        const barSpacing = this.plotWidth / this.data.length * 0.2;
        
        this.ctx.fillStyle = this.options.colors[0];
        
        this.data.forEach((point, index) => {
            const x = this.options.padding + (this.plotWidth / this.data.length) * index + barSpacing / 2;
            const height = ((point.value - minValue) / range) * this.plotHeight;
            const y = this.options.padding + this.plotHeight - height;
            
            this.ctx.fillRect(x, y, barWidth, height);
        });
    }
    
    drawLabels() {
        this.ctx.fillStyle = this.options.textColor;
        this.ctx.font = '12px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif';
        this.ctx.textAlign = 'center';
        
        // Y-axis labels (values)
        const isMultiSeries = this.data.length > 0 && Array.isArray(this.data[0].values);
        const allValues = isMultiSeries ? 
            this.data.flatMap(d => d.values) : 
            this.data.map(d => d.value);
        
        if (allValues.length > 0) {
            const minValue = Math.min(...allValues);
            const maxValue = Math.max(...allValues);
            
            // Draw min and max value labels
            this.ctx.textAlign = 'right';
            this.ctx.fillText(maxValue.toFixed(1), this.options.padding - 5, this.options.padding + 5);
            this.ctx.fillText(minValue.toFixed(1), this.options.padding - 5, this.options.padding + this.plotHeight);
        }
        
        // X-axis labels (time)
        if (this.data.length > 1) {
            this.ctx.textAlign = 'center';
            const firstTime = new Date(this.data[0].timestamp).toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'});
            const lastTime = new Date(this.data[this.data.length - 1].timestamp).toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'});
            
            this.ctx.fillText(firstTime, this.options.padding, this.height - 5);
            this.ctx.fillText(lastTime, this.options.padding + this.plotWidth, this.height - 5);
        }
    }
    
    drawNoData() {
        this.ctx.fillStyle = this.options.textColor;
        this.ctx.font = '14px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.fillText('No data available', this.width / 2, this.height / 2);
    }
    
    animateToNewData() {
        // Simple animation by re-rendering
        // In a more complex implementation, this would interpolate between old and new values
        this.render();
    }
    
    updateTheme(isDark) {
        if (isDark) {
            this.options.gridColor = '#4a4a4a';
            this.options.textColor = '#b0b0b0';
        } else {
            this.options.gridColor = '#dee2e6';
            this.options.textColor = '#6c757d';
        }
        this.render();
    }
}

// Chart manager for the dashboard
class DashboardCharts {
    constructor() {
        this.charts = {};
        this.activeChart = 'battery';
        this.dataBuffers = {
            battery: [],
            detections: [],
            memory: []
        };
        
        this.init();
    }
    
    init() {
        // Initialize the main metrics chart
        this.charts.metrics = new SimpleChart('metrics-chart', {
            type: 'line',
            colors: ['#3498db', '#27ae60', '#f39c12'],
            maxDataPoints: 50,
            animate: true
        });
        
        // Listen for chart type changes
        document.addEventListener('chartTypeChange', (e) => {
            this.switchChart(e.detail.type);
        });
        
        // Listen for theme changes
        document.addEventListener('themeChange', (e) => {
            this.updateTheme(e.detail.isDark);
        });
        
        // Start with battery chart
        this.switchChart('battery');
    }
    
    switchChart(type) {
        this.activeChart = type;
        
        switch (type) {
            case 'battery':
                this.charts.metrics.setData(this.dataBuffers.battery);
                this.charts.metrics.options.colors = ['#27ae60'];
                break;
            case 'detections':
                this.charts.metrics.setData(this.dataBuffers.detections);
                this.charts.metrics.options.colors = ['#3498db'];
                break;
            case 'memory':
                this.charts.metrics.setData(this.dataBuffers.memory);
                this.charts.metrics.options.colors = ['#f39c12', '#e74c3c'];
                break;
        }
        
        this.charts.metrics.render();
    }
    
    addBatteryData(percentage, timestamp = Date.now()) {
        const data = { value: percentage, timestamp };
        this.dataBuffers.battery.push(data);
        
        if (this.dataBuffers.battery.length > 50) {
            this.dataBuffers.battery.shift();
        }
        
        if (this.activeChart === 'battery') {
            this.charts.metrics.addData(percentage);
        }
    }
    
    addDetectionData(count, timestamp = Date.now()) {
        const data = { value: count, timestamp };
        this.dataBuffers.detections.push(data);
        
        if (this.dataBuffers.detections.length > 50) {
            this.dataBuffers.detections.shift();
        }
        
        if (this.activeChart === 'detections') {
            this.charts.metrics.addData(count);
        }
    }
    
    addMemoryData(free, total, timestamp = Date.now()) {
        const freeKB = free / 1024;
        const totalKB = total / 1024;
        const data = { values: [freeKB, totalKB], timestamp };
        this.dataBuffers.memory.push(data);
        
        if (this.dataBuffers.memory.length > 50) {
            this.dataBuffers.memory.shift();
        }
        
        if (this.activeChart === 'memory') {
            this.charts.metrics.addMultiData([freeKB, totalKB]);
        }
    }
    
    updateFromSystemStatus(data) {
        if (data.battery && data.battery.percentage !== undefined) {
            this.addBatteryData(data.battery.percentage);
        }
        
        if (data.captures && data.captures.daily !== undefined) {
            this.addDetectionData(data.captures.daily);
        }
        
        if (data.memory && data.memory.free !== undefined && data.memory.total !== undefined) {
            this.addMemoryData(data.memory.free, data.memory.total);
        }
    }
    
    updateTheme(isDark) {
        Object.values(this.charts).forEach(chart => {
            chart.updateTheme(isDark);
        });
    }
    
    destroy() {
        // Clean up charts if needed
        this.charts = {};
        this.dataBuffers = { battery: [], detections: [], memory: [] };
    }
}

// Initialize charts when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.dashboardCharts = new DashboardCharts();
    
    // Integrate with existing dashboard
    if (window.dashboard) {
        // Listen for system updates to feed chart data
        window.addEventListener('systemStatusUpdate', (e) => {
            window.dashboardCharts.updateFromSystemStatus(e.detail.data);
        });
        
        // Handle chart tab clicks
        document.querySelectorAll('.chart-tab').forEach(tab => {
            tab.addEventListener('click', (e) => {
                const chartType = e.target.dataset.chart;
                window.dashboardCharts.switchChart(chartType);
                
                // Update active tab
                document.querySelectorAll('.chart-tab').forEach(t => t.classList.remove('active'));
                e.target.classList.add('active');
            });
        });
    }
});

// Export for use in other modules
if (typeof window !== 'undefined') {
    window.SimpleChart = SimpleChart;
    window.DashboardCharts = DashboardCharts;
}