#include <Arduino.h>
#include <Stepper.h>

//  switch in2 and in3 to spin ccw; 
//  instead of {19, 18, 5, 17}
//          do {19, 5, 18, 17}

#define in1 19
#define in2 5
#define in3 18
#define in4 17

const int stepsPerRevolution = 2048;

Stepper stepper(stepsPerRevolution, in1, in2, in3, in4);

void zeroPins();

void setup() {
    stepper.setSpeed(5);
    Serial.begin(9600);
}

unsigned long time1 = 0, time2 = 0;

void loop() {
    time1 = millis();
    Serial.println("clockwise");
    stepper.step(stepsPerRevolution);
    zeroPins();
    delay(1000);

    time2 = millis();
    Serial.println("counter-clockwise");
    stepper.step(-stepsPerRevolution);
    zeroPins();

    Serial.println(String((time2 - time1) / 1000.0, 2) + " seconds per revoluton");
    delay(1000);
}

void zeroPins() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}