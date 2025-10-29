# Contributing to WildCAM_ESP32

Thank you for your interest in contributing to WildCAM_ESP32! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Pull Request Process](#pull-request-process)
- [Community Guidelines](#community-guidelines)

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inspiring community for everyone. We pledge to make participation in our project a harassment-free experience for all, regardless of:

- Age, body size, disability, ethnicity, gender identity and expression
- Level of experience, education, socio-economic status
- Nationality, personal appearance, race, religion
- Sexual identity and orientation

### Our Standards

**Positive behaviors include:**
- Using welcoming and inclusive language
- Being respectful of differing viewpoints and experiences
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

**Unacceptable behaviors include:**
- Trolling, insulting/derogatory comments, and personal or political attacks
- Public or private harassment
- Publishing others' private information without permission
- Other conduct which could reasonably be considered inappropriate

### Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may be reported to the project maintainers at wildlife-tech@example.com. All complaints will be reviewed and investigated promptly and fairly.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates. When creating a bug report, include as many details as possible:

**Bug Report Template:**

```markdown
**Describe the bug**
A clear description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Go to '...'
2. Click on '...'
3. See error

**Expected behavior**
What you expected to happen.

**Actual behavior**
What actually happened.

**Hardware:**
- Board: [ESP32-CAM AI-Thinker / ESP32-S3]
- SD Card: [Brand, capacity, class]
- Power Supply: [USB / Battery / Solar]
- Additional components: [PIR sensor, etc.]

**Software:**
- Firmware version: [e.g., v0.1.0]
- PlatformIO version: [e.g., 6.1.0]
- Operating system: [e.g., Windows 10, Ubuntu 22.04]

**Serial output**
```
Paste relevant serial monitor output here
```

**Additional context**
Any other context, photos, or screenshots.
```

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion:

**Enhancement Template:**

```markdown
**Is your feature request related to a problem?**
A clear description of the problem. Ex. I'm frustrated when [...]

**Describe the solution you'd like**
A clear description of what you want to happen.

**Describe alternatives you've considered**
Alternative solutions or features you've considered.

**Use case**
How would this feature be used in real-world scenarios?

**Additional context**
Mockups, diagrams, or examples from other projects.
```

### Contributing Code

We welcome code contributions! Areas where help is especially appreciated:

1. **Bug Fixes** - Fix reported issues
2. **New Features** - Implement roadmap items or community requests
3. **Optimization** - Improve performance, memory usage, or power consumption
4. **Hardware Support** - Add support for new sensors or boards
5. **Documentation** - Improve guides, add examples, fix typos

### Contributing Documentation

Documentation improvements are always welcome:

- Fix typos or unclear explanations
- Add missing information
- Create tutorials or guides
- Improve code comments
- Add diagrams or illustrations

### Field Testing

Help test WildCAM_ESP32 in real-world conditions:

- Deploy in different environments (forest, desert, wetlands)
- Test with various hardware configurations
- Document battery life under different usage patterns
- Share wildlife photos and data
- Report compatibility issues

## Getting Started

### Development Environment Setup

1. **Fork the Repository**
   ```bash
   # Click "Fork" on GitHub
   # Then clone your fork
   git clone https://github.com/YOUR_USERNAME/WildCAM_ESP32.git
   cd WildCAM_ESP32
   ```

2. **Install Development Tools**
   ```bash
   # Install PlatformIO
   pip install platformio
   
   # Install Git hooks (optional but recommended)
   # We'll set up pre-commit hooks for code formatting
   ```

3. **Set Up Hardware**
   - ESP32-CAM board
   - USB-to-Serial adapter
   - SD card (for testing)
   - PIR sensor (optional for motion testing)

4. **Build and Test**
   ```bash
   # Build firmware
   pio run -e esp32cam
   
   # Upload to device
   pio run -e esp32cam -t upload
   
   # Monitor serial output
   pio device monitor -b 115200
   ```

### Development Branch Strategy

- `main` - Stable releases only
- `develop` - Integration branch for features
- `feature/*` - Individual feature branches
- `bugfix/*` - Bug fix branches
- `release/*` - Release preparation branches

## Development Workflow

### 1. Create a Branch

```bash
# Update your fork
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
# or for bug fixes
git checkout -b bugfix/issue-number-description
```

### 2. Make Changes

- Write clean, readable code
- Follow existing code style
- Add comments for complex logic
- Update documentation as needed
- Test on real hardware

### 3. Commit Changes

```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "Add feature: brief description

Detailed explanation of what changed and why.
Fixes #issue_number (if applicable)"
```

**Commit Message Guidelines:**
- Use present tense ("Add feature" not "Added feature")
- First line: brief summary (50 chars or less)
- Blank line, then detailed description
- Reference issues with "Fixes #123" or "Relates to #456"

### 4. Push and Create Pull Request

```bash
# Push to your fork
git push origin feature/your-feature-name

# Then create Pull Request on GitHub
```

## Coding Standards

### C++ Code Style

**Naming Conventions:**
```cpp
// Classes: PascalCase
class CameraManager { };

// Functions: camelCase
void captureImage() { }

// Variables: camelCase
int batteryLevel = 0;

// Constants: UPPER_SNAKE_CASE
#define MAX_RETRY_COUNT 3

// Private members: camelCase with trailing underscore (optional)
private:
    int retryCount_;
```

**Code Formatting:**
```cpp
// Indentation: 4 spaces (no tabs)
// Braces: same line for functions, next line for classes

void exampleFunction() {
    if (condition) {
        // Do something
    } else {
        // Do something else
    }
}

class ExampleClass 
{
public:
    ExampleClass();
    void method();
    
private:
    int variable_;
};
```

**Comments:**
```cpp
/**
 * @brief Brief description of function
 * @param paramName Description of parameter
 * @return Description of return value
 * 
 * Detailed explanation if needed.
 */
bool complexFunction(int paramName) {
    // Inline comment for specific line
    int result = paramName * 2;
    
    /* Multi-line comment
       for complex logic */
    return result > 0;
}
```

### Configuration Files

**config.h Guidelines:**
- Group related settings together
- Add descriptive comments for each setting
- Provide sensible defaults
- Include units in variable names or comments
- Use `#ifndef` guards

```cpp
//==============================================================================
// CATEGORY NAME
//==============================================================================

/**
 * @brief Clear description of setting
 * @note Additional notes or warnings
 * @default Default value
 */
#define SETTING_NAME value  // Units if applicable
```

### Error Handling

```cpp
// Always check return values
bool result = functionCall();
if (!result) {
    Serial.println("ERROR: Function failed - reason");
    return false;
}

// Use early returns for error cases
if (!initialized) {
    Serial.println("ERROR: Not initialized");
    return false;
}

// Log errors with context
if (sdCard.mount() != SD_MOUNT_SUCCESS) {
    Serial.printf("ERROR: SD card mount failed - error code: %d\n", errorCode);
    return false;
}
```

### Memory Management

```cpp
// Free camera frame buffers
camera_fb_t* fb = esp_camera_fb_get();
if (fb) {
    // Use frame buffer
    esp_camera_fb_return(fb);  // Always return!
}

// Avoid memory leaks
char* buffer = (char*)malloc(SIZE);
if (buffer) {
    // Use buffer
    free(buffer);  // Always free!
}

// Use stack allocation when possible
char smallBuffer[64];  // Better than malloc for small buffers
```

## Testing Guidelines

### Hardware Testing

Before submitting a PR, test on real hardware:

1. **Basic Functionality**
   - Camera initialization
   - Image capture and saving
   - Motion detection
   - Battery monitoring
   - Web server access

2. **Edge Cases**
   - SD card full
   - Low battery
   - WiFi disconnection
   - Rapid motion triggers
   - Power cycle recovery

3. **Performance**
   - Image capture speed
   - SD write speed
   - Memory usage
   - Power consumption

### Serial Monitor Testing

```bash
# Monitor output and verify:
# - No error messages
# - Proper initialization sequence
# - Expected behavior on motion
# - Correct voltage readings
pio device monitor -b 115200
```

### Integration Testing

If your change affects multiple components:

1. Test complete workflow (motion → capture → save → web view)
2. Verify backward compatibility
3. Check for memory leaks (run for extended periods)
4. Test power cycling and recovery

### Documentation Testing

- Build and verify all code examples
- Test links in documentation
- Verify accuracy of technical information
- Check for typos and grammar

## Documentation

### Code Documentation

- Add header comments to all files
- Document all public functions and classes
- Explain complex algorithms
- Include example usage in comments

### README Updates

When adding features, update relevant sections:
- Features list
- Hardware Requirements (if new hardware)
- Configuration options
- Usage instructions
- Known Issues (if introducing limitations)

### Creating Examples

When contributing examples:

```cpp
/**
 * @file example_name.cpp
 * @brief Brief description of what this example demonstrates
 * 
 * This example shows how to:
 * - Feature 1
 * - Feature 2
 * - Feature 3
 * 
 * Hardware requirements:
 * - List required components
 * 
 * Wiring:
 * - Pin connections
 * 
 * @author Your Name
 * @date YYYY-MM-DD
 */

#include <Arduino.h>
// Include required headers

// Configuration constants
#define EXAMPLE_PIN 13

void setup() {
    // Initialization code with comments
}

void loop() {
    // Main logic with comments
}
```

## Pull Request Process

### Before Submitting

1. **Self Review**
   - Read through all your changes
   - Remove debug code and print statements
   - Check for commented-out code
   - Verify code style consistency

2. **Testing**
   - Test on real ESP32-CAM hardware
   - Verify no regressions in existing features
   - Test edge cases

3. **Documentation**
   - Update relevant documentation
   - Add code comments
   - Update CHANGELOG (for significant changes)

### PR Description Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix (non-breaking change fixing an issue)
- [ ] New feature (non-breaking change adding functionality)
- [ ] Breaking change (fix or feature causing existing functionality to change)
- [ ] Documentation update

## Related Issues
Fixes #(issue number)
Relates to #(issue number)

## Changes Made
- Detailed list of changes
- What was modified and why

## Testing
Describe testing performed:
- Hardware tested on
- Test scenarios covered
- Results

## Screenshots (if applicable)
Add screenshots showing changes

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-reviewed code
- [ ] Commented complex code sections
- [ ] Updated documentation
- [ ] Tested on ESP32-CAM hardware
- [ ] No new warnings or errors
- [ ] Backward compatible (or breaking changes documented)
```

### Review Process

1. **Automated Checks**
   - Build verification
   - Code style checks (if configured)

2. **Maintainer Review**
   - Code quality and style
   - Functionality and correctness
   - Documentation completeness
   - Test coverage

3. **Feedback**
   - Address reviewer comments
   - Make requested changes
   - Re-request review when ready

4. **Merge**
   - Maintainer will merge when approved
   - Your contribution will be in next release!

## Community Guidelines

### Communication

- **Be respectful** - Treat others as you want to be treated
- **Be patient** - Maintainers are volunteers with limited time
- **Be constructive** - Offer solutions, not just criticism
- **Be helpful** - Answer questions from newcomers
- **Be collaborative** - Work together towards common goals

### Discussions

Use GitHub Discussions for:
- Questions about usage
- Feature brainstorming
- Community showcase
- General conversations

Use GitHub Issues for:
- Bug reports
- Specific feature requests
- Documentation improvements

### Getting Help

Stuck? Here's how to get help:

1. **Search existing issues and discussions**
2. **Check documentation** - README, code comments
3. **Ask in Discussions** - Community can help
4. **Create detailed issue** - If bug or feature request

## Recognition

### Contributors

All contributors will be recognized:
- Listed in CONTRIBUTORS.md
- Mentioned in release notes
- Credited in documentation

### Types of Contributions

We value all contributions equally:
- 💻 Code contributions
- 📝 Documentation improvements
- 🐛 Bug reports
- 💡 Feature suggestions
- 🧪 Testing and validation
- 📸 Field deployment data
- 🤝 Helping others in discussions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

## Questions?

If you have questions about contributing:
- Open a [GitHub Discussion](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- Email: wildlife-tech@example.com

**Thank you for contributing to WildCAM_ESP32 and helping protect wildlife through technology!** 🦌🌿
