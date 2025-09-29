#!/usr/bin/env python3
"""
ESP32WildlifeCAM TODO Validation Script
Verifies that critical TODOs mentioned in the development plan are accurately represented
"""

import os
import re
from pathlib import Path

def validate_todo_references():
    """Validate that TODO references in the plan match actual code"""
    repo_root = Path("/home/runner/work/ESP32WildlifeCAM/ESP32WildlifeCAM")
    
    # Critical TODOs mentioned in the plan
    critical_todos = [
        {
            "file": "firmware/src/multi_board/board_node.cpp",
            "line": 596,
            "text": "TODO: Implement data transmission",
            "description": "Data transmission implementation"
        },
        {
            "file": "firmware/src/multi_board/multi_board_system.cpp", 
            "line": 227,
            "text": "TODO: Implement coordinator discovery trigger",
            "description": "Coordinator discovery trigger"
        },
        {
            "file": "src/blockchain/Block.cpp",
            "line": 361,
            "text": "TODO: Add digital signature verification",
            "description": "Digital signature verification"
        },
        {
            "file": "firmware/src/multi_board/discovery_protocol.cpp",
            "line": 287,
            "text": "TODO: Get actual signal strength",
            "description": "Signal strength calculation"
        },
        {
            "file": "firmware/src/multi_board/discovery_protocol.cpp",
            "line": 288,
            "text": "TODO: Calculate hop count", 
            "description": "Hop count calculation"
        }
    ]
    
    print("🔍 ESP32WildlifeCAM TODO Validation")
    print("=" * 50)
    
    validated_count = 0
    total_count = len(critical_todos)
    
    for todo in critical_todos:
        file_path = repo_root / todo["file"]
        
        if not file_path.exists():
            print(f"❌ File not found: {todo['file']}")
            continue
            
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
                
            # Check if the TODO exists around the specified line
            found = False
            search_range = range(max(0, todo["line"] - 5), min(len(lines), todo["line"] + 5))
            
            for line_num in search_range:
                if line_num < len(lines):
                    line_content = lines[line_num].strip()
                    if "TODO" in line_content and any(word in line_content for word in todo["text"].split()[1:3]):
                        print(f"✅ {todo['description']}")
                        print(f"   📁 {todo['file']}:{line_num + 1}")
                        print(f"   💬 {line_content}")
                        validated_count += 1
                        found = True
                        break
                        
            if not found:
                print(f"⚠️  {todo['description']} - TODO not found at expected location")
                print(f"   📁 {todo['file']} around line {todo['line']}")
                
        except Exception as e:
            print(f"❌ Error reading {todo['file']}: {e}")
            
        print()
    
    print("📊 Validation Summary")
    print("-" * 30)
    print(f"TODOs Validated: {validated_count}/{total_count}")
    print(f"Success Rate: {(validated_count/total_count)*100:.1f}%")
    
    if validated_count == total_count:
        print("✅ All critical TODOs validated successfully!")
        return True
    else:
        print("⚠️  Some TODOs need verification")
        return False

def count_all_todos():
    """Count all TODO items in the codebase"""
    repo_root = Path("/home/runner/work/ESP32WildlifeCAM/ESP32WildlifeCAM")
    firmware_src = repo_root / "firmware" / "src"
    src_dir = repo_root / "src"
    
    todo_count = 0
    files_with_todos = []
    
    for directory in [firmware_src, src_dir]:
        if directory.exists():
            for file_path in directory.rglob("*.cpp"):
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        file_todos = len(re.findall(r'TODO[:)]?', content, re.IGNORECASE))
                        if file_todos > 0:
                            todo_count += file_todos
                            files_with_todos.append((str(file_path.relative_to(repo_root)), file_todos))
                except Exception as e:
                    continue
                    
            for file_path in directory.rglob("*.h"):
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        file_todos = len(re.findall(r'TODO[:)]?', content, re.IGNORECASE))
                        if file_todos > 0:
                            todo_count += file_todos
                            files_with_todos.append((str(file_path.relative_to(repo_root)), file_todos))
                except Exception as e:
                    continue
    
    print(f"\n📋 Total TODO Count Analysis")
    print("-" * 30)
    print(f"Total TODOs found: {todo_count}")
    print(f"Files with TODOs: {len(files_with_todos)}")
    
    if files_with_todos:
        print("\n📁 Files with most TODOs:")
        sorted_files = sorted(files_with_todos, key=lambda x: x[1], reverse=True)
        for file_path, count in sorted_files[:5]:
            print(f"   {file_path}: {count} TODOs")
    
    return todo_count

if __name__ == "__main__":
    # Validate critical TODOs
    validation_success = validate_todo_references()
    
    # Count all TODOs 
    total_todos = count_all_todos()
    
    print(f"\n🎯 Summary")
    print("=" * 30)
    print(f"Critical TODOs: {'✅ Validated' if validation_success else '⚠️ Needs Review'}")
    print(f"Total TODOs: {total_todos} items")
    print(f"Plan Accuracy: {'✅ Accurate' if validation_success else '⚠️ Needs Update'}")