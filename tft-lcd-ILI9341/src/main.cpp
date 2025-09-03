#include "esp_flash.h"
#include "esp_partition.h"
#include <Arduino.h>

#include <bb_spi_lcd.h>

#include <AnimatedGIF.h>

#include "windows_startup.h" //plays Windows XP starting sequence

//TFT LCD
// CS   15
// DC   2
// RST  4
// LED  
// MISO
// MOSI 23
// CLK  18

BB_SPI_LCD tft;

#define GifData windows_startup

AnimatedGIF *openGif(uint8_t *gifdata, size_t gifsize);
void *GIFAlloc(uint32_t u32Size);
void GIFFree(void *p);
void GIFDraw(GIFDRAW *pDraw);
void printGifErrorMessage(int errorCode);

void setup()
{
    Serial.begin(115200);
    tft.begin(LCD_ILI9341, FLAGS_NONE, 40000000, 15, 2, 4, -1, -1, 23, 18); //
    tft.setRotation(LCD_ORIENTATION_0);                                   // Make sure you have the right orientation based on your GIF
                                                                            // or the GIF will show incorrectly, even garbage output
                                                                            // Values : LCD_ORIENTATION_0, LCD_ORIENTATION_90, LCD_ORIENTATION_180 or LCD_ORIENTATION_270
    tft.fillScreen(TFT_BLACK);

    AnimatedGIF *gif;
    gif = openGif((uint8_t *)GifData, sizeof(GifData));
    if (gif == NULL)
    {
        Serial.println("Cannot open GIF");
        while (true)
        {
        //  no need to continue
        }
    }

    while (true)
    {
        gif->playFrame(true, NULL);
    }
}

void loop()
{
    delay(1);
}

// Open Gif and allocate memory
AnimatedGIF *openGif(uint8_t *gifdata, size_t gifsize)
{
    AnimatedGIF *gif;
    gif = (AnimatedGIF *)malloc(sizeof(AnimatedGIF));
    if (gif == NULL)
    {
        Serial.println("Not RAM Enough memory for GIF structure");
        return NULL;
    }

    gif->begin(GIF_PALETTE_RGB565_BE); // Set the cooked output type we want (compatible with SPI LCDs)

    if (gif->open(gifdata, gifsize, GIFDraw))
    {
        Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif->getCanvasWidth(), gif->getCanvasHeight());
        Serial.printf("GIF memory size is %ld (%2.2f MB)", gifsize, (float)gifsize / (1024 * 1024));
        gif->setDrawType(GIF_DRAW_COOKED); // We want the Animated GIF library to generate ready-made pixels
        if (gif->allocFrameBuf(GIFAlloc) != GIF_SUCCESS)
        {
        Serial.println("Not Enough RAM memory for frame buffer");
        return NULL;
        }
        return gif;
    }
    else
    {
        printGifErrorMessage(gif->getLastError());
        return NULL;
    }
}

//
// The memory management functions are needed to keep operating system
// dependencies out of the core library code
//
// memory allocation callback function
void *GIFAlloc(uint32_t u32Size)
{
    return malloc(u32Size);
} /* GIFAlloc() */
// memory free callback function
void GIFFree(void *p)
{
    free(p);
}

// Draw callback from the AnimatedGIF decoder
void GIFDraw(GIFDRAW *pDraw)
{
    if (pDraw->y == 0)
    { // set the memory window (once per frame) when the first line is rendered
        tft.setAddrWindow(pDraw->iX, pDraw->iY, pDraw->iWidth, pDraw->iHeight);
    }
    // For all other lines, just push the pixels to the display. We requested 'COOKED'big-endian RGB565 and
    tft.pushPixels((uint16_t *)pDraw->pPixels, pDraw->iWidth);
}

// Get human-readable error related to GIF
void printGifErrorMessage(int errorCode)
{
    switch (errorCode)
    {
    case GIF_DECODE_ERROR:
        Serial.println("GIF Decoding Error");
        break;
    case GIF_TOO_WIDE:
        Serial.println("GIF Too Wide");
        break;
    case GIF_INVALID_PARAMETER:
        Serial.println("Invalid Parameter for gif open");
        break;
    case GIF_UNSUPPORTED_FEATURE:
        Serial.println("Unsupported feature in GIF");
        break;
    case GIF_FILE_NOT_OPEN:
        Serial.println("GIF File not open");
        break;
    case GIF_EARLY_EOF:
        Serial.println("GIF early end of file");
        break;
    case GIF_EMPTY_FRAME:
        Serial.println("GIF with empty frame");
        break;
    case GIF_BAD_FILE:
        Serial.println("GIF bad file");
        break;
    case GIF_ERROR_MEMORY:
        Serial.println("GIF memory Error");
        break;
    default:
        Serial.println("Unknown Error");
        break;
    }
}