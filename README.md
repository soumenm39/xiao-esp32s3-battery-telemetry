# XIAO ESP32-S3 INA219 Battery Charging Monitor

IoT battery monitoring system using a Seeed Studio XIAO ESP32-S3 Sense, CJMCU-219/INA219, DFRobot Solar Power Manager 5V, and ThingSpeak.

## Features
- Battery voltage, current, shunt voltage and power
- Charging / discharging / idle detection
- ThingSpeak logging
- Automatic Wi-Fi reconnection after router/internet outages
- Continues measuring while Wi-Fi is unavailable
- Optional Serial Monitor; firmware never waits forever for USB serial

## Hardware
- XIAO ESP32-S3 Sense
- CJMCU-219 INA219, R100 (0.1 ohm) shunt
- DFRobot Solar Power Manager 5V (DFR0559/CN3165)
- 1S 3.7 V Li-ion battery pack

## INA219 wiring

| CJMCU-219 | XIAO ESP32-S3 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | D4 / GPIO5 |
| SCL | D5 / GPIO6 |

Expected INA219 address: `0x40`.

## ThingSpeak fields

| Field | Meaning |
|---|---|
| 1 | Battery Voltage (V) |
| 2 | Current (mA) |
| 3 | Power (mW) |
| 4 | Shunt Voltage (mV) |
| 5 | Status: 0 idle, 1 charging, 2 discharging |

## Setup

Install Arduino libraries:
- Adafruit INA219
- ThingSpeak

Edit `src/battery_monitor.ino`:

```cpp
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
unsigned long myChannelNumber = YOUR_CHANNEL_NUMBER;
const char *myWriteAPIKey = "YOUR_THINGSPEAK_WRITE_API_KEY";
```

## Wi-Fi outage behavior

The firmware retries Wi-Fi every 10 seconds. If the router is off for one hour or longer, the XIAO keeps running and measuring. When Wi-Fi returns, ThingSpeak uploads resume automatically, provided the XIAO remains powered.

Measurements are **not buffered** during an outage in this version.

## INA219 calibration warning

The code currently uses:

```cpp
ina219.setCalibration_32V_2A();
```

as a temporary initial-test calibration. The installed CJMCU-219 has an R100 (0.1 ohm) shunt. Before using current, mAh, Wh or SOC as quantitative measurements, use an R100-specific calibration and verify it against a known reference.

## Battery safety

This firmware is not a battery protection system. Keep the battery's BMS/protection circuit connected. Use a proper 1S Li-ion CC/CV charger. Never short-circuit the pack, bypass its BMS, or connect 5 V directly to a Li-ion cell. Do not leave an abnormal/damaged Li-ion pack charging unattended.

<img width="2060" height="1370" alt="image" src="https://github.com/user-attachments/assets/8395eeb7-721a-4a7c-bd72-b944d05d5e2c" />

<img width="1280" height="960" alt="photo_2026-10-07_23-20-17" src="https://github.com/user-attachments/assets/1f45f79d-c50f-4a48-8f13-a0da4ba6cd85" />


## Roadmap
- R100-specific INA219 calibration
- mAh/Wh accumulation
- SOC estimation
- microSD offline logging
- Telegram alerts
- low-voltage and over-current alarms
- charging-failure detection

## License
MIT
