#include <heltec.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid = "YOUR_WIFI";
const char *password = "YOUR_PASS";
const char *mqtt_server = "192.168.1.10";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

static bool pumpOn = false;

void updateDisplay() {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, pumpOn ? "Pump ON" : "Pump OFF");
    Heltec.display->display();
}

void publishState() {
    mqttClient.publish("pump_controller/switch/state", pumpOn ? "ON" : "OFF", true);
    updateDisplay();
}

void sendLoRa(const char *msg) {
    Heltec.LoRa.beginPacket();
    Heltec.LoRa.print(msg);
    Heltec.LoRa.endPacket();
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    String cmd;
    for (unsigned int i = 0; i < length; i++) {
        cmd += (char)payload[i];
    }
    if (cmd == "ON") {
        pumpOn = true;
        sendLoRa("ON");
    } else if (cmd == "OFF") {
        pumpOn = false;
        sendLoRa("OFF");
    }
    publishState();
}

void sendDiscovery() {
    const char *discoveryTopic = "homeassistant/switch/pump_controller/config";
    String payload = "{\"name\":\"Pump\",\"command_topic\":\"pump_controller/switch/set\",\"state_topic\":\"pump_controller/switch/state\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"unique_id\":\"pump_controller\",\"device\":{\"identifiers\":[\"pump_controller\"],\"name\":\"Pump Controller\",\"model\":\"Heltec WiFi LoRa 32 V3\",\"manufacturer\":\"Heltec\"}}";
    mqttClient.publish(discoveryTopic, payload.c_str(), true);
}

void ensureMqtt() {
    while (!mqttClient.connected()) {
        mqttClient.connect("pump_controller");
        if (!mqttClient.connected()) {
            delay(500);
        }
    }
    mqttClient.subscribe("pump_controller/switch/set");
}

void setup() {
    Heltec.begin(true, true, true, true);
    updateDisplay();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqttCallback);

    ensureMqtt();
    sendDiscovery();
    publishState();
}

void loop() {
    if (!mqttClient.connected()) {
        ensureMqtt();
    }
    mqttClient.loop();
}

