//Arduino UNO
//SCL   A5
//SDA   A4
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
String words = "test 0123456789";

void setup() {
    lcd.init();
    lcd.backlight();
}

void loop() {
    lcd.clear();
    lcd.setCursor(0, 0);
    for(unsigned int i = 0; i < words.length(); i++)
    {
        lcd.print(words.charAt(i));
        delay(500);
    }

    lcd.setCursor(0, 1);
    lcd.print("Hello World!");
    delay(1000);
}