#include "VideoPlayer.h"

void VideoPlayerClass::Play(const char *fileName)
{
    M5m.update();
    dacWrite(25, 0); // Speaker OFF
    M5m.Lcd.fillScreen(TFT_BLACK);
    file = My_SD.open(fileName);
    if (!file)
    {
        Serial.println("Error opening video file.");
        return;
    }
    M5m.Lcd.setSwapBytes(true);
    while(!M5m.BtnB.wasPressed() && file.read(videoBuffer, 93960))
    {
        M5m.Lcd.pushImage(15,36,290,162,(uint16_t*)videoBuffer);
        M5m.update();
    }
    file.close();
   	dacWrite(25, 0); // Speaker OFF
    return;
}

VideoPlayerClass::VideoPlayerClass()
{
     videoBuffer = (uint8_t*)heap_caps_malloc(93960,MALLOC_CAP_DEFAULT);
}

VideoPlayerClass::~VideoPlayerClass()
{
    heap_caps_free(videoBuffer);
    M5m.drawAppMenu(F("SD BROWSER"), F("EXIT"), F("OPEN"), F(">"));
    M5m.showList();
}