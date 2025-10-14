/**
 * Service Worker for Progressive Web App (PWA) support
 * Enables offline functionality and background sync for WildCAM
 */

const CACHE_NAME = 'wildcam-v1';
const urlsToCache = [
  '/',
  '/static/css/main.css',
  '/static/js/main.js',
  '/manifest.json',
];

// Install event - cache essential resources
self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME)
      .then((cache) => {
        console.log('Opened cache');
        return cache.addAll(urlsToCache);
      })
  );
});

// Fetch event - serve from cache, fallback to network
self.addEventListener('fetch', (event) => {
  event.respondWith(
    caches.match(event.request)
      .then((response) => {
        // Cache hit - return response
        if (response) {
          return response;
        }

        return fetch(event.request).then(
          (response) => {
            // Check if valid response
            if (!response || response.status !== 200 || response.type !== 'basic') {
              return response;
            }

            // Clone the response
            const responseToCache = response.clone();

            caches.open(CACHE_NAME)
              .then((cache) => {
                cache.put(event.request, responseToCache);
              });

            return response;
          }
        );
      })
  );
});

// Activate event - clean up old caches
self.addEventListener('activate', (event) => {
  const cacheWhitelist = [CACHE_NAME];

  event.waitUntil(
    caches.keys().then((cacheNames) => {
      return Promise.all(
        cacheNames.map((cacheName) => {
          if (cacheWhitelist.indexOf(cacheName) === -1) {
            return caches.delete(cacheName);
          }
        })
      );
    })
  );
});

// Background sync for offline data
self.addEventListener('sync', (event) => {
  if (event.tag === 'sync-annotations') {
    event.waitUntil(syncAnnotations());
  }
  if (event.tag === 'sync-field-notes') {
    event.waitUntil(syncFieldNotes());
  }
});

async function syncAnnotations() {
  // Retrieve queued annotations from IndexedDB and sync to server
  console.log('Syncing annotations...');
  // Implementation would use IndexedDB to retrieve queued items
}

async function syncFieldNotes() {
  // Retrieve queued field notes from IndexedDB and sync to server
  console.log('Syncing field notes...');
  // Implementation would use IndexedDB to retrieve queued items
}

// Push notifications support
self.addEventListener('push', (event) => {
  const options = {
    body: event.data ? event.data.text() : 'New wildlife detection!',
    icon: '/logo192.png',
    badge: '/badge.png',
    vibrate: [100, 50, 100],
    data: {
      dateOfArrival: Date.now(),
      primaryKey: 1
    },
    actions: [
      {
        action: 'explore',
        title: 'View Detection',
        icon: '/check.png'
      },
      {
        action: 'close',
        title: 'Dismiss',
        icon: '/cross.png'
      }
    ]
  };

  event.waitUntil(
    self.registration.showNotification('WildCAM Alert', options)
  );
});

// Notification click handler
self.addEventListener('notificationclick', (event) => {
  event.notification.close();

  if (event.action === 'explore') {
    event.waitUntil(
      clients.openWindow('/detections')
    );
  }
});
