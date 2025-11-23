import os
import shutil

def create_jansson_cpp():
    # Create main directory
    os.makedirs("jansson-cpp", exist_ok=True)
    
    # Create subdirectories
    os.makedirs("jansson-cpp/include", exist_ok=True)
    os.makedirs("jansson-cpp/src", exist_ok=True)
    
    # Get all .c and .h files from original src directory
    original_src = "src"
    if not os.path.exists(original_src):
        print(f"Error: {original_src} directory not found!")
        return
    
    # Copy and rename .c files to .cpp
    for file in os.listdir(original_src):
        if file.endswith('.c'):
            src_file = os.path.join(original_src, file)
            dst_file = os.path.join("jansson-cpp/src", file[:-2] + '.cpp')
            shutil.copy2(src_file, dst_file)
            print(f"Copied {src_file} to {dst_file}")
    
    # Copy and rename .h files to .hpp
    for file in os.listdir(original_src):
        if file.endswith('.h'):
            src_file = os.path.join(original_src, file)
            dst_file = os.path.join("jansson-cpp/include", file[:-2] + '.hpp')
            shutil.copy2(src_file, dst_file)
            print(f"Copied {src_file} to {dst_file}")

if __name__ == "__main__":
    create_jansson_cpp()
