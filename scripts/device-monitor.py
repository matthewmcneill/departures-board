#!/usr/bin/env python3
import subprocess
import time
import os
import signal
import sys
from datetime import datetime

# Serial Logging Configuration
LOG_DIR = "logs"
LATEST_LOG = os.path.join(LOG_DIR, "latest-monitor.log")
PID_FILE = os.path.join(LOG_DIR, "device-monitor.pid")

def get_env():
    """Detect PlatformIO environment."""
    try:
        # Check for specific boards first
        output = subprocess.check_output(["pio", "device", "list"], text=True)
        if "Arduino Nano ESP32" in output or "2341:0070" in output:
            return "esp32s3nano"
        if "ESP32 Dev Module" in output or "CP210" in output or "CH340" in output:
            return "esp32dev"
        
        # Fallback to checking platformio.ini for default envs
        if os.path.exists("platformio.ini"):
            with open("platformio.ini", "r") as f:
                content = f.read()
                if "default_envs = " in content:
                    envs = content.split("default_envs = ")[1].split("\n")[0]
                    first_env = envs.split(",")[0].strip()
                    return first_env
        
        return "esp32dev"
    except Exception:
        return "esp32dev"

def kill_existing_monitors():
    """Kill any existing pio monitor or miniterm processes."""
    print("[*] Cleaning up existing monitor processes...")
    try:
        # Kill by process name/args
        subprocess.run(["pkill", "-f", "pio device monitor"], stderr=subprocess.DEVNULL)
        subprocess.run(["pkill", "-f", "miniterm"], stderr=subprocess.DEVNULL)
    except Exception:
        pass

def monitor():
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    # Write PID file
    with open(PID_FILE, "w") as f:
        f.write(str(os.getpid()))

    env = get_env()
    print(f"[*] Detected Environment: {env}")
    print(f"[*] Continuous monitoring started (PID: {os.getpid()}).")
    print(f"[*] Press Ctrl+C to stop.")
    
    kill_existing_monitors()
    
    while True:
        # Check for pause lock
        if os.path.exists(os.path.join(LOG_DIR, ".flash-lock")):
            print("\n[*] Serial port locked, waiting for release...")
            while os.path.exists(os.path.join(LOG_DIR, ".flash-lock")):
                time.sleep(0.5)

        timestamp = datetime.now().strftime("%y%m%d-%H%M%S")
        log_file = os.path.join(LOG_DIR, f"device-monitor-{timestamp}.log")
        
        # Build command
        # --raw: avoid control character mangling
        # --no-reconnect: we handle reconnection ourselves in the loop to get new log files
        cmd = ["pio", "device", "monitor", "-e", env, "--raw", "--no-reconnect"]
        
        try:
            print(f"[*] Starting session: {log_file}")
            
            # Open log file and latest log link
            with open(log_file, "w", buffering=1) as f, open(LATEST_LOG, "w") as latest_f:
                # We use subprocess.PIPE to capture output and tee it.
                process = subprocess.Popen(
                    cmd, 
                    stdout=subprocess.PIPE, 
                    stderr=subprocess.STDOUT, 
                    text=True, 
                    bufsize=1,
                    env={**os.environ, "PYTHONUNBUFFERED": "1"}
                )
                
                # Tee output to both stdout and the log file
                try:
                    while True:
                        line = process.stdout.readline()
                        if not line and process.poll() is not None:
                            break
                        if line:
                            sys.stdout.write(line)
                            sys.stdout.flush()
                            f.write(line)
                            latest_f.write(line)
                            latest_f.flush()
                except Exception as e:
                    print(f"\n[!] Read error: {e}")
                finally:
                    if process.poll() is None:
                        process.terminate()
                        process.wait()
                
        except KeyboardInterrupt:
            print("\n[*] Stopping monitor...")
            break
        except Exception as e:
            print(f"\n[!] Error: {e}")
        
        # Wait before retrying (device might be uploading or disconnected)
        # We use a short 1s interval to catch the port as soon as an upload finishes.
        print("\n[*] Port busy or device disconnected. Retrying in 1s...")
        time.sleep(1)

if __name__ == "__main__":
    # Ensure logs directory exists
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    def signal_handler(sig, frame):
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
        print("\n[*] Exiting.")
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    try:
        monitor()
    finally:
        if os.path.exists(PID_FILE):
            os.remove(PID_FILE)
