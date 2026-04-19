# Configuring the Device

This guide covers everything you need to know about navigating the web configuration interface, enabling API keys, and setting up the display logic.

## First Time Configuration

When you first power on the device, or after a factory reset, it will start up in setup mode. 

### 1. Connect to the Device
The OLED display will show that it is broadcasting a temporary WiFi AP:

> **[PLACEHOLDER: assets/oled_ap_mode.png - Hardware OLED in AP Mode]**
<!-- ![OLED AP Mode](assets/oled_ap_mode.png) -->

1. Connect to the temporary WiFi network (usually named **"Departures Board"**).
2. A captive portal should automatically open. If it doesn't, navigate to `192.168.4.1` in your browser.

### 2. Configure WiFi
Follow the on-screen instructions in the web portal to connect the board to your local WiFi network.

> **[PLACEHOLDER: assets/web_wifi_config.webp - Browser WebP Video of WiFi Selection]**
<!-- ![WiFi Portal Walkthrough](assets/web_wifi_config.webp) -->

Once successfully connected, the display will restart and show its new local IP address on your network:

> **[PLACEHOLDER: assets/oled_connection_success.png - Hardware OLED showing assigned IP]**
<!-- ![OLED Connection Success](assets/oled_connection_success.png) -->

### 3. Setup API Keys
Navigate to the IP address shown on the screen using your browser to continue setup. You'll be prompted to enter your API keys.

> **[PLACEHOLDER: assets/web_api_keys.webp - Browser WebP Video of API Key Entry]**
<!-- ![API Keys Setup](assets/web_api_keys.webp) -->

Select a default station location by type the location name; valid choices will be displayed as you type. *(Note: If you do not enter a National Rail token, the board will only operate in Tube and Bus modes).*

---

## Web GUI

At any time, the device's IP address is displayed briefly at boot. To change the station or to configure other miscellaneous settings, open the web page at that address.

> **[PLACEHOLDER: assets/web_main_settings.webp - Browser WebP Video scrolling through Main Settings]**
<!-- ![Web Portal Main Settings](assets/web_main_settings.webp) --> 

### Basic Settings
- **Board Mode** - Switch between National Rail Departures, London Underground Arrivals, or UK Bus Stops modes.
- **Station** - Select a National Rail station from the drop-down.
- **Underground Station** - Select an Underground or DLR station.
- **Bus Stop ATCO code** - Monitor a specific bus stop (see [Bus Stop ATCO codes](#bus-stop-atco-codes)).
- **Brightness** - Adjusts the OLED screen brightness.
- **Show the date on screen** - Displays the date in the upper-right corner.

### Filtering (Rail)
- **Only show services calling at** - Filter services to only show trains going *to* a specific station.
- **Only show these platforms** - Filter services based on departure platform.
- **Alternate station** - Automatically switch to an alternate station during specific hours.

### Advanced Options (*)
- **Include bus replacement services** - Show a bus icon in place of the platform number.
- **Include current weather at station location** - Requires valid OpenWeather Map API key.
- **Enable daily check for firmware updates** - Check for automatic updates just after midnight.
- **Enable overnight sleep mode (screensaver)** - Turn off display overnight to prevent burn-in.
- **Flip the display 180°** - Rotates the display to fit different case angles.
- **Custom (non-UK) time zone** - Set the clock to display local time (see [Custom Time Zones](#custom-time-zones)).
- **Display RSS news headlines feed** - Display top headlines from selected feeds after service messages.

### System Actions Dropdown
A drop-down menu (top-right) adds the following options:
- **Check for Updates** - Manually check for updates.
- **Edit API Keys** - View/edit API keys.
- **Clear WiFi Settings** - Deletes the stored WiFi credentials and reboots in setup mode.
- **Restart System** - Reboots the ESP32.

### Other Web GUI Endpoints
- `/factoryreset` - Factory reset erasing all configuration and WiFi credentials.
- `/update` - Manual OTA firmware updates via file upload. (Use `firmware.bin` only).
- `/info` - Diagnostics info.
- `/formatffs` - Formats the file system (keeps WiFi).
- `/dir` and `/upload` - Advanced filesystem utilities.
- `/control` - Endpoint for automation. E.g., `/control?sleep=1&clock=0` forces sleep mode.

---

## Bus Stop ATCO codes
Every UK bus stop has a unique ATCO code number. To find the ATCO code you want:
1. Go to [bustimes.org/search](https://bustimes.org/search).
2. Type a location in the search box and select the specific stop.
3. The ATCO code is shown on the stop information page.
4. Enter the code in the settings screen and tap **Verify**.

<img src="assets/atco_code_verification.jpg" align="center">

---

## Custom Time Zones
To set a custom time zone for the clock display, enter the POSIX time zone string for your location. 
* Examples: `CST6CDT,M3.2.0/2,M11.1.0/2` (Canada Central) and `AEST-10AEDT,M10.1.0,M4.1.0/3` (Australia Eastern). 
* Ask any AI chat engine: *"What is the POSIX time zone string for ..."* if unsure.
*(Note: Service times are always shown in UK time, only the clock is affected).*
