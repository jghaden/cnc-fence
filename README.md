
# CNC Fence

CNC fence machine firmware

# Overview

1. [Description](#description)
2. [Features](#features)
3. [Manual](#manual)
4. [Requirements](#requirements)
5. [Flashing](#flashing)
6. [Serial](#serial)

# Description

This firmware is flashed to an **ATmega2560 MCU** to drive the CNC fence using **STEP/DIR** pulses for 2 stepper motors. The CNC fence has a **20x4 LCD display** and can be interacted with a **4x4 keypad matrix**.

# Features

- Page cycle
- Set target depth for fence ( inches )
- Set speed ( in/s )
- Jogging ( -Y/+Y )
- Fractional input
- Configurable Threads Per Inch ( TPI ) and fence depth value
- Load custom config values out of reset

```
                              Page 1-3

                        ┌──────────────────┐     ┌──────────────────┐
 Target: 0.000"         │Position: 0.000"  │     │  TPI: 20.000     │
 Speed: 1.250 in/s      │Speed: 1.250 in/s │     │Depth: 50.000"    │
                        └──────────── Jog ─┘     └───────── Config ─┘
```

# Manual

**Default**
| Key | Function |
|--|--|
| A | Edit top value |
| B | Edit bottom value |
| C | Page cycle |
| D | Home |
#
**Jog page**
| Key | Function |
|--|--|
| * | Jog ( -Y ) |
| # | Jog ( +Y ) |
#
**Edit mode**
| Key | Function |
|--|--|
| A | Set top value |
| B | Set bottom value |
| D | Fraction bar ( / ) |
| * | Decimal point ( . ) |
#

# Requirements

This project is managed under Visual Studio 2017 and not the [**Arduino IDE**](https://www.arduino.cc/en/software).

Install [**Visual Micro**](https://www.visualmicro.com/) to properly flash to an Arduino board. This can be done through the  **Extensions and Updates** menu in Visual Studio.

The project uses the **LiquidCrystal I2C** and **Adafruit Keypad** library, both which can be installed through **Visual Micro**

# Flashing

Flashing the microcontroller is as simple as pressing **F5** or **Debug->Start Debugging**.

Make sure there is no terminal open and connected to the serial port while flashing, or it will not be able to communicate to the board.

# Serial

The default **baud rate** is set to **9600** and can be manually changed by the following:

```c
void setup()
{
    Serial.begin(9600);
}
```