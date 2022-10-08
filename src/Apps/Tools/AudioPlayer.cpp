#include "AudioPlayer.h"

void AudioPlayerClass::getvolume()
{
    preferences.begin("Volume", false);
    M5m.vol = preferences.getFloat("vol", 15.0f);
    preferences.end();
}

void AudioPlayerClass::setVolume(int *v)
{
    float volume = *v / 71.4f; // volme max value can be 3.99
    out->SetGain(volume);
}

void AudioPlayerClass::drawSpectrum(int a, int b, int c, int d, int e, int f, int g)
{
    aH = ((a * height) / 100);
    aY = ys + (height - aH);
    M5m.Lcd.fillRect(xs, ys, widthS, height, 0);
    M5m.Lcd.fillRect(xs, aY, widthS, aH, TFT_LIGHTGREY); //0xe8e4

    bH = ((b * height) / 100);
    bY = ys + (height - bH);
    bX = xs + widthS + padding;
    M5m.Lcd.fillRect(bX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(bX, bY, widthS, bH, TFT_LIGHTGREY); //0xff80

    cH = ((c * height) / 100);
    cY = ys + (height - cH);
    cX = bX + widthS + padding;
    M5m.Lcd.fillRect(cX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(cX, cY, widthS, cH, TFT_LIGHTGREY); //0x2589

    dH = ((d * height) / 100);
    dY = ys + (height - dH);
    dX = cX + widthS + padding;
    M5m.Lcd.fillRect(dX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(dX, dY, widthS, dH, TFT_LIGHTGREY); //0x051d

    eH = ((e * height) / 100);
    eY = ys + (height - eH);
    eX = dX + widthS + padding;
    M5m.Lcd.fillRect(eX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(eX, eY, widthS, eH, TFT_LIGHTGREY); //0x051d

    fH = ((f * height) / 100);
    fY = ys + (height - fH);
    fX = eX + widthS + padding;
    M5m.Lcd.fillRect(fX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(fX, fY, widthS, fH, TFT_LIGHTGREY); //0x051d

    gH = ((g * height) / 100);
    gY = ys + (height - gH);
    gX = fX + widthS + padding;
    M5m.Lcd.fillRect(gX, ys, widthS, height, 0);
    M5m.Lcd.fillRect(gX, gY, widthS, gH, TFT_LIGHTGREY); //0x051d
}

void AudioPlayerClass::genSpectrum()
{
    currentMillis = millis();
    if (currentMillis - genSpectrum_previousMillis > 100)
    {
        genSpectrum_previousMillis = currentMillis;
        drawSpectrum(random(0, 101), random(0, 101), random(0, 101), random(0, 101), random(0, 101), random(0, 101), random(0, 101));
    }
}

void AudioPlayerClass::drawTimeline()
{
    currentMillis = millis();
    if (currentMillis - drawTimeline_previousMillis > 250)
    {
        yClear = y - (heightMark / 2);
        wClear = width + (widthMark / 2);

        drawTimeline_previousMillis = currentMillis;
        M5m.Lcd.fillRect(x, yClear + 2, wClear, heightMark, 0);
        M5m.Lcd.fillRect(x, y, width, heightLine, 31727);
        size_ = file->getSize();
        pos_ = file->getPos();
        xPos = x + ((pos_ * (width - (widthMark / 2))) / size_);
        M5m.Lcd.fillRect(xPos, yClear + 2, widthMark, heightMark, 59620);
        M5m.Lcd.setTextColor(TFT_BLACK);
        M5m.Lcd.drawNumber((oldpos_), width + 50, y - 7, 2);
        M5m.Lcd.setTextColor(TFT_WHITE);
        M5m.Lcd.drawNumber((pos_), width + 50, y - 7, 2);
        oldpos_ = pos_;
    }
}

void AudioPlayerClass::Play(String *fileName, int fileType)
{
    M5m.windowClr();
    M5m.Lcd.setTextColor(TFT_CYAN);
    M5m.Lcd.drawCentreString(*fileName, 158, 140, 2);
    M5m.Lcd.setTextColor(TFT_WHITE);
    getvolume();
    file = new AudioFileSourceSD((*fileName).c_str());
    // out = new AudioOutputI2S(0, 1);
    out = new AudioOutputI2S(0, 1, 32);

    if (fileType == 1) // MP3
    {
        sndStream = new AudioGeneratorMP3();
    }
    else if (fileType == 2) // WAV
    {
        sndStream = new AudioGeneratorWAV();
    }
    else if (fileType == 3) // AAC
    {
        sndStream = new AudioGeneratorAAC();
    }
    else if (fileType == 4) // FLAC
    {
        sndStream = new AudioGeneratorFLAC();
    }
    
    out->SetChannels(2);
    sndStream->begin(file, out);
    setVolume(&M5m.vol);
    M5m.old_vol = M5m.vol;
    M5m.Lcd.setTextColor(TFT_ORANGE);
    M5m.Lcd.drawCentreString("Volume: " + String(M5m.vol), 158, 190, 2);
    M5m.Lcd.setTextColor(TFT_WHITE);

    while (!M5m.BtnB.wasPressed())
    {
        if (sndStream->isRunning())
        {
            if (!sndStream->loop())
            {
                sndStream->stop();
                break;
            }
            genSpectrum();
            drawTimeline();
        }
        if (M5m.BtnA.wasPressed() && M5m.vol > 0)
        {
            M5m.vol -= 5;
            setVolume(&M5m.vol);
        }
        if (M5m.BtnC.wasPressed() && M5m.vol < 100)
        {
            M5m.vol += 5;
            setVolume(&M5m.vol);
        }
        if (M5m.vol != M5m.old_vol)
        {
            M5m.Lcd.setTextColor(TFT_ORANGE);
            M5m.Lcd.fillRect(120, 190, 80, 14, TFT_BLACK);
            M5m.Lcd.drawCentreString("Volume: " + String(M5m.vol), 158, 190, 2);
            M5m.Lcd.setTextColor(TFT_WHITE);
            M5m.old_vol = M5m.vol;
        }
        M5m.update();
    }
    preferences.begin("Volume", false);
    preferences.putFloat("vol", M5m.vol);
    preferences.end();
    sndStream->stop();
    out->stop();
    file->close();
    delete sndStream;
    delete out;
    delete file;
    sndStream = NULL;
    out = NULL;
    file = NULL;
    dacWrite(25, 0);
    dacWrite(26, 0);
    M5m.windowClr();
}

AudioPlayerClass::AudioPlayerClass()
{
    M5m.update();
    M5m.drawAppMenu(F("AudioPlayer"), F("VOL-"), F("EXIT"), F("VOL+"));
}

AudioPlayerClass::~AudioPlayerClass()
{
    M5m.drawAppMenu(F("SD BROWSER"), F("EXIT"), F("OPEN"), F(">"));
    M5m.showList();
}