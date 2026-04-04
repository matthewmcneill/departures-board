# Developer Guide

Welcome to the Departures Board project! This guide will take you step-by-step through setting up your development environment, building the software, and flashing it to your ESP32 device.

## Prerequisites
* An ESP32 D1 Mini board (or clone).
* A 3.12" 256x64 OLED Display Panel (SSD1322).
* Wiring connections established (see `README.md` for specific wiring details).
* A USB cable to connect the ESP32 to your computer.

## Tool Installations

1. **Git**: Required for cloning the repository and managing version control.
   - Download from: [git-scm.com](https://git-scm.com/)
2. **Visual Studio Code (VSCode)** (or **Antigravity**): The recommended editors for this project.
   - **VSCode**: Download from [code.visualstudio.com](https://code.visualstudio.com/)
   - **Antigravity** (by DeepMind): If you want to use agentic coding, we highly recommend using Antigravity, which is a powerful VSCode fork designed specifically for agentic, AI-assisted coding and task management within the repository. Download from: [antigravity.google/download](https://antigravity.google/download)
3. **Extensions**: For the best development experience, install the following extensions:
   - **C/C++** (by Microsoft): Provides intellisense, debugging, and code formatting.
   - **PlatformIO IDE** (by PlatformIO): Essential for managing the toolchain, handling library dependencies, and compiling/flashing the ESP32 firmware.
4. **Python**: PlatformIO requires Python (usually installed automatically with the extension). It is also used for running `extra_scripts` like building fonts and the portal builder.

## Cloning the Repository

To get started, clone the repository to your local machine:

```bash
git clone https://github.com/gadec-uk/departures-board.git
cd departures-board
```

*(If you are contributing, you may want to fork the repository first and clone your fork.)*

## Building the Software

We use PlatformIO to manage the build process. You can build the software using the UI or the command line.

### Using the PlatformIO VSCode UI
1. Open the `departures-board` folder in VSCode.
2. Wait a moment for PlatformIO to initialize and download any necessary toolchains or libraries (this may take a few minutes the first time).
3. Click the **Build** icon (a checkmark `✓`) in the PlatformIO toolbar at the very bottom of the VSCode window.

### Using the PlatformIO CLI
Alternatively, you can build from the terminal:
```bash
# Build for ESP32 Dev Board
pio run -e esp32dev

# Or build for ESP32-S3 Nano
pio run -e esp32s3nano
```

## Flashing the Device

Once the build is successful, you can flash the firmware to your ESP32.

1. Connect your ESP32 to your computer via USB.
2. **Using the UI**: Click the **Upload** icon (a right-pointing arrow `→`) next to the Build icon in the PlatformIO toolbar.
3. **Using the CLI**:
   ```bash
   pio run -t upload -e esp32dev
   ```
*Note: PlatformIO usually auto-detects the correct serial port. If it fails, you may need to explicitly specify your `upload_port` in `platformio.ini`.*

## Debugging & Serial Console

If you are developing new features or troubleshooting, you'll want to view the serial output from the device.

### Enabling Debug Logging
By default, verbose debug logging is disabled to save memory and processing time. To enable it, you simply need to change the core debug level, which now controls both the ESP32 core logs and our custom application `Logger` output.
1. Open `platformio.ini`.
2. Locate the environment you are building for (e.g., `[env:esp32dev]`).
3. Under `build_flags`, find `-DCORE_DEBUG_LEVEL=0` and change it to your desired verbosity level:
   - `0` = None
   - `1` = Error
   - `2` = Warn
   - `3` = Info (and Splash messages)
   - `4` = Debug
   - `5` = Verbose
   *(Example: `-DCORE_DEBUG_LEVEL=4`)*
4. Rebuild and flash the device.

### Monitoring the Serial Console
The application communicates over the serial port at a baud rate of **115200**.
- **Using the UI**: Click the **Serial Monitor** icon (a plug `🔌`) in the PlatformIO toolbar.
- **Using the CLI**:
  ```bash
  pio device monitor -b 115200
  ```

 ---
 
 ## Advanced Documentation & References
 
 For a complete understanding of how the system is designed at a low level, please begin with our formal specifications. These documents serve as the gateways to our extensive library of deep-dive architectural references:
 
 * **[System Specification Document](SystemSpecificationDocument.md)**: Details the complete firmware architecture, data models, memory strategies, component diagrams, and the embedded web design system. It bridges into our internal `reference/` guides for specific subsystems.
 * **[Designer Specification Document](DesignerSpecificationDocument.md)**: Details the UI layout simulator, the WebAssembly engine pipeline, and how layouts are converted from JSON to C++.
 * **[Display Layouts Reference](DisplayLayoutsReference.md)**: Comprehensive guide on creating layouts, widget APIs.
 * **[Upstream Merges](UpstreamMerges.md)**: Tracks changes published by the original `gadec-uk` upstream repository and documents how those modifications have been integrated, adapted, or rejected in this v3.0 object-oriented architecture.
 
 ---
 
 ## Layout Simulator
 
 To speed up UI development without needing physical hardware for every change, the project includes a **WebAssembly-based Layout Simulator**. This allows you to preview and debug your JSON layout definitions in real-time in your browser.
 
 ### Running the Simulator
 1.  Ensure you have Python 3 installed.
 2.  Start the development server from the repo root:
     ```bash
     python3 tools/layoutsim/scripts/dev_server.py
     ```
 3.  Access the simulator at: [http://localhost:8000/tools/layoutsim/web/index.html](http://localhost:8000/tools/layoutsim/web/index.html)
 
 ---
 
 ## Appendix: Agentic Coding and the `/.agents` Folder

This project supports **Antigravity Workflows** for agentic coding. If you are using the Antigravity extension, the `/.agents` folder contains metadata and rules that guide the AI's behavior and maintain project context across sessions.

### Key Files and Directories
* **`project_log.md`**: Tracks the overall history, major architectural decisions, and accomplished tasks.
* **`todo_list.md`**: A backlog of planned tasks and improvements.
* **`queue.md`**: Manages the agent execution queue state, preventing hardware/build conflicts when multiple agent sessions are running.
* **`rules/`**: Contains project-specific rules for the AI (e.g., coding standards, file structures).
* **`skills/`**: Contains custom tools and specific behaviors the agent can invoke during tasks (e.g., embedded systems focus, architectural refactoring).
* **`workflows/`**: Pre-defined sequences of steps to automate common processes.

### Useful Antigravity Commands

This project uses **Antigravity Workflows** for managing multiple agent sessions in parallel. To prevent hardware and build conflicts, developers or agents should use the following native commands via the chat interface:

- `/plan-start` / `/queue-do-this`: Claim the hardware lock for this session and start work.
- `/plan-queue` / `/queue-plan`: Queue the current implementation plan for execution.
- `/queue-list`: List the current state of the execution queue.
- `/queue-release`: Force release the current hardware lock.
- `/flash-test`: Safely build and flash firmware via serial without conflicts.
