# ha-relay-controller

A PlatformIO project for a LoRa based pump control system using Heltec WiFi LoRa 32 V3 boards. Two identical devices communicate over LoRa and can operate in one of two roles controlled by the `isController` flag in `pump-controller/src/main.cpp`.

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
- `pump_station/switch/pulse` – payload is a number of seconds to turn the relay on once. The controller sends a `PULSE` LoRa message containing the duration and also publishes an `OFF` message when the time expires.
- `pump_station/tx_power/controller/set` – integer transmit power in dBm for the controller.
- `pump_station/tx_power/receiver/set` – integer transmit power in dBm for the receiver.

### Home Assistant Discovery

The controller publishes MQTT discovery messages for easy integration with Home Assistant.
It exposes a `switch` entity for basic on/off control and `number` entities named
`Pump Pulse`, `Controller Tx Power` and `Receiver Tx Power`.
It also publishes `sensor` entities `Controller Status` and `Receiver Status`
which report the JSON data sent to `pump_station/status/controller` and
`pump_station/status/receiver`.
The controller enforces a minimum transmit power defined by `MIN_TX_OUTPUT_POWER`.

On boot the controller sends a `STATUS` request to the receiver. The receiver
responds with a `HELLO` message containing its current configuration (currently
just transmit power). If the values differ from those the controller has
persisted from previous MQTT commands it will resend the appropriate
configuration messages.
