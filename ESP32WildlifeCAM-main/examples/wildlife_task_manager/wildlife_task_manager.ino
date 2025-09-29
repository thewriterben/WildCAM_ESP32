/**
 * @file wildlife_task_manager.ino
 * @brief Wildlife Camera Task Management System
 * 
 * A web-based task management application specifically designed for 
 * wildlife camera operations and research activities. Features include:
 * - Task creation and management for wildlife monitoring
 * - Local storage using ESP32 preferences
 * - Responsive web interface
 * - Wildlife-specific task categories
 * - Mobile-friendly design
 * 
 * Task Categories:
 * - Equipment Maintenance
 * - Data Collection  
 * - Monitoring Schedule
 * - Wildlife Observations
 * - System Updates
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>

// Configuration
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
const uint16_t WEB_PORT = 80;

// Components
AsyncWebServer server(WEB_PORT);
Preferences preferences;

// Task structure
struct Task {
    int id;
    String title;
    String description;
    String category;
    bool completed;
    unsigned long created;
    unsigned long updated;
};

// Global variables
std::vector<Task> tasks;
int nextTaskId = 1;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("===============================");
    Serial.println("ESP32 Wildlife Camera");
    Serial.println("Task Management System");
    Serial.println("===============================");
    
    // Initialize preferences for local storage
    preferences.begin("tasks", false);
    nextTaskId = preferences.getInt("nextTaskId", 1);
    
    // Load existing tasks from preferences
    loadTasksFromStorage();
    
    // Initialize WiFi
    if (!initializeWiFi()) {
        Serial.println("ERROR: WiFi initialization failed");
        return;
    }
    
    // Setup NTP for timestamps
    configTime(0, 0, "pool.ntp.org");
    
    // Setup web interface
    setupWebServer();
    
    // Start the web server
    server.begin();
    
    Serial.println("\nWildlife Task Manager ready!");
    Serial.println("==============================");
    Serial.print("Device IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Task Manager URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    Serial.println("==============================");
    Serial.println();
    Serial.println("Available endpoints:");
    Serial.println("- /           : Main task interface");
    Serial.println("- /api/tasks  : Task API (GET, POST)");
    Serial.println("- /api/task/ID: Individual task API (PUT, DELETE)");
    Serial.println();
}

void loop() {
    // Keep WiFi connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting reconnection...");
        initializeWiFi();
    }
    
    delay(1000);
}

bool initializeWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected successfully");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed");
        return false;
    }
}

void setupWebServer() {
    // Serve main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = generateMainPage();
        request->send(200, "text/html", html);
    });
    
    // API: Get all tasks
    server.on("/api/tasks", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = tasksToJson();
        request->send(200, "application/json", json);
    });
    
    // API: Create new task
    server.on("/api/tasks", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Handle POST data
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String body = String((char*)data).substring(0, len);
        Task newTask = parseTaskFromJson(body);
        
        if (!newTask.title.isEmpty()) {
            newTask.id = nextTaskId++;
            newTask.created = millis();
            newTask.updated = millis();
            
            tasks.push_back(newTask);
            saveTasksToStorage();
            
            String json = taskToJson(newTask);
            request->send(201, "application/json", json);
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid task data\"}");
        }
    });
    
    // API: Update task
    server.on("^\\/api\\/task\\/(\\d+)$", HTTP_PUT, [](AsyncWebServerRequest *request) {
        String taskId = request->pathArg(0);
        int id = taskId.toInt();
        
        // Find task by ID and update it
        for (auto& task : tasks) {
            if (task.id == id) {
                // Task found, will be updated in body handler
                return;
            }
        }
        request->send(404, "application/json", "{\"error\":\"Task not found\"}");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String taskId = request->pathArg(0);
        int id = taskId.toInt();
        String body = String((char*)data).substring(0, len);
        
        for (auto& task : tasks) {
            if (task.id == id) {
                Task updatedTask = parseTaskFromJson(body);
                if (!updatedTask.title.isEmpty()) {
                    task.title = updatedTask.title;
                    task.description = updatedTask.description;
                    task.category = updatedTask.category;
                    task.completed = updatedTask.completed;
                    task.updated = millis();
                    
                    saveTasksToStorage();
                    String json = taskToJson(task);
                    request->send(200, "application/json", json);
                } else {
                    request->send(400, "application/json", "{\"error\":\"Invalid task data\"}");
                }
                return;
            }
        }
    });
    
    // API: Delete task
    server.on("^\\/api\\/task\\/(\\d+)$", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        String taskId = request->pathArg(0);
        int id = taskId.toInt();
        
        auto it = std::remove_if(tasks.begin(), tasks.end(), 
            [id](const Task& task) { return task.id == id; });
        
        if (it != tasks.end()) {
            tasks.erase(it, tasks.end());
            saveTasksToStorage();
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(404, "application/json", "{\"error\":\"Task not found\"}");
        }
    });
    
    // API: Clear all tasks
    server.on("/api/tasks/clear", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        tasks.clear();
        preferences.clear();
        nextTaskId = 1;
        preferences.putInt("nextTaskId", nextTaskId);
        request->send(200, "application/json", "{\"success\":true}");
    });
}

String generateMainPage() {
    return R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wildlife Camera Task Manager</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }
        
        .main-content {
            padding: 30px;
        }
        
        .controls {
            display: flex;
            gap: 15px;
            margin-bottom: 30px;
            flex-wrap: wrap;
            align-items: center;
        }
        
        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 500;
            transition: all 0.3s ease;
            text-decoration: none;
            display: inline-block;
        }
        
        .btn-primary {
            background: #667eea;
            color: white;
        }
        
        .btn-primary:hover {
            background: #5a6fd8;
            transform: translateY(-2px);
        }
        
        .btn-danger {
            background: #e74c3c;
            color: white;
        }
        
        .btn-danger:hover {
            background: #c0392b;
        }
        
        .btn-secondary {
            background: #95a5a6;
            color: white;
        }
        
        .btn-secondary:hover {
            background: #7f8c8d;
        }
        
        .filter-buttons {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }
        
        .filter-btn {
            padding: 8px 16px;
            border: 2px solid #667eea;
            background: transparent;
            color: #667eea;
            border-radius: 20px;
            cursor: pointer;
            transition: all 0.3s ease;
            font-size: 12px;
        }
        
        .filter-btn.active, .filter-btn:hover {
            background: #667eea;
            color: white;
        }
        
        .add-task-form {
            background: #f8f9fa;
            padding: 25px;
            border-radius: 10px;
            margin-bottom: 30px;
            display: none;
        }
        
        .form-group {
            margin-bottom: 20px;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: #333;
        }
        
        .form-group input, .form-group textarea, .form-group select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e1e8ed;
            border-radius: 8px;
            font-size: 14px;
            transition: border-color 0.3s ease;
        }
        
        .form-group input:focus, .form-group textarea:focus, .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .form-group textarea {
            resize: vertical;
            min-height: 100px;
        }
        
        .form-actions {
            display: flex;
            gap: 10px;
        }
        
        .tasks-container {
            display: grid;
            gap: 20px;
        }
        
        .task-card {
            background: white;
            border: 2px solid #e1e8ed;
            border-radius: 10px;
            padding: 20px;
            transition: all 0.3s ease;
            position: relative;
        }
        
        .task-card:hover {
            border-color: #667eea;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        
        .task-card.completed {
            opacity: 0.7;
            background: #f8f9fa;
        }
        
        .task-header {
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
            margin-bottom: 15px;
        }
        
        .task-title {
            font-size: 1.2em;
            font-weight: 600;
            color: #333;
            margin-bottom: 5px;
        }
        
        .task-title.completed {
            text-decoration: line-through;
            color: #7f8c8d;
        }
        
        .task-category {
            background: #667eea;
            color: white;
            padding: 4px 12px;
            border-radius: 15px;
            font-size: 12px;
            font-weight: 500;
        }
        
        .task-description {
            color: #666;
            line-height: 1.5;
            margin-bottom: 15px;
        }
        
        .task-meta {
            display: flex;
            justify-content: space-between;
            align-items: center;
            font-size: 12px;
            color: #999;
            margin-bottom: 15px;
        }
        
        .task-actions {
            display: flex;
            gap: 10px;
        }
        
        .task-actions button {
            padding: 6px 12px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 12px;
            transition: all 0.3s ease;
        }
        
        .edit-btn {
            background: #f39c12;
            color: white;
        }
        
        .edit-btn:hover {
            background: #e67e22;
        }
        
        .delete-btn {
            background: #e74c3c;
            color: white;
        }
        
        .delete-btn:hover {
            background: #c0392b;
        }
        
        .complete-btn {
            background: #27ae60;
            color: white;
        }
        
        .complete-btn:hover {
            background: #2ecc71;
        }
        
        .incomplete-btn {
            background: #95a5a6;
            color: white;
        }
        
        .incomplete-btn:hover {
            background: #7f8c8d;
        }
        
        .no-tasks {
            text-align: center;
            padding: 60px 20px;
            color: #7f8c8d;
        }
        
        .no-tasks i {
            font-size: 4em;
            margin-bottom: 20px;
            opacity: 0.5;
        }
        
        @media (max-width: 768px) {
            .controls {
                flex-direction: column;
                align-items: stretch;
            }
            
            .filter-buttons {
                justify-content: center;
            }
            
            .form-actions {
                flex-direction: column;
            }
            
            .task-header {
                flex-direction: column;
                gap: 10px;
            }
            
            .task-meta {
                flex-direction: column;
                gap: 5px;
                align-items: flex-start;
            }
            
            .task-actions {
                flex-wrap: wrap;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🦌 Wildlife Camera Task Manager</h1>
            <p>Organize and track your wildlife monitoring activities</p>
        </div>
        
        <div class="main-content">
            <div class="controls">
                <button class="btn btn-primary" onclick="toggleAddForm()">+ Add New Task</button>
                <button class="btn btn-danger" onclick="clearAllTasks()">Clear All Tasks</button>
                
                <div class="filter-buttons">
                    <button class="filter-btn active" data-filter="all">All Tasks</button>
                    <button class="filter-btn" data-filter="incomplete">Active</button>
                    <button class="filter-btn" data-filter="completed">Completed</button>
                    <button class="filter-btn" data-filter="Equipment Maintenance">Maintenance</button>
                    <button class="filter-btn" data-filter="Data Collection">Data Collection</button>
                    <button class="filter-btn" data-filter="Monitoring Schedule">Monitoring</button>
                    <button class="filter-btn" data-filter="Wildlife Observations">Observations</button>
                    <button class="filter-btn" data-filter="System Updates">Updates</button>
                </div>
            </div>
            
            <div class="add-task-form" id="addTaskForm">
                <h3>Add New Task</h3>
                <form id="taskForm">
                    <div class="form-group">
                        <label for="taskTitle">Task Title *</label>
                        <input type="text" id="taskTitle" required placeholder="Enter task title...">
                    </div>
                    
                    <div class="form-group">
                        <label for="taskDescription">Description</label>
                        <textarea id="taskDescription" placeholder="Optional task description..."></textarea>
                    </div>
                    
                    <div class="form-group">
                        <label for="taskCategory">Category *</label>
                        <select id="taskCategory" required>
                            <option value="">Select category...</option>
                            <option value="Equipment Maintenance">Equipment Maintenance</option>
                            <option value="Data Collection">Data Collection</option>
                            <option value="Monitoring Schedule">Monitoring Schedule</option>
                            <option value="Wildlife Observations">Wildlife Observations</option>
                            <option value="System Updates">System Updates</option>
                        </select>
                    </div>
                    
                    <div class="form-actions">
                        <button type="submit" class="btn btn-primary">Add Task</button>
                        <button type="button" class="btn btn-secondary" onclick="cancelAddTask()">Cancel</button>
                    </div>
                </form>
            </div>
            
            <div class="tasks-container" id="tasksContainer">
                <!-- Tasks will be loaded here -->
            </div>
        </div>
    </div>

    <script>
        let tasks = [];
        let currentFilter = 'all';
        let editingTaskId = null;

        // Initialize the application
        document.addEventListener('DOMContentLoaded', function() {
            loadTasks();
            setupEventListeners();
        });

        function setupEventListeners() {
            // Task form submission
            document.getElementById('taskForm').addEventListener('submit', function(e) {
                e.preventDefault();
                if (editingTaskId) {
                    updateTask();
                } else {
                    addTask();
                }
            });

            // Filter buttons
            document.querySelectorAll('.filter-btn').forEach(btn => {
                btn.addEventListener('click', function() {
                    document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
                    this.classList.add('active');
                    currentFilter = this.dataset.filter;
                    renderTasks();
                });
            });
        }

        function toggleAddForm() {
            const form = document.getElementById('addTaskForm');
            const isVisible = form.style.display !== 'none';
            
            if (isVisible) {
                form.style.display = 'none';
                resetForm();
            } else {
                form.style.display = 'block';
                document.getElementById('taskTitle').focus();
            }
        }

        function cancelAddTask() {
            document.getElementById('addTaskForm').style.display = 'none';
            resetForm();
        }

        function resetForm() {
            document.getElementById('taskForm').reset();
            editingTaskId = null;
            document.querySelector('#addTaskForm h3').textContent = 'Add New Task';
            document.querySelector('#taskForm button[type="submit"]').textContent = 'Add Task';
        }

        async function loadTasks() {
            try {
                const response = await fetch('/api/tasks');
                if (response.ok) {
                    tasks = await response.json();
                    renderTasks();
                } else {
                    console.error('Failed to load tasks');
                }
            } catch (error) {
                console.error('Error loading tasks:', error);
            }
        }

        async function addTask() {
            const title = document.getElementById('taskTitle').value.trim();
            const description = document.getElementById('taskDescription').value.trim();
            const category = document.getElementById('taskCategory').value;

            if (!title || !category) {
                alert('Please fill in all required fields');
                return;
            }

            const taskData = {
                title: title,
                description: description,
                category: category,
                completed: false
            };

            try {
                const response = await fetch('/api/tasks', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(taskData)
                });

                if (response.ok) {
                    const newTask = await response.json();
                    tasks.push(newTask);
                    renderTasks();
                    cancelAddTask();
                } else {
                    console.error('Failed to add task');
                    alert('Failed to add task. Please try again.');
                }
            } catch (error) {
                console.error('Error adding task:', error);
                alert('Error adding task. Please check your connection.');
            }
        }

        async function updateTask() {
            const title = document.getElementById('taskTitle').value.trim();
            const description = document.getElementById('taskDescription').value.trim();
            const category = document.getElementById('taskCategory').value;

            if (!title || !category) {
                alert('Please fill in all required fields');
                return;
            }

            const taskData = {
                title: title,
                description: description,
                category: category,
                completed: false // Will be preserved from existing task
            };

            try {
                const response = await fetch(`/api/task/${editingTaskId}`, {
                    method: 'PUT',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(taskData)
                });

                if (response.ok) {
                    await loadTasks(); // Reload all tasks to get updated data
                    cancelAddTask();
                } else {
                    console.error('Failed to update task');
                    alert('Failed to update task. Please try again.');
                }
            } catch (error) {
                console.error('Error updating task:', error);
                alert('Error updating task. Please check your connection.');
            }
        }

        async function deleteTask(taskId) {
            if (!confirm('Are you sure you want to delete this task?')) {
                return;
            }

            try {
                const response = await fetch(`/api/task/${taskId}`, {
                    method: 'DELETE'
                });

                if (response.ok) {
                    tasks = tasks.filter(task => task.id !== taskId);
                    renderTasks();
                } else {
                    console.error('Failed to delete task');
                    alert('Failed to delete task. Please try again.');
                }
            } catch (error) {
                console.error('Error deleting task:', error);
                alert('Error deleting task. Please check your connection.');
            }
        }

        async function toggleTaskComplete(taskId) {
            const task = tasks.find(t => t.id === taskId);
            if (!task) return;

            const updatedTask = {
                ...task,
                completed: !task.completed
            };

            try {
                const response = await fetch(`/api/task/${taskId}`, {
                    method: 'PUT',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(updatedTask)
                });

                if (response.ok) {
                    task.completed = !task.completed;
                    renderTasks();
                } else {
                    console.error('Failed to update task status');
                    alert('Failed to update task status. Please try again.');
                }
            } catch (error) {
                console.error('Error updating task status:', error);
                alert('Error updating task status. Please check your connection.');
            }
        }

        function editTask(taskId) {
            const task = tasks.find(t => t.id === taskId);
            if (!task) return;

            editingTaskId = taskId;
            document.getElementById('taskTitle').value = task.title;
            document.getElementById('taskDescription').value = task.description;
            document.getElementById('taskCategory').value = task.category;
            
            document.querySelector('#addTaskForm h3').textContent = 'Edit Task';
            document.querySelector('#taskForm button[type="submit"]').textContent = 'Update Task';
            document.getElementById('addTaskForm').style.display = 'block';
            document.getElementById('taskTitle').focus();
        }

        async function clearAllTasks() {
            if (!confirm('Are you sure you want to delete ALL tasks? This action cannot be undone.')) {
                return;
            }

            try {
                const response = await fetch('/api/tasks/clear', {
                    method: 'DELETE'
                });

                if (response.ok) {
                    tasks = [];
                    renderTasks();
                } else {
                    console.error('Failed to clear tasks');
                    alert('Failed to clear tasks. Please try again.');
                }
            } catch (error) {
                console.error('Error clearing tasks:', error);
                alert('Error clearing tasks. Please check your connection.');
            }
        }

        function renderTasks() {
            const container = document.getElementById('tasksContainer');
            const filteredTasks = filterTasks();

            if (filteredTasks.length === 0) {
                container.innerHTML = `
                    <div class="no-tasks">
                        <div style="font-size: 4em; margin-bottom: 20px; opacity: 0.5;">📋</div>
                        <h3>No tasks found</h3>
                        <p>Add a new task to get started with wildlife monitoring!</p>
                    </div>
                `;
                return;
            }

            container.innerHTML = filteredTasks.map(task => `
                <div class="task-card ${task.completed ? 'completed' : ''}">
                    <div class="task-header">
                        <div>
                            <div class="task-title ${task.completed ? 'completed' : ''}">${escapeHtml(task.title)}</div>
                            <div class="task-category">${escapeHtml(task.category)}</div>
                        </div>
                    </div>
                    
                    ${task.description ? `<div class="task-description">${escapeHtml(task.description)}</div>` : ''}
                    
                    <div class="task-meta">
                        <span>Created: ${formatDate(task.created)}</span>
                        ${task.updated && task.updated !== task.created ? `<span>Updated: ${formatDate(task.updated)}</span>` : ''}
                    </div>
                    
                    <div class="task-actions">
                        <button class="${task.completed ? 'incomplete-btn' : 'complete-btn'}" 
                                onclick="toggleTaskComplete(${task.id})">
                            ${task.completed ? 'Mark Incomplete' : 'Mark Complete'}
                        </button>
                        <button class="edit-btn" onclick="editTask(${task.id})">Edit</button>
                        <button class="delete-btn" onclick="deleteTask(${task.id})">Delete</button>
                    </div>
                </div>
            `).join('');
        }

        function filterTasks() {
            switch (currentFilter) {
                case 'completed':
                    return tasks.filter(task => task.completed);
                case 'incomplete':
                    return tasks.filter(task => !task.completed);
                case 'all':
                    return tasks;
                default:
                    return tasks.filter(task => task.category === currentFilter);
            }
        }

        function formatDate(timestamp) {
            const date = new Date(timestamp);
            return date.toLocaleDateString() + ' ' + date.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'});
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }
    </script>
</body>
</html>
)html";
}

void loadTasksFromStorage() {
    int taskCount = preferences.getInt("taskCount", 0);
    tasks.clear();
    
    for (int i = 0; i < taskCount; i++) {
        String key = "task_" + String(i);
        String taskJson = preferences.getString(key.c_str(), "");
        
        if (!taskJson.isEmpty()) {
            Task task = parseTaskFromJson(taskJson);
            if (task.id > 0) {
                tasks.push_back(task);
            }
        }
    }
    
    Serial.printf("Loaded %d tasks from storage\n", tasks.size());
}

void saveTasksToStorage() {
    preferences.clear();
    preferences.putInt("nextTaskId", nextTaskId);
    preferences.putInt("taskCount", tasks.size());
    
    for (size_t i = 0; i < tasks.size(); i++) {
        String key = "task_" + String(i);
        String taskJson = taskToJson(tasks[i]);
        preferences.putString(key.c_str(), taskJson);
    }
    
    Serial.printf("Saved %d tasks to storage\n", tasks.size());
}

String tasksToJson() {
    DynamicJsonDocument doc(8192);
    JsonArray taskArray = doc.to<JsonArray>();
    
    for (const auto& task : tasks) {
        JsonObject taskObj = taskArray.createNestedObject();
        taskObj["id"] = task.id;
        taskObj["title"] = task.title;
        taskObj["description"] = task.description;
        taskObj["category"] = task.category;
        taskObj["completed"] = task.completed;
        taskObj["created"] = task.created;
        taskObj["updated"] = task.updated;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

String taskToJson(const Task& task) {
    DynamicJsonDocument doc(1024);
    doc["id"] = task.id;
    doc["title"] = task.title;
    doc["description"] = task.description;
    doc["category"] = task.category;
    doc["completed"] = task.completed;
    doc["created"] = task.created;
    doc["updated"] = task.updated;
    
    String result;
    serializeJson(doc, result);
    return result;
}

Task parseTaskFromJson(const String& json) {
    Task task = {};
    DynamicJsonDocument doc(1024);
    
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return task;
    }
    
    task.id = doc["id"] | 0;
    task.title = doc["title"] | "";
    task.description = doc["description"] | "";
    task.category = doc["category"] | "";
    task.completed = doc["completed"] | false;
    task.created = doc["created"] | 0;
    task.updated = doc["updated"] | 0;
    
    return task;
}