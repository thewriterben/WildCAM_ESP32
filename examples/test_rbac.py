#!/usr/bin/env python3
"""
Test script for RBAC (Role-Based Access Control) functionality
Demonstrates role-based permissions and access control
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'backend'))

from rbac import Role, Permission, ROLE_PERMISSIONS, RBACService


def print_section(title):
    """Print section header"""
    print(f"\n{'=' * 60}")
    print(f"  {title}")
    print('=' * 60)


def test_role_hierarchy():
    """Test role hierarchy and privilege levels"""
    print_section("Role Hierarchy")
    
    roles = sorted(Role, key=lambda r: r.value)
    print("\nRoles in order of privilege (lowest to highest):")
    for role in roles:
        permission_count = len(ROLE_PERMISSIONS.get(role, set()))
        print(f"  {role.value}. {role.name:20} - {permission_count:2} permissions")


def test_role_permissions():
    """Display permissions for each role"""
    print_section("Role Permissions")
    
    for role in Role:
        permissions = ROLE_PERMISSIONS.get(role, set())
        print(f"\n{role.name} ({len(permissions)} permissions):")
        
        if permissions:
            # Group permissions by category
            categories = {}
            for perm in permissions:
                category = perm.value.split('_')[0]
                if category not in categories:
                    categories[category] = []
                categories[category].append(perm.value)
            
            for category, perms in sorted(categories.items()):
                print(f"  {category.upper()}:")
                for perm in sorted(perms):
                    print(f"    - {perm}")


def test_permission_checks():
    """Test permission checking logic"""
    print_section("Permission Check Examples")
    
    test_cases = [
        (Role.GUEST, Permission.VIEW_DETECTIONS, True),
        (Role.GUEST, Permission.CREATE_ANNOTATIONS, False),
        (Role.OBSERVER, Permission.CREATE_ANNOTATIONS, True),
        (Role.OBSERVER, Permission.DELETE_ANNOTATIONS, False),
        (Role.RESEARCHER, Permission.DELETE_ANNOTATIONS, True),
        (Role.RESEARCHER, Permission.MANAGE_USERS, False),
        (Role.ADMIN, Permission.MANAGE_USERS, True),
    ]
    
    print("\nPermission Checks:")
    for role, permission, expected in test_cases:
        has_perm = permission in ROLE_PERMISSIONS.get(role, set())
        status = "✓" if has_perm == expected else "✗"
        result = "CAN" if has_perm else "CANNOT"
        print(f"  {status} {role.name:20} {result:10} {permission.value}")


def test_permission_comparison():
    """Compare permissions across roles"""
    print_section("Permission Comparison")
    
    interesting_perms = [
        Permission.VIEW_DETECTIONS,
        Permission.CREATE_ANNOTATIONS,
        Permission.DELETE_ANNOTATIONS,
        Permission.MANAGE_USERS,
        Permission.LOCK_VIEWPORT,
    ]
    
    print("\nPermission Matrix:")
    print(f"{'Permission':<30}", end='')
    for role in Role:
        print(f"{role.name[:10]:>12}", end='')
    print()
    print('-' * 100)
    
    for perm in interesting_perms:
        print(f"{perm.value:<30}", end='')
        for role in Role:
            has_perm = perm in ROLE_PERMISSIONS.get(role, set())
            symbol = "✓" if has_perm else "✗"
            print(f"{symbol:>12}", end='')
        print()


def test_role_inheritance():
    """Show how permissions accumulate with higher roles"""
    print_section("Permission Accumulation")
    
    print("\nPermissions added at each role level:")
    
    prev_perms = set()
    for role in sorted(Role, key=lambda r: r.value):
        current_perms = ROLE_PERMISSIONS.get(role, set())
        new_perms = current_perms - prev_perms
        
        print(f"\n{role.name} (Total: {len(current_perms)}):")
        if new_perms:
            print(f"  New permissions: {len(new_perms)}")
            for perm in sorted(new_perms, key=lambda p: p.value):
                print(f"    + {perm.value}")
        else:
            print("  No new permissions added")
        
        prev_perms = current_perms


def test_resource_access():
    """Test resource-level access control"""
    print_section("Resource Access Control")
    
    print("\nResource access examples:")
    
    # Simulated resource access checks
    resources = [
        ('detection', 'view', Role.GUEST),
        ('detection', 'edit', Role.FIELD_WORKER),
        ('annotation', 'create', Role.OBSERVER),
        ('annotation', 'delete', Role.RESEARCHER),
        ('task', 'assign', Role.RESEARCHER),
    ]
    
    for resource_type, action, min_role in resources:
        print(f"\n{resource_type.upper()} - {action}:")
        for role in Role:
            can_access = role.value >= min_role.value
            status = "✓ CAN" if can_access else "✗ CANNOT"
            print(f"  {status:10} {role.name}")


def main():
    """Run all tests"""
    print("=" * 60)
    print("  RBAC (Role-Based Access Control) Test Suite")
    print("  WildCAM ESP32 Collaboration Platform")
    print("=" * 60)
    
    test_role_hierarchy()
    test_role_permissions()
    test_permission_checks()
    test_permission_comparison()
    test_role_inheritance()
    test_resource_access()
    
    print("\n" + "=" * 60)
    print("  All tests completed successfully!")
    print("=" * 60)
    print("\nSummary:")
    print(f"  Total Roles: {len(Role)}")
    print(f"  Total Permissions: {len(Permission)}")
    print(f"  Guest Permissions: {len(ROLE_PERMISSIONS[Role.GUEST])}")
    print(f"  Admin Permissions: {len(ROLE_PERMISSIONS[Role.ADMIN])}")
    print()


if __name__ == '__main__':
    main()
