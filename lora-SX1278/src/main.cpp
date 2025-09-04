#include <Arduino.h>

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

#define MODE TX    // set mode to TX or RX
#define NSS 5
#define RST 21
#define DIO0 22

int io[] = {32, 33, 25, 26, 27, 14, 13, 4};
byte data = 0;

void setup() {
    Serial.begin(115200);

    LoRa.setPins(NSS, RST, DIO0);
    
    if (!LoRa.begin(433E6)) {
        while(1);
    }

#if MODE == TX
    for (auto i : io) pinMode(i, INPUT);
    Serial.println("LoRa in Transmitter Mode.");
#elif MODE == RX
    for (auto i : io) pinMode(i, OUTPUT);
    Serial.println("LoRa Receiver Mode.");
#endif
}

void loop() {

#if MODE == TX
    data = 0;
    for(int i = 0; i < 8; i++) {
        data |= (!digitalRead(io[i]) << i);
    }

    LoRa.beginPacket();
    LoRa.write(data);
    LoRa.endPacket();
    delay(50);

#elif MODE == RX
    int parsePacket = LoRa.parsePacket();

    if (parsePacket) {
        Serial.println("data received: ");
        while(LoRa.available()) {
            data = LoRa.read();
        }
        Serial.println(data, BIN);

        for (int i = 7; i >= 0; i--) digitalWrite(io[i], data & (1 << i));
    }
#endif
}
