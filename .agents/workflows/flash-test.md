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

pio run -e $ENV -t upload && pio device monitor -e $ENV
```
