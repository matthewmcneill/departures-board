# Context Bridge

## 📍 Current State & Focus
The project has successfully completed the prerequisite **Hardware Framebuffer Screenshot Tool** architecture specifically intended to assist with generating setup documentation. The `getRawFramebuffer` feature is fully shipped, allowing exact native OLED replication via the `screenshot.html` browser client. 
We have successfully wrapped up the hardware build and flashing, resolving all UI and dependency quirks, and effectively closed the hardware tools chapter. Now, the main focus is pivoting entirely back to overhauling `docs/ConfiguringDevice.md`. The user intends to trace an actual live configuration workflow covering a completely fresh or factory-reset ESP32, capturing both the Web Portal screens and the physical OLED screens.

## 🎯 Next Immediate Actions
The immediate next step is to initiate the configuration walkthrough documentation flow. 
Once the device is wiped and broadcasting the `Gadec Setup` AP (or is network-reachable), the arriving agent needs to grab its IP address from the user, instantiate a **Browser Subagent** to physically click through the entire step-by-step ESP32 wizard, intercepting and saving WebP recordings. Concurrently, the user or agent will fetch `/api/screenshot` images to document what the physical hardware display is emitting at each stage.
Ensure to ask the user how they would like the blurred censoring of WiFi SSIDs and Tokens to be executed (likely post-processing or via browser subagent DOM modification).

## 🧠 Decisions & Designs
- **Browser Automation Setup Documenting:** We will exclusively use the browser subagent paired with `/api/screenshot` endpoint to seamlessly generate the most accurate, high fidelity imagery without using messy smartphone camera photos.
- **Hardware Integration Status:** The `screenshot.html` is permanently fused into `.rodata` assets, requiring zero layout configurations from the user to operate out of the box. 

## 🐛 Active Quirks, Bugs & Discoveries
- **Hardware Lock Racing:** Continue to be cautious with hardware flashing lock collisions. Execute changes only via `./.agents/skills/hardware-testing/scripts/safe-flash.sh`. (Though hardware work is done for now).

## 💻 Commands Reference
- **No Active Processes:** The hardware monitor currently sits safely bound to the user terminal, so no immediate scripts need capturing.

## 🌿 Execution Environment
- Real hardware is attached with `esp32dev` environment natively running.
- The next agent will be focusing heavily on Markdown updates and browser sub-agent control rather than terminal compilation.
