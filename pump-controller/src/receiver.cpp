#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"

#include "config.h"
#include "display.h"
#include "device-config.h"
#include "receiver.h"

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

Receiver *instance;

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle = true;

static RadioEvents_t RadioEvents;

void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

static bool pumpOn = false;

Receiver::Receiver(Display &display, bool enableWifi) : mDisplay(display), mWifiEnabled(enableWifi)
{
    ::instance = this;

    mLastMessage = "";
    mLastMessageSize = 0;
    mLastRssi = 0;
    mLastSnr = 0;
}

void Receiver::updateDisplay()
{
    mDisplay.display.clearDisplay();
    mDisplay.display.setTextSize(2); // Draw 2X-scale text
    mDisplay.display.setTextColor(SSD1306_WHITE);
    mDisplay.display.setCursor(10, 0);
    mDisplay.display.println(F("Receiver"));
    mDisplay.display.setTextSize(1); // Draw 2X-scale text

    mDisplay.display.setCursor(10, 22);
    mDisplay.display.printf("LM: %s", mLastMessage.substring(0, 12).c_str());

    mDisplay.display.setCursor(10, 36);
    mDisplay.display.printf("SIZE: %d", mLastMessageSize);

    mDisplay.display.setCursor(10, 50);
    mDisplay.display.printf("RSSI: %d SNR: %d", mLastRssi, mLastSnr);

    mDisplay.display.setCursor(10, 50);
    mDisplay.display.printf("RSSI: %d SNR: %d", mLastRssi, mLastSnr);

    mDisplay.display.display();
}

void Receiver::setup()
{
    Serial.println("Setting up");
    Serial.begin(115200);

    mDisplay.display.clearDisplay();
    mDisplay.display.display();

    if (mWifiEnabled)
    {
        Serial.println("Init Wifi");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
        }
        Serial.println("Init Wifi - complete");
    }
    Mcu.begin();

    //    if(firstrun)
    //    {
    // LoRaWAN.displayMcuInit();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    //   display.display.display();
    //   delay(1000); // Pause for 2 seconds

    // Clear the buffer
    //     display.clearDisplay();

    //   firstrun = false;
    //  }

    // Draw a single pixel in white
    // display.drawPixel(10, 10, SSD1306_WHITE);

    // testscrolltext();
    //  Show the display buffer on the screen. You MUST call display() after
    //  drawing commands to make them visible on screen!
    // display.display();
    // delay(2000);

    txNumber = 0;

    RadioEvents.TxDone = ::OnTxDone;
    RadioEvents.TxTimeout = ::OnTxTimeout;
    RadioEvents.RxDone = ::OnRxDone;
    Serial.println("Init Radio.");

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    setIdle();

    Serial.println("Init Radio - complete");
}

void Receiver::setIdle()
{
    Serial.println("Radio - in read mode: waiting for incoming mesages");
    //
    // Begin in continuous receive mode so incoming packets are processed
    //
    Radio.Rx(0);

    //
    // We're idle if we're waiting.
    //
    lora_idle = true;
}

unsigned long lastUpdate = 0;

unsigned long lastScreenUpdate = 0;

void Receiver::loop()
{

    if (millis() - lastScreenUpdate > 1000)
    {
        updateDisplay();
        lastScreenUpdate = millis();
    }
    // if(lora_idle == false)
    {
        Radio.IrqProcess();
    }
}

void Receiver::OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    // Rssi=rssi;
    // rxSize=size;
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    Radio.Sleep();

    mLastMessage = (char *)rxpacket;
    mLastMessageSize = size;
    mLastRssi = rssi;
    mLastSnr = snr;

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n", rxpacket, rssi, size);
    // Serial.println("wait to send next packet");

    // lora_idle = true;
    // Resume listening for the next packet
    // Radio.Rx( 0 );
    setIdle();
}

void Receiver::OnTxDone(void)
{
    Serial.println("TX done......");
    setIdle();
    // lora_idle = true;
    // // After transmitting, return to receive mode
    // Radio.Rx( 0 );
    // state=STATE_RX;
}

void Receiver::OnTxTimeout()
{
    Radio.Sleep();
    Serial.println("TX Timeout......");
    setIdle();
    // lora_idle = true;
    // // Return to receive mode after a timeout
    // Radio.Rx( 0 );
    // state=STATE_RX;
}

//
//
//

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    instance->OnRxDone(payload, size, rssi, snr);
}

void OnTxDone()
{
    instance->OnTxDone();
}
void OnTxTimeout()
{
    instance->OnTxTimeout();
}
