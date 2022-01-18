// Copyright (c) M5StackMod. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "M5StackMod.h"

M5StackMod::M5StackMod()
{
  levelIDX = 0;
  menuCount[levelIDX] = 0;
  menuIDX = 0;
  menucolor = getrgb(0, 0, 128);
  windowcolor = getrgb(0, 0, 0);
  menutextcolor = getrgb(255, 255, 255);
  clearList();
}

void M5StackMod::setListCaption(String inCaption)
{
  listCaption = inCaption;
}

void M5StackMod::clearList()
{
  list_count = 0;
  list_pages = 0;
  list_page = 0;
  list_lastpagelines = 0;
  list_idx = 0;
  list_labels.clear();
  list_labels.shrink_to_fit();
  listCaption = "";
}

void M5StackMod::addList(String inStr)
{
  if (inStr.length() <= LIST_MAX_LABEL_SIZE && inStr.length() > 0)
  {
    list_labels.push_back(inStr);
    list_count++;
  }
  if (list_count > 0)
  {
    if (list_count > LIST_PAGE_LABELS)
    {
      list_lastpagelines = list_count % LIST_PAGE_LABELS;
      if (list_lastpagelines > 0)
      {
        list_pages = (list_count - list_lastpagelines) / LIST_PAGE_LABELS;
        list_pages++;
      }
      else
      {
        list_pages = list_count / LIST_PAGE_LABELS;
      }
    }
    else
    {
      list_pages = 1;
    }
  }
}

unsigned int M5StackMod::getListID()
{
  return list_idx;
}

String M5StackMod::getListString()
{
  return list_labels[list_idx];
}

void M5StackMod::nextList()
{
  if (list_idx < list_page * LIST_PAGE_LABELS + list_lines - 1)
  {
    list_idx++;
  }
  else
  {
    if (list_page < list_pages - 1)
    {
      list_page++;
    }
    else
    {
      list_page = 0;
    }
    list_idx = list_page * LIST_PAGE_LABELS;
  }
  showList();
}

void M5StackMod::drawListItem(uint32_t inIDX, uint32_t postIDX)
{
  if (inIDX == list_idx)
  {
    Lcd.drawString(list_labels[inIDX], 15, 80 + (postIDX * 20), 2);
    Lcd.drawString(">", 3, 80 + (postIDX * 20), 2);
  }
  else
  {
    Lcd.drawString(list_labels[inIDX], 15, 80 + (postIDX * 20), 2);
  }
}

void M5StackMod::showList()
{
  windowClr();
  unsigned int labelid = 0;
  Lcd.drawCentreString(listCaption, Lcd.width() / 2, 45, 2);
  if ((list_page + 1) == list_pages)
  {
    if (list_lastpagelines == 0 && list_count >= LIST_PAGE_LABELS)
    {
      list_lines = LIST_PAGE_LABELS;
      for (uint32_t i = 0; i < LIST_PAGE_LABELS; i++)
      {
        labelid = i + (list_page * LIST_PAGE_LABELS);
        drawListItem(labelid, i);
      }
    }
    else
    {
      if (list_pages > 1)
      {
        list_lines = list_lastpagelines;
        for (uint32_t i = 0; i < list_lastpagelines; i++)
        {
          labelid = i + (list_page * LIST_PAGE_LABELS);
          drawListItem(labelid, i);
        }
      }
      else
      {
        list_lines = list_count;
        for (uint32_t i = 0; i < list_count; i++)
        {
          labelid = i + (list_page * LIST_PAGE_LABELS);
          drawListItem(labelid, i);
        }
      }
    }
  }
  else
  {
    list_lines = LIST_PAGE_LABELS;
    for (uint32_t i = 0; i < LIST_PAGE_LABELS; i++)
    {
      labelid = i + (list_page * LIST_PAGE_LABELS);
      drawListItem(labelid, i);
    }
  }
}

void M5StackMod::up()
{
  if (menuIDX < menuCount[levelIDX] - 1)
  {
    menuIDX++;
  }
  else
  {
    menuIDX = 0;
  }
  show();
}

void M5StackMod::down()
{
  if (menuIDX > 0)
  {
    menuIDX--;
  }
  else
  {
    menuIDX = menuCount[levelIDX] - 1;
  }
  show();
}

void M5StackMod::GoToLevel(uint32_t inlevel)
{
  levelIDX = inlevel;
  menuIDX = 0;
  show();
}

void M5StackMod::execute()
{
  if (menuList[levelIDX][menuIDX].gotoLevel == -1)
  {
    (*menuList[levelIDX][menuIDX].function)();
  }
  else
  {
    GoToLevel(menuList[levelIDX][menuIDX].gotoLevel);
  }
}

void M5StackMod::addMenuItem(uint32_t levelID, const char *menu_title, const char *btnA_title, const char *btnB_title,
                             const char *btnC_title, signed char goto_level, const char *Menu_Img, void (*function)())
{
  uint32_t mCnt = menuCount[levelID];
  menuList[levelID] = (MenuCommandCallback *)realloc(menuList[levelID], (mCnt + 1) * sizeof(MenuCommandCallback));
  strncpy(menuList[levelID][mCnt].title, menu_title, MENU_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnAtitle, btnA_title, BTN_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnBtitle, btnB_title, BTN_TITLE_MAX_SIZE);
  strncpy(menuList[levelID][mCnt].btnCtitle, btnC_title, BTN_TITLE_MAX_SIZE);
  menuList[levelID][mCnt].gotoLevel = goto_level;
  menuList[levelID][mCnt].MenuImg = Menu_Img;
  menuList[levelID][mCnt].function = function;
  menuCount[levelID]++;
}

void M5StackMod::show()
{
  drawMenu(menuList[levelIDX][menuIDX].title, menuList[levelIDX][menuIDX].btnAtitle, menuList[levelIDX][menuIDX].btnBtitle,
           menuList[levelIDX][menuIDX].btnCtitle, menucolor, windowcolor, menuList[levelIDX][menuIDX].MenuImg, menutextcolor);
}

void M5StackMod::windowClr()
{
  Lcd.fillRoundRect(0, 29, Lcd.width(), Lcd.height() - 28 - 28, 5, windowcolor);
}

unsigned int M5StackMod::getrgb(uint8_t inred, uint8_t ingrn, uint8_t inblue)
{
  inred = map(inred, 0, 255, 0, 31);
  ingrn = map(ingrn, 0, 255, 0, 63);
  inblue = map(inblue, 0, 255, 0, 31);
  return inred << 11 | ingrn << 5 | inblue;
}

void M5StackMod::drawAppMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl)
{
  drawMenu(inmenuttl, inbtnAttl, inbtnBttl, inbtnCttl, menucolor, windowcolor, NULL, menutextcolor);
  Lcd.setTextColor(menutextcolor, windowcolor);
}

void M5StackMod::setColorSchema(unsigned int inmenucolor, unsigned int inwindowcolor, unsigned int intextcolor)
{
  menucolor = inmenucolor;
  windowcolor = inwindowcolor;
  menutextcolor = intextcolor;
}

void M5StackMod::btnRestore()
{
  Lcd.setTextColor(menutextcolor);
  Lcd.fillRoundRect(0, Lcd.height() - 28, Lcd.width(), 28, 5, 0x00);
  Lcd.fillRoundRect(31, Lcd.height() - 28, 60, 28, 5, menucolor);
  Lcd.fillRoundRect(126, Lcd.height() - 28, 60, 28, 5, menucolor);
  Lcd.fillRoundRect(221, Lcd.height() - 28, 60, 28, 5, menucolor);
  Lcd.drawCentreString(lastBtnTittle[0], 31 + 30, Lcd.height() - 28 + 6, 2);
  Lcd.drawCentreString(lastBtnTittle[1], 126 + 30, Lcd.height() - 28 + 6, 2);
  Lcd.drawCentreString(lastBtnTittle[2], 221 + 30, Lcd.height() - 28 + 6, 2);
  Lcd.setTextColor(menutextcolor, windowcolor);
}

void M5StackMod::drawMenu(String inmenuttl, String inbtnAttl, String inbtnBttl, String inbtnCttl, unsigned int inmenucolor,
                          unsigned int inwindowcolor, const char *iMenuImg, unsigned int intxtcolor)
{
  lastBtnTittle[0] = inbtnAttl;
  lastBtnTittle[1] = inbtnBttl;
  lastBtnTittle[2] = inbtnCttl;
  Lcd.fillRoundRect(31, Lcd.height() - 28, 60, 28, 5, inmenucolor);
  Lcd.fillRoundRect(126, Lcd.height() - 28, 60, 28, 5, inmenucolor);
  Lcd.fillRoundRect(221, Lcd.height() - 28, 60, 28, 5, inmenucolor);
  Lcd.fillRoundRect(0, 0, Lcd.width(), 28, 5, inmenucolor);
  Lcd.fillRoundRect(0, 29, Lcd.width(), Lcd.height() - 28 - 28, 5, inwindowcolor);
  if (iMenuImg != NULL)
  {
    Lcd.drawJpg((uint8_t *)iMenuImg, (sizeof(iMenuImg) / sizeof(iMenuImg[0])), 0, 30);
  }

  Lcd.setTextColor(intxtcolor);
  Lcd.drawCentreString(inmenuttl, Lcd.width() / 2, 6, 2);

  Lcd.drawCentreString(inbtnAttl, 31 + 30, Lcd.height() - 28 + 6, 2);
  Lcd.drawCentreString(inbtnBttl, 126 + 30, Lcd.height() - 28 + 6, 2);
  Lcd.drawCentreString(inbtnCttl, 221 + 30, Lcd.height() - 28 + 6, 2);
}

void M5StackMod::begin()
{
  // UART
  Serial.begin(115200);
  Serial.flush();
  Serial.print("M5StackMod initializing...");

  // I2C
  #ifdef M5STACK
  pinMode(SCL_PIN, OUTPUT);
  digitalWrite(SDA_PIN, 1);
  Wire.begin(SDA_PIN,SCL_PIN);
  #endif

  // Setup the button with an internal pull-up
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);

  // M5 LCD INIT
  Lcd.begin();
  Lcd.fillScreen(TFT_BLACK);
  Lcd.setCursor(0, 0);
  Lcd.setTextColor(TFT_WHITE);
  Lcd.setTextSize(1);
  Lcd.setBrightness(50);

// TF Card & SPIFFS
#ifdef M5STACK
  My_SD.begin(TFCARD_CS_PIN, SPI, 40000000);
#else
  My_SD.begin();
#endif
  vTaskDelay(10 / portTICK_RATE_MS);

  if (!SPIFFS.begin())
  {
    SPIFFS.format();
    vTaskDelay(10 / portTICK_RATE_MS);
    SPIFFS.begin();
  }
  // Set wakeup button
  setWakeupButton(BUTTON_A_PIN);

  Serial.println("OK");
}

void M5StackMod::update()
{
  //Button update
  BtnA.read();
  BtnB.read();
  BtnC.read();
}

void M5StackMod::setWakeupButton(uint8_t button)
{
  _wakeupPin = button;
}

void M5StackMod::powerOFF()
{
  // power off the Lcd
  Lcd.setBrightness(0);
  Lcd.sleep();

  // ESP32 into deep sleep
  esp_sleep_enable_ext0_wakeup((gpio_num_t)_wakeupPin, LOW);

  while (digitalRead(_wakeupPin) == LOW)
  {
    delay(10);
  }
  USE_SERIAL.println("On deep sleep mode.");
  esp_deep_sleep_start();
  USE_SERIAL.println("On power OFF fail!");
}

M5StackMod::~M5StackMod()
{
}

Preferences preferences;
M5StackMod M5m;
