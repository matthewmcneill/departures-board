# Graveyard: Port Contention during Flash

## Problem: 'Device not configured' Error
During the verification phase, a firmware flash attempt failed with the following error:
```text
esptool v5.1.2
Serial port /dev/cu.usbmodem806599A18E902:
Connecting..............
A serial exception error occurred: Could not configure port: (6, 'Device not configured')
```

## Root Cause
The **Background Spooler** was already active on the same port. On macOS, if a process (like the MCP monitor) has an open handle to the USB-Serial port, `esptool` may fail to properly reset or configure the port for the DFU handshake, resulting in a "Device not configured" exception from `pySerial`.

## Lesson Learned
Before every flash attempt on the physical hardware:
1.  **Explicitly stop** any active serial spooling via `mcp_platformio_stop_spooling`.
2.  **Verify port name**: S3 boards (like the Nano ESP32) can change port names between DFU mode and Application mode (e.g., from `usbmodem14201` to `usbmodem806599...`).
3.  **Order of Operation**: `Stop Spooler` -> `Flash` -> `Start Spooler`.

## Status
**Resolved** by retrying with the spooler stopped. Future agents should use the `upload_firmware` tool with `startSpoolingAfter: true` to avoid this manual overhead.
