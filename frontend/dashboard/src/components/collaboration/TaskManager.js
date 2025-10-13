/**
 * TaskManager Component
 * Manage field work tasks and assignments
 */

import React, { useState, useEffect } from 'react';
import {
  Box,
  Paper,
  Typography,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  MenuItem,
  List,
  ListItem,
  ListItemText,
  Chip,
  IconButton,
  Tab,
  Tabs
} from '@mui/material';
import {
  Assignment,
  Add,
  CheckCircle,
  Schedule,
  Cancel
} from '@mui/icons-material';
import moment from 'moment';

import WebSocketService from '../../services/WebSocketService';
import collaborationApi from '../../services/collaborationApi';

function TaskManager() {
  const [tasks, setTasks] = useState([]);
  const [openDialog, setOpenDialog] = useState(false);
  const [filter, setFilter] = useState('pending');
  const [newTask, setNewTask] = useState({
    title: '',
    description: '',
    priority: 'medium',
    task_type: 'data_collection',
    assigned_to: null
  });

  const taskTypes = [
    { value: 'maintenance', label: 'Maintenance' },
    { value: 'data_collection', label: 'Data Collection' },
    { value: 'verification', label: 'Verification' },
    { value: 'analysis', label: 'Analysis' }
  ];

  const priorities = [
    { value: 'low', label: 'Low', color: '#4caf50' },
    { value: 'medium', label: 'Medium', color: '#ff9800' },
    { value: 'high', label: 'High', color: '#f44336' },
    { value: 'urgent', label: 'Urgent', color: '#9c27b0' }
  ];

  useEffect(() => {
    loadTasks();

    // Subscribe to task updates
    WebSocketService.subscribeToTasks(handleTaskUpdate);
    WebSocketService.subscribeToTaskAssignments(handleTaskAssigned);

    return () => {
      WebSocketService.off('task_created', handleTaskUpdate);
      WebSocketService.off('task_updated', handleTaskUpdate);
      WebSocketService.off('task_assigned', handleTaskAssigned);
    };
  }, [filter]);

  const loadTasks = async () => {
    try {
      const taskData = await collaborationApi.getTasks(true, filter === 'all' ? null : filter);
      setTasks(taskData);
    } catch (error) {
      console.error('Error loading tasks:', error);
    }
  };

  const handleTaskUpdate = () => {
    loadTasks();
  };

  const handleTaskAssigned = (data) => {
    // Show notification
    console.log('Task assigned:', data);
    loadTasks();
  };

  const handleCreateTask = async () => {
    try {
      await collaborationApi.createTask(
        newTask.title,
        newTask.description,
        newTask.task_type,
        newTask.priority,
        newTask.assigned_to
      );
      
      setOpenDialog(false);
      setNewTask({
        title: '',
        description: '',
        priority: 'medium',
        task_type: 'data_collection',
        assigned_to: null
      });
    } catch (error) {
      console.error('Error creating task:', error);
    }
  };

  const handleUpdateStatus = async (taskId, status) => {
    try {
      await collaborationApi.updateTaskStatus(taskId, status);
    } catch (error) {
      console.error('Error updating task:', error);
    }
  };

  const getPriorityColor = (priority) => {
    const p = priorities.find(p => p.value === priority);
    return p ? p.color : '#9e9e9e';
  };

  const getStatusIcon = (status) => {
    switch (status) {
      case 'completed':
        return <CheckCircle sx={{ color: '#4caf50' }} />;
      case 'in_progress':
        return <Schedule sx={{ color: '#ff9800' }} />;
      case 'cancelled':
        return <Cancel sx={{ color: '#f44336' }} />;
      default:
        return <Assignment />;
    }
  };

  return (
    <Paper elevation={2} sx={{ p: 2 }}>
      {/* Header */}
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          <Assignment />
          <Typography variant="h6">Tasks</Typography>
        </Box>
        <Button
          variant="contained"
          startIcon={<Add />}
          onClick={() => setOpenDialog(true)}
        >
          New Task
        </Button>
      </Box>

      {/* Filter Tabs */}
      <Tabs value={filter} onChange={(e, v) => setFilter(v)} sx={{ mb: 2 }}>
        <Tab label="Pending" value="pending" />
        <Tab label="In Progress" value="in_progress" />
        <Tab label="Completed" value="completed" />
        <Tab label="All" value="all" />
      </Tabs>

      {/* Task List */}
      <List>
        {tasks.length === 0 ? (
          <Typography variant="body2" color="text.secondary" sx={{ textAlign: 'center', py: 4 }}>
            No tasks found
          </Typography>
        ) : (
          tasks.map((task) => (
            <ListItem
              key={task.id}
              sx={{
                mb: 1,
                border: 1,
                borderColor: 'divider',
                borderRadius: 1,
                flexDirection: 'column',
                alignItems: 'stretch'
              }}
            >
              <Box sx={{ display: 'flex', alignItems: 'flex-start', width: '100%', gap: 2 }}>
                <Box sx={{ mt: 0.5 }}>
                  {getStatusIcon(task.status)}
                </Box>
                
                <Box sx={{ flex: 1 }}>
                  <ListItemText
                    primary={
                      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                        <Typography variant="subtitle1">{task.title}</Typography>
                        <Chip
                          label={task.priority}
                          size="small"
                          sx={{
                            bgcolor: getPriorityColor(task.priority),
                            color: 'white',
                            fontSize: 10
                          }}
                        />
                        {task.task_type && (
                          <Chip
                            label={task.task_type.replace('_', ' ')}
                            size="small"
                            variant="outlined"
                            sx={{ fontSize: 10 }}
                          />
                        )}
                      </Box>
                    }
                    secondary={
                      <>
                        <Typography variant="body2" color="text.secondary">
                          {task.description}
                        </Typography>
                        {task.due_date && (
                          <Typography variant="caption" color="text.secondary">
                            Due: {moment(task.due_date).format('MMM D, YYYY')}
                          </Typography>
                        )}
                      </>
                    }
                  />
                </Box>
                
                {task.status !== 'completed' && task.status !== 'cancelled' && (
                  <Box sx={{ display: 'flex', gap: 1 }}>
                    {task.status === 'pending' && (
                      <Button
                        size="small"
                        onClick={() => handleUpdateStatus(task.id, 'in_progress')}
                      >
                        Start
                      </Button>
                    )}
                    {task.status === 'in_progress' && (
                      <Button
                        size="small"
                        color="success"
                        onClick={() => handleUpdateStatus(task.id, 'completed')}
                      >
                        Complete
                      </Button>
                    )}
                  </Box>
                )}
              </Box>
            </ListItem>
          ))
        )}
      </List>

      {/* Create Task Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="sm" fullWidth>
        <DialogTitle>Create New Task</DialogTitle>
        <DialogContent>
          <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2, mt: 1 }}>
            <TextField
              label="Title"
              fullWidth
              value={newTask.title}
              onChange={(e) => setNewTask({ ...newTask, title: e.target.value })}
            />
            
            <TextField
              label="Description"
              fullWidth
              multiline
              rows={3}
              value={newTask.description}
              onChange={(e) => setNewTask({ ...newTask, description: e.target.value })}
            />
            
            <TextField
              select
              label="Type"
              fullWidth
              value={newTask.task_type}
              onChange={(e) => setNewTask({ ...newTask, task_type: e.target.value })}
            >
              {taskTypes.map((type) => (
                <MenuItem key={type.value} value={type.value}>
                  {type.label}
                </MenuItem>
              ))}
            </TextField>
            
            <TextField
              select
              label="Priority"
              fullWidth
              value={newTask.priority}
              onChange={(e) => setNewTask({ ...newTask, priority: e.target.value })}
            >
              {priorities.map((priority) => (
                <MenuItem key={priority.value} value={priority.value}>
                  {priority.label}
                </MenuItem>
              ))}
            </TextField>
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpenDialog(false)}>Cancel</Button>
          <Button
            onClick={handleCreateTask}
            variant="contained"
            disabled={!newTask.title}
          >
            Create
          </Button>
        </DialogActions>
      </Dialog>
    </Paper>
  );
}

export default TaskManager;
