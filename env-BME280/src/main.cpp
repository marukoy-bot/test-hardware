#include <Arduino.h>
#include "DFRobot_BME280.h"
#include "Wire.h"

TwoWire bme_wire = TwoWire(0);  //  SDA = 21; SCL = 22

typedef DFRobot_BME280_IIC BME;
BME bme(&bme_wire, 0x77);

#define SEA_LEVEL_PRESSURE 1015.0f

void getOpStatus(BME::eStatus_t status);

void setup() {
    Serial.begin(9600);
    bme_wire.begin(21, 22);
    bme.reset();
    while (bme.begin() != BME::eStatusOK) {
        Serial.println("BME initialization failed");
        getOpStatus(bme.lastOperateStatus);
        delay(2000);
    }

    Serial.println("BME ready.");
    delay(100);
}

void loop() {
    float temperature = bme.getTemperature();
    uint32_t pressure = bme.getPressure();
    float altitude = bme.calAltitude(SEA_LEVEL_PRESSURE, pressure);
    float humidity = bme.getHumidity();

    Serial.println("------------------------------");
    Serial.println("temperature: " + String(temperature) + "°C");
    Serial.println("humidity: " + String(humidity) + " %");
    Serial.println("pressure: " + String(pressure) + " pa");
    Serial.println("altitude: " + String(altitude) + " m");

    delay(1000);
}

void getOpStatus(BME::eStatus_t eStatus) {
    switch (eStatus) {
        case BME::eStatusOK:                    Serial.println("Status: OK"); break;
        case BME::eStatusErr:                   Serial.println("Status: Unknown Error"); break;
        case BME::eStatusErrDeviceNotDetected:  Serial.println("Status: Unknown Device or Deveice not detected"); break;
        case BME::eStatusErrParameter:          Serial.println("Status: Error Parameter"); break;
        default: Serial.println("Status: Unknown"); 
    }
}