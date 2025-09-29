#include <Arduino.h>
#include <SoftwareSerial.h>
#include "debug.h"

#define rx 17   // tx (white)
#define tx 16   // rx (yellow)

unsigned char data[4] = {0};

SoftwareSerial us(rx, tx);

float getDistance();

void setup() {
#if DEBUG == 1
    Serial.begin(115200);
#endif
    us.begin(115200);
}

void loop() {
    debugln(String(getDistance()));
    delay(50);
}

float getDistance() {
    float dist = 0;
    us.flush();
    delay(30);
    digitalWrite(tx, HIGH);
    delay(30);
    digitalWrite(tx, LOW);
    delay(30);
    digitalWrite(tx, HIGH);
    delay(60);

    for (int i = 0; i < 4; i++)
    {
        data[i] = us.read();
    }

    dist = (data[1] * 256) + data[2];
    dist = dist / 10;
}