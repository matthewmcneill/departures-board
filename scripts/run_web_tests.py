import os
import subprocess
import sys

# PlatformIO 'env' instance is globally available when run as an extra_script
try:
    from SCons.Script import Import
    Import("env")
except ImportError:
    class MockEnv:
        def get(self, key, default):
            return os.environ.get(key, default)
    env = MockEnv()

PROJECT_DIR = env.get("PROJECT_DIR", os.getcwd())
TEST_WEB_DIR = os.path.join(PROJECT_DIR, "test", "web")

def run_web_tests():
    # Check if we should skip tests
    if os.environ.get("SKIP_WEB_TESTS"):
        print("SKIP_WEB_TESTS is set. Skipping web portal tests.")
        return

    # Check if we are in a CI environment (optional logic)
    # is_ci = os.environ.get("CI") or os.environ.get("GITHUB_ACTIONS")

    if not os.path.exists(TEST_WEB_DIR):
        print(f"Notice: Web test directory {TEST_WEB_DIR} not found. Skipping.")
        return

    print("--- Running Web Portal Automated Tests ---")
    
    # Check if node_modules exists, if not, skip with warning or run npm install
    if not os.path.exists(os.path.join(TEST_WEB_DIR, "node_modules")):
        print("Warning: node_modules not found in test/web. Run 'npm install' in that directory.")
        # We could run npm install here, but it might be too slow for a build script
        return

    try:
        # Run playwright tests
        # We only run the local mocked tests by default during build
        cmd = ["npx", "playwright", "test"]
        result = subprocess.run(cmd, cwd=TEST_WEB_DIR, text=True, capture_output=True)
        
        if result.returncode == 0:
            print("Web Portal Tests: PASSED")
        else:
            print("Web Portal Tests: FAILED")
            print(result.stdout)
            print(result.stderr)
            # Stop the build if tests fail
            sys.exit(1)
            
    except Exception as e:
        print(f"Error running web tests: {e}")
        # Don't necessarily stop the build for script errors, but maybe we should
        pass

if __name__ == "__main__":
    run_web_tests()
