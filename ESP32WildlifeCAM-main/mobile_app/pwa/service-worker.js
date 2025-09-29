/**
 * Service Worker for Wildlife Camera PWA
 * Enables offline functionality and background sync
 */

const CACHE_NAME = 'wildlife-cam-v1.0.0';
const STATIC_CACHE_NAME = 'wildlife-cam-static-v1.0.0';
const DYNAMIC_CACHE_NAME = 'wildlife-cam-dynamic-v1.0.0';

// Static assets to cache on install
const STATIC_ASSETS = [
    '/',
    '/assets/dashboard.css',
    '/assets/dashboard.js',
    '/assets/websocket.js',
    '/assets/charts.js',
    '/manifest.json',
    '/assets/icon-192.png',
    '/assets/icon-512.png'
];

// API endpoints that should be cached
const CACHEABLE_APIS = [
    '/api/status',
    '/api/config',
    '/api/power',
    '/api/storage',
    '/api/images'
];

// Install event - cache static assets
self.addEventListener('install', event => {
    console.log('Service Worker: Installing...');
    
    event.waitUntil(
        caches.open(STATIC_CACHE_NAME)
            .then(cache => {
                console.log('Service Worker: Caching static assets');
                return cache.addAll(STATIC_ASSETS);
            })
            .then(() => {
                console.log('Service Worker: Static assets cached');
                return self.skipWaiting();
            })
            .catch(error => {
                console.error('Service Worker: Failed to cache static assets:', error);
            })
    );
});

// Activate event - clean up old caches
self.addEventListener('activate', event => {
    console.log('Service Worker: Activating...');
    
    event.waitUntil(
        caches.keys()
            .then(cacheNames => {
                return Promise.all(
                    cacheNames.map(cacheName => {
                        // Delete old cache versions
                        if (cacheName !== STATIC_CACHE_NAME && 
                            cacheName !== DYNAMIC_CACHE_NAME &&
                            cacheName.startsWith('wildlife-cam-')) {
                            console.log('Service Worker: Deleting old cache:', cacheName);
                            return caches.delete(cacheName);
                        }
                    })
                );
            })
            .then(() => {
                console.log('Service Worker: Activated');
                return self.clients.claim();
            })
    );
});

// Fetch event - serve from cache when offline
self.addEventListener('fetch', event => {
    const { request } = event;
    const url = new URL(request.url);
    
    // Skip cross-origin requests
    if (url.origin !== location.origin) {
        return;
    }
    
    // Handle different types of requests
    if (request.method === 'GET') {
        if (isStaticAsset(request.url)) {
            // Static assets - cache first strategy
            event.respondWith(handleStaticAsset(request));
        } else if (isAPIRequest(request.url)) {
            // API requests - network first with cache fallback
            event.respondWith(handleAPIRequest(request));
        } else if (isImageRequest(request.url)) {
            // Images - cache first strategy
            event.respondWith(handleImageRequest(request));
        } else {
            // Other requests - network first
            event.respondWith(handleNetworkFirst(request));
        }
    }
});

// Background sync for offline actions
self.addEventListener('sync', event => {
    console.log('Service Worker: Background sync triggered');
    
    if (event.tag === 'capture-photo') {
        event.waitUntil(handleOfflineCapture());
    } else if (event.tag === 'upload-settings') {
        event.waitUntil(handleOfflineSettings());
    }
});

// Push notification handling
self.addEventListener('push', event => {
    console.log('Service Worker: Push notification received');
    
    const options = {
        body: 'Wildlife detected!',
        icon: '/assets/icon-192.png',
        badge: '/assets/badge-72.png',
        tag: 'wildlife-detection',
        requireInteraction: true,
        actions: [
            {
                action: 'view',
                title: 'View Image',
                icon: '/assets/view-icon.png'
            },
            {
                action: 'dismiss',
                title: 'Dismiss',
                icon: '/assets/dismiss-icon.png'
            }
        ]
    };
    
    if (event.data) {
        const data = event.data.json();
        options.body = data.message || options.body;
        options.data = data;
    }
    
    event.waitUntil(
        self.registration.showNotification('Wildlife Camera Alert', options)
    );
});

// Notification click handling
self.addEventListener('notificationclick', event => {
    console.log('Service Worker: Notification clicked');
    
    event.notification.close();
    
    if (event.action === 'view') {
        // Open the app to view the detected wildlife
        event.waitUntil(
            clients.openWindow('/?action=gallery&latest=true')
        );
    } else if (event.action === 'dismiss') {
        // Just close the notification
        return;
    } else {
        // Default action - open the main dashboard
        event.waitUntil(
            clients.openWindow('/')
        );
    }
});

// Helper functions
function isStaticAsset(url) {
    return STATIC_ASSETS.some(asset => url.endsWith(asset)) ||
           url.includes('/assets/') ||
           url.endsWith('.css') ||
           url.endsWith('.js') ||
           url.endsWith('.png') ||
           url.endsWith('.jpg') ||
           url.endsWith('.svg');
}

function isAPIRequest(url) {
    return CACHEABLE_APIS.some(api => url.includes(api));
}

function isImageRequest(url) {
    return url.includes('/api/thumbnail') || 
           url.includes('/images/') ||
           (url.includes('.jpg') || url.includes('.png')) && !url.includes('/assets/');
}

async function handleStaticAsset(request) {
    try {
        const cachedResponse = await caches.match(request);
        if (cachedResponse) {
            return cachedResponse;
        }
        
        const networkResponse = await fetch(request);
        if (networkResponse && networkResponse.status === 200) {
            const cache = await caches.open(STATIC_CACHE_NAME);
            cache.put(request, networkResponse.clone());
        }
        return networkResponse;
    } catch (error) {
        console.error('Service Worker: Failed to handle static asset:', error);
        return new Response('Offline - Asset not available', { status: 503 });
    }
}

async function handleAPIRequest(request) {
    try {
        // Try network first
        const networkResponse = await fetch(request);
        if (networkResponse && networkResponse.status === 200) {
            // Cache successful responses
            const cache = await caches.open(DYNAMIC_CACHE_NAME);
            cache.put(request, networkResponse.clone());
            return networkResponse;
        }
        throw new Error('Network response not ok');
    } catch (error) {
        console.log('Service Worker: Network failed, trying cache for API request');
        const cachedResponse = await caches.match(request);
        if (cachedResponse) {
            return cachedResponse;
        }
        return new Response(JSON.stringify({
            error: 'Offline - API not available',
            offline: true
        }), {
            status: 503,
            headers: { 'Content-Type': 'application/json' }
        });
    }
}

async function handleImageRequest(request) {
    try {
        const cachedResponse = await caches.match(request);
        if (cachedResponse) {
            return cachedResponse;
        }
        
        const networkResponse = await fetch(request);
        if (networkResponse && networkResponse.status === 200) {
            const cache = await caches.open(DYNAMIC_CACHE_NAME);
            cache.put(request, networkResponse.clone());
        }
        return networkResponse;
    } catch (error) {
        console.error('Service Worker: Failed to handle image request:', error);
        return new Response('Offline - Image not available', { status: 503 });
    }
}

async function handleNetworkFirst(request) {
    try {
        return await fetch(request);
    } catch (error) {
        const cachedResponse = await caches.match(request);
        return cachedResponse || new Response('Offline', { status: 503 });
    }
}

async function handleOfflineCapture() {
    console.log('Service Worker: Handling offline capture sync');
    // Store capture request for when online
    // This would be implemented based on the specific requirements
}

async function handleOfflineSettings() {
    console.log('Service Worker: Handling offline settings sync');
    // Store settings changes for when online
    // This would be implemented based on the specific requirements
}