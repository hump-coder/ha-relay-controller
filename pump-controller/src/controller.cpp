
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"

#include "config.h"
#include "display.h"
#include "device-config.h"
#include "controller.h"

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

Controller *controllerInstance;

void COnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    controllerInstance->OnRxDone(payload, size, rssi, snr);
}

void COnTxDone()
{
    controllerInstance->OnTxDone();
}
void COnTxTimeout()
{
    controllerInstance->OnTxTimeout();
}


Controller::Controller(Display &display) : mDisplay(display), mqttClient(espClient)
{
    controllerInstance = this;
    mLastMessage = "";
    mLastMessageSize = 0;
    mLastRssi = 0;
    mLastSnr = 0;
    mStateId = 1;
    relayState = RelayState::UNKNOWN;
    requestedRelayState = RelayState::UNKNOWN;
}

String relayStateToString(RelayState state)
{
  switch (state)
  {
  case RelayState::OFF:
    return "OFF";
  case RelayState::ON:
    return "ON";  
  }
  
  return "UNKNOWN";  
}

void Controller::updateDisplay()
{
    mDisplay.display.clearDisplay();
    mDisplay.display.setTextSize(1); // Draw 2X-scale text
    mDisplay.display.setTextColor(SSD1306_WHITE);
    mDisplay.display.setCursor(10, 0);
    mDisplay.display.printf("Controller: %s", relayStateToString(relayState));
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


void Controller::publishState() {
    mqttClient.publish("pump_station/switch/state", relayStateToString(relayState).c_str(), true);
    updateDisplay();
}

void controllerMqttCallback(char *topic, byte *payload, unsigned int length) 
{
  controllerInstance->mqttCallback(topic, payload, length);
}

void Controller::mqttCallback(char *topic, byte *payload, unsigned int length) {
    String cmd;

    Serial.println("MQTT CALLBACK");


    for (unsigned int i = 0; i < length; i++) {
        cmd += (char)payload[i];
    }
    if (cmd == "ON") {
        setRelayState(true);    
    } else if (cmd == "OFF") {
        setRelayState(false);        
    }

    publishState();
}

#define BASE_TOPIC "esp32-lora-controller"

#define MQTT_DEVICE \
    "\"device\": {" \
      "\"identifiers\": [\"esp32-lora-controller\"]," \
      "\"name\": \"ESP32 LoRa Controller\"," \
      "\"manufacturer\": \"HumpTech\"," \
      "\"model\": \"ESP32-LORA-PC\"," \
      "\"sw_version\": \"1.0\"" \
    "}"

void Controller::sendDiscovery() {
    const char *discoveryTopic = "homeassistant/switch/pump_station/config";
    String payload = "{\"name\":\"Pump\",\"command_topic\":\"pump_station/switch/set\",\"state_topic\":\"pump_station/switch/state\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"unique_id\":\"pump_station\",\"device\":{\"identifiers\":[\"pump_station\"],\"name\":\"Pump Controller\",\"model\":\"Heltec WiFi LoRa 32 V3\",\"manufacturer\":\"Heltec\"}}";
    mqttClient.publish(discoveryTopic, payload.c_str(), true);
}



// void Controller::sendDiscovery()
// {

//   char topic[128];
//   char payload[2048];

//   const char *THING_NAME = "pump_station";

//   snprintf(topic, sizeof(topic), "homeassistant/switch/%s/config", THING_NAME);
//   snprintf(payload, sizeof(payload),
//            "{"
//            "%s,"
//            "\"name\": \"%s\","
//            "\"unique_id\": \"%s\","
//            "\"command_topic\": \"%s/switch/set\","
//            "\"state_topic\": \"%s/switch/state\","
//         //    "\"availability_topic\": \"%s/status\","
//            "\"payload_on\": \"ON\","
//            "\"payload_off\": \"OFF\""
//            "}",
//            MQTT_DEVICE,
//            THING_NAME,           
//            THING_NAME,
//            BASE_TOPIC,
//            BASE_TOPIC,
//            BASE_TOPIC);

//            mqttClient.publish(topic, payload, true);
// }

void Controller::ensureMqtt() {
    while (!mqttClient.connected()) {
        mqttClient.connect("pump_station", MQTT_USER, MQTT_PASS);
        if (!mqttClient.connected()) {
            delay(500);
        }
    }
    mqttClient.subscribe("pump_station/switch/set");
}

void Controller::setup() {

    Wire.begin(17, 18);

    Serial.println("Init Wifi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("Init Wifi - complete");

    //
    // Setup radio.
    //

    Mcu.begin();
    txNumber=0;

    RadioEvents.TxDone = ::COnTxDone;
    RadioEvents.TxTimeout = ::COnTxTimeout;
    RadioEvents.RxDone = ::COnRxDone;
    Serial.println("Init Radio.");

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 


    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    // Begin in continuous receive mode so incoming packets are processed
    Radio.Rx( 0 );

    Serial.println("Init Radio - complete");


    Serial.println("Init MQTT");
    mqttClient.setServer(MQTT_SERVER, 1883);
    mqttClient.setCallback(controllerMqttCallback);
    

    ensureMqtt();
    sendDiscovery();
    publishState();
    
    Serial.println("Init MQTT - complete");
}

void Controller::sendMessage(const char *msg)
{
        //snprintf(txpacket, sizeof(txpacket), msg.c_str());
 		sprintf(txpacket,"C:%ld:%s",mStateId, msg);  //start a package
   
		Serial.printf("Sending packet \"%s\", length %d\r\n",txpacket, strlen(txpacket));

        lora_idle = false;
		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
       
        Serial.println("packet sent.");   
}

void Controller::sendAckReceived(uint16_t stateId)
{
        sprintf(txpacket, "A:%u", stateId);

        Serial.printf("Sending ack receipt \"%s\", length %d\r\n", txpacket, strlen(txpacket));

        lora_idle = false;
        Radio.Send((uint8_t *)txpacket, strlen(txpacket));

        Serial.println("ack receipt sent.");
}


void Controller::setRelayState(bool pumpOn)
{
  ++mStateId;
  requestedRelayState = pumpOn ? RelayState::ON : RelayState::OFF;
  relayState = RelayState::UNKNOWN;  
  sendMessage(pumpOn ?"ON" : "OFF");
}


void Controller::loop() {

    if(lora_idle && millis() - lastSend > 5000)
    {
        if(requestedRelayState != relayState)
        {
            // if(requestedRelayState == RelayState::ON)
            // {
            //    sendMessage("ON");
            // }
            // else
            // {
            //     sendMessage("OFF");
            // }
        }


		// //txNumber += 0.01;
		// sprintf(txpacket,"ON:%ld",++messageNumnber);  //start a package
   
		// Serial.printf("Sending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

        // lora_idle = false;
		// Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
       
        Serial.print(".");

      lastSend = millis();      
    }
    


	// if(lora_idle == true)
	// {
    //     delay(1000);
	// 	txNumber += 0.01;
	// 	sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
   
	// 	Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

	// 	Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	

    //     Serial.println("packet sent.");

    //     lora_idle = false;
	// }
    

    if(lora_idle)
    {
      Radio.Rx( 0 );
    }


    // if(lora_idle == false)
    {
      Radio.IrqProcess( );
    }
    
    if (!mqttClient.connected()) {
        ensureMqtt();
    }
    mqttClient.loop();
}



void Controller::processReceived(char *rxpacket)
{
    char *strings[10];
    char *ptr = NULL;
    int index = 0;
    ptr = strtok(rxpacket, ":;"); // takes a list of delimiters
    while (ptr != NULL)
    {
        strings[index] = ptr;
        index++;
        ptr = strtok(NULL, ":;"); // takes a list of delimiters
    }

    
    if (index >= 3)
    {
        if (strlen(strings[0]) == 1 && strings[0][0] == 'R')
        {
            uint16_t stateId = atoi(strings[1]);
            RelayState confirmedRelayState = (strcasecmp(strings[2], "on") == 0) ? RelayState::ON : RelayState::OFF;
            
            if(mStateId == stateId && confirmedRelayState == requestedRelayState)
            {
                Serial.printf("Relay state confirmed - publishing new state: %s\n", confirmedRelayState == RelayState::ON ? "ON": "OFF");
                relayState = confirmedRelayState;
                //
                // Increment state id to ensure we don't process acks for this message again.
                //
                ++mStateId;
                publishState();
                sendAckReceived(stateId);
            }
            else
            {                
                Serial.printf("Skipping - likely an old ack expected stateid: %d, state: %s = %s.\n",mStateId , relayStateToString(confirmedRelayState).c_str(), strings[2]);
            }
        }
    }
}

void Controller::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    //Rssi=rssi;
    //rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep();

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,rssi,size);
    Serial.println("wait to send next packet");

    lora_idle = true;
    // Resume listening for the next packet
    Radio.Rx( 0 );

    processReceived((char *) rxpacket);
}

void Controller::OnTxDone( void )
{
        Serial.println("TX done......");
        lora_idle = true;
        // After transmitting, return to receive mode
        Radio.Rx( 0 );
    //state=STATE_RX;
}

void Controller::OnTxTimeout( void )
{
    Radio.Sleep();
    Serial.println("TX Timeout......");
    lora_idle = true;
    // Return to receive mode after a timeout
    Radio.Rx( 0 );
     //state=STATE_RX;
}

