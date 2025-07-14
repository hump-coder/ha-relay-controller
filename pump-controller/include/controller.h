#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include "LoRaWan_APP.h"
#include <WiFi.h>
#include <PubSubClient.h>

#include "device.h"
#include "display.h"
#include "device-config.h"

enum RelayState
{
    UNKNOWN,
    ON,
    OFF
};

class Controller : public Device
{
public:
    Controller(Display &display);

    void setup() override;
    void loop() override;

    void mqttCallback(char *topic, byte *payload, unsigned int length);
    void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    void OnTxDone();
    void OnTxTimeout();

    void processReceived(char *rxpacket);

private:
    Display &mDisplay;
    String mLastMessage;
    uint16_t mLastMessageSize;
    int16_t mLastRssi;
    int8_t mLastSnr;
    uint16_t mStateId;
    RelayState relayState;
    RelayState requestedRelayState;

    char txpacket[BUFFER_SIZE];
    char rxpacket[BUFFER_SIZE];

    double txNumber;

    bool lora_idle = true;

    RadioEvents_t RadioEvents;

    WiFiClient espClient;
    PubSubClient mqttClient; //(espClient);
    void ensureMqtt();
    void updateDisplay();
    void sendMessage(const char *msg);
    void sendAckReceived(uint16_t stateId);
    void setRelayState(bool pumpOn, unsigned int onTime = DEFAULT_ON_TIME_SEC);
    void pulseRelay(unsigned int onTime);

    unsigned long nextOnSend = 0;
    unsigned int onTimeSec = DEFAULT_ON_TIME_SEC;
    bool heartbeatEnabled = true;
    unsigned long autoOffTime = 0;

    // unsigned int messageNumnber = 0;
    void publishState();
    void sendDiscovery();
};

#endif // PUMP_CONTROLLER_H
