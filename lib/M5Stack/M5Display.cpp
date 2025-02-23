#include "M5Display.h"

#ifdef FRI3DBADGE2022
#include <Wire.h>
#endif

#define BLK_PWM_CHANNEL 7 //LEDC_CHANNEL_7

M5Display::M5Display() : M5m_eSPI() {}

void M5Display::begin()
{
  M5m_eSPI::begin();
  #ifdef M5STACK
  setRotation(1); // M5stack landscape
//  setRotation(2); // M5stack portrait
  #else
  //  setRotation(3); 
#ifdef FRI3DBADGE2022
  setRotation(6); // non M5STACK portrait
  #else
  setRotation(6); // non M5STACK portrait
  // setRotation(7); // non M5stack landscape
  #endif
  #endif
  fillScreen(0);

  // Init the back-light LED PWM
  ledcSetup(BLK_PWM_CHANNEL, 50000, 8);  
#ifdef TFT_BL
  ledcAttachPin(TFT_BL, BLK_PWM_CHANNEL);
#endif
  ledcWrite(BLK_PWM_CHANNEL, 80);
}

inline void M5Display::startWrite(void)
{
#if defined(SPI_HAS_TRANSACTION) && defined(SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
  if (locked)
  {
    locked = false;
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  }
#endif
  CS_L;
}

inline void M5Display::endWrite(void)
{
#if defined(SPI_HAS_TRANSACTION) && defined(SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
  if (!inTransaction)
  {
    if (!locked)
    {
      locked = true;
      SPI.endTransaction();
    }
  }
#endif
  CS_H;
}

inline void M5Display::writePixels(uint16_t *colors, uint32_t len)
{
  SPI.writePixels((uint8_t *)colors, len * 2);
}

void M5Display::sleep()
{
  #ifdef ILI9341_DRIVER
  startWrite();
  writecommand(ILI9341_SLPIN); // Software reset
  endWrite();
  #else
  startWrite();
  writecommand(TFT_SLPIN); // Software reset
  endWrite();
  #endif
}

void M5Display::setBrightness(uint8_t brightness)
{
#ifdef M5STACK
  ledcWrite(BLK_PWM_CHANNEL, brightness);
#elif defined(FRI3DBADGE2022)
  if (brightness) {
    Wire.begin();
    Wire.beginTransmission(0x18);
    Wire.write(0x25);
    Wire.write(0x0A);
    Wire.endTransmission(true);
  } else {
    Wire.begin();
    Wire.beginTransmission(0x18);
    Wire.write(0x25);
    Wire.write(0x00);
    Wire.endTransmission(true);
  }
  #else
  ledcWrite(BLK_PWM_CHANNEL, 255 - brightness);
  #endif
}

void M5Display::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data)
{
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data);
}

void M5Display::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint16_t *data)
{
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data);
}

void M5Display::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint16_t *data, uint16_t transparent)
{
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, data, transparent);
}

void M5Display::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, const uint8_t *data)
{
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, (const uint16_t *)data);
}

void M5Display::drawBitmap(int16_t x0, int16_t y0, int16_t w, int16_t h, uint8_t *data)
{
  setSwapBytes(true);
  pushImage((int32_t)x0, (int32_t)y0, (uint32_t)w, (uint32_t)h, (uint16_t *)data);
}

void M5Display::progressBar(int x, int y, int w, int h, uint8_t val)
{
  drawRect(x, y, w, h, 0x09F1);
  fillRect(x + 1, y + 1, w * (((float)val) / 100.0), h - 1, 0x09F1);
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Bodmers BMP image rendering function
void M5Display::drawBmpFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y)
{

  if ((x >= width()) || (y >= height()))
    return;

  // Open requested file on SD card
  File bmpFS = fs.open(path, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
      }
      Serial.print("Loaded in ");
      Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else
      Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

/*
 * JPEG
 * */

#include "rom/tjpgd.h"

#define jpgColor(c)                                \
  (((uint16_t)(((uint8_t *)(c))[0] & 0xF8) << 8) | \
   ((uint16_t)(((uint8_t *)(c))[1] & 0xFC) << 3) | \
   ((((uint8_t *)(c))[2] & 0xF8) >> 3))

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
const char *jd_errors[] = {"Succeeded",
                           "Interrupted by output function",
                           "Device error or wrong termination of input stream",
                           "Insufficient memory pool for the image",
                           "Insufficient stream input buffer",
                           "Parameter error",
                           "Data format error",
                           "Right format but not supported",
                           "Not supported JPEG standard"};
#endif

typedef struct
{
  uint16_t x;
  uint16_t y;
  uint16_t maxWidth;
  uint16_t maxHeight;
  uint16_t offX;
  uint16_t offY;
  jpeg_div_t scale;
  const void *src;
  size_t len;
  size_t index;
  M5Display *tft;
  uint16_t outWidth;
  uint16_t outHeight;
} jpg_file_decoder_t;

static uint32_t jpgReadFile(JDEC *decoder, uint8_t *buf, uint32_t len)
{
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  File *file = (File *)jpeg->src;
  if (buf)
  {
    return file->read(buf, len);
  }
  else
  {
    file->seek(len, SeekCur);
  }
  return len;
}

static uint32_t jpgRead(JDEC *decoder, uint8_t *buf, uint32_t len)
{
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  if (buf)
  {
    memcpy(buf, (const uint8_t *)jpeg->src + jpeg->index, len);
  }
  jpeg->index += len;
  return len;
}

static uint32_t jpgWrite(JDEC *decoder, void *bitmap, JRECT *rect)
{
  jpg_file_decoder_t *jpeg = (jpg_file_decoder_t *)decoder->device;
  uint16_t x = rect->left;
  uint16_t y = rect->top;
  uint16_t w = rect->right + 1 - x;
  uint16_t h = rect->bottom + 1 - y;
  uint16_t oL = 0, oR = 0;
  uint8_t *data = (uint8_t *)bitmap;

  if (rect->right < jpeg->offX)
  {
    return 1;
  }
  if (rect->left >= (jpeg->offX + jpeg->outWidth))
  {
    return 1;
  }
  if (rect->bottom < jpeg->offY)
  {
    return 1;
  }
  if (rect->top >= (jpeg->offY + jpeg->outHeight))
  {
    return 1;
  }
  if (rect->top < jpeg->offY)
  {
    uint16_t linesToSkip = jpeg->offY - rect->top;
    data += linesToSkip * w * 3;
    h -= linesToSkip;
    y += linesToSkip;
  }
  if (rect->bottom >= (jpeg->offY + jpeg->outHeight))
  {
    uint16_t linesToSkip = (rect->bottom + 1) - (jpeg->offY + jpeg->outHeight);
    h -= linesToSkip;
  }
  if (rect->left < jpeg->offX)
  {
    oL = jpeg->offX - rect->left;
  }
  if (rect->right >= (jpeg->offX + jpeg->outWidth))
  {
    oR = (rect->right + 1) - (jpeg->offX + jpeg->outWidth);
  }

  uint16_t pixBuf[32];
  uint8_t pixIndex = 0;
  uint16_t line;

  jpeg->tft->startWrite();
  // jpeg->tft->setAddrWindow(x - jpeg->offX + jpeg->x + oL, y - jpeg->offY +
  // jpeg->y, w - (oL + oR), h);
  jpeg->tft->setAddrWindow(x - jpeg->offX + jpeg->x + oL,
                           y - jpeg->offY + jpeg->y,
                           x - jpeg->offX + jpeg->x + oL + w - (oL + oR) - 1,
                           y - jpeg->offY + jpeg->y + h - 1);

  while (h--)
  {
    data += 3 * oL;
    line = w - (oL + oR);
    while (line--)
    {
      pixBuf[pixIndex++] = jpgColor(data);
      data += 3;
      if (pixIndex == 32)
      {
        jpeg->tft->writePixels(pixBuf, 32);
        // SPI.writePixels((uint8_t *)pixBuf, 64);
        pixIndex = 0;
      }
    }
    data += 3 * oR;
  }
  if (pixIndex)
  {
    jpeg->tft->writePixels(pixBuf, pixIndex);
    // SPI.writePixels((uint8_t *)pixBuf, pixIndex * 2);
  }
  jpeg->tft->endWrite();
  return 1;
}

static bool jpgDecode(jpg_file_decoder_t *jpeg,
                      uint32_t (*reader)(JDEC *, uint8_t *, uint32_t))
{
  static uint8_t work[3100];
  JDEC decoder;

  JRESULT jres = jd_prepare(&decoder, reader, work, 3100, jpeg);
  if (jres != JDR_OK)
  {
    log_e("jd_prepare failed! %s", jd_errors[jres]);
    return false;
  }

  uint16_t jpgWidth = decoder.width / (1 << (uint8_t)(jpeg->scale));
  uint16_t jpgHeight = decoder.height / (1 << (uint8_t)(jpeg->scale));

  if (jpeg->offX >= jpgWidth || jpeg->offY >= jpgHeight)
  {
    log_e("Offset Outside of JPEG size");
    return false;
  }

  size_t jpgMaxWidth = jpgWidth - jpeg->offX;
  size_t jpgMaxHeight = jpgHeight - jpeg->offY;

  jpeg->outWidth =
      (jpgMaxWidth > jpeg->maxWidth) ? jpeg->maxWidth : jpgMaxWidth;
  jpeg->outHeight =
      (jpgMaxHeight > jpeg->maxHeight) ? jpeg->maxHeight : jpgMaxHeight;

  jres = jd_decomp(&decoder, jpgWrite, (uint8_t)jpeg->scale);
  if (jres != JDR_OK)
  {
    log_e("jd_decomp failed! %s", jd_errors[jres]);
    return false;
  }

  return true;
}

void M5Display::HprogressBar(int x, int y, int w, int h, uint32_t color, uint8_t val, bool redraw)
{
  if (redraw)
  {
    fillRoundRect(x + 1, y + 1, w, h - 2, 7, 0);
  }
  drawRoundRect(x, y, w, h, 8, color);
  fillRoundRect(x + 1, y + 1, w * (((float)val) / 100.0), h - 2, 7, color);
}

void M5Display::VprogressBar(int x, int y, int w, int h, uint32_t color, uint8_t val, bool redraw)
{
  if (redraw)
  {
    fillRoundRect(x + 1, y + 1, w - 1, h - 1, 10, 0);
  }
  drawRoundRect(x, y, w, h, 10, color);
  fillRoundRect(x + 1, y + (h - (h * (((float)val) / 100.0))), w - 2, h * (((float)val) / 100.0), 10, color);
}

void M5Display::drawJpg(const uint8_t *jpg_data, size_t jpg_len, uint16_t x,
                        uint16_t y, uint16_t maxWidth, uint16_t maxHeight,
                        uint16_t offX, uint16_t offY, jpeg_div_t scale)
{
  if ((x + maxWidth) > width() || (y + maxHeight) > height())
  {
    log_e("Bad dimensions given");
    return;
  }

  jpg_file_decoder_t jpeg;

  if (!maxWidth)
  {
    maxWidth = width() - x;
  }
  if (!maxHeight)
  {
    maxHeight = height() - y;
  }

  jpeg.src = jpg_data;
  jpeg.len = jpg_len;
  jpeg.index = 0;
  jpeg.x = x;
  jpeg.y = y;
  jpeg.maxWidth = maxWidth;
  jpeg.maxHeight = maxHeight;
  jpeg.offX = offX;
  jpeg.offY = offY;
  jpeg.scale = scale;
  jpeg.tft = this;

  jpgDecode(&jpeg, jpgRead);
}

void M5Display::drawJpgFile(fs::FS &fs, const char *path, uint16_t x, uint16_t y,
                            uint16_t maxWidth, uint16_t maxHeight, uint16_t offX,
                            uint16_t offY, jpeg_div_t scale)
{
  if ((x + maxWidth) > width() || (y + maxHeight) > height())
  {
    log_e("Bad dimensions given");
    return;
  }

  File file = fs.open(path);
  if (!file)
  {
    log_e("Failed to open file for reading");
    return;
  }

  jpg_file_decoder_t jpeg;

  if (!maxWidth)
  {
    maxWidth = width() - x;
  }
  if (!maxHeight)
  {
    maxHeight = height() - y;
  }

  jpeg.src = &file;
  jpeg.len = file.size();
  jpeg.index = 0;
  jpeg.x = x;
  jpeg.y = y;
  jpeg.maxWidth = maxWidth;
  jpeg.maxHeight = maxHeight;
  jpeg.offX = offX;
  jpeg.offY = offY;
  jpeg.scale = scale;
  jpeg.tft = this;

  jpgDecode(&jpeg, jpgReadFile);

  file.close();
}