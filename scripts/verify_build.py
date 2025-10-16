#!/usr/bin/env python3
"""
WildCAM ESP32 Build Verification Script
========================================

Comprehensive build verification for PlatformIO-based ESP32 firmware.
Checks code quality, consistency, and builds for multiple board types.

Requirements:
    - Python 3.6+
    - PlatformIO CLI (installed via pip or system package)

Usage:
    python scripts/verify_build.py
    python scripts/verify_build.py --boards esp32cam_advanced
    python scripts/verify_build.py --skip-build
"""

import os
import sys
import re
import subprocess
import json
from pathlib import Path
from typing import List, Dict, Tuple, Set
from dataclasses import dataclass, field
import argparse


@dataclass
class BuildResult:
    """Store build results for a board."""
    board: str
    success: bool
    errors: List[str] = field(default_factory=list)
    warnings: List[str] = field(default_factory=list)
    binary_size: int = 0
    flash_size: int = 0
    build_time: float = 0.0


@dataclass
class VerificationReport:
    """Overall verification report."""
    header_cpp_issues: List[str] = field(default_factory=list)
    undefined_externs: List[str] = field(default_factory=list)
    undocumented_todos: List[str] = field(default_factory=list)
    build_results: List[BuildResult] = field(default_factory=list)
    
    def success(self) -> bool:
        """Check if all verifications passed."""
        return (len(self.header_cpp_issues) == 0 and
                len(self.undefined_externs) == 0 and
                all(br.success for br in self.build_results))


class BuildVerifier:
    """Main build verification class."""
    
    # Board configurations with flash sizes (in bytes)
    BOARD_CONFIGS = {
        'esp32cam_advanced': {
            'name': 'AI-Thinker ESP32-CAM',
            'flash_size': 4 * 1024 * 1024,  # 4MB
            'env_name': 'esp32cam_advanced'
        },
        'esp32s3cam_ai': {
            'name': 'ESP32-S3-CAM',
            'flash_size': 8 * 1024 * 1024,  # 8MB
            'env_name': 'esp32s3cam_ai'
        },
        'ttgo-t-camera-display': {
            'name': 'ESP-EYE (TTGO T-Camera)',
            'flash_size': 4 * 1024 * 1024,  # 4MB
            'env_name': 'ttgo-t-camera-display'
        }
    }
    
    # Header-only patterns (files that don't need .cpp implementations)
    HEADER_ONLY_PATTERNS = [
        r'.*_config\.h$',
        r'config\.h$',
        r'.*_types\.h$',
        r'.*_constants\.h$',
    ]
    
    def __init__(self, firmware_path: Path, repo_root: Path):
        self.firmware_path = firmware_path
        self.repo_root = repo_root
        self.report = VerificationReport()
        
    def run_verification(self, boards: List[str] = None, skip_build: bool = False) -> VerificationReport:
        """Run all verification checks."""
        print("=" * 70)
        print("WildCAM ESP32 Build Verification")
        print("=" * 70)
        print()
        
        # Check 1: Header/CPP consistency
        print("1. Checking header/cpp file consistency...")
        self._check_header_cpp_consistency()
        self._print_results("Header/CPP Consistency", self.report.header_cpp_issues)
        
        # Check 2: Undefined externs
        print("\n2. Checking for undefined extern variables...")
        self._check_undefined_externs()
        self._print_results("Undefined Externs", self.report.undefined_externs)
        
        # Check 3: TODO documentation
        print("\n3. Checking TODO documentation...")
        self._check_todo_documentation()
        self._print_results("Undocumented TODOs", self.report.undocumented_todos)
        
        # Check 4: Multi-board builds
        if not skip_build:
            print("\n4. Building firmware for multiple boards...")
            if boards is None:
                boards = list(self.BOARD_CONFIGS.keys())
            
            for board_key in boards:
                if board_key not in self.BOARD_CONFIGS:
                    print(f"   WARNING: Unknown board '{board_key}', skipping...")
                    continue
                    
                board_config = self.BOARD_CONFIGS[board_key]
                print(f"\n   Building for {board_config['name']}...")
                build_result = self._build_board(board_config)
                self.report.build_results.append(build_result)
                self._print_build_result(build_result)
        else:
            print("\n4. Skipping board builds (--skip-build specified)")
        
        # Print summary
        print("\n" + "=" * 70)
        print("VERIFICATION SUMMARY")
        print("=" * 70)
        self._print_summary()
        
        return self.report
    
    def _check_header_cpp_consistency(self):
        """Check that all header files have corresponding .cpp files."""
        # Find all .h files
        header_files = list(self.firmware_path.rglob("*.h"))
        
        for header_file in header_files:
            # Skip test files
            if 'test' in str(header_file):
                continue
                
            # Check if it's a header-only file
            is_header_only = any(
                re.match(pattern, header_file.name) 
                for pattern in self.HEADER_ONLY_PATTERNS
            )
            
            if is_header_only:
                continue
            
            # Check if it's actually a header-only file by looking for class/struct definitions only
            if self._is_likely_header_only(header_file):
                continue
            
            # Look for corresponding .cpp file
            cpp_file = header_file.with_suffix('.cpp')
            if not cpp_file.exists():
                # Check in common alternative locations
                alt_locations = [
                    header_file.parent / 'src' / (header_file.stem + '.cpp'),
                    header_file.parent.parent / 'src' / (header_file.stem + '.cpp'),
                ]
                
                if not any(alt.exists() for alt in alt_locations):
                    self.report.header_cpp_issues.append(
                        f"Missing .cpp for header: {header_file.relative_to(self.firmware_path)}"
                    )
    
    def _is_likely_header_only(self, header_file: Path) -> bool:
        """Check if a header file is likely header-only (templates, inline functions, etc.)."""
        try:
            content = header_file.read_text(encoding='utf-8', errors='ignore')
            
            # Check for template definitions
            if 'template' in content and re.search(r'template\s*<.*>\s*class', content):
                return True
            
            # Check if all functions are inline or static inline
            function_matches = re.findall(r'\n\s*(?:inline\s+)?(?:static\s+)?(?:\w+\s+)+\w+\s*\([^)]*\)\s*{', content)
            inline_static_matches = re.findall(r'\n\s*(?:inline|static)\s+', content)
            
            # If there are functions but all are inline/static, it's header-only
            if len(function_matches) > 0 and len(inline_static_matches) >= len(function_matches) * 0.8:
                return True
            
            # Check if all methods are defined inside the class (inline by default)
            class_matches = re.findall(r'class\s+\w+\s*{([^}]+)}', content, re.DOTALL)
            if class_matches:
                # If all methods have bodies (inline), it's header-only
                all_inline = True
                for class_body in class_matches:
                    # Look for method declarations without bodies
                    method_decls = re.findall(r'\w+\s+\w+\s*\([^)]*\)\s*;', class_body)
                    # Skip pure virtual and simple constructors/destructors
                    method_decls = [m for m in method_decls if '= 0' not in m and '~' not in m]
                    if method_decls:
                        all_inline = False
                        break
                
                if all_inline and class_matches:
                    return True
            
            # Check for typedef/enum only headers
            has_class_decl = re.search(r'class\s+\w+\s*{', content)
            has_function_decl = re.search(r'\w+\s+\w+\s*\([^)]*\)\s*;', content)
            
            # If no class with methods or function declarations, likely header-only
            if not has_class_decl and not has_function_decl:
                return True
            
            # Remove comments
            stripped = re.sub(r'//.*', '', content)
            stripped = re.sub(r'/\*.*?\*/', '', stripped, flags=re.DOTALL)
            
            # Check if there are only simple inline implementations
            has_non_inline_decl = re.search(
                r'\n(?!.*(?:inline|static|template)).*\w+\s+\w+\s*\([^)]*\)\s*;', 
                stripped
            )
            
            if not has_non_inline_decl:
                return True
                
        except Exception as e:
            # If we can't read the file, assume it needs a .cpp
            pass
        
        return False
    
    def _check_undefined_externs(self):
        """Check for extern variable declarations without definitions."""
        # Find all extern declarations
        externs: Dict[str, List[Tuple[Path, int]]] = {}
        definitions: Set[str] = set()
        
        # Scan all source files
        for source_file in self.firmware_path.rglob("*"):
            if source_file.suffix not in ['.h', '.cpp', '.c']:
                continue
            
            try:
                content = source_file.read_text(encoding='utf-8', errors='ignore')
                lines = content.split('\n')
                
                for line_num, line in enumerate(lines, 1):
                    # Remove comments
                    line_clean = re.sub(r'//.*', '', line)
                    line_clean = re.sub(r'/\*.*?\*/', '', line_clean)
                    
                    # Find extern declarations
                    extern_match = re.match(r'^\s*extern\s+.*?(\w+)\s*;', line_clean)
                    if extern_match:
                        var_name = extern_match.group(1)
                        if var_name not in externs:
                            externs[var_name] = []
                        externs[var_name].append((source_file, line_num))
                    
                    # Find definitions (non-extern global variables)
                    # Look for variable definitions at file scope
                    if not line_clean.strip().startswith('extern'):
                        # Match global variable definitions
                        def_match = re.match(
                            r'^\s*(?:const\s+)?(?:static\s+)?[\w:]+(?:\s*\*|\s+)(\w+)(?:\s*\[.*?\])?\s*[=;]',
                            line_clean
                        )
                        if def_match and not re.match(r'^\s*(?:if|while|for|return)', line_clean):
                            var_name = def_match.group(1)
                            definitions.add(var_name)
                            
            except Exception as e:
                # Skip files we can't read
                pass
        
        # Check for undefined externs
        for var_name, locations in externs.items():
            if var_name not in definitions:
                for source_file, line_num in locations:
                    self.report.undefined_externs.append(
                        f"Undefined extern '{var_name}' at "
                        f"{source_file.relative_to(self.firmware_path)}:{line_num}"
                    )
    
    def _check_todo_documentation(self):
        """Check that all TODO comments are documented."""
        # Look for TODO.md or similar documentation file
        todo_docs = [
            self.repo_root / 'TODO.md',
            self.repo_root / 'TODOS.md',
            self.firmware_path / 'TODO.md',
        ]
        
        documented_todos = set()
        for todo_doc in todo_docs:
            if todo_doc.exists():
                try:
                    content = todo_doc.read_text(encoding='utf-8', errors='ignore')
                    # Extract TODO items from documentation
                    todo_matches = re.findall(r'TODO:?\s*(.*?)(?:\n|$)', content, re.IGNORECASE)
                    documented_todos.update(todo.strip().lower() for todo in todo_matches)
                except Exception:
                    pass
        
        # Find all TODO comments in source code
        todo_pattern = re.compile(r'//\s*TODO:?\s*(.*?)$', re.IGNORECASE)
        
        for source_file in self.firmware_path.rglob("*"):
            if source_file.suffix not in ['.h', '.cpp', '.c', '.ino']:
                continue
            
            try:
                content = source_file.read_text(encoding='utf-8', errors='ignore')
                lines = content.split('\n')
                
                for line_num, line in enumerate(lines, 1):
                    match = todo_pattern.search(line)
                    if match:
                        todo_text = match.group(1).strip().lower()
                        
                        # Check if this TODO is documented
                        is_documented = any(
                            todo_text in doc_todo or doc_todo in todo_text
                            for doc_todo in documented_todos
                        )
                        
                        if not is_documented:
                            self.report.undocumented_todos.append(
                                f"{source_file.relative_to(self.firmware_path)}:{line_num} - {match.group(1).strip()}"
                            )
                            
            except Exception:
                pass
    
    def _build_board(self, board_config: Dict) -> BuildResult:
        """Build firmware for a specific board."""
        import time
        
        result = BuildResult(
            board=board_config['name'],
            success=False,
            flash_size=board_config['flash_size']
        )
        
        env_name = board_config['env_name']
        
        # Check if PlatformIO is installed
        try:
            subprocess.run(['pio', '--version'], capture_output=True, check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            result.errors.append("PlatformIO CLI not found. Install with: pip install platformio")
            return result
        
        # Run the build
        start_time = time.time()
        try:
            process = subprocess.run(
                ['pio', 'run', '-e', env_name, '-d', str(self.firmware_path)],
                capture_output=True,
                text=True,
                timeout=600  # 10 minute timeout
            )
            result.build_time = time.time() - start_time
            
            # Parse output for errors and warnings
            output = process.stdout + process.stderr
            
            # Check for network/dependency errors
            if 'HTTPClientError' in output or 'Could not install package' in output:
                result.errors.append(
                    "Network error: Cannot download PlatformIO dependencies. "
                    "This is expected in sandboxed environments."
                )
                result.warnings.append(
                    "To run builds locally, ensure you have internet access and run: pio pkg install"
                )
            else:
                # Extract compilation errors
                error_matches = re.findall(r'error:.*', output, re.IGNORECASE)
                result.errors.extend(error_matches[:10])  # Limit to first 10 errors
            
            # Extract warnings
            warning_matches = re.findall(r'warning:.*', output, re.IGNORECASE)
            result.warnings.extend(warning_matches[:20])  # Limit to first 20 warnings
            
            # Check if build succeeded
            result.success = (process.returncode == 0 and 
                            ('SUCCESS' in output.upper() or 'SUCCEEDED' in output.upper()))
            
            # Get binary size
            if result.success:
                result.binary_size = self._get_binary_size(env_name)
                
                # Check if binary size exceeds flash size
                if result.binary_size > 0:
                    usage_percent = (result.binary_size / result.flash_size) * 100
                    if usage_percent > 95:
                        result.errors.append(
                            f"Binary size ({result.binary_size} bytes) exceeds 95% of "
                            f"flash capacity ({result.flash_size} bytes)"
                        )
                        result.success = False
                    elif usage_percent > 85:
                        result.warnings.append(
                            f"Binary size ({result.binary_size} bytes) is at "
                            f"{usage_percent:.1f}% of flash capacity"
                        )
                        
        except subprocess.TimeoutExpired:
            result.errors.append("Build timed out after 10 minutes")
        except Exception as e:
            result.errors.append(f"Build failed with exception: {str(e)}")
        
        return result
    
    def _get_binary_size(self, env_name: str) -> int:
        """Get the size of the compiled binary."""
        # Look for .elf file in build directory
        build_dir = self.firmware_path / '.pio' / 'build' / env_name
        if not build_dir.exists():
            return 0
        
        elf_files = list(build_dir.glob('firmware.elf'))
        if not elf_files:
            elf_files = list(build_dir.glob('*.elf'))
        
        if elf_files:
            return elf_files[0].stat().st_size
        
        # Try to find .bin file
        bin_files = list(build_dir.glob('firmware.bin'))
        if not bin_files:
            bin_files = list(build_dir.glob('*.bin'))
            
        if bin_files:
            return bin_files[0].stat().st_size
        
        return 0
    
    def _print_results(self, check_name: str, issues: List[str]):
        """Print results for a verification check."""
        if not issues:
            print(f"   ✓ {check_name}: PASSED")
        else:
            print(f"   ✗ {check_name}: FAILED ({len(issues)} issue(s))")
            for issue in issues[:5]:  # Show first 5 issues
                print(f"     - {issue}")
            if len(issues) > 5:
                print(f"     ... and {len(issues) - 5} more")
    
    def _print_build_result(self, result: BuildResult):
        """Print build result for a board."""
        status = "✓ SUCCESS" if result.success else "✗ FAILED"
        print(f"   {status} - {result.board}")
        print(f"      Build time: {result.build_time:.1f}s")
        
        if result.binary_size > 0:
            usage_percent = (result.binary_size / result.flash_size) * 100
            print(f"      Binary size: {result.binary_size:,} bytes "
                  f"({usage_percent:.1f}% of {result.flash_size:,} bytes)")
        
        if result.warnings:
            print(f"      Warnings: {len(result.warnings)}")
            for warning in result.warnings[:3]:
                print(f"        - {warning}")
        
        if result.errors:
            print(f"      Errors: {len(result.errors)}")
            for error in result.errors[:3]:
                print(f"        - {error}")
    
    def _print_summary(self):
        """Print overall verification summary."""
        total_issues = (
            len(self.report.header_cpp_issues) +
            len(self.report.undefined_externs) +
            len(self.report.undocumented_todos)
        )
        
        print(f"Header/CPP Issues: {len(self.report.header_cpp_issues)}")
        print(f"Undefined Externs: {len(self.report.undefined_externs)}")
        print(f"Undocumented TODOs: {len(self.report.undocumented_todos)}")
        
        if self.report.build_results:
            successful_builds = sum(1 for br in self.report.build_results if br.success)
            total_builds = len(self.report.build_results)
            print(f"Build Results: {successful_builds}/{total_builds} successful")
        
        print()
        if self.report.success():
            print("✓ ALL CHECKS PASSED")
            return 0
        else:
            print("✗ VERIFICATION FAILED")
            return 1


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description='WildCAM ESP32 Build Verification Script',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/verify_build.py
  python scripts/verify_build.py --boards esp32cam_advanced esp32s3cam_ai
  python scripts/verify_build.py --skip-build
        """
    )
    
    parser.add_argument(
        '--boards',
        nargs='+',
        choices=list(BuildVerifier.BOARD_CONFIGS.keys()),
        help='Specific boards to build (default: all boards)'
    )
    
    parser.add_argument(
        '--skip-build',
        action='store_true',
        help='Skip board builds, only run code checks'
    )
    
    parser.add_argument(
        '--firmware-path',
        type=Path,
        help='Path to firmware directory (default: auto-detect)'
    )
    
    args = parser.parse_args()
    
    # Determine paths
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    
    if args.firmware_path:
        firmware_path = args.firmware_path
    else:
        # Try to find firmware directory
        firmware_path = repo_root / 'firmware'
        if not firmware_path.exists():
            firmware_path = repo_root / 'ESP32WildlifeCAM-main' / 'firmware'
        
        if not firmware_path.exists():
            print("ERROR: Could not find firmware directory")
            print(f"Looked in: {repo_root / 'firmware'}")
            return 1
    
    if not firmware_path.exists():
        print(f"ERROR: Firmware path does not exist: {firmware_path}")
        return 1
    
    print(f"Firmware path: {firmware_path}")
    print(f"Repository root: {repo_root}")
    print()
    
    # Run verification
    verifier = BuildVerifier(firmware_path, repo_root)
    report = verifier.run_verification(boards=args.boards, skip_build=args.skip_build)
    
    # Return exit code
    return 0 if report.success() else 1


if __name__ == '__main__':
    sys.exit(main())
