---
description: View the most recent serial monitor logs from the departures-board
---

To view the most recent serial logs from the physical hardware (without restarting the monitor), execute the following:

```bash
# View the last 100 lines and continue monitoring
tail -n 100 -f logs/latest-monitor.log
```

Alternatively, for a one-off snapshot:

```bash
# View the last 50 lines only
tail -n 50 logs/latest-monitor.log
```
