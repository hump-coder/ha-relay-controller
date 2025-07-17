#include "battery.h"

void Battery::setup() {
    pinMode(CHARGE_STATUS_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

int Battery::getPercentage() {
    int raw = analogRead(BATTERY_VOLTAGE_PIN);
    float voltage = ((float)raw / 4095.0f) * 2.0f * 3.3f; // assume 2:1 divider
    int percent = (int)((voltage - 3.2f) / (4.2f - 3.2f) * 100.0f);
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;
    return percent;
}

bool Battery::isCharging() {
    return digitalRead(CHARGE_STATUS_PIN) == HIGH;
}
