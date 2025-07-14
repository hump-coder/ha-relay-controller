#include "config.h"
#include "Arduino.h"

#include "controller.h"
#include "receiver.h"
#include "display.h"

bool isController = false;
bool enableWifi = isController;

Device *device = 0;
Display display;

void setup() {

    Serial.begin(115200);
    Serial.println("Setting up");

    display.setup();

    if(isController)
    {
        device = new Controller(display);        
    }
    else
    {
        device = new Receiver(display, enableWifi);
    }

    
    device->setup();
    Serial.println("Setup complete");
}


void loop() {
    
    device->loop();
    // display.loop();
}

