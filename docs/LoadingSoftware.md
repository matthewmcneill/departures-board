# Loading Software

This guide covers everything you need to know about flashing the firmware to your ESP32 device to run the departures board.

## Prerequisites

Before installing the software, you'll need the following API keys to fully utilize all modes:
- **National Rail Darwin Lite API token:** Free of charge. Request one [here](https://realtime.nationalrail.co.uk/OpenLDBWSRegistration).
- **OpenWeather Map API token:** Optional, used to display weather conditions at the selected station. Sign-up for a free developer account [here](https://home.openweathermap.org/users/sign_up).

---

## Installing the firmware

The project uses the Arduino framework and the ESP32 core. If you want to build from source, you'll need [PlatformIO](https://platformio.org).

### Option 1: Web Based Installer (Easiest)
The easiest way to install the firmware for the first time is to use the online web-based installer [here](https://departures-board.github.io). You will need to use Chrome or Edge as your browser, as Safari and Firefox do not support Web Serial.

### Option 2: esptool
Alternatively, you can download pre-compiled firmware images from the [releases](https://github.com/gadec-uk/departures-board/releases). These can be installed over the USB serial connection using [esptool](https://github.com/espressif/esptool). 

If you have Python installed, install with:
```bash
pip install esptool
```

For convenience, a pre-compiled executable version for Windows is included [here](https://github.com/gadec-uk/departures-board/tree/main/esptool).

Attach the ESP32 Mini board via its USB port and use the following command to flash the firmware:
```bash
esptool.py --chip esp32 --baud 460800 write_flash -z \
  0x1000 bootloader.bin \
  0xe000 boot_app0.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```
The tool should automatically find the correct serial port. If it fails, manually specify the correct port by adding `--port COMx` (or `/dev/ttyUSB0`, etc.).
