#!/usr/bin/env python3
"""
Collaboration Features Demo
Demonstrates the usage of collaboration API endpoints
"""

import requests
import json
from datetime import datetime, timedelta

# Configuration
API_BASE = 'http://localhost:5000/api'
USERNAME = 'demo_user'
PASSWORD = 'demo_password'

class CollaborationDemo:
    def __init__(self):
        self.token = None
        self.user_id = None
        
    def login(self):
        """Login and get JWT token"""
        print("🔐 Logging in...")
        response = requests.post(f'{API_BASE}/auth/login', json={
            'username': USERNAME,
            'password': PASSWORD
        })
        
        if response.status_code == 200:
            data = response.json()
            self.token = data['access_token']
            self.user_id = data['user']['id']
            print(f"✅ Logged in as {data['user']['username']}")
            return True
        else:
            print(f"❌ Login failed: {response.text}")
            return False
    
    def get_headers(self):
        """Get auth headers"""
        return {'Authorization': f'Bearer {self.token}'}
    
    def demo_active_users(self):
        """Demonstrate active users endpoint"""
        print("\n👥 Getting Active Users...")
        response = requests.get(
            f'{API_BASE}/collaboration/users/active',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            data = response.json()
            print(f"✅ Found {data['count']} active users:")
            for user in data['active_users']:
                print(f"  - {user['username']} ({user['role']}) on {user.get('current_page', 'unknown page')}")
        else:
            print(f"❌ Failed: {response.text}")
    
    def demo_chat(self):
        """Demonstrate chat functionality"""
        print("\n💬 Testing Chat...")
        
        # Send a message
        print("Sending message to general channel...")
        response = requests.post(
            f'{API_BASE}/collaboration/chat',
            json={
                'channel': 'general',
                'message': 'Hello team! This is a demo message from the collaboration system.'
            },
            headers=self.get_headers()
        )
        
        if response.status_code == 201:
            print("✅ Message sent successfully")
            msg_data = response.json()['message']
            print(f"  Message ID: {msg_data['id']}")
        else:
            print(f"❌ Failed to send message: {response.text}")
        
        # Get messages
        print("\nFetching chat history...")
        response = requests.get(
            f'{API_BASE}/collaboration/chat?channel=general&limit=10',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            messages = response.json()['messages']
            print(f"✅ Retrieved {len(messages)} messages:")
            for msg in messages[-3:]:  # Show last 3
                print(f"  [{msg['created_at']}] {msg.get('user', {}).get('username', 'Unknown')}: {msg['message']}")
        else:
            print(f"❌ Failed to get messages: {response.text}")
    
    def demo_annotations(self, detection_id=1):
        """Demonstrate annotation functionality"""
        print(f"\n📝 Testing Annotations on Detection #{detection_id}...")
        
        # Create annotation
        print("Creating annotation...")
        response = requests.post(
            f'{API_BASE}/collaboration/annotations',
            json={
                'detection_id': detection_id,
                'annotation_type': 'marker',
                'content': 'Notice the distinctive stripe pattern here',
                'position': {'x': 250, 'y': 180},
                'metadata': {'color': '#ff0000', 'size': 'medium'}
            },
            headers=self.get_headers()
        )
        
        if response.status_code == 201:
            print("✅ Annotation created successfully")
            ann_data = response.json()['annotation']
            print(f"  Annotation ID: {ann_data['id']}")
            print(f"  Type: {ann_data['annotation_type']}")
        else:
            print(f"❌ Failed to create annotation: {response.text}")
        
        # Get annotations
        print("\nFetching annotations...")
        response = requests.get(
            f'{API_BASE}/collaboration/annotations?detection_id={detection_id}',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            annotations = response.json()['annotations']
            print(f"✅ Retrieved {len(annotations)} annotations:")
            for ann in annotations:
                print(f"  - {ann['annotation_type']}: {ann['content']}")
        else:
            print(f"❌ Failed to get annotations: {response.text}")
    
    def demo_bookmarks(self):
        """Demonstrate bookmark functionality"""
        print("\n🔖 Testing Bookmarks...")
        
        # Create bookmark
        print("Creating shared bookmark...")
        response = requests.post(
            f'{API_BASE}/collaboration/bookmarks',
            json={
                'title': 'Rare Tiger Sighting',
                'description': 'First tiger sighting in this region in 5 years',
                'detection_id': 1,
                'tags': ['tiger', 'rare', 'important'],
                'is_shared': True,
                'shared_with': ['all']
            },
            headers=self.get_headers()
        )
        
        if response.status_code == 201:
            print("✅ Bookmark created successfully")
            bm_data = response.json()['bookmark']
            print(f"  Bookmark ID: {bm_data['id']}")
            print(f"  Title: {bm_data['title']}")
        else:
            print(f"❌ Failed to create bookmark: {response.text}")
        
        # Get bookmarks
        print("\nFetching bookmarks...")
        response = requests.get(
            f'{API_BASE}/collaboration/bookmarks',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            bookmarks = response.json()['bookmarks']
            print(f"✅ Retrieved {len(bookmarks)} bookmarks:")
            for bm in bookmarks:
                print(f"  - {bm['title']} (tags: {', '.join(bm.get('tags', []))})")
        else:
            print(f"❌ Failed to get bookmarks: {response.text}")
    
    def demo_tasks(self):
        """Demonstrate task management"""
        print("\n📋 Testing Task Management...")
        
        # Create task
        print("Creating task...")
        due_date = (datetime.now() + timedelta(days=7)).isoformat()
        response = requests.post(
            f'{API_BASE}/collaboration/tasks',
            json={
                'title': 'Check Camera Battery Levels',
                'description': 'Inspect all cameras in Zone A and replace low batteries',
                'task_type': 'maintenance',
                'priority': 'high',
                'due_date': due_date
            },
            headers=self.get_headers()
        )
        
        if response.status_code == 201:
            print("✅ Task created successfully")
            task_data = response.json()['task']
            print(f"  Task ID: {task_data['id']}")
            print(f"  Title: {task_data['title']}")
            print(f"  Priority: {task_data['priority']}")
            task_id = task_data['id']
            
            # Update task status
            print("\nUpdating task status to 'in_progress'...")
            response = requests.patch(
                f'{API_BASE}/collaboration/tasks/{task_id}',
                json={'status': 'in_progress'},
                headers=self.get_headers()
            )
            
            if response.status_code == 200:
                print("✅ Task status updated")
            else:
                print(f"❌ Failed to update task: {response.text}")
        else:
            print(f"❌ Failed to create task: {response.text}")
        
        # Get tasks
        print("\nFetching tasks...")
        response = requests.get(
            f'{API_BASE}/collaboration/tasks',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            tasks = response.json()['tasks']
            print(f"✅ Retrieved {len(tasks)} tasks:")
            for task in tasks[:3]:  # Show first 3
                print(f"  - [{task['status']}] {task['title']} (priority: {task['priority']})")
        else:
            print(f"❌ Failed to get tasks: {response.text}")
    
    def demo_field_notes(self):
        """Demonstrate field notes"""
        print("\n📓 Testing Field Notes...")
        
        # Create field note
        print("Creating field note...")
        response = requests.post(
            f'{API_BASE}/collaboration/field-notes',
            json={
                'title': 'Unusual Animal Behavior',
                'content': 'Observed a group of deer displaying unusual feeding patterns. ' +
                          'They appeared to be feeding during midday, which is uncommon for this species. ' +
                          'Weather conditions: Clear, 25°C. Further monitoring recommended.',
                'note_type': 'observation',
                'camera_id': 1,
                'tags': ['behavior', 'deer', 'monitoring']
            },
            headers=self.get_headers()
        )
        
        if response.status_code == 201:
            print("✅ Field note created successfully")
            note_data = response.json()['note']
            print(f"  Note ID: {note_data['id']}")
            print(f"  Title: {note_data['title']}")
        else:
            print(f"❌ Failed to create field note: {response.text}")
        
        # Get field notes
        print("\nFetching field notes...")
        response = requests.get(
            f'{API_BASE}/collaboration/field-notes',
            headers=self.get_headers()
        )
        
        if response.status_code == 200:
            notes = response.json()['notes']
            print(f"✅ Retrieved {len(notes)} field notes:")
            for note in notes[:2]:  # Show first 2
                print(f"  - {note['title']} ({note.get('note_type', 'general')})")
                print(f"    {note['content'][:100]}...")
        else:
            print(f"❌ Failed to get field notes: {response.text}")
    
    def run_demo(self):
        """Run all demos"""
        print("=" * 60)
        print("WildCAM Collaboration Features Demo")
        print("=" * 60)
        
        if not self.login():
            print("\n❌ Demo aborted: Login failed")
            print("\nℹ️  Make sure:")
            print("  1. Backend server is running (python app.py)")
            print("  2. Database is set up with collaboration tables")
            print("  3. Demo user exists (or change USERNAME/PASSWORD)")
            return
        
        try:
            self.demo_active_users()
            self.demo_chat()
            self.demo_annotations()
            self.demo_bookmarks()
            self.demo_tasks()
            self.demo_field_notes()
            
            print("\n" + "=" * 60)
            print("✅ Demo completed successfully!")
            print("=" * 60)
            
        except Exception as e:
            print(f"\n❌ Demo error: {e}")
            import traceback
            traceback.print_exc()

if __name__ == '__main__':
    demo = CollaborationDemo()
    demo.run_demo()
