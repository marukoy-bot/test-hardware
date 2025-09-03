#include <Arduino.h>
#include "OV7670.h"
#include "BMP.h"
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "esp32-hal-psram.h"

uint16_t *scaled;

const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int VSYNC = 34;
const int HREF = 35;

const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 17;
const int D2 = 16;
const int D3 = 15;
const int D4 = 14;
const int D5 = 13;
const int D6 = 12;
const int D7 = 4;

//DIN <- MOSI 23
//CLK <- SCK 18

#define AP_SSID        "00000000"
#define AP_PASSWORD    "00000000"

OV7670 *camera;
TFT_eSPI tft = TFT_eSPI();
TaskHandle_t ServeTask;
TaskHandle_t CameraTask;

WiFiServer server(80);

unsigned char bmpHeader[BMP::headerSize];

inline uint16_t swap565(uint16_t c) {
    uint16_t r = (c & 0xF800) >> 11;
    uint16_t g = (c & 0x07E0);
    uint16_t b = (c & 0x001F) << 11;
    return (b | g | r);
}

void lcdDisplay() {
    if (camera->frame != nullptr) {
        uint16_t *src = (uint16_t*)camera->frame;
        static uint16_t lineBuf[320];  // one scaled line, 320px wide

        for (int y = 0; y < 120; y++) {
            // expand one camera line into 2 TFT lines
            for (int rep = 0; rep < 2; rep++) {
                for (int x = 0; x < 160; x++) {
                    uint16_t c = src[y * 160 + x];

                    // swap red/blue
                    uint16_t r = (c & 0xF800) >> 11;
                    uint16_t g = (c & 0x07E0);
                    uint16_t b = (c & 0x001F) << 11;
                    c = (b | g | r);

                    // duplicate pixel horizontally
                    lineBuf[x * 2]     = c;
                    lineBuf[x * 2 + 1] = c;
                }
                // push the expanded line
                tft.pushImage(0, y * 2 + rep, 320, 1, lineBuf);
            }
        }
    }
}

void cameraFrame(void * parameters) {
	Serial.println("Camera frame task running on core " + String(xPortGetCoreID()));

	for (;;) {
		camera->oneFrame();
		lcdDisplay();
		vTaskDelay(1);
	}
}

void serve(void * parameters) {
	Serial.println("Server streaming task running on core " + String(xPortGetCoreID()));

	for (;;) {
		WiFiClient client = server.available();
		if (client) {
			String currentLine = "";
			while (client.connected()) {
				if (client.available()) {
					char c = client.read();
	
					if (c == '\n') {
						if (currentLine.length() == 0) {
							client.println("HTTP/1.1 200 OK");
							client.println("Content-type:text/html");
							client.println();
							client.print(
								"<style>body{margin: 0}\nimg{height: auto; width: 100%}</style>"
								"<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
								"<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>"
							);
							client.println();
							break;
						}
						else {
							currentLine = "";
						}
					} 
					else if (c != '\r') {
						currentLine += c;
					}
	
					if (currentLine.endsWith("GET /camera")) {
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:image/bmp");
						client.println();
	
						client.write(bmpHeader, BMP::headerSize);
						client.write(camera->frame, camera->xres * camera->yres * 2);
					}
				}
			}
			client.stop();
		}
		vTaskDelay(1);
	}
}

void initializeTFT() {
	tft.init();
	tft.setRotation(1); 
	tft.fillScreen(TFT_BLACK);
}

void setup() {
	Serial.begin(115200);

	if (psramFound()) {
        scaled = (uint16_t*)ps_malloc(320 * 240 * 2);
    } else {
        Serial.println("No PSRAM found!");
    }
	
	xTaskCreatePinnedToCore(
		serve,
		"ServeTask",
		10000,
		NULL,
		1,
		&ServeTask,
		0
	);
	delay(500);

	xTaskCreatePinnedToCore(
		cameraFrame,
		"cameraTask",
		10000,
		NULL,
		1,
		&CameraTask,
		1
	);
	delay(500);

	initializeTFT();

	WiFi.mode(WIFI_AP);
	WiFi.softAP(AP_SSID, AP_PASSWORD);

	Serial.println("*Wifi AP Started*");
    Serial.print("SSID:\t"); Serial.println(AP_SSID);
    Serial.print("Password:\t"); Serial.println(AP_PASSWORD);
    Serial.print("IP Address:\t"); Serial.println(WiFi.softAPIP());
  
    camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
    BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

    Serial.print("Camera Resolution:\t"); Serial.print(camera->xres); Serial.print("x"); Serial.println(camera->yres);
    server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
	//camera->oneFrame();
  	//serve();
}