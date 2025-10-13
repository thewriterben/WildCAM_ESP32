/**
 * Collaboration API Service
 * Handles all collaboration-related API calls
 */

import axios from 'axios';

const API_BASE = process.env.REACT_APP_API_URL || 'http://localhost:5000/api';

// Get auth token from localStorage
const getAuthHeaders = () => {
  const token = localStorage.getItem('access_token');
  return token ? { Authorization: `Bearer ${token}` } : {};
};

const collaborationApi = {
  // ===== USER PRESENCE =====
  getActiveUsers: async () => {
    try {
      const response = await axios.get(`${API_BASE}/collaboration/users/active`, {
        headers: getAuthHeaders()
      });
      return response.data;
    } catch (error) {
      console.error('Get active users error:', error);
      throw error;
    }
  },

  // ===== ANNOTATIONS =====
  getAnnotations: async (detectionId) => {
    try {
      const response = await axios.get(`${API_BASE}/collaboration/annotations`, {
        params: { detection_id: detectionId },
        headers: getAuthHeaders()
      });
      return response.data.annotations;
    } catch (error) {
      console.error('Get annotations error:', error);
      throw error;
    }
  },

  createAnnotation: async (detectionId, annotationType, content, position, metadata = {}) => {
    try {
      const response = await axios.post(
        `${API_BASE}/collaboration/annotations`,
        {
          detection_id: detectionId,
          annotation_type: annotationType,
          content,
          position,
          metadata
        },
        { headers: getAuthHeaders() }
      );
      return response.data.annotation;
    } catch (error) {
      console.error('Create annotation error:', error);
      throw error;
    }
  },

  // ===== CHAT =====
  getChatMessages: async (channel = 'general', limit = 50) => {
    try {
      const response = await axios.get(`${API_BASE}/collaboration/chat`, {
        params: { channel, limit },
        headers: getAuthHeaders()
      });
      return response.data.messages;
    } catch (error) {
      console.error('Get chat messages error:', error);
      throw error;
    }
  },

  sendChatMessage: async (message, channel = 'general', parentId = null, mentions = null) => {
    try {
      const response = await axios.post(
        `${API_BASE}/collaboration/chat`,
        {
          message,
          channel,
          parent_id: parentId,
          mentions
        },
        { headers: getAuthHeaders() }
      );
      return response.data.message;
    } catch (error) {
      console.error('Send chat message error:', error);
      throw error;
    }
  },

  // ===== BOOKMARKS =====
  getBookmarks: async () => {
    try {
      const response = await axios.get(`${API_BASE}/collaboration/bookmarks`, {
        headers: getAuthHeaders()
      });
      return response.data.bookmarks;
    } catch (error) {
      console.error('Get bookmarks error:', error);
      throw error;
    }
  },

  createBookmark: async (title, description, detectionId = null, cameraId = null, 
                        tags = [], isShared = false, sharedWith = []) => {
    try {
      const response = await axios.post(
        `${API_BASE}/collaboration/bookmarks`,
        {
          title,
          description,
          detection_id: detectionId,
          camera_id: cameraId,
          tags,
          is_shared: isShared,
          shared_with: sharedWith
        },
        { headers: getAuthHeaders() }
      );
      return response.data.bookmark;
    } catch (error) {
      console.error('Create bookmark error:', error);
      throw error;
    }
  },

  // ===== TASKS =====
  getTasks: async (myTasks = true, status = null) => {
    try {
      const params = { my_tasks: myTasks };
      if (status) params.status = status;
      
      const response = await axios.get(`${API_BASE}/collaboration/tasks`, {
        params,
        headers: getAuthHeaders()
      });
      return response.data.tasks;
    } catch (error) {
      console.error('Get tasks error:', error);
      throw error;
    }
  },

  createTask: async (title, description, taskType, priority, assignedTo, 
                     cameraId = null, detectionId = null, dueDate = null) => {
    try {
      const response = await axios.post(
        `${API_BASE}/collaboration/tasks`,
        {
          title,
          description,
          task_type: taskType,
          priority,
          assigned_to: assignedTo,
          camera_id: cameraId,
          detection_id: detectionId,
          due_date: dueDate
        },
        { headers: getAuthHeaders() }
      );
      return response.data.task;
    } catch (error) {
      console.error('Create task error:', error);
      throw error;
    }
  },

  updateTaskStatus: async (taskId, status) => {
    try {
      const response = await axios.patch(
        `${API_BASE}/collaboration/tasks/${taskId}`,
        { status },
        { headers: getAuthHeaders() }
      );
      return response.data.task;
    } catch (error) {
      console.error('Update task error:', error);
      throw error;
    }
  },

  // ===== FIELD NOTES =====
  getFieldNotes: async (cameraId = null, detectionId = null) => {
    try {
      const params = {};
      if (cameraId) params.camera_id = cameraId;
      if (detectionId) params.detection_id = detectionId;
      
      const response = await axios.get(`${API_BASE}/collaboration/field-notes`, {
        params,
        headers: getAuthHeaders()
      });
      return response.data.notes;
    } catch (error) {
      console.error('Get field notes error:', error);
      throw error;
    }
  },

  createFieldNote: async (title, content, noteType, cameraId = null, 
                         detectionId = null, tags = [], attachments = []) => {
    try {
      const response = await axios.post(
        `${API_BASE}/collaboration/field-notes`,
        {
          title,
          content,
          note_type: noteType,
          camera_id: cameraId,
          detection_id: detectionId,
          tags,
          attachments
        },
        { headers: getAuthHeaders() }
      );
      return response.data.note;
    } catch (error) {
      console.error('Create field note error:', error);
      throw error;
    }
  }
};

export default collaborationApi;
