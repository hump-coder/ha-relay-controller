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

### MQTT Topics

- `pump_station/switch/set` – payload `ON[:seconds]` or `OFF` to control the relay. If `seconds` is omitted the controller uses `DEFAULT_ON_TIME_SEC`.
- `pump_station/switch/pulse` – payload is a number of seconds to turn the relay on once. The controller automatically sends `OFF` after the duration and no heartbeat messages are sent.

### Home Assistant Discovery

The controller publishes MQTT discovery messages for easy integration with Home Assistant.
It exposes a `switch` entity for basic on/off control and a `number` entity named
`Pump Pulse` that publishes its value to `pump_station/switch/pulse` whenever the
number is changed. The number represents the pulse duration in seconds.
