# Wildlife Camera Task Manager

A web-based task management application specifically designed for wildlife camera operations and research activities. This application provides a user-friendly interface for organizing and tracking wildlife monitoring tasks with local storage capabilities.

## Features

### Core Functionality
- ✅ **Add Tasks**: Create new tasks with title, description, and category
- ✅ **Edit Tasks**: Modify existing tasks inline
- ✅ **Delete Tasks**: Remove individual tasks with confirmation
- ✅ **Mark as Complete**: Toggle task completion status
- ✅ **Filter Tasks**: Filter by status (all, active, completed) and categories
- ✅ **Persist Data**: Uses ESP32 preferences for local storage
- ✅ **Clear All Tasks**: Remove all tasks with confirmation prompt

### Wildlife-Specific Categories
1. **Equipment Maintenance**: Battery checks, solar panel cleaning, camera calibration
2. **Data Collection**: Download images, review wildlife logs, backup data
3. **Monitoring Schedule**: Check motion sensors, verify connectivity, status reports
4. **Wildlife Observations**: Log species sightings, behavior notes, environmental data
5. **System Updates**: Firmware updates, configuration changes, troubleshooting

### User Interface
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Modern Styling**: Clean, intuitive interface with wildlife-themed design
- **Real-time Updates**: Immediate feedback for all operations
- **Mobile-Friendly**: Touch-optimized buttons and layouts
- **Visual Indicators**: Color-coded categories and completion status

## Hardware Requirements

- ESP32 development board (ESP32-S3 recommended)
- WiFi connectivity
- Minimum 4MB flash memory for application and data storage

## Software Dependencies

- ESP32 Arduino Core
- WiFi library
- ESPAsyncWebServer library
- ArduinoJson library
- Preferences library (built-in)

## Installation

1. **Clone the Repository**
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM/examples/wildlife_task_manager
   ```

2. **Install Required Libraries**
   ```bash
   # Using PlatformIO
   pio lib install "me-no-dev/ESPAsyncWebServer" "bblanchon/ArduinoJson"
   
   # Or using Arduino IDE Library Manager:
   # - ESPAsyncWebServer
   # - ArduinoJson
   ```

3. **Configure WiFi**
   Edit the WiFi credentials in `wildlife_task_manager.ino`:
   ```cpp
   const char* WIFI_SSID = "Your_WiFi_SSID";
   const char* WIFI_PASSWORD = "Your_WiFi_Password";
   ```

4. **Upload to ESP32**
   - Use Arduino IDE or PlatformIO to compile and upload
   - Monitor serial output for IP address assignment

## Usage

### Getting Started

1. **Power on your ESP32** and wait for WiFi connection
2. **Find the IP address** from serial monitor output
3. **Open web browser** and navigate to `http://[ESP32_IP_ADDRESS]/`
4. **Start managing tasks** immediately!

### Web Interface

#### Main Dashboard
- Access the main task interface at `http://[ESP32_IP]/`
- View all tasks in an organized, card-based layout
- Use filter buttons to show specific categories or completion status

#### Adding Tasks
1. Click **"+ Add New Task"** button
2. Fill in the task form:
   - **Title**: Brief, descriptive task name (required)
   - **Description**: Optional detailed description
   - **Category**: Select from wildlife-specific categories (required)
3. Click **"Add Task"** to save

#### Managing Tasks
- **Complete/Incomplete**: Click the status button to toggle completion
- **Edit**: Click "Edit" to modify task details
- **Delete**: Click "Delete" with confirmation prompt

#### Filtering Tasks
- **All Tasks**: Show all tasks regardless of status
- **Active**: Show only incomplete tasks
- **Completed**: Show only completed tasks
- **Category Filters**: Show tasks from specific categories

### API Endpoints

The application provides RESTful API endpoints for integration:

- `GET /api/tasks` - Retrieve all tasks
- `POST /api/tasks` - Create new task
- `PUT /api/task/{id}` - Update existing task
- `DELETE /api/task/{id}` - Delete specific task
- `DELETE /api/tasks/clear` - Delete all tasks

### Example API Usage

```javascript
// Get all tasks
fetch('/api/tasks')
  .then(response => response.json())
  .then(tasks => console.log(tasks));

// Create new task
fetch('/api/tasks', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    title: 'Check camera battery',
    description: 'Verify battery level and solar charging',
    category: 'Equipment Maintenance',
    completed: false
  })
});
```

## Data Persistence

Tasks are automatically saved to ESP32 flash memory using the Preferences library:
- Data persists across power cycles and reboots
- No external storage required
- Efficient storage format using JSON serialization
- Automatic cleanup and organization

## Wildlife Monitoring Use Cases

### Equipment Maintenance
- Battery voltage checks
- Solar panel cleaning schedules
- Camera lens cleaning
- Motion sensor calibration
- Weatherproofing inspections

### Data Collection
- SD card data downloads
- Image review and cataloging
- Log file analysis
- Backup verification
- Storage space monitoring

### Monitoring Schedule
- Daily status checks
- Connectivity verification
- Motion detection testing
- Environmental sensor readings
- Alert system testing

### Wildlife Observations
- Species identification logs
- Behavior pattern notes
- Migration tracking
- Feeding activity records
- Habitat usage documentation

### System Updates
- Firmware update schedules
- Configuration changes
- Network setting updates
- Security patches
- Performance optimizations

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in code
- Check WiFi signal strength at deployment location
- Monitor serial output for connection status

### Storage Issues
- Tasks not saving: Check ESP32 flash memory availability
- Corrupted data: Use "Clear All Tasks" to reset storage

### Web Interface Issues
- Page not loading: Verify ESP32 IP address
- Buttons not working: Check browser JavaScript support
- Mobile display: Ensure viewport meta tag is present

## Development

### Code Structure
- `wildlife_task_manager.ino`: Main application file
- HTML/CSS/JavaScript: Embedded in C++ strings
- Local storage: ESP32 Preferences API
- Web server: ESPAsyncWebServer library

### Customization
- Modify task categories in HTML select options
- Adjust styling in embedded CSS
- Add new API endpoints as needed
- Extend task data structure for additional fields

### Testing
- Use browser developer tools for debugging
- Monitor ESP32 serial output for backend issues
- Test on multiple devices for responsive design

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on actual hardware
5. Submit a pull request

## License

This project is part of the ESP32 Wildlife Camera system and follows the same licensing terms as the main repository.

## Support

For issues and questions:
- Check the troubleshooting section above
- Review ESP32 and library documentation
- Submit GitHub issues for bugs or feature requests

---

*This task manager is designed specifically for wildlife researchers and conservationists using ESP32-based camera systems. It provides a practical tool for organizing field work and maintaining professional wildlife monitoring operations.*