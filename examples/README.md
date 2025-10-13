# Collaboration Features Examples

This directory contains examples demonstrating the real-time collaboration features of the WildCAM ESP32 system.

## Files

- **collaboration_demo.py**: Complete Python script demonstrating all collaboration API endpoints

## Running the Demo

### Prerequisites

1. Backend server running on `http://localhost:5000`
2. Database set up with collaboration tables
3. Valid user credentials

### Setup

```bash
# 1. Create database tables
cd ../backend
python -c "from app import app, db; app.app_context().push(); db.create_all()"

# Or use the SQL migration script
psql -U wildlife_user -d wildlife_db -f migrations/create_collaboration_tables.sql

# 2. Create a demo user (if not exists)
python -c "
from app import app, db
from models import User
with app.app_context():
    user = User.query.filter_by(username='demo_user').first()
    if not user:
        user = User(
            username='demo_user',
            email='demo@example.com',
            role='researcher'
        )
        user.set_password('demo_password')
        db.session.add(user)
        db.session.commit()
        print('Demo user created')
    else:
        print('Demo user already exists')
"
```

### Run the Demo

```bash
cd examples
python collaboration_demo.py
```

### Expected Output

```
============================================================
WildCAM Collaboration Features Demo
============================================================
🔐 Logging in...
✅ Logged in as demo_user

👥 Getting Active Users...
✅ Found 1 active users:
  - demo_user (researcher) on unknown page

💬 Testing Chat...
Sending message to general channel...
✅ Message sent successfully
  Message ID: 1

📝 Testing Annotations on Detection #1...
Creating annotation...
✅ Annotation created successfully
  Annotation ID: 1
  Type: marker

🔖 Testing Bookmarks...
Creating shared bookmark...
✅ Bookmark created successfully
  Bookmark ID: 1
  Title: Rare Tiger Sighting

📋 Testing Task Management...
Creating task...
✅ Task created successfully
  Task ID: 1
  Title: Check Camera Battery Levels
  Priority: high

📓 Testing Field Notes...
Creating field note...
✅ Field note created successfully
  Note ID: 1
  Title: Unusual Animal Behavior

============================================================
✅ Demo completed successfully!
============================================================
```

## Customization

Edit the script to change:

```python
# Configuration at the top of collaboration_demo.py
API_BASE = 'http://localhost:5000/api'  # Your API URL
USERNAME = 'your_username'              # Your username
PASSWORD = 'your_password'              # Your password
```

## Using in Your Own Code

The demo script shows exactly how to use each API endpoint. You can copy and adapt the methods for your own applications:

```python
# Example: Create a task programmatically
import requests

API_BASE = 'http://localhost:5000/api'
token = 'your_jwt_token_here'

response = requests.post(
    f'{API_BASE}/collaboration/tasks',
    json={
        'title': 'Camera Maintenance',
        'description': 'Check and clean Camera #5',
        'task_type': 'maintenance',
        'priority': 'high',
        'assigned_to': 42  # User ID
    },
    headers={'Authorization': f'Bearer {token}'}
)

if response.status_code == 201:
    task = response.json()['task']
    print(f"Task created: {task['id']}")
```

## Frontend Integration

See the React components in `frontend/dashboard/src/components/collaboration/` for UI examples.

## Troubleshooting

### "Login failed"

- Verify the backend server is running
- Check the username and password in the script
- Ensure the user exists in the database

### "Connection refused"

- Backend server not running
- Wrong API_BASE URL
- Port blocked by firewall

### Database Errors

- Run the migration script to create tables
- Check database connection string in backend config
- Verify PostgreSQL is running

## Next Steps

1. Explore the documentation in `/docs/COLLABORATION_FEATURES.md`
2. Read the integration guide in `/docs/COLLABORATION_INTEGRATION_GUIDE.md`
3. Try the React components in the frontend dashboard
4. Extend the features for your specific needs

## Support

For issues or questions:
1. Check the documentation files
2. Review the API code in `backend/app.py`
3. Test with this demo script to isolate problems
4. Open an issue on GitHub with details
