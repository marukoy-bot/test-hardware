//  tested on:
//
//  Arduino UNO
//  ESP32 Dev Module
//

#include <Arduino.h>

#define trig 12
#define echo 11

long duration;
int distance;

float getDistance();

void setup() {
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    Serial.begin(9600);
}

void loop() {
    Serial.print("Distance: ");
    Serial.print(String(getDistance()));
    Serial.println(" cm");
    delay(200);
}

float getDistance()
{
    digitalWrite(trig, LOW);
    delayMicroseconds(5);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    duration = pulseIn(echo, HIGH);

    return duration * 0.034 / 2;
}
