/**
 * useRBAC Hook
 * 
 * React hook for accessing RBAC (Role-Based Access Control) information
 * and checking permissions in frontend components.
 */

import { useState, useEffect, useCallback } from 'react';
import axios from 'axios';

/**
 * Custom hook for RBAC functionality
 * 
 * @returns {Object} RBAC state and helper functions
 */
export const useRBAC = () => {
  const [rbacInfo, setRbacInfo] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  // Fetch user's RBAC information on mount
  useEffect(() => {
    fetchRBACInfo();
  }, []);

  const fetchRBACInfo = async () => {
    try {
      setLoading(true);
      const token = localStorage.getItem('token');
      
      if (!token) {
        setError('No authentication token found');
        setLoading(false);
        return;
      }

      const response = await axios.get('/api/rbac/permissions', {
        headers: {
          'Authorization': `Bearer ${token}`
        }
      });

      setRbacInfo(response.data.rbac);
      setError(null);
    } catch (err) {
      console.error('Error fetching RBAC info:', err);
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  /**
   * Check if user has a specific permission
   * 
   * @param {string} permission - Permission to check (e.g., 'create_annotations')
   * @returns {boolean} True if user has permission
   */
  const hasPermission = useCallback((permission) => {
    if (!rbacInfo || !rbacInfo.permissions) {
      return false;
    }
    return rbacInfo.permissions.includes(permission);
  }, [rbacInfo]);

  /**
   * Check if user has any of the specified permissions
   * 
   * @param {Array<string>} permissions - Array of permissions to check
   * @returns {boolean} True if user has at least one permission
   */
  const hasAnyPermission = useCallback((permissions) => {
    if (!rbacInfo || !rbacInfo.permissions) {
      return false;
    }
    return permissions.some(perm => rbacInfo.permissions.includes(perm));
  }, [rbacInfo]);

  /**
   * Check if user has all of the specified permissions
   * 
   * @param {Array<string>} permissions - Array of permissions to check
   * @returns {boolean} True if user has all permissions
   */
  const hasAllPermissions = useCallback((permissions) => {
    if (!rbacInfo || !rbacInfo.permissions) {
      return false;
    }
    return permissions.every(perm => rbacInfo.permissions.includes(perm));
  }, [rbacInfo]);

  /**
   * Check if user has a specific role
   * 
   * @param {string} role - Role to check (e.g., 'ADMIN', 'RESEARCHER')
   * @returns {boolean} True if user has the role
   */
  const hasRole = useCallback((role) => {
    if (!rbacInfo || !rbacInfo.role) {
      return false;
    }
    return rbacInfo.role === role;
  }, [rbacInfo]);

  /**
   * Check if user's role level is at least the specified level
   * 
   * @param {number} minLevel - Minimum role level (0=GUEST, 5=ADMIN)
   * @returns {boolean} True if user meets or exceeds the level
   */
  const hasMinimumRoleLevel = useCallback((minLevel) => {
    if (!rbacInfo || rbacInfo.role_level === undefined) {
      return false;
    }
    return rbacInfo.role_level >= minLevel;
  }, [rbacInfo]);

  /**
   * Check permission against backend API
   * Useful for critical operations that need server verification
   * 
   * @param {string} permission - Permission to check
   * @returns {Promise<boolean>} True if user has permission
   */
  const checkPermissionOnServer = async (permission) => {
    try {
      const token = localStorage.getItem('token');
      const response = await axios.post('/api/rbac/check-permission', 
        { permission },
        {
          headers: {
            'Authorization': `Bearer ${token}`
          }
        }
      );
      return response.data.has_permission;
    } catch (err) {
      console.error('Error checking permission on server:', err);
      return false;
    }
  };

  return {
    // State
    rbacInfo,
    loading,
    error,
    
    // Helper functions
    hasPermission,
    hasAnyPermission,
    hasAllPermissions,
    hasRole,
    hasMinimumRoleLevel,
    checkPermissionOnServer,
    
    // Utilities
    refresh: fetchRBACInfo,
    
    // Convenience checks
    isAdmin: rbacInfo?.is_admin || false,
    isLeadOrHigher: rbacInfo?.is_lead_or_higher || false,
    
    // Role info
    role: rbacInfo?.role || 'UNKNOWN',
    roleLevel: rbacInfo?.role_level || 0,
    permissions: rbacInfo?.permissions || [],
  };
};

/**
 * Permission constants for easy reference
 * Keep in sync with backend/rbac.py
 */
export const Permissions = {
  // Viewing
  VIEW_DETECTIONS: 'view_detections',
  VIEW_ANALYTICS: 'view_analytics',
  VIEW_CAMERA_FEEDS: 'view_camera_feeds',
  VIEW_FIELD_NOTES: 'view_field_notes',
  
  // Basic collaboration
  SEND_CHAT_MESSAGES: 'send_chat_messages',
  CREATE_ANNOTATIONS: 'create_annotations',
  CREATE_BOOKMARKS: 'create_bookmarks',
  
  // Advanced collaboration
  EDIT_ANNOTATIONS: 'edit_annotations',
  DELETE_ANNOTATIONS: 'delete_annotations',
  CREATE_TASKS: 'create_tasks',
  ASSIGN_TASKS: 'assign_tasks',
  
  // Management
  MANAGE_USERS: 'manage_users',
  MANAGE_PROJECTS: 'manage_projects',
  MANAGE_CAMERAS: 'manage_cameras',
  MANAGE_SETTINGS: 'manage_settings',
  
  // Data
  EXPORT_DATA: 'export_data',
  DELETE_DATA: 'delete_data',
  
  // Advanced features
  LOCK_VIEWPORT: 'lock_viewport',
  INITIATE_CALLS: 'initiate_calls',
  RECORD_MEETINGS: 'record_meetings',
};

/**
 * Role constants
 */
export const Roles = {
  GUEST: 'GUEST',
  OBSERVER: 'OBSERVER',
  FIELD_WORKER: 'FIELD_WORKER',
  RESEARCHER: 'RESEARCHER',
  LEAD_RESEARCHER: 'LEAD_RESEARCHER',
  ADMIN: 'ADMIN',
};

/**
 * Role level constants
 */
export const RoleLevels = {
  GUEST: 0,
  OBSERVER: 1,
  FIELD_WORKER: 2,
  RESEARCHER: 3,
  LEAD_RESEARCHER: 4,
  ADMIN: 5,
};

export default useRBAC;
