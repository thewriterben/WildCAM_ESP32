# Build Verification Integration Guide

This guide shows how to integrate the build verification script into your development workflow.

## Quick Start

### 1. Local Development

Run before each commit:

```bash
python scripts/verify_build.py --skip-build
```

This performs quick code quality checks without building.

### 2. Pre-Commit Hook

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash
set -e

echo "Running build verification..."
python scripts/verify_build.py --skip-build

if [ $? -ne 0 ]; then
    echo "❌ Build verification failed!"
    echo "Please fix the issues above before committing."
    exit 1
fi

echo "✅ Build verification passed!"
```

Make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

### 3. Full Build Verification

Before pushing to main branch:

```bash
python scripts/verify_build.py
```

This builds all board types and performs all checks.

### 4. Specific Board Testing

Test a single board:

```bash
python scripts/verify_build.py --boards esp32cam_advanced
```

## CI/CD Integration

### GitHub Actions

See `.github/workflows/build-verification-example.yml` for a complete example.

Key sections:

```yaml
- name: Install dependencies
  run: |
    pip install platformio

- name: Run verification
  run: |
    python scripts/verify_build.py
```

### GitLab CI

Create `.gitlab-ci.yml`:

```yaml
stages:
  - verify
  - build

code-quality:
  stage: verify
  image: python:3.9
  script:
    - python scripts/verify_build.py --skip-build
  allow_failure: false

build-firmware:
  stage: build
  image: python:3.9
  before_script:
    - pip install platformio
  script:
    - python scripts/verify_build.py
  artifacts:
    paths:
      - firmware/.pio/build/*/firmware.bin
    expire_in: 1 week
```

### Jenkins

Create `Jenkinsfile`:

```groovy
pipeline {
    agent any
    
    stages {
        stage('Code Quality') {
            steps {
                sh 'python scripts/verify_build.py --skip-build'
            }
        }
        
        stage('Build Firmware') {
            steps {
                sh 'pip install platformio'
                sh 'python scripts/verify_build.py'
            }
        }
    }
    
    post {
        always {
            archiveArtifacts artifacts: 'firmware/.pio/build/*/firmware.bin', 
                           allowEmptyArchive: true
        }
    }
}
```

## Development Workflow

### Recommended Workflow

1. **Write code** - Make your changes
2. **Quick check** - `python scripts/verify_build.py --skip-build`
3. **Fix issues** - Address any reported problems
4. **Full verification** - `python scripts/verify_build.py` (before PR)
5. **Commit** - Git commit (pre-commit hook runs automatically)
6. **Push** - CI pipeline runs full verification

### When to Use Each Mode

| Scenario | Command | Time | Use Case |
|----------|---------|------|----------|
| During development | `--skip-build` | ~10s | Quick feedback loop |
| Before commit | `--skip-build` | ~10s | Pre-commit check |
| Before PR | Full build | ~5-15min | Comprehensive validation |
| In CI pipeline | Full build | ~5-15min | Automated verification |

## Handling Failures

### Header/CPP Issues

If you see:
```
Missing .cpp for header: src/mymodule/myclass.h
```

**Solutions:**
1. Create the corresponding `.cpp` file if the class has implementations
2. If it's header-only, ensure all methods are inline or defined in the header
3. Add to header-only patterns if it's a config/types header

### Undefined Externs

If you see:
```
Undefined extern 'g_myvar' at src/globals.h:42
```

**Solutions:**
1. Add definition in a `.cpp` file: `MyType g_myvar;`
2. Or remove the extern if it's not needed
3. Check for typos in variable names

### Undocumented TODOs

If you see:
```
main.cpp:123 - Implement feature X
```

**Solutions:**
1. Create/update `TODO.md` with the TODO item
2. Or complete the TODO and remove the comment
3. Or add GitHub issue reference in the comment

### Build Failures

If build fails with errors:

1. **Check compilation errors** - Fix syntax/logic errors
2. **Check binary size** - Optimize if too large
3. **Check dependencies** - Ensure PlatformIO can access packages

### Network Errors

In sandboxed/offline environments:

```bash
# Use skip-build for code checks only
python scripts/verify_build.py --skip-build
```

## Customization

### Add New Board

Edit `scripts/verify_build.py`:

```python
BOARD_CONFIGS = {
    'my_new_board': {
        'name': 'My New Board Name',
        'flash_size': 4 * 1024 * 1024,  # 4MB
        'env_name': 'my_platformio_env'
    }
}
```

### Adjust Thresholds

Edit `scripts/verify_build.py`:

```python
# Binary size thresholds
if usage_percent > 95:  # Fail threshold
    result.errors.append(...)
elif usage_percent > 85:  # Warning threshold
    result.warnings.append(...)
```

### Custom Header Patterns

Add patterns for header-only files:

```python
HEADER_ONLY_PATTERNS = [
    r'.*_config\.h$',
    r'.*_types\.h$',
    r'my_custom_pattern\.h$',
]
```

## Best Practices

### DO

✅ Run `--skip-build` frequently during development  
✅ Run full builds before submitting PRs  
✅ Document all TODOs in `TODO.md` or GitHub issues  
✅ Keep binary sizes under 85% of flash capacity  
✅ Define all extern variables  
✅ Create `.cpp` files for non-trivial headers  

### DON'T

❌ Skip verification before commits  
❌ Ignore undefined extern warnings  
❌ Let binary size exceed 95% of flash  
❌ Leave undocumented TODOs in production code  
❌ Commit code that fails verification  

## Troubleshooting

### Script Not Found

```bash
# Ensure you're in repository root
cd /path/to/WildCAM_ESP32
python scripts/verify_build.py --skip-build
```

### Permission Denied

```bash
# Make script executable
chmod +x scripts/verify_build.py
```

### PlatformIO Not Found

```bash
# Install PlatformIO
pip install platformio

# Or use system package manager
sudo apt install python3-platformio  # Ubuntu/Debian
brew install platformio               # macOS
```

### Import Errors

```bash
# Ensure Python 3.6+
python3 --version

# Check if script is valid Python
python3 -m py_compile scripts/verify_build.py
```

## Support

For issues or questions:
1. Check this guide
2. Read `scripts/README_VERIFY_BUILD.md`
3. Open a GitHub issue
4. Contact the development team

## Version History

- **v1.0** - Initial release with all core features
  - Header/CPP consistency checking
  - Extern variable verification
  - Multi-board build support
  - Binary size validation
  - TODO documentation tracking
