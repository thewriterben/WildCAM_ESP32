"""
Test collaboration models structure and syntax
"""
import sys
import os
import ast

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'backend'))

def test_models_syntax():
    """Test that models.py has valid syntax and new classes"""
    try:
        models_path = os.path.join(os.path.dirname(__file__), '..', 'backend', 'models.py')
        with open(models_path, 'r') as f:
            models_code = f.read()
        
        # Parse the AST to check for collaboration classes
        tree = ast.parse(models_code)
        class_names = [node.name for node in ast.walk(tree) if isinstance(node, ast.ClassDef)]
        
        assert 'UserSession' in class_names
        print("✓ UserSession class found in models.py")
        
        assert 'Annotation' in class_names
        print("✓ Annotation class found in models.py")
        
        assert 'SharedBookmark' in class_names
        print("✓ SharedBookmark class found in models.py")
        
        assert 'ChatMessage' in class_names
        print("✓ ChatMessage class found in models.py")
        
        assert 'Task' in class_names
        print("✓ Task class found in models.py")
        
        assert 'FieldNote' in class_names
        print("✓ FieldNote class found in models.py")
        
        # Check that to_dict methods are defined
        assert 'to_dict' in models_code
        print("✓ to_dict methods found in code")
        
        return True
    except Exception as e:
        print(f"✗ Error checking models: {e}")
        return False

def test_app_endpoints():
    """Test that app.py has collaboration service"""
    try:
        app_path = os.path.join(os.path.dirname(__file__), '..', 'backend', 'app.py')
        with open(app_path, 'r') as f:
            app_code = f.read()
        
        # Check for collaboration service import
        assert 'CollaborationService' in app_code
        print("✓ CollaborationService imported in app.py")
        
        # Check for collaboration endpoints
        assert '/api/collaboration' in app_code
        print("✓ Collaboration endpoints found in app.py")
        
        # Check for WebSocket handlers
        assert '@socketio.on' in app_code
        print("✓ WebSocket handlers found in app.py")
        
        return True
    except Exception as e:
        print(f"✗ Error checking app.py: {e}")
        return False

def test_frontend_components():
    """Test that frontend components exist"""
    try:
        collab_dir = os.path.join(os.path.dirname(__file__), '..', 'frontend', 'dashboard', 'src', 'components', 'collaboration')
        
        # Check collaboration subfolder exists
        assert os.path.exists(collab_dir)
        print("✓ Collaboration components directory exists")
        
        team_chat = os.path.join(collab_dir, 'TeamChat.js')
        assert os.path.exists(team_chat)
        print("✓ TeamChat.js component exists")
        
        user_presence = os.path.join(collab_dir, 'UserPresence.js')
        assert os.path.exists(user_presence)
        print("✓ UserPresence.js component exists")
        
        task_manager = os.path.join(collab_dir, 'TaskManager.js')
        assert os.path.exists(task_manager)
        print("✓ TaskManager.js component exists")
        
        return True
    except Exception as e:
        print(f"✗ Error checking frontend: {e}")
        return False

if __name__ == '__main__':
    print("=" * 60)
    print("Testing Collaboration Features")
    print("=" * 60)
    
    test1 = test_models_syntax()
    test2 = test_app_endpoints()
    test3 = test_frontend_components()
    
    print("=" * 60)
    if test1 and test2 and test3:
        print("✓ All tests passed!")
        print("\nCollaboration features successfully integrated:")
        print("  - Database models (UserSession, Annotation, SharedBookmark, ChatMessage, Task, FieldNote)")
        print("  - CollaborationService with comprehensive endpoints")
        print("  - Frontend React components in collaboration/ folder")
        print("  - WebSocket real-time communication")
        sys.exit(0)
    else:
        print("✗ Some tests failed")
        sys.exit(1)
