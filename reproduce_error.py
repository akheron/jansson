#!/usr/bin/env python3
import os
import subprocess
import shutil

def run_command(cmd, cwd=None):
    print(f"Running: {cmd}")
    result = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True)
    print(f"Return code: {result.returncode}")
    if result.stdout:
        print(f"STDOUT:\n{result.stdout}")
    if result.stderr:
        print(f"STDERR:\n{result.stderr}")
    return result.returncode == 0

def main():
    # Clean up any previous build
    if os.path.exists("build"):
        shutil.rmtree("build")
    
    # Create build directory
    os.makedirs("build")
    
    # Try to configure with cmake
    if not run_command("cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DJANSSON_BUILD_DOCS=OFF", cwd="build"):
        print("CMake configuration failed!")
        return False
    
    # Try to build with ninja
    if not run_command("ninja", cwd="build"):
        print("Build failed!")
        return False
    
    print("Success!")
    return True

if __name__ == "__main__":
    main()
