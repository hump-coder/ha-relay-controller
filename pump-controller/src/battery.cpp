#include "battery.h"

void Battery::setup() {
    pinMode(CHARGE_STATUS_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

int Battery::getPercentage() {
    int raw = analogRead(BATTERY_VOLTAGE_PIN);
    // Heltec WiFi LoRa 32 V3 uses a ~4.9:1 voltage divider on the VBAT pin.
    // The raw ADC reading therefore needs to be scaled by this factor as well
    // as the ADC reference voltage. Empirically the ADC tends to report a
    // slightly lower voltage than measured with a multimeter so a small
    // calibration factor is applied (see Heltec example code).

    const float adcMaxVoltage = 3.3f;
    const float adcResolution = 4095.0f; // 12‑bit ADC
    const float dividerRatio = (390.0f + 100.0f) / 100.0f; // 390k/100k
    const float calibration = 4.2f / 4.095f; // measured / reported

    float voltage = ((float)raw / adcResolution) * adcMaxVoltage * dividerRatio * calibration;

    int percent = (int)((voltage - 3.2f) / (4.2f - 3.2f) * 100.0f);
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;
    return percent;
}

bool Battery::isCharging() {
    return digitalRead(CHARGE_STATUS_PIN) == HIGH;
}
