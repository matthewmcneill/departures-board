# Hardware Guide

This document outlines the hardware requirements and assembly instructions for building the departures board.

## Required Components

1. **ESP32 Microcontroller**
   Recommended: An ESP32 D1 Mini board (or clone) — either USB-C or Micro-USB version with CH9102. For example, from [AliExpress](https://www.aliexpress.com/item/1005005972627549.html).
2. **OLED Display Panel**
   A 3.12" 256x64 OLED Display Panel with SSD1322 display controller onboard. For example, from [AliExpress](https://www.aliexpress.com/item/1005008558326731.html).
3. **3D Printed Case**
   STL files are provided for a custom 3D printed case. You can find them in the `stl` directory. If you don't have a 3D printer, you can use a 3D print service, local library, or maker group.
4. **Soldering Materials**
   Some intermediate soldering skills and wire to connect the display.

<img src="docs/assets/hardware_parts.jpg" align="center">

---

## Preparing the OLED display for 4-Wire SPI Mode

<img src="docs/assets/oled_spi_mod.png" align="right">

As supplied, the display is usually shipped with 8-bit 80XX mode enabled. This needs to be changed to **4-Wire SPI mode** by removing one link and adding another. 

The image shown indicates where to make these changes on the rear of the circuit board.

<br clear="both" />

---

## Wiring Guide

Solder the 4 SPI connections, plus power and ground. The wires **MUST** be soldered to the **BACK** of the ESP32 Mini board (the side without the components) to enable it to sit in place in the case. 

You can solder directly to the pins on the OLED screen. Alternatively, for the best fit (if you are a more experienced solderer), you can de-solder and remove the header pins and solder directly to the board. 

> [!WARNING]
> You cannot use Dupont connectors; they will not fit the custom case design.

### Pinout Table

| OLED Pin | ESP32 Mini Pin |
|:---------|:-------------:|
| 1 VSS | GND |
| 2 VCC_IN | 3.3v |
| 4 D0/CLK | IO18 |
| 5 D1/DIN | IO23 |
| 14 D/C# | IO5 |
| 16 CS# | IO26 |

<img src="docs/assets/wiring_diagram.jpg" align="center">
