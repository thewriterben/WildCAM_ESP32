# Research Platform Testing Guide

## Overview

This document provides testing guidelines for the WildCAM ESP32 Scientific Research Platform.

## Test Infrastructure

### Prerequisites

```bash
# Install Python dependencies
pip install -r backend/requirements.txt

# Or install testing dependencies separately
pip install flask pytest pytest-cov requests
```

### Running Tests

```bash
# Run all research platform tests
cd /path/to/WildCAM_ESP32
python tests/test_research_platform.py

# Run with pytest (recommended)
pytest tests/test_research_platform.py -v

# Run with coverage
pytest tests/test_research_platform.py --cov=backend/research_platform --cov=backend/research_analytics
```

## Test Categories

### 1. Unit Tests

Tests individual components in isolation:

- **University Registration**: Test university partner registration
- **Researcher Verification**: Test credential verification
- **Project Management**: Test project creation and updates
- **Study Management**: Test longitudinal study operations
- **Data Publication**: Test FAIR dataset publication
- **Analytics**: Test statistical and predictive models

### 2. Integration Tests

Tests component interactions:

- **API Endpoints**: Test REST API endpoints
- **Database Operations**: Test data persistence
- **Authentication**: Test JWT token verification
- **Authorization**: Test role-based access control

### 3. End-to-End Tests

Tests complete workflows:

- **Research Project Workflow**: From registration to publication
- **Data Collection Workflow**: From observation to analysis
- **Collaboration Workflow**: Multi-institutional projects

## Manual Testing Procedures

### Test 1: University Registration

```bash
# Start backend server
cd backend
flask run

# Register university (requires admin token)
curl -X POST http://localhost:5000/api/research/universities \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "stanford",
    "name": "Stanford University",
    "country": "USA",
    "research_domains": ["conservation biology"],
    "contact_email": "research@stanford.edu"
  }'

# Expected: 201 Created with university_id
```

### Test 2: Research Project Creation

```bash
# Create research project
curl -X POST http://localhost:5000/api/research/projects \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "test_project",
    "title": "Tiger Conservation Study",
    "principal_investigator": "researcher_001",
    "universities": ["stanford"],
    "research_type": "longitudinal",
    "species_focus": ["Panthera tigris"]
  }'

# Expected: 201 Created with project_id
```

### Test 3: Longitudinal Study

```bash
# Create study
curl -X POST http://localhost:5000/api/research/studies \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "id": "tiger_study",
    "title": "20-Year Tiger Study",
    "project_id": "test_project",
    "species": "Panthera tigris",
    "planned_duration_years": 20
  }'

# Add observation
curl -X POST http://localhost:5000/api/research/studies/tiger_study/observations \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "observer_id": "researcher_001",
    "data": {"individuals_detected": 5},
    "environmental_conditions": {"temperature": 22.5}
  }'

# Expected: 201 Created
```

### Test 4: FAIR Dataset Publication

```bash
curl -X POST http://localhost:5000/api/research/datasets \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Tiger Camera Trap Dataset",
    "creators": ["researcher_001"],
    "license": "CC-BY-4.0",
    "doi": "10.1234/test",
    "format": "Darwin Core Archive"
  }'

# Expected: 201 Created with dataset_id
```

### Test 5: Conservation Impact Analysis

```bash
curl -X POST http://localhost:5000/api/research/projects/test_project/impact \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "metrics": {
      "population_change": 0.25,
      "habitat_improvement": 0.40,
      "biodiversity_index": 0.35,
      "species_recovery_rate": 0.20
    }
  }'

# Expected: 200 OK with effectiveness_score
```

### Test 6: Ecosystem Analysis

```bash
curl -X POST http://localhost:5000/api/research/ecosystem/food-web \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "ecosystem_id": "test_ecosystem",
    "species_interactions": [
      {
        "predator": "Panthera tigris",
        "prey": "Cervus elaphus",
        "interaction_strength": 0.8
      }
    ]
  }'

# Expected: 200 OK with food web analysis
```

### Test 7: Predictive Modeling

```bash
curl -X POST http://localhost:5000/api/research/prediction/distribution \
  -H "Authorization: Bearer YOUR_RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "species": "Panthera tigris",
    "environmental_data": {
      "temperature_mean": 22.5,
      "precipitation_mm": 2500
    },
    "climate_scenario": "rcp4.5"
  }'

# Expected: 200 OK with distribution prediction
```

## Automated Test Examples

### Python Unit Test Example

```python
import unittest
from research_platform import ResearchPlatformService

class TestResearchPlatform(unittest.TestCase):
    def setUp(self):
        self.service = ResearchPlatformService()
    
    def test_register_university(self):
        data = {
            'id': 'test_uni',
            'name': 'Test University'
        }
        result, status = self.service.register_university(data)
        self.assertEqual(status, 201)
        self.assertIn('university_id', result)

if __name__ == '__main__':
    unittest.main()
```

### Integration Test Example

```python
import requests

def test_research_api_workflow():
    base_url = 'http://localhost:5000'
    
    # Login to get token
    login_response = requests.post(
        f'{base_url}/api/auth/login',
        json={'username': 'researcher', 'password': 'password'}
    )
    token = login_response.json()['access_token']
    
    headers = {'Authorization': f'Bearer {token}'}
    
    # Create project
    project_response = requests.post(
        f'{base_url}/api/research/projects',
        json={'id': 'test', 'title': 'Test Project'},
        headers=headers
    )
    assert project_response.status_code == 201
    
    # Get project
    get_response = requests.get(
        f'{base_url}/api/research/projects/test',
        headers=headers
    )
    assert get_response.status_code == 200
    assert get_response.json()['project']['title'] == 'Test Project'
```

## Test Data

### Sample Universities

```python
SAMPLE_UNIVERSITIES = [
    {
        'id': 'harvard',
        'name': 'Harvard University',
        'country': 'USA'
    },
    {
        'id': 'oxford',
        'name': 'University of Oxford',
        'country': 'UK'
    },
    {
        'id': 'stanford',
        'name': 'Stanford University',
        'country': 'USA'
    }
]
```

### Sample Research Projects

```python
SAMPLE_PROJECTS = [
    {
        'id': 'tiger_study',
        'title': 'Long-term Tiger Population Study',
        'species_focus': ['Panthera tigris'],
        'research_type': 'longitudinal'
    },
    {
        'id': 'bear_behavior',
        'title': 'Bear Behavioral Ecology Study',
        'species_focus': ['Ursus arctos'],
        'research_type': 'behavioral'
    }
]
```

## Performance Testing

### Load Testing

```bash
# Install Apache Bench
apt-get install apache2-utils

# Test endpoint performance
ab -n 1000 -c 10 \
  -H "Authorization: Bearer TOKEN" \
  http://localhost:5000/api/research/statistics

# Expected: < 100ms average response time
```

### Stress Testing

```python
import concurrent.futures
import requests

def create_observation(i):
    response = requests.post(
        'http://localhost:5000/api/research/studies/test/observations',
        json={'observer_id': f'obs_{i}', 'data': {}},
        headers={'Authorization': f'Bearer {TOKEN}'}
    )
    return response.status_code

# Concurrent requests
with concurrent.futures.ThreadPoolExecutor(max_workers=50) as executor:
    futures = [executor.submit(create_observation, i) for i in range(1000)]
    results = [f.result() for f in concurrent.futures.as_completed(futures)]

success_rate = sum(1 for r in results if r == 201) / len(results)
print(f"Success rate: {success_rate * 100}%")
```

## Security Testing

### Authentication Tests

```bash
# Test without token (should fail)
curl -X POST http://localhost:5000/api/research/projects \
  -H "Content-Type: application/json" \
  -d '{"title": "Test"}'

# Expected: 401 Unauthorized

# Test with invalid token (should fail)
curl -X POST http://localhost:5000/api/research/projects \
  -H "Authorization: Bearer INVALID_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"title": "Test"}'

# Expected: 401 Unauthorized
```

### Authorization Tests

```bash
# Test researcher accessing admin endpoint (should fail)
curl -X POST http://localhost:5000/api/research/universities \
  -H "Authorization: Bearer RESEARCHER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"id": "test"}'

# Expected: 403 Forbidden
```

## Continuous Integration

### GitHub Actions Workflow

```yaml
name: Research Platform Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Set up Python
      uses: actions/setup-python@v2
      with:
        python-version: '3.12'
    
    - name: Install dependencies
      run: |
        pip install -r backend/requirements.txt
        pip install pytest pytest-cov
    
    - name: Run tests
      run: |
        pytest tests/test_research_platform.py -v --cov=backend
    
    - name: Upload coverage
      uses: codecov/codecov-action@v2
```

## Test Coverage Goals

- **Unit Tests**: > 80% code coverage
- **Integration Tests**: All API endpoints
- **End-to-End Tests**: Critical workflows
- **Performance Tests**: Key endpoints < 200ms

## Reporting

### Test Results

Tests generate reports in:
- `test_results.xml` (JUnit format)
- `coverage.html` (HTML coverage report)
- `test_report.json` (JSON summary)

### Viewing Coverage

```bash
# Generate HTML coverage report
pytest tests/test_research_platform.py --cov=backend --cov-report=html

# Open in browser
open htmlcov/index.html
```

## Troubleshooting

### Common Issues

1. **Module Not Found**: Install dependencies with `pip install -r requirements.txt`
2. **Authentication Failure**: Ensure valid JWT token
3. **Database Error**: Check database connection string
4. **Permission Denied**: Verify user role has appropriate permissions

### Debug Mode

```python
# Enable debug logging
import logging
logging.basicConfig(level=logging.DEBUG)

# Run tests with verbose output
pytest tests/test_research_platform.py -vv -s
```

## Best Practices

1. **Isolation**: Each test should be independent
2. **Cleanup**: Always clean up test data
3. **Fixtures**: Use fixtures for common setup
4. **Mocking**: Mock external dependencies
5. **Documentation**: Document test purpose and expectations
6. **Assertions**: Use descriptive assertion messages
7. **Coverage**: Aim for comprehensive coverage

## Resources

- [Flask Testing Documentation](https://flask.palletsprojects.com/en/2.3.x/testing/)
- [pytest Documentation](https://docs.pytest.org/)
- [unittest Documentation](https://docs.python.org/3/library/unittest.html)
- [API Testing Best Practices](https://swagger.io/blog/api-testing/)

## Support

For testing issues or questions:
- Email: testing@wildcam.org
- Slack: #research-platform-testing
- Issue Tracker: https://github.com/thewriterben/WildCAM_ESP32/issues
