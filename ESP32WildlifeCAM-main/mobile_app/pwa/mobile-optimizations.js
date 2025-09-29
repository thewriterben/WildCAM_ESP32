/**
 * Mobile-specific optimizations for Wildlife Camera PWA
 * Enhanced touch controls, gestures, and mobile UI improvements
 */

class MobileOptimizations {
    constructor() {
        this.isMobile = this.detectMobile();
        this.isTouch = 'ontouchstart' in window;
        this.viewport = this.getViewport();
        this.orientation = this.getOrientation();
        
        this.init();
    }
    
    init() {
        if (this.isMobile) {
            this.setupMobileUI();
            this.setupTouchGestures();
            this.setupInstallPrompt();
            this.setupMobileOptimizations();
            this.registerServiceWorker();
        }
        
        this.setupResponsiveImages();
        this.setupViewportHandling();
    }
    
    detectMobile() {
        return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ||
               window.innerWidth <= 768;
    }
    
    getViewport() {
        return {
            width: window.innerWidth,
            height: window.innerHeight,
            ratio: window.devicePixelRatio || 1
        };
    }
    
    getOrientation() {
        return window.innerHeight > window.innerWidth ? 'portrait' : 'landscape';
    }
    
    setupMobileUI() {
        document.body.classList.add('mobile-optimized');
        
        // Add mobile-specific CSS classes
        if (this.isTouch) {
            document.body.classList.add('touch-enabled');
        }
        
        // Optimize button sizes for touch
        this.optimizeTouchTargets();
        
        // Setup mobile navigation
        this.setupMobileNavigation();
        
        // Optimize form inputs
        this.optimizeMobileInputs();
    }
    
    optimizeTouchTargets() {
        const minTouchSize = 44; // iOS HIG minimum
        
        // Increase button sizes
        const buttons = document.querySelectorAll('button, .btn, .touch-target');
        buttons.forEach(button => {
            const rect = button.getBoundingClientRect();
            if (rect.width < minTouchSize || rect.height < minTouchSize) {
                button.style.minWidth = `${minTouchSize}px`;
                button.style.minHeight = `${minTouchSize}px`;
                button.style.padding = '8px 12px';
            }
            
            // Add touch feedback
            button.addEventListener('touchstart', this.addTouchFeedback.bind(this));
            button.addEventListener('touchend', this.removeTouchFeedback.bind(this));
        });
        
        // Optimize card spacing for finger navigation
        const cards = document.querySelectorAll('.card, .panel');
        cards.forEach(card => {
            card.style.marginBottom = '12px';
            card.style.padding = '16px';
        });
    }
    
    setupMobileNavigation() {
        // Create mobile hamburger menu if it doesn't exist
        if (!document.querySelector('.mobile-menu-toggle')) {
            this.createMobileMenu();
        }
        
        // Setup swipe navigation between sections
        this.setupSwipeNavigation();
    }
    
    createMobileMenu() {
        const header = document.querySelector('header') || document.querySelector('.dashboard-header');
        if (!header) return;
        
        const menuToggle = document.createElement('button');
        menuToggle.className = 'mobile-menu-toggle';
        menuToggle.innerHTML = '☰';
        menuToggle.setAttribute('aria-label', 'Toggle menu');
        
        const nav = document.querySelector('nav') || document.querySelector('.dashboard-nav');
        if (nav) {
            nav.classList.add('mobile-nav');
            
            menuToggle.addEventListener('click', () => {
                nav.classList.toggle('nav-open');
                menuToggle.classList.toggle('menu-open');
                menuToggle.innerHTML = nav.classList.contains('nav-open') ? '✕' : '☰';
            });
            
            header.insertBefore(menuToggle, header.firstChild);
        }
    }
    
    setupSwipeNavigation() {
        let startX, startY, currentX, currentY;
        let isSwipeNavigation = false;
        
        document.addEventListener('touchstart', (e) => {
            if (e.touches.length === 1) {
                startX = e.touches[0].clientX;
                startY = e.touches[0].clientY;
                isSwipeNavigation = true;
            }
        }, { passive: true });
        
        document.addEventListener('touchmove', (e) => {
            if (!isSwipeNavigation || e.touches.length !== 1) return;
            
            currentX = e.touches[0].clientX;
            currentY = e.touches[0].clientY;
            
            const deltaX = currentX - startX;
            const deltaY = currentY - startY;
            
            // If vertical swipe is dominant, cancel horizontal navigation
            if (Math.abs(deltaY) > Math.abs(deltaX) * 2) {
                isSwipeNavigation = false;
            }
        }, { passive: true });
        
        document.addEventListener('touchend', (e) => {
            if (!isSwipeNavigation) return;
            
            const deltaX = currentX - startX;
            const threshold = 100;
            
            if (Math.abs(deltaX) > threshold) {
                if (deltaX > 0) {
                    this.navigateToNextSection('prev');
                } else {
                    this.navigateToNextSection('next');
                }
            }
            
            isSwipeNavigation = false;
        }, { passive: true });
    }
    
    navigateToNextSection(direction) {
        const sections = ['dashboard', 'gallery', 'settings'];
        const currentSection = this.getCurrentSection();
        const currentIndex = sections.indexOf(currentSection);
        
        let nextIndex;
        if (direction === 'next') {
            nextIndex = (currentIndex + 1) % sections.length;
        } else {
            nextIndex = (currentIndex - 1 + sections.length) % sections.length;
        }
        
        this.showSection(sections[nextIndex]);
    }
    
    getCurrentSection() {
        const activeSection = document.querySelector('.section.active, .panel.active');
        return activeSection ? activeSection.id : 'dashboard';
    }
    
    showSection(sectionId) {
        // Hide all sections
        document.querySelectorAll('.section, .panel').forEach(section => {
            section.classList.remove('active');
        });
        
        // Show target section
        const targetSection = document.getElementById(sectionId);
        if (targetSection) {
            targetSection.classList.add('active');
            
            // Update navigation state
            this.updateNavigationState(sectionId);
        }
    }
    
    updateNavigationState(sectionId) {
        document.querySelectorAll('.nav-item').forEach(item => {
            item.classList.remove('active');
        });
        
        const navItem = document.querySelector(`[data-section="${sectionId}"]`);
        if (navItem) {
            navItem.classList.add('active');
        }
    }
    
    setupTouchGestures() {
        // Pull-to-refresh gesture
        this.setupPullToRefresh();
        
        // Long press for additional options
        this.setupLongPress();
        
        // Pinch-to-zoom for images
        this.setupPinchZoom();
    }
    
    setupPullToRefresh() {
        let startY = 0;
        let pullDistance = 0;
        let isPulling = false;
        const threshold = 80;
        
        const pullIndicator = this.createPullIndicator();
        
        document.addEventListener('touchstart', (e) => {
            if (window.scrollY === 0 && e.touches.length === 1) {
                startY = e.touches[0].clientY;
                isPulling = true;
            }
        }, { passive: true });
        
        document.addEventListener('touchmove', (e) => {
            if (!isPulling) return;
            
            const currentY = e.touches[0].clientY;
            pullDistance = Math.max(0, currentY - startY);
            
            if (pullDistance > 0) {
                e.preventDefault();
                this.updatePullIndicator(pullIndicator, pullDistance, threshold);
            }
        });
        
        document.addEventListener('touchend', () => {
            if (isPulling && pullDistance > threshold) {
                this.triggerRefresh();
            }
            
            this.resetPullIndicator(pullIndicator);
            isPulling = false;
            pullDistance = 0;
        });
    }
    
    createPullIndicator() {
        const indicator = document.createElement('div');
        indicator.className = 'pull-refresh-indicator';
        indicator.innerHTML = `
            <div class="pull-refresh-spinner"></div>
            <span class="pull-refresh-text">Pull to refresh</span>
        `;
        document.body.insertBefore(indicator, document.body.firstChild);
        return indicator;
    }
    
    updatePullIndicator(indicator, distance, threshold) {
        const progress = Math.min(distance / threshold, 1);
        indicator.style.transform = `translateY(${distance}px)`;
        indicator.style.opacity = progress;
        
        const text = indicator.querySelector('.pull-refresh-text');
        if (distance > threshold) {
            text.textContent = 'Release to refresh';
            indicator.classList.add('ready');
        } else {
            text.textContent = 'Pull to refresh';
            indicator.classList.remove('ready');
        }
    }
    
    resetPullIndicator(indicator) {
        indicator.style.transform = 'translateY(-100%)';
        indicator.style.opacity = '0';
        indicator.classList.remove('ready');
    }
    
    triggerRefresh() {
        console.log('Mobile: Triggering pull-to-refresh');
        
        // Show loading state
        const indicator = document.querySelector('.pull-refresh-indicator');
        indicator.querySelector('.pull-refresh-text').textContent = 'Refreshing...';
        indicator.classList.add('refreshing');
        
        // Trigger data refresh
        if (window.dashboard && window.dashboard.loadInitialData) {
            window.dashboard.loadInitialData();
        }
        
        // Reset after delay
        setTimeout(() => {
            this.resetPullIndicator(indicator);
            indicator.classList.remove('refreshing');
        }, 2000);
    }
    
    setupLongPress() {
        let pressTimer;
        const longPressDuration = 500;
        
        document.addEventListener('touchstart', (e) => {
            pressTimer = setTimeout(() => {
                this.handleLongPress(e);
            }, longPressDuration);
        });
        
        document.addEventListener('touchend', () => {
            clearTimeout(pressTimer);
        });
        
        document.addEventListener('touchmove', () => {
            clearTimeout(pressTimer);
        });
    }
    
    handleLongPress(e) {
        const target = e.target.closest('.card, .image-item, .capture-btn');
        if (target) {
            this.showContextMenu(target, e.touches[0]);
        }
    }
    
    showContextMenu(element, touch) {
        const menu = document.createElement('div');
        menu.className = 'context-menu mobile-context-menu';
        
        const options = this.getContextOptions(element);
        menu.innerHTML = options.map(option => 
            `<button class="context-option" data-action="${option.action}">
                ${option.icon} ${option.label}
            </button>`
        ).join('');
        
        menu.style.left = `${touch.clientX}px`;
        menu.style.top = `${touch.clientY}px`;
        
        document.body.appendChild(menu);
        
        // Handle option selection
        menu.addEventListener('click', (e) => {
            const action = e.target.dataset.action;
            if (action) {
                this.handleContextAction(element, action);
            }
            menu.remove();
        });
        
        // Remove menu on outside click
        setTimeout(() => {
            document.addEventListener('click', () => menu.remove(), { once: true });
        }, 100);
    }
    
    getContextOptions(element) {
        if (element.classList.contains('image-item')) {
            return [
                { action: 'view', label: 'View Full Size', icon: '🔍' },
                { action: 'download', label: 'Download', icon: '📥' },
                { action: 'share', label: 'Share', icon: '📤' },
                { action: 'delete', label: 'Delete', icon: '🗑️' }
            ];
        } else if (element.classList.contains('capture-btn')) {
            return [
                { action: 'single', label: 'Single Capture', icon: '📷' },
                { action: 'burst', label: 'Burst Mode', icon: '📸' },
                { action: 'timer', label: 'Timer Capture', icon: '⏱️' }
            ];
        }
        return [];
    }
    
    handleContextAction(element, action) {
        console.log('Mobile: Context action:', action, element);
        
        switch (action) {
            case 'view':
                this.viewImageFullscreen(element);
                break;
            case 'download':
                this.downloadImage(element);
                break;
            case 'share':
                this.shareImage(element);
                break;
            case 'single':
                this.capturePhoto('single');
                break;
            case 'burst':
                this.capturePhoto('burst');
                break;
            case 'timer':
                this.capturePhoto('timer');
                break;
        }
    }
    
    setupPinchZoom() {
        let initialDistance = 0;
        let currentScale = 1;
        
        document.addEventListener('touchstart', (e) => {
            if (e.touches.length === 2) {
                initialDistance = this.getDistance(e.touches[0], e.touches[1]);
            }
        });
        
        document.addEventListener('touchmove', (e) => {
            if (e.touches.length === 2) {
                e.preventDefault();
                
                const currentDistance = this.getDistance(e.touches[0], e.touches[1]);
                const scale = currentDistance / initialDistance;
                
                const target = e.target.closest('img, .zoomable');
                if (target) {
                    currentScale *= scale;
                    currentScale = Math.max(0.5, Math.min(3, currentScale));
                    target.style.transform = `scale(${currentScale})`;
                }
                
                initialDistance = currentDistance;
            }
        });
        
        document.addEventListener('touchend', (e) => {
            if (e.touches.length < 2) {
                const target = document.querySelector('[style*="scale"]');
                if (target && currentScale < 1.1) {
                    target.style.transform = '';
                    currentScale = 1;
                }
            }
        });
    }
    
    getDistance(touch1, touch2) {
        const dx = touch1.clientX - touch2.clientX;
        const dy = touch1.clientY - touch2.clientY;
        return Math.sqrt(dx * dx + dy * dy);
    }
    
    setupInstallPrompt() {
        let deferredPrompt;
        
        window.addEventListener('beforeinstallprompt', (e) => {
            e.preventDefault();
            deferredPrompt = e;
            this.showInstallPrompt();
        });
        
        window.addEventListener('appinstalled', () => {
            console.log('Mobile: PWA installed successfully');
            this.hideInstallPrompt();
        });
    }
    
    showInstallPrompt() {
        if (localStorage.getItem('installPromptDismissed')) return;
        
        const prompt = document.createElement('div');
        prompt.className = 'install-prompt';
        prompt.innerHTML = `
            <div class="install-prompt-content">
                <div class="install-prompt-icon">📱</div>
                <h3>Install Wildlife Camera</h3>
                <p>Add to your home screen for quick access and offline features</p>
                <div class="install-prompt-actions">
                    <button class="btn btn-primary" id="install-btn">Install</button>
                    <button class="btn btn-secondary" id="dismiss-btn">Not Now</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(prompt);
        
        prompt.querySelector('#install-btn').addEventListener('click', () => {
            if (deferredPrompt) {
                deferredPrompt.prompt();
                deferredPrompt.userChoice.then((result) => {
                    if (result.outcome === 'accepted') {
                        console.log('Mobile: PWA installation accepted');
                    }
                    deferredPrompt = null;
                });
            }
            prompt.remove();
        });
        
        prompt.querySelector('#dismiss-btn').addEventListener('click', () => {
            localStorage.setItem('installPromptDismissed', 'true');
            prompt.remove();
        });
    }
    
    hideInstallPrompt() {
        const prompt = document.querySelector('.install-prompt');
        if (prompt) {
            prompt.remove();
        }
    }
    
    setupMobileOptimizations() {
        // Prevent zooming on input focus (iOS)
        this.preventInputZoom();
        
        // Optimize scrolling performance
        this.optimizeScrolling();
        
        // Handle orientation changes
        this.handleOrientationChange();
        
        // Optimize image loading
        this.optimizeImageLoading();
    }
    
    preventInputZoom() {
        const inputs = document.querySelectorAll('input[type="text"], input[type="email"], input[type="password"], input[type="number"], textarea');
        inputs.forEach(input => {
            if (parseFloat(input.style.fontSize) < 16) {
                input.style.fontSize = '16px';
            }
        });
    }
    
    optimizeScrolling() {
        // Add momentum scrolling for iOS
        document.body.style.webkitOverflowScrolling = 'touch';
        
        // Optimize scroll event handling
        let scrollTimer;
        window.addEventListener('scroll', () => {
            if (scrollTimer) {
                clearTimeout(scrollTimer);
            }
            
            document.body.classList.add('scrolling');
            
            scrollTimer = setTimeout(() => {
                document.body.classList.remove('scrolling');
            }, 150);
        }, { passive: true });
    }
    
    handleOrientationChange() {
        window.addEventListener('orientationchange', () => {
            setTimeout(() => {
                this.viewport = this.getViewport();
                this.orientation = this.getOrientation();
                
                // Trigger layout updates
                this.updateLayoutForOrientation();
                
                // Fix viewport height issues on mobile
                this.fixViewportHeight();
            }, 100);
        });
    }
    
    updateLayoutForOrientation() {
        const isLandscape = this.orientation === 'landscape';
        document.body.classList.toggle('landscape', isLandscape);
        document.body.classList.toggle('portrait', !isLandscape);
        
        // Adjust grid layouts
        const grids = document.querySelectorAll('.image-grid, .dashboard-grid');
        grids.forEach(grid => {
            if (isLandscape) {
                grid.style.gridTemplateColumns = 'repeat(auto-fit, minmax(200px, 1fr))';
            } else {
                grid.style.gridTemplateColumns = 'repeat(auto-fit, minmax(150px, 1fr))';
            }
        });
    }
    
    fixViewportHeight() {
        // Fix 100vh issues on mobile browsers
        const vh = window.innerHeight * 0.01;
        document.documentElement.style.setProperty('--vh', `${vh}px`);
    }
    
    optimizeImageLoading() {
        // Implement lazy loading for images
        const images = document.querySelectorAll('img[data-src]');
        
        if ('IntersectionObserver' in window) {
            const imageObserver = new IntersectionObserver((entries) => {
                entries.forEach(entry => {
                    if (entry.isIntersecting) {
                        const img = entry.target;
                        img.src = img.dataset.src;
                        img.classList.remove('lazy');
                        imageObserver.unobserve(img);
                    }
                });
            });
            
            images.forEach(img => imageObserver.observe(img));
        } else {
            // Fallback for browsers without IntersectionObserver
            images.forEach(img => {
                img.src = img.dataset.src;
                img.classList.remove('lazy');
            });
        }
    }
    
    setupResponsiveImages() {
        // Setup responsive image loading based on device pixel ratio
        const images = document.querySelectorAll('img[data-src-1x][data-src-2x]');
        images.forEach(img => {
            const src = this.viewport.ratio > 1 ? img.dataset.src2x : img.dataset.src1x;
            img.dataset.src = src;
        });
    }
    
    setupViewportHandling() {
        // Handle viewport meta tag dynamically
        let viewportMeta = document.querySelector('meta[name="viewport"]');
        if (!viewportMeta) {
            viewportMeta = document.createElement('meta');
            viewportMeta.name = 'viewport';
            document.head.appendChild(viewportMeta);
        }
        
        const viewportContent = this.isMobile 
            ? 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'
            : 'width=device-width, initial-scale=1.0';
            
        viewportMeta.content = viewportContent;
        
        // Fix viewport height
        this.fixViewportHeight();
        window.addEventListener('resize', this.fixViewportHeight.bind(this));
    }
    
    registerServiceWorker() {
        if ('serviceWorker' in navigator) {
            navigator.serviceWorker.register('/service-worker.js')
                .then(registration => {
                    console.log('Mobile: Service Worker registered successfully');
                    
                    // Handle service worker updates
                    registration.addEventListener('updatefound', () => {
                        const newWorker = registration.installing;
                        newWorker.addEventListener('statechange', () => {
                            if (newWorker.state === 'installed' && navigator.serviceWorker.controller) {
                                this.showUpdateAvailable();
                            }
                        });
                    });
                })
                .catch(error => {
                    console.error('Mobile: Service Worker registration failed:', error);
                });
        }
    }
    
    showUpdateAvailable() {
        const notification = document.createElement('div');
        notification.className = 'update-notification';
        notification.innerHTML = `
            <div class="update-content">
                <span>New version available!</span>
                <button class="btn btn-sm btn-primary" onclick="location.reload()">Update</button>
            </div>
        `;
        document.body.appendChild(notification);
        
        setTimeout(() => {
            notification.classList.add('show');
        }, 100);
    }
    
    addTouchFeedback(e) {
        e.currentTarget.classList.add('touch-active');
    }
    
    removeTouchFeedback(e) {
        setTimeout(() => {
            e.currentTarget.classList.remove('touch-active');
        }, 150);
    }
    
    // Integration methods with existing dashboard
    viewImageFullscreen(element) {
        const imageUrl = element.dataset.fullsize || element.querySelector('img')?.src;
        if (imageUrl && window.dashboard && window.dashboard.showImageModal) {
            window.dashboard.showImageModal(imageUrl);
        }
    }
    
    downloadImage(element) {
        const imageUrl = element.dataset.fullsize || element.querySelector('img')?.src;
        if (imageUrl) {
            const a = document.createElement('a');
            a.href = imageUrl;
            a.download = `wildlife-${Date.now()}.jpg`;
            a.click();
        }
    }
    
    async shareImage(element) {
        if (navigator.share) {
            const imageUrl = element.dataset.fullsize || element.querySelector('img')?.src;
            try {
                await navigator.share({
                    title: 'Wildlife Camera Capture',
                    text: 'Check out this wildlife photo!',
                    url: imageUrl
                });
            } catch (error) {
                console.log('Mobile: Share cancelled or failed:', error);
            }
        }
    }
    
    capturePhoto(mode) {
        if (window.dashboard && window.dashboard.capturePhoto) {
            window.dashboard.capturePhoto(mode);
        }
    }
}

// Initialize mobile optimizations
document.addEventListener('DOMContentLoaded', () => {
    window.mobileOptimizations = new MobileOptimizations();
});

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = MobileOptimizations;
}