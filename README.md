# ha-relay-controller

This repository contains a PlatformIO project for a LoRa based pump control system using Heltec WiFi LoRa 32 V3 boards. Two identical devices communicate over LoRa and can operate in one of two roles controlled by the `isController` flag in `pump-controller/src/main.cpp`.

- **Controller mode** (`isController = true`): connects to WiFi and MQTT, sends `ON`/`OFF` messages and processes acknowledgements.
- **Receiver mode** (`isController = false`): listens for LoRa commands and toggles a relay. WiFi is disabled by default.

## Building

1. Copy `pump-controller/include/config-example.h` to `pump-controller/include/config.h` and enter your WiFi and MQTT credentials.
2. Build the firmware with PlatformIO:

```bash
cd pump-controller
pio run
```

The default environment targets the Heltec WiFi LoRa 32 V3 board.

## Usage

Flash the compiled firmware to two boards. Set `isController` as required before compiling each device. Both devices display basic status information on the onboard OLED screen.
