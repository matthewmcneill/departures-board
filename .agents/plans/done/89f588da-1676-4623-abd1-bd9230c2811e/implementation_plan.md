# Goal Description

The goal is to create a new `docs/DeveloperGuide.md` that provides a step-by-step onboarding guide for a new developer who has just cloned the repository. This guide will improve upon the existing `README.md` by explicitly laying out the tool installations, commands for building the software, flashing the device, and a dedicated section on enabling the debug logger via `platformio.ini` flags and monitoring the serial console.

## Proposed Changes

### Documentation

#### [NEW] `docs/DeveloperGuide.md`
We will create a structured and comprehensive developer guide containing the following sections:

1.  **Introduction & Prerequisites**
    *   Brief welcome and overview of what the developer will achieve.
    *   Hardware requirements (ESP32, display, wiring - referencing `README.md` for wiring if needed, or briefly summarizing).
2.  **Tool Installations**
    *   **Git**: For cloning the repository.
    *   **Visual Studio Code (VSCode)**: Highly recommended editor.
        *   **Recommended Extensions**:
            *   *C/C++* (Microsoft)
            *   *PlatformIO IDE* (PlatformIO) for configuring, building, and flashing.
            *   *Antigravity* (DeepMind) as an option for agentic, AI-assisted coding and task management.
    *   **PlatformIO IDE Extension**: For managing the toolchain, dependencies, and building/flashing. PlatformIO Core (CLI) will also be mentioned.
    *   **Python/Node.js**: Mention any prerequisites for the `extra_scripts` (e.g., `scripts/build_fonts.py`, `scripts/portalBuilder.py`) if they aren't fully self-contained or auto-resolved by PlatformIO.
3.  **Cloning the Repository**
    *   Standard `git clone` command and navigating into the project directory.
4.  **Building the Software**
    *   How to build using the PlatformIO VSCode UI (the checkmark icon).
    *   How to build using the PlatformIO CLI (`pio run -e esp32dev` or `pio run -e esp32s3nano`).
5.  **Flashing the Device**
    *   Connecting the ESP32 via USB.
    *   How to flash using the PlatformIO VSCode UI (the arrow icon).
    *   How to flash using the CLI (`pio run -t upload`).
    *   Mentioning `upload_port` troubleshooting if auto-detection fails.
6.  **Debugging & Serial Console**
    *   **Enabling Debug Logging**: Explain how to modify `platformio.ini` to enable debug output. Specifically, instructing to change `-DCORE_DEBUG_LEVEL=0` to a higher level (e.g., `4` for DEBUG or `5` for VERBOSE) and ensuring `-DENABLE_DEBUG_LOG=1` is set.
    *   **Monitoring**: How to open the Serial Monitor in VSCode or using the CLI (`pio device monitor -b 115200`). Emphasize the `115200` baud rate configured in `platformio.ini`.
7.  **Appendix: Agentic Coding and the `/.agents` Folder**
    *   Overview of **Antigravity Workflows** for agentic coding.
    *   Explanation of the `/.agents` directory structure, including:
        *   `project_log.md` and `todo_list.md`: For task tracking and history.
        *   `queue.md`: Execution queue state.
        *   Subdirectories like `rules/`, `skills/`, and `workflows/`.
    *   List of useful commands like `/queue-plan` and `/queue-do-this` based on the README.

## Verification Plan

### Manual Verification
1.  Read through `docs/DeveloperGuide.md` to ensure all instructions flow logically and correctly describe the PlatformIO workflow for this specific project.
2.  Verify the markdown formatting renders correctly.
3.  Ensure the debug flag instructions accurately reflect the `platformio.ini` file's current state (pointing out `-DCORE_DEBUG_LEVEL` and `-DENABLE_DEBUG_LOG`).
