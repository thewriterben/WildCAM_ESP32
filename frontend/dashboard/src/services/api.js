/**
 * API Client for WildCAM Dashboard
 * Handles authentication and API requests to the backend
 */

import axios from 'axios';

class APIService {
  constructor() {
    this.baseURL = process.env.REACT_APP_API_URL || 'http://localhost:5000';
    this.client = axios.create({
      baseURL: this.baseURL,
      timeout: 30000,
      headers: {
        'Content-Type': 'application/json',
      },
    });

    // Request interceptor to add auth token
    this.client.interceptors.request.use(
      (config) => {
        const token = this.getToken();
        if (token) {
          config.headers.Authorization = `Bearer ${token}`;
        }
        return config;
      },
      (error) => Promise.reject(error)
    );

    // Response interceptor to handle token refresh
    this.client.interceptors.response.use(
      (response) => response,
      async (error) => {
        const original = error.config;

        if (error.response?.status === 401 && !original._retry) {
          original._retry = true;

          try {
            const refreshToken = this.getRefreshToken();
            if (refreshToken) {
              const response = await this.refreshToken();
              this.setToken(response.data.access_token);
              return this.client(original);
            }
          } catch (refreshError) {
            this.logout();
            window.location.href = '/login';
            return Promise.reject(refreshError);
          }
        }

        return Promise.reject(error);
      }
    );
  }

  // Token management
  setToken(token) {
    localStorage.setItem('access_token', token);
  }

  getToken() {
    return localStorage.getItem('access_token');
  }

  setRefreshToken(token) {
    localStorage.setItem('refresh_token', token);
  }

  getRefreshToken() {
    return localStorage.getItem('refresh_token');
  }

  logout() {
    localStorage.removeItem('access_token');
    localStorage.removeItem('refresh_token');
    localStorage.removeItem('user');
  }

  // Authentication methods
  async login(username, password) {
    try {
      const response = await this.client.post('/api/auth/login', {
        username,
        password,
      });

      if (response.data.access_token) {
        this.setToken(response.data.access_token);
        this.setRefreshToken(response.data.refresh_token);
        localStorage.setItem('user', JSON.stringify(response.data.user));
      }

      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Login failed');
    }
  }

  async register(userData) {
    try {
      const response = await this.client.post('/api/auth/register', userData);

      if (response.data.access_token) {
        this.setToken(response.data.access_token);
        this.setRefreshToken(response.data.refresh_token);
        localStorage.setItem('user', JSON.stringify(response.data.user));
      }

      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Registration failed');
    }
  }

  async refreshToken() {
    const refreshToken = this.getRefreshToken();
    return this.client.post('/api/auth/refresh', {}, {
      headers: {
        Authorization: `Bearer ${refreshToken}`,
      },
    });
  }

  async getCurrentUser() {
    try {
      const response = await this.client.get('/api/auth/me');
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to get user info');
    }
  }

  // Camera management
  async getCameras() {
    try {
      const response = await this.client.get('/api/cameras');
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to fetch cameras');
    }
  }

  async registerCamera(cameraData) {
    try {
      const response = await this.client.post('/api/cameras/register', cameraData);
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to register camera');
    }
  }

  // Detection management
  async getDetections(params = {}) {
    try {
      const response = await this.client.get('/api/detections', { params });
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to fetch detections');
    }
  }

  async uploadDetection(formData) {
    try {
      const response = await this.client.post('/api/detections', formData, {
        headers: {
          'Content-Type': 'multipart/form-data',
        },
      });
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to upload detection');
    }
  }

  // Analytics
  async getSpeciesAnalytics(days = 30) {
    try {
      const response = await this.client.get('/api/analytics/species', {
        params: { days },
      });
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to fetch species analytics');
    }
  }

  async getActivityPatterns(days = 7) {
    try {
      const response = await this.client.get('/api/analytics/activity', {
        params: { days },
      });
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to fetch activity patterns');
    }
  }

  // Alerts
  async getAlerts(unreadOnly = false) {
    try {
      const response = await this.client.get('/api/alerts', {
        params: { unread_only: unreadOnly },
      });
      return response.data;
    } catch (error) {
      throw new Error(error.response?.data?.error || 'Failed to fetch alerts');
    }
  }

  // Utility methods
  isAuthenticated() {
    return !!this.getToken();
  }

  getCurrentUserId() {
    const user = localStorage.getItem('user');
    if (user) {
      return JSON.parse(user).id;
    }
    return null;
  }

  getUserInfo() {
    const user = localStorage.getItem('user');
    if (user) {
      return JSON.parse(user);
    }
    return null;
  }
}

// Create and export a singleton instance
const apiService = new APIService();
export default apiService;