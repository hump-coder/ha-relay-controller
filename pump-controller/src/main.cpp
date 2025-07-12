#include <WiFi.h>
#include <PubSubClient.h>
#include "LoRaWan_APP.h"
//#include “HT_SSD1306Wire.h”
#include "config.h"
#include "Arduino.h"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     21 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );



WiFiClient espClient;
PubSubClient mqttClient(espClient);

static bool pumpOn = false;

void updateDisplay() {
    // Heltec.display->clear();
    // Heltec.display->drawString(0, 0, pumpOn ? "Pump ON" : "Pump OFF");
    // Heltec.display->display();
}

void publishState() {
    mqttClient.publish("pump_controller/switch/state", pumpOn ? "ON" : "OFF", true);
    updateDisplay();
}

void sendLoRa(const char *msg) {
    // Heltec.LoRa.beginPacket();
    // Heltec.LoRa.print(msg);
    // Heltec.LoRa.endPacket();
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
        mqttClient.connect("pump_controller", mqtt_user, mqtt_pass);
        if (!mqttClient.connected()) {
            delay(500);
        }
    }
    mqttClient.subscribe("pump_controller/switch/set");
}

// RTC_DATA_ATTR bool firstrun = true;

void testfillroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    // The INVERSE color is used so round-rects alternate white/black
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}

void setup() {
    // Heltec.begin(true, true, true, true);
    // updateDisplay();


    Serial.println("Setting up");
    Serial.begin(115200);

    Wire.begin(17, 18);

    Serial.println("Init display.");

    //Light on
//    pinMode(35, OUTPUT);
//    digitalWrite(35, HIGH);


    pinMode(36, OUTPUT);
    digitalWrite(36, LOW);
    // Heltec delays for 100ms in their example
    delay(100);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
    //    for(;;); // Don't proceed, loop forever
    }
    Serial.println("Init display - complete.");


    Serial.println("Init Wifi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("Init Wifi - complete");

    
    Mcu.begin();

//    if(firstrun)
//    {
      //LoRaWAN.displayMcuInit();

      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    
      // Show initial display buffer contents on the screen --
      // the library initializes this with an Adafruit splash screen.
      display.display();
      delay(1000); // Pause for 2 seconds

      // Clear the buffer
 //     display.clearDisplay();

    //   firstrun = false;
  //  }
	
    // Draw a single pixel in white
    //display.drawPixel(10, 10, SSD1306_WHITE);

    //testscrolltext();
    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    //display.display();
    //delay(2000);

    txNumber=0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    Serial.println("Init Radio.");

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

    Serial.println("Init Radio - complete");


    Serial.println("Init MQTT");
    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqttCallback);
    

    ensureMqtt();
    sendDiscovery();
    publishState();
    
    Serial.println("Init MQTT - complete");

   	deviceState = DEVICE_STATE_INIT;
}

unsigned long lastUpdate = 0;

void loop() {

    if(millis() - lastUpdate > 1000)
    {
      Serial.printf("RSSI: %d\n", Radio.Rssi);
      lastUpdate = millis();      
    }
    


	if(lora_idle == true)
	{
        delay(1000);
		txNumber += 0.01;
		sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
   
		Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

		Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
        lora_idle = false;
	}
    


    if (!mqttClient.connected()) {
        ensureMqtt();
    }
    mqttClient.loop();
}




void OnTxDone( void )
{
	Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
}
