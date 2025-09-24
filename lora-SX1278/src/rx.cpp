//   Pin    |   ESP23 Dev Module    |   Arduino UNO     |
//          |       (36 pins)       |                   |
//----------|-----------------------|-------------------|------------------
//  NSS     |           5           |       10          |
//  MOSI    |           23          |       11          |
//  MISO    |           19          |       12          |
//  SCK     |           18          |       13          |
//  RST     |           21          |       A1          |   <user-defined>
//  DIO0    |           22          |       A0          |   <user-defined>
//  LED     |           2           |   LED_BUILTIN     |

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <LoRa.h>

#define NSS 5
#define RST 32
#define DIO0 33
#define LED 2

LiquidCrystal_I2C lcd(0x27, 20, 4);

int io[] = {32, 33, 25, 26, 27, 14, 13, 4};
int num = 0;
byte data = 0;
String lora_id = "0x01";

void printLCD(String message, int row);

void setup() {
    Serial.begin(115200);

    lcd.init();
    lcd.backlight();

    //SPI.begin(SCK, MISO, MOSI, NSS);  //initialize SPI explicitly
    LoRa.setPins(NSS, RST, DIO0);

    for (auto i : io) pinMode(i, INPUT); // initialize 8 pins as inputs
    pinMode(LED, OUTPUT); 
    
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa initialization failed.");
        while(1);
    }

    Serial.println("LoRa RX ready.");

    lcd.setCursor(0, 0);
    lcd.print("LoRa RX");
    lcd.setCursor(0, 1);
    lcd.print("initiated successfully.");
}

void loop() {
    String message, lora_id, msgPart, numPart;
    int o_bracketIndex = -1, c_bracketIndex = -1, colonIndex = -1;

    if (LoRa.parsePacket()) {
        lcd.clear();
        message = "";
        while(LoRa.available()) {
            message += (char)LoRa.read();
        }
        Serial.println(message);
        message.trim();
        
        o_bracketIndex = message.indexOf('<');
        c_bracketIndex = message.indexOf('>');
        colonIndex = message.indexOf(':');
        
        if (o_bracketIndex != -1 && c_bracketIndex != -1 && colonIndex != -1) {
            lora_id = message.substring(o_bracketIndex + 1, c_bracketIndex);
            msgPart = message.substring(c_bracketIndex + 1, colonIndex);
            msgPart.trim();
            numPart = message.substring(colonIndex + 1);
            numPart.trim();
        }   
        
        for (int i = 7; i >= 0; i--) digitalWrite(io[i], data & (1 << i));
    }

    printLCD("LoRa TX ID: " + lora_id, 0);
    printLCD(msgPart, 1);
}

void printLCD(String message, int row) {
    lcd.setCursor(0, row);
    lcd.print(message);
}