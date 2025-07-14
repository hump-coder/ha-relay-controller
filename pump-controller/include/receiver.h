#ifndef PUMP_RECEIVER_H
#define PUMP_RECEIVER_H

#include "device.h"
#include "display.h"

class Receiver : public Device
{
    public:
    Receiver(Display &display, bool enableWifi);
    void setup() override;
    void loop() override;

    void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
    void OnTxDone();
    void OnTxTimeout();

    private:
    void updateDisplay();
    void setIdle();
    bool mWifiEnabled=false;
    private:
    Display &mDisplay;
    String mLastMessage;
    uint16_t mLastMessageSize;
    int16_t mLastRssi;
    int8_t mLastSnr;
    bool mRelayState;
    int acksRemaining;
    uint16_t ackStateId;
    bool ackConfirmed;
    void sendAck(char *rxpacket);
    void setRelayState(bool newRelayState);
    void processReceived(char *rxpacket);


};


#endif // PUMP_CONTROLLER_H
