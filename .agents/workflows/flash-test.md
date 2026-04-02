---
description: Kill serial monitor, build, flash, and monitor the attached device
---

Execute the following script to kill any existing serial monitor processes to free the port, build the firmware and flash it to the currently attached device, and open the serial port for monitoring.

```bash
#!/bin/bash
pgrep -f "pio device monitor" | xargs kill -9 2>/dev/null || true
pgrep -f "miniterm" | xargs kill -9 2>/dev/null || true

if pio device list | grep -q "Arduino Nano ESP32\|2341:0070"; then
    echo "Detected Arduino Nano ESP32 (esp32s3nano)"
    ENV="esp32s3nano"
else
    echo "Detected ESP32 Dev Module (esp32dev)"
    ENV="esp32dev"
fi

# Serial Logging Configuration
LOG_DIR="logs"
mkdir -p "$LOG_DIR"
TIMESTAMP=$(date +"%y%m%d-%H%M%S")
LOG_FILE="$LOG_DIR/device-monitor-$TIMESTAMP.log"
LATEST_LOG="$LOG_DIR/latest-monitor.log"

echo "Logging serial output to: $LOG_FILE"
echo "Live link: $LATEST_LOG"

# Build, Upload, and Monitor with unbuffered output
# We use PYTHONUNBUFFERED=1 and stdbuf (if available) to ensure tee and tail show updates immediately.
# The --raw flag is used to avoid control character mangling in the log file.
STDBUF_CMD=$(which stdbuf 2>/dev/null && echo "stdbuf -oL")
export PYTHONUNBUFFERED=1

$STDBUF_CMD pio run -e $ENV -t upload && \
$STDBUF_CMD pio device monitor -e $ENV --raw | $STDBUF_CMD tee "$LOG_FILE" | $STDBUF_CMD tee "$LATEST_LOG"
```
