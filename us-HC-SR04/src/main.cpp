//  tested on:
//
//  Arduino UNO
//  ESP32 Dev Module
//

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MODE_UART 1
#define MODE_NORMAL 0

#define MODE MODE_UART

#define trig 12
#define echo 14

#define rx 17   // tx (white)
#define tx 16   // rx (yellow)

unsigned char data[4] = {0};

SoftwareSerial us(rx, tx);

long duration;
float distance;

float getDistance();

void setup() {
    Serial.begin(9600);

#if MODE == MODE_NORMAL
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
#elif MODE == MODE_UART
    us.begin(9600);
#endif
}

void loop() {

    Serial.print("Distance: ");
    Serial.print(String(getDistance()));
    Serial.println(" cm");
    delay(1000);
}

float getDistance()
{
    float dist;
#if MODE == MODE_UART
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
#elif MODE == MODE_NORMAL
    digitalWrite(trig, LOW);
    delayMicroseconds(5);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    duration = pulseIn(echo, HIGH);
    dist = duration * 0.034 / 2;    
#endif
    return dist;
}
