//   Pin    |   ESP23 Dev Module    |   Arduino UNO     |
//----------|-----------------------|-------------------|
//  SCL     |           22          |       A5          |
//  SDA     |           21          |       A4          |

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
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

    lcd.setCursor(0, 2);
    lcd.print("test0123456789");

    lcd.setCursor(0, 3);
    lcd.print("8888888888888888");
    delay(1000);
}