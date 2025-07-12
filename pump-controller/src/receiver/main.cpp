#include <heltec.h>

const int PUMP_PIN = 2; // example GPIO
static bool pumpOn = false;

void updateDisplay() {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, pumpOn ? "Pump ON" : "Pump OFF");
    Heltec.display->display();
}

void setPump(bool on) {
    pumpOn = on;
    digitalWrite(PUMP_PIN, on ? HIGH : LOW);
    updateDisplay();
}

void setup() {
    pinMode(PUMP_PIN, OUTPUT);
    Heltec.begin(true, true, true, true);
    updateDisplay();
}

void loop() {
    int size = Heltec.LoRa.parsePacket();
    if (size) {
        String cmd;
        while (Heltec.LoRa.available()) {
            cmd += (char)Heltec.LoRa.read();
        }
        if (cmd == "ON") {
            setPump(true);
        } else if (cmd == "OFF") {
            setPump(false);
        }
    }
}
