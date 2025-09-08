#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <LoRa.h>

//   Pin    |   ESP23 Dev Module    |   Arduino UNO     |
//----------|-----------------------|-------------------|
//  NSS     |           5           |       10          |
//  MOSI    |           23          |       11          |
//  MISO    |           19          |       12          |
//  SCK     |           18          |       13          |
//  RST     |           21          |       A1          |   <user-defined>
//  DIO0    |           22          |       A0          |   <user-defined>
//  LED     |           2           |   LED_BUILTIN     |

#define MODE_TX 0 
#define MODE_RX 1

#define MODE MODE_RX    // set mode to MODE_TX or MODE_RX

#define NSS 5
#define RST 32
#define DIO0 33

LiquidCrystal_I2C lcd(0x27, 20, 4);

int io[] = {32, 33, 25, 26, 27, 14, 13, 4};
byte data = 0;
int num = 0;

void setup() {
    Serial.begin(115200);

    LoRa.setPins(NSS, RST, DIO0);
    pinMode(2, OUTPUT);
    
    if (!LoRa.begin(433E6)) {
        while(1);
    }

#if MODE == MODE_TX
    for (auto i : io) pinMode(i, INPUT);
    Serial.println("LoRa in Transmitter Mode.");
#elif MODE == MODE_RX
    lcd.init();
    lcd.backlight();
    for (auto i : io) pinMode(i, OUTPUT);
    Serial.println("LoRa Receiver Mode.");

    lcd.setCursor(0, 0);
    lcd.print("LoRa receiver.");
    lcd.setCursor(0, 1);
    lcd.print("initiated successfully.");
#endif
}

void loop() {
#if MODE == MODE_TX
    data = 0x55;    // dummy data
    // for(int i = 0; i < 8; i++) {
    //     data |= (!digitalRead(io[i]) << i);
    // }

    LoRa.beginPacket();
    LoRa.println(String(num++) + "Sending message from TX");
    //LoRa.write(data);
    LoRa.endPacket();
    delay(1000);

#elif MODE == MODE_RX
String message;
int parsePacket = LoRa.parsePacket();

if (parsePacket) {
    lcd.clear();
    Serial.println("data received: ");
    while(LoRa.available()) {
        message += (char)LoRa.read();
    }
    Serial.println(message);
    //Serial.println(data, BIN);
    
    
    
    //for (int i = 7; i >= 0; i--) digitalWrite(io[i], data & (1 << i));
}

    lcd.setCursor(0, 0);
    lcd.print(message);
#endif
}
