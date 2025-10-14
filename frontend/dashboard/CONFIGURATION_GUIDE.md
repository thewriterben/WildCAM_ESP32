# ⚙️ WildCAM Dashboard - Advanced Configuration Guide

**Complete guide for customizing and configuring the advanced user dashboard**

## 📋 Table of Contents

- [Environment Configuration](#-environment-configuration)
- [Theme Customization](#-theme-customization)
- [API Configuration](#-api-configuration)
- [Feature Flags](#-feature-flags)
- [Performance Tuning](#-performance-tuning)
- [Advanced Settings](#-advanced-settings)
- [Multi-Environment Setup](#-multi-environment-setup)
- [Security Configuration](#-security-configuration)

## 🔧 Environment Configuration

### Basic Setup

Create `.env` file in the dashboard root:

```bash
# Copy example configuration
cp .env.example .env

# Edit with your values
nano .env
```

### Core Environment Variables

```bash
# ==================================
# API Configuration
# ==================================
REACT_APP_API_URL=http://localhost:5000
REACT_APP_WS_URL=ws://localhost:5000
REACT_APP_API_TIMEOUT=30000

# ==================================
# Authentication
# ==================================
REACT_APP_AUTH_ENABLED=true
REACT_APP_TOKEN_REFRESH_INTERVAL=300000
REACT_APP_SESSION_TIMEOUT=3600000

# ==================================
# Feature Flags
# ==================================
REACT_APP_ENABLE_COLLABORATION=true
REACT_APP_ENABLE_MAPS=true
REACT_APP_ENABLE_EXPORT=true
REACT_APP_ENABLE_ANALYTICS=true
REACT_APP_ENABLE_REAL_TIME=true

# ==================================
# UI Configuration
# ==================================
REACT_APP_THEME_MODE=dark
REACT_APP_DEFAULT_LANGUAGE=en
REACT_APP_ITEMS_PER_PAGE=20
REACT_APP_MAX_UPLOAD_SIZE=10485760

# ==================================
# Performance
# ==================================
REACT_APP_AUTO_REFRESH_INTERVAL=30000
REACT_APP_DEBOUNCE_DELAY=300
REACT_APP_CACHE_ENABLED=true
REACT_APP_CACHE_TTL=300000

# ==================================
# Map Configuration
# ==================================
REACT_APP_MAP_TILE_URL=https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png
REACT_APP_MAP_ATTRIBUTION=© OpenStreetMap contributors
REACT_APP_DEFAULT_LAT=40.7128
REACT_APP_DEFAULT_LNG=-74.0060
REACT_APP_DEFAULT_ZOOM=10

# ==================================
# Analytics
# ==================================
REACT_APP_ANALYTICS_ENABLED=false
REACT_APP_ANALYTICS_ID=
REACT_APP_ERROR_TRACKING=false
REACT_APP_SENTRY_DSN=

# ==================================
# Development
# ==================================
REACT_APP_DEBUG=false
REACT_APP_MOCK_DATA=false
```

### Environment-Specific Files

```bash
# Development
.env.development

# Testing
.env.test

# Production
.env.production

# Local overrides (not committed)
.env.local
```

## 🎨 Theme Customization

### Custom Theme Configuration

Create `src/theme/customTheme.js`:

```javascript
import { createTheme } from '@mui/material/styles';

const customTheme = createTheme({
  // ===========================
  // Color Palette
  // ===========================
  palette: {
    mode: 'dark', // or 'light'
    
    primary: {
      main: '#2e7d32',      // Main primary color
      light: '#60ad5e',     // Lighter variant
      dark: '#005005',      // Darker variant
      contrastText: '#fff', // Text on primary
    },
    
    secondary: {
      main: '#1976d2',
      light: '#4791db',
      dark: '#115293',
      contrastText: '#fff',
    },
    
    error: {
      main: '#f44336',
      light: '#e57373',
      dark: '#d32f2f',
    },
    
    warning: {
      main: '#ff9800',
      light: '#ffb74d',
      dark: '#f57c00',
    },
    
    info: {
      main: '#2196f3',
      light: '#64b5f6',
      dark: '#1976d2',
    },
    
    success: {
      main: '#4caf50',
      light: '#81c784',
      dark: '#388e3c',
    },
    
    background: {
      default: '#0a1929',
      paper: '#132f4c',
    },
    
    text: {
      primary: 'rgba(255, 255, 255, 0.87)',
      secondary: 'rgba(255, 255, 255, 0.6)',
      disabled: 'rgba(255, 255, 255, 0.38)',
    },
    
    divider: 'rgba(255, 255, 255, 0.12)',
  },
  
  // ===========================
  // Typography
  // ===========================
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    
    h1: {
      fontSize: '2.5rem',
      fontWeight: 600,
      lineHeight: 1.2,
    },
    
    h2: {
      fontSize: '2rem',
      fontWeight: 600,
      lineHeight: 1.3,
    },
    
    h3: {
      fontSize: '1.75rem',
      fontWeight: 600,
      lineHeight: 1.4,
    },
    
    h4: {
      fontSize: '1.5rem',
      fontWeight: 600,
      lineHeight: 1.4,
    },
    
    h5: {
      fontSize: '1.25rem',
      fontWeight: 600,
      lineHeight: 1.5,
    },
    
    h6: {
      fontSize: '1rem',
      fontWeight: 600,
      lineHeight: 1.5,
    },
    
    body1: {
      fontSize: '1rem',
      lineHeight: 1.5,
    },
    
    body2: {
      fontSize: '0.875rem',
      lineHeight: 1.5,
    },
    
    button: {
      textTransform: 'none', // Don't uppercase buttons
      fontWeight: 500,
    },
  },
  
  // ===========================
  // Spacing
  // ===========================
  spacing: 8, // Base unit (1 = 8px)
  
  // ===========================
  // Breakpoints
  // ===========================
  breakpoints: {
    values: {
      xs: 0,
      sm: 600,
      md: 900,
      lg: 1200,
      xl: 1536,
    },
  },
  
  // ===========================
  // Shape
  // ===========================
  shape: {
    borderRadius: 8, // Default border radius
  },
  
  // ===========================
  // Component Overrides
  // ===========================
  components: {
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8,
          padding: '8px 16px',
          textTransform: 'none',
        },
        contained: {
          boxShadow: 'none',
          '&:hover': {
            boxShadow: '0 2px 8px rgba(0,0,0,0.15)',
          },
        },
      },
      defaultProps: {
        disableElevation: true,
      },
    },
    
    MuiCard: {
      styleOverrides: {
        root: {
          borderRadius: 12,
          backgroundImage: 'none',
        },
      },
      defaultProps: {
        elevation: 2,
      },
    },
    
    MuiChip: {
      styleOverrides: {
        root: {
          borderRadius: 16,
          fontWeight: 500,
        },
      },
    },
    
    MuiTextField: {
      defaultProps: {
        variant: 'outlined',
      },
    },
    
    MuiDrawer: {
      styleOverrides: {
        paper: {
          backgroundImage: 'none',
        },
      },
    },
    
    MuiAppBar: {
      styleOverrides: {
        root: {
          backgroundImage: 'none',
        },
      },
      defaultProps: {
        elevation: 0,
      },
    },
  },
  
  // ===========================
  // Shadows
  // ===========================
  shadows: [
    'none',
    '0px 2px 4px rgba(0,0,0,0.1)',
    '0px 4px 8px rgba(0,0,0,0.12)',
    '0px 8px 16px rgba(0,0,0,0.14)',
    // ... customize other shadow levels
  ],
});

export default customTheme;
```

### Using Custom Theme

Update `src/App.js`:

```javascript
import { ThemeProvider } from '@mui/material/styles';
import customTheme from './theme/customTheme';

function App() {
  return (
    <ThemeProvider theme={customTheme}>
      {/* Your app */}
    </ThemeProvider>
  );
}
```

### Dynamic Theme Switching

```javascript
import { useState, useMemo } from 'react';
import { createTheme, ThemeProvider } from '@mui/material/styles';
import { lightTheme, darkTheme } from './theme';

function App() {
  const [mode, setMode] = useState('dark');
  
  const theme = useMemo(() => 
    createTheme(mode === 'dark' ? darkTheme : lightTheme),
    [mode]
  );
  
  const toggleTheme = () => {
    setMode(prevMode => prevMode === 'dark' ? 'light' : 'dark');
    localStorage.setItem('theme', mode === 'dark' ? 'light' : 'dark');
  };
  
  return (
    <ThemeProvider theme={theme}>
      {/* Pass toggleTheme to your components */}
    </ThemeProvider>
  );
}
```

## 🔌 API Configuration

### API Client Setup

Create `src/config/api.config.js`:

```javascript
export const apiConfig = {
  baseURL: process.env.REACT_APP_API_URL || 'http://localhost:5000',
  timeout: parseInt(process.env.REACT_APP_API_TIMEOUT) || 30000,
  
  headers: {
    'Content-Type': 'application/json',
    'Accept': 'application/json',
  },
  
  // Retry configuration
  retry: {
    retries: 3,
    retryDelay: 1000,
    retryCondition: (error) => {
      return error.response?.status >= 500 || !error.response;
    },
  },
  
  // Request interceptor
  onRequest: (config) => {
    const token = localStorage.getItem('access_token');
    if (token) {
      config.headers.Authorization = `Bearer ${token}`;
    }
    return config;
  },
  
  // Response interceptor
  onResponse: (response) => {
    return response;
  },
  
  // Error interceptor
  onError: (error) => {
    if (error.response?.status === 401) {
      // Handle unauthorized
      localStorage.removeItem('access_token');
      window.location.href = '/login';
    }
    return Promise.reject(error);
  },
};
```

### Axios Instance

```javascript
import axios from 'axios';
import { apiConfig } from './config/api.config';

const apiClient = axios.create({
  baseURL: apiConfig.baseURL,
  timeout: apiConfig.timeout,
  headers: apiConfig.headers,
});

// Request interceptor
apiClient.interceptors.request.use(
  apiConfig.onRequest,
  error => Promise.reject(error)
);

// Response interceptor
apiClient.interceptors.response.use(
  apiConfig.onResponse,
  apiConfig.onError
);

export default apiClient;
```

### WebSocket Configuration

Create `src/config/websocket.config.js`:

```javascript
export const wsConfig = {
  url: process.env.REACT_APP_WS_URL || 'ws://localhost:5000',
  
  options: {
    reconnection: true,
    reconnectionAttempts: 5,
    reconnectionDelay: 1000,
    reconnectionDelayMax: 5000,
    timeout: 20000,
    autoConnect: true,
    transports: ['websocket', 'polling'],
  },
  
  events: {
    // Events to subscribe to
    detections: 'new_detection',
    deviceUpdates: 'device_status_update',
    systemAlerts: 'system_alert',
    cameraStatus: 'camera_status',
  },
  
  // Authentication
  auth: {
    token: () => localStorage.getItem('access_token'),
  },
};
```

## 🚩 Feature Flags

### Feature Flag Configuration

Create `src/config/features.config.js`:

```javascript
const features = {
  // Core features
  authentication: {
    enabled: process.env.REACT_APP_AUTH_ENABLED === 'true',
    providers: ['local', 'oauth'],
  },
  
  // Real-time updates
  realTime: {
    enabled: process.env.REACT_APP_ENABLE_REAL_TIME !== 'false',
    autoConnect: true,
    reconnect: true,
  },
  
  // Collaboration
  collaboration: {
    enabled: process.env.REACT_APP_ENABLE_COLLABORATION === 'true',
    features: {
      annotations: true,
      comments: true,
      sharedViews: true,
      activityFeed: true,
    },
  },
  
  // Maps
  maps: {
    enabled: process.env.REACT_APP_ENABLE_MAPS === 'true',
    provider: 'openstreetmap', // 'openstreetmap', 'mapbox', 'google'
    clustering: true,
    heatmap: true,
  },
  
  // Data export
  export: {
    enabled: process.env.REACT_APP_ENABLE_EXPORT === 'true',
    formats: ['json', 'csv', 'pdf'],
    maxRecords: 10000,
  },
  
  // Analytics
  analytics: {
    enabled: process.env.REACT_APP_ENABLE_ANALYTICS === 'true',
    charts: {
      pie: true,
      bar: true,
      line: true,
      heatmap: true,
      area: true,
    },
  },
  
  // Advanced features
  advanced: {
    customDashboards: true,
    apiAccess: true,
    webhooks: false,
    scheduling: false,
  },
  
  // Experimental features
  experimental: {
    aiAnnotations: false,
    predictiveAnalytics: false,
    voiceCommands: false,
  },
};

// Feature flag check helper
export const isFeatureEnabled = (featurePath) => {
  const keys = featurePath.split('.');
  let current = features;
  
  for (const key of keys) {
    if (current[key] === undefined) return false;
    current = current[key];
  }
  
  return current === true || current?.enabled === true;
};

export default features;
```

### Using Feature Flags

```javascript
import { isFeatureEnabled } from './config/features.config';

function Dashboard() {
  return (
    <div>
      {isFeatureEnabled('collaboration') && (
        <CollaborationPanel />
      )}
      
      {isFeatureEnabled('maps.enabled') && (
        <MapView />
      )}
      
      {isFeatureEnabled('export.enabled') && (
        <ExportButton />
      )}
    </div>
  );
}
```

## ⚡ Performance Tuning

### Performance Configuration

Create `src/config/performance.config.js`:

```javascript
export const performanceConfig = {
  // Auto-refresh intervals (ms)
  autoRefresh: {
    dashboard: parseInt(process.env.REACT_APP_AUTO_REFRESH_INTERVAL) || 30000,
    detections: 15000,
    analytics: 60000,
    devices: 30000,
  },
  
  // Pagination
  pagination: {
    itemsPerPage: parseInt(process.env.REACT_APP_ITEMS_PER_PAGE) || 20,
    maxItemsPerPage: 100,
  },
  
  // Debounce delays (ms)
  debounce: {
    search: parseInt(process.env.REACT_APP_DEBOUNCE_DELAY) || 300,
    filter: 300,
    resize: 150,
  },
  
  // Cache settings
  cache: {
    enabled: process.env.REACT_APP_CACHE_ENABLED !== 'false',
    ttl: parseInt(process.env.REACT_APP_CACHE_TTL) || 300000, // 5 minutes
    maxSize: 50, // Max cached items
  },
  
  // Image optimization
  images: {
    lazyLoad: true,
    thumbnailSize: 200,
    maxSize: 1920,
    quality: 85,
    format: 'webp', // 'webp', 'jpeg', 'png'
  },
  
  // Chart optimization
  charts: {
    maxDataPoints: 100,
    animationDuration: 300,
    updateMode: 'lazy', // 'lazy', 'eager'
  },
  
  // WebSocket optimization
  websocket: {
    batchUpdates: true,
    batchDelay: 100,
    maxBatchSize: 10,
  },
};
```

### Implementing Performance Optimizations

#### Debounced Search

```javascript
import { useState, useCallback } from 'react';
import { debounce } from 'lodash';
import { performanceConfig } from './config/performance.config';

function SearchBox() {
  const [search, setSearch] = useState('');
  
  const debouncedSearch = useCallback(
    debounce((value) => {
      // Perform search
      console.log('Searching for:', value);
    }, performanceConfig.debounce.search),
    []
  );
  
  const handleChange = (e) => {
    const value = e.target.value;
    setSearch(value);
    debouncedSearch(value);
  };
  
  return (
    <TextField 
      value={search}
      onChange={handleChange}
      placeholder="Search..."
    />
  );
}
```

#### Lazy Loading Images

```javascript
import { LazyLoadImage } from 'react-lazy-load-image-component';

function DetectionImage({ src, alt }) {
  return (
    <LazyLoadImage
      src={src}
      alt={alt}
      effect="blur"
      threshold={100}
      placeholder={<Skeleton variant="rectangular" height={200} />}
    />
  );
}
```

#### Memoization

```javascript
import { useMemo } from 'react';

function ExpensiveComponent({ data }) {
  const processedData = useMemo(() => {
    // Expensive calculation
    return data.map(item => ({
      ...item,
      calculated: complexCalculation(item),
    }));
  }, [data]);
  
  return <ChartComponent data={processedData} />;
}
```

## 🔐 Security Configuration

### Security Settings

Create `src/config/security.config.js`:

```javascript
export const securityConfig = {
  // Authentication
  auth: {
    tokenKey: 'access_token',
    refreshTokenKey: 'refresh_token',
    tokenExpiry: 3600, // seconds
    refreshThreshold: 300, // refresh 5 min before expiry
  },
  
  // CORS
  cors: {
    allowedOrigins: [
      'http://localhost:3000',
      'https://dashboard.wildcam.example.com',
    ],
  },
  
  // Content Security Policy
  csp: {
    'default-src': ["'self'"],
    'script-src': ["'self'", "'unsafe-inline'"],
    'style-src': ["'self'", "'unsafe-inline'"],
    'img-src': ["'self'", 'data:', 'https:'],
    'connect-src': ["'self'", process.env.REACT_APP_API_URL],
  },
  
  // Rate limiting
  rateLimit: {
    enabled: true,
    maxRequests: 100,
    windowMs: 60000, // 1 minute
  },
  
  // Input validation
  validation: {
    maxFileSize: parseInt(process.env.REACT_APP_MAX_UPLOAD_SIZE) || 10485760, // 10MB
    allowedFileTypes: ['image/jpeg', 'image/png', 'image/webp'],
    maxTextLength: 5000,
  },
  
  // XSS protection
  xss: {
    sanitizeHtml: true,
    allowedTags: ['b', 'i', 'em', 'strong', 'a'],
    allowedAttributes: {
      'a': ['href', 'target'],
    },
  },
};
```

### Implementing Security Measures

#### Sanitize User Input

```javascript
import DOMPurify from 'dompurify';
import { securityConfig } from './config/security.config';

function sanitizeInput(input) {
  if (securityConfig.xss.sanitizeHtml) {
    return DOMPurify.sanitize(input, {
      ALLOWED_TAGS: securityConfig.xss.allowedTags,
      ALLOWED_ATTR: securityConfig.xss.allowedAttributes,
    });
  }
  return input;
}
```

#### Token Refresh

```javascript
import { securityConfig } from './config/security.config';

function setupTokenRefresh() {
  const interval = (securityConfig.auth.tokenExpiry - 
                   securityConfig.auth.refreshThreshold) * 1000;
  
  return setInterval(async () => {
    const refreshToken = localStorage.getItem(securityConfig.auth.refreshTokenKey);
    if (refreshToken) {
      try {
        const response = await apiService.refreshToken();
        localStorage.setItem(securityConfig.auth.tokenKey, response.access_token);
      } catch (error) {
        console.error('Token refresh failed:', error);
        // Redirect to login
      }
    }
  }, interval);
}
```

## 🌍 Multi-Environment Setup

### Environment Files

```bash
# .env.development
REACT_APP_API_URL=http://localhost:5000
REACT_APP_DEBUG=true
REACT_APP_MOCK_DATA=true

# .env.staging
REACT_APP_API_URL=https://staging-api.wildcam.example.com
REACT_APP_DEBUG=false
REACT_APP_ANALYTICS_ENABLED=true

# .env.production
REACT_APP_API_URL=https://api.wildcam.example.com
REACT_APP_DEBUG=false
REACT_APP_ANALYTICS_ENABLED=true
REACT_APP_ERROR_TRACKING=true
```

### Build Scripts

Update `package.json`:

```json
{
  "scripts": {
    "start": "react-scripts start",
    "start:staging": "env-cmd -f .env.staging react-scripts start",
    "build": "react-scripts build",
    "build:staging": "env-cmd -f .env.staging react-scripts build",
    "build:production": "env-cmd -f .env.production react-scripts build",
    "test": "react-scripts test",
    "deploy:staging": "npm run build:staging && scp -r build/* staging-server:/var/www",
    "deploy:production": "npm run build:production && scp -r build/* prod-server:/var/www"
  }
}
```

### Environment-Specific Code

```javascript
const isDevelopment = process.env.NODE_ENV === 'development';
const isProduction = process.env.NODE_ENV === 'production';
const isStaging = process.env.REACT_APP_ENV === 'staging';

if (isDevelopment) {
  // Development-only code
  console.log('Running in development mode');
}

if (isProduction) {
  // Production optimizations
  console.log = () => {}; // Disable console logs
}
```

## 📊 Monitoring Configuration

### Analytics Setup

```javascript
// src/config/analytics.config.js
export const analyticsConfig = {
  enabled: process.env.REACT_APP_ANALYTICS_ENABLED === 'true',
  
  // Google Analytics
  googleAnalytics: {
    trackingId: process.env.REACT_APP_GA_ID,
    options: {
      anonymizeIp: true,
    },
  },
  
  // Custom events
  events: {
    pageView: true,
    errors: true,
    performance: true,
    userActions: true,
  },
  
  // Performance monitoring
  performance: {
    enabled: true,
    sampleRate: 0.1, // 10% of users
    metrics: ['FCP', 'LCP', 'FID', 'CLS'],
  },
};
```

### Error Tracking

```javascript
// src/config/error-tracking.config.js
import * as Sentry from '@sentry/react';

export const setupErrorTracking = () => {
  if (process.env.REACT_APP_ERROR_TRACKING === 'true') {
    Sentry.init({
      dsn: process.env.REACT_APP_SENTRY_DSN,
      environment: process.env.NODE_ENV,
      tracesSampleRate: 1.0,
      
      beforeSend(event, hint) {
        // Filter sensitive data
        if (event.request) {
          delete event.request.cookies;
        }
        return event;
      },
    });
  }
};
```

---

## 📚 Additional Resources

- [Environment Variables in React](https://create-react-app.dev/docs/adding-custom-environment-variables/)
- [Material-UI Theming](https://mui.com/material-ui/customization/theming/)
- [Axios Configuration](https://axios-http.com/docs/config_defaults)
- [React Performance Optimization](https://reactjs.org/docs/optimizing-performance.html)

---

**Configuration Guide Version**: 3.0.0  
**Last Updated**: 2025-10-14  
**Maintained by**: WildCAM ESP32 Team
