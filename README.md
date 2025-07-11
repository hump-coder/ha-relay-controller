# ha-relay-controller

Example ESPHome configuration for a LoRa based pump controller. Two Heltec WiFi LoRa 32 V3 boards communicate over LoRa using the [lora_sx126x](https://github.com/PaulSchulz/esphome-lora-sx126x) external component.

- `pump_controller.yaml` sends on/off commands.
- `pump_station.yaml` listens for those commands via a text sensor and toggles a relay.

Both YAML files require WiFi credentials and API/OTA secrets via the standard ESPHome `secrets.yaml` mechanism. LoRa pins are configured for the Heltec V3 board.

Place the files in your ESPHome configuration directory and compile using Home Assistant's ESPHome integration (2025.7.1 or later).
