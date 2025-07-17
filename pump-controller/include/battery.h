#ifndef PUMP_BATTERY_H
#define PUMP_BATTERY_H

#include <Arduino.h>
#include "device-config.h"

class Battery {
public:
    void setup();
    int getPercentage();
    bool isCharging();
};

#endif // PUMP_BATTERY_H
