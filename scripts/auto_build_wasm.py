import os
import sys
import subprocess

def check_and_build():
    try:
        # PlatformIO provides the environment variable for the project directory
        # Normally sys.argv[0] is the script path
        script_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(script_dir)
        
        wasm_output = os.path.join(project_root, "tools", "layoutsim", "web", "dist", "layout_engine.js")
        build_script = os.path.join(project_root, "tools", "layoutsim", "scripts", "build_wasm.py")
        
        watch_dirs = [
            os.path.join(project_root, "modules", "displayManager", "boards"),
            os.path.join(project_root, "modules", "displayManager", "widgets")
        ]
        
        needs_build = not os.path.exists(wasm_output)
        trigger_reason = "Missing JS output"
        
        if not needs_build:
            wasm_mtime = os.path.getmtime(wasm_output)
            
            for watch_dir in watch_dirs:
                if not os.path.exists(watch_dir):
                    continue
                    
                for root, _, files in os.walk(watch_dir):
                    for file in files:
                        if file.endswith((".hpp", ".cpp", ".h", ".c", ".json")):
                            filepath = os.path.join(root, file)
                            if os.path.getmtime(filepath) > wasm_mtime:
                                needs_build = True
                                trigger_reason = f"Modified file: {file}"
                                break
                    if needs_build:
                        break
                if needs_build:
                    break
        
        if needs_build:
            print(f"Triggering WASM rebuild: {trigger_reason}")
            result = subprocess.run([sys.executable, build_script])
            if result.returncode != 0:
                print("WARNING: Layout Simulator WASM build failed. Physical firmware compilation will still proceed.", file=sys.stderr)
                # We do not strictly fail the hardware loop compilation here because they are independent
        else:
            print("Layout Simulator WASM artifact is up-to-date. Skipping rebuild.")
            
    except Exception as e:
        print(f"Failed to execute auto-build script: {e}")

if __name__ == "__main__":
    check_and_build()
