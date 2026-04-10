# Graveyard: Arduino Nano ESP32 Flashing via PlatformIO/esptool

## The Problem
Uploading firmware to the connected Arduino Nano ESP32 board (`esp32s3nano` environment on `/dev/cu.usbmodem806599A18E902`) fails natively via the MCP `upload_firmware` wrapper, giving the distinct exception:
`Usage: esptool write-flash [OPTIONS] <address> <filename>...`
`Invalid value for '<address> <filename>...': Must be pairs of an address and the binary filename to write there.`

## Attempts & Dead-Ends
1. **Adding `skipBuild: true`**: We implemented and pushed `skipBuild` to the MCP Server configuration. It successfully bypassed the initial 115000ms compilation bottleneck but still resulted in the precise same parameter drop to `esptool.py`.
2. **Forced Project Cleans**: We suspected the PIO native cache generated malformed upload targets during linking skipping. We triggered `mcp_platformio_clean_project` and a fully flushed compile cycle. Compilation succeeded, but uploading still yielded the exact same `esptool write-flash` failure.
3. **Execution Context**: We tried utilizing the native `pio run -e esp32s3nano -t upload` wrapper directly over background shell tasks outside of the MCP server context, which encountered compilation stalls unrelated to the MCP logic.

## Conclusion
The issue is fundamentally tied to the `.pio/build` logic mapping for `board_build.partitions = arduino_nano_esp32_partitions.csv` within `framework-arduinoespressif32` (`pioarduino`). Essentially, `platformio.ini` does not export valid partition space mapping arguments into the local instance of `esptool.py`. It is effectively running `esptool.py write-flash 0x0 ""` which violently fails the positional parameter parsing logic.
