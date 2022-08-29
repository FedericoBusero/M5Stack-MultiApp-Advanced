//======================== intro =======================================
//      Space Shooter, basically knock-off Space Invaders
//             && also maybe a bit of Galaga
//   Written by Tyler Edwards for the Hackerbox #0020 badge
//  But should work on any ESP32 && Adafruit ILI9341 display
//        I am sorry for the messy code, you'll just
//                  have to live with it
//      Tyler on GitHub: https://github.com/HailTheBDFL/
//          Hackerboxes: http://hackerboxes.com/
//=========================== setup ===================================
// Space Shooter with M5STACK : 2018.01.12 Transplant by macsbug
// Controller   : Buttons A = LEFT, B = RIGHT, C = START || SHOOTING
// Github:https://macsbug.wordpress.com/2018/01/12/esp32-spaceshooter-with-m5stack/
//===================================================================
#include "SpaceShooter.h"

//=============================== setup && loop ==================

void SpaceShooterClass::initVars()
{
  //============================= game variables =========================
  offsetM = 0;
  offsetT = 0;
  offsetF = 0;
  offsetB = 0;
  offsetA = 0;
  offsetAF = 0;
  offsetAB = 0;
  offsetS = 0;
  threshold = 40;
  startPrinted = false;
  beginGame = false;
  beginGame2 = true;
  play = false;
  alien_shooter_score = 0;
  alien_shooter_scoreInc = 10;
  level = 1;
  //---------------------Player---------------------------------------
  shipX = 147;
  shipY = 190;
  oldShipX = 0;
  oldShipY = 0;
  changeShipX = 0;
  changeShipY = 0;
  shipSpeed = 50;
  doSplode = false;
  fire = false;
  //--------------------------Aliens----------------------------------
  alienLiveCount = numAliens;
  alienX = 7;
  alienY = 25;
  oldAlienX = 7;
  oldAlienY = 25;
  changeAlienX = 6;
  changeAlienY = 0;
  alienSpeed = 200;
  oldAlienSpeed = 0;
  chanceOfFire = 2;
}
//==================================================================

void SpaceShooterClass::drawBitmap(char img[], int imgW, int imgH, int x, int y, int scale)
{
  uint16_t cellColor = 0;
  char curPix;
  for (int i = 0; i < imgW * imgH; i++)
  {
    curPix = img[i];
    if (curPix == 'W')
    {
      cellColor = TFT_WHITE;
    }
    else if (curPix == 'Y')
    {
      cellColor = TFT_YELLOW;
    }
    else if (curPix == 'B')
    {
      cellColor = TFT_BLUE;
    }
    else if (curPix == 'R')
    {
      cellColor = TFT_RED;
    }
    else if (curPix == 'G')
    {
      cellColor = 0x5E85;
    }
    if (curPix != 'Z' && scale == 1)
    {
      M5m.Lcd.drawPixel(x + i % imgW, y + i / imgW, cellColor);
    }
    else if (curPix != 'Z' && scale > 1)
    {
      M5m.Lcd.fillRect(x + scale * (i % imgW), y + scale * (i / imgW), scale, scale, cellColor);
    }
  }
}
//==================================================================
void SpaceShooterClass::drawalien_shooter_score(bool win)
{
  M5m.Lcd.setTextColor(TFT_WHITE);
  M5m.Lcd.setTextSize(2);
  if (win)
  {
    M5m.Lcd.drawString("LEVEL UP!", 53, 40);
  }
  else
  {
    M5m.Lcd.drawString("GAME OVER", 53, 40);
  }
  for (; millis() - offsetM <= 1000;)
    M5m.Lcd.drawString("Score: " + String(alien_shooter_score), 80, 89);
  offsetM = millis();
  for (; millis() - offsetM <= 1000;)
  {
  }
  M5m.Lcd.drawString("Level: " + String(level), 80, 128);
}
// functions =======================================================
void SpaceShooterClass::gameOver()
{
  play = false;
  if (doSplode)
  {
    drawBitmap(splodedImg, splodedImgW, splodedImgH, shipX, shipY, 2);
  }
  M5m.Lcd.fillScreen(TFT_BLACK);
  drawalien_shooter_score(false);
  delay(1000);
  M5m.Lcd.drawString("Start", M5m.Lcd.width()*33/80, 221);
  M5m.Lcd.drawString("Exit", M5m.Lcd.width()*56/80, 221);
  while (1)
  {
    // wait for push button
    if (M5m.BtnB.wasPressed())
    {
      break;
    }
    if (M5m.BtnC.wasPressed())
    {
      return;
    }
    M5m.update();
  }
}
//==================================================================
void SpaceShooterClass::levelUp()
{
  play = false;
  memset(alienLive, true, numAliens);
  memset(fFireX, 0, 5*sizeof(int));
  memset(fFireY, 0, 5*sizeof(int));
  memset(fFireAge, 0, 5*sizeof(int));
  alienX = 7;
  alienY = 25;
  oldAlienX = 7;
  oldAlienY = 25;
  alienSpeed = oldAlienSpeed;
  if (alienSpeed > 100)
  {
    alienSpeed -= 10;
    chanceOfFire -= 10;
  }
  else if (alienSpeed > 50)
  {
    alienSpeed -= 10;
    chanceOfFire -= 5;
  }
  else if (alienSpeed > 25)
  {
    alienSpeed -= 5;
    chanceOfFire -= 1;
  }
  alien_shooter_score += 50;
  alien_shooter_scoreInc += 5;
  changeShipX = 0;
  changeShipY = 0;
  for (unsigned long i = millis(); millis() - i <= 1600;)
  {
    if (millis() - offsetM >= 20)
    {
      M5m.Lcd.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
      drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
      drawBitmap(flamesImg, flamesImgW, flamesImgH, shipX + 1,
                 shipY + 32, 2);
      oldShipX = shipX;
      oldShipY = shipY;
      shipY -= 6;
      offsetM = millis();
    }
  }
  M5m.Lcd.fillRect(oldShipX, 0, 28, 44, TFT_BLACK);
  drawalien_shooter_score(true);
  level += 1;
  shipX = 147;
  shipY = 190;
  for (; millis() - offsetM <= 4000;)
  {
  }
  M5m.Lcd.fillScreen(TFT_BLACK);
  offsetM = millis();
  play = true;
}
//==================================================================
int SpaceShooterClass::findAlienX(int num) { return alienX + 42 * (num % numAliensRow); }
//==================================================================
int SpaceShooterClass::findAlienY(int num) { return alienY + 33 * (num / numAliensRow); }
//==================================================================
int SpaceShooterClass::findOldAlienX(int num) { return oldAlienX + 42 * (num % numAliensRow); }
//==================================================================
int SpaceShooterClass::findOldAlienY(int num) { return oldAlienY + 33 * (num / numAliensRow); }
//==================================================================
bool SpaceShooterClass::alienShot(int num)
{
  for (int i = 0; i < 5; i++)
  {
    if (fFireAge[i] < 20 && fFireAge[i] > 0)
    {
      if (fFireX[i] > findAlienX(num) - 4 && fFireX[i] < findAlienX(num) + 28 && fFireY[i] < findAlienY(num) + 22 && fFireY[i] > findAlienY(num) + 4)
      {
        fFireAge[i] = 20;
        return true;
      }
    }
  }
  return false;
}
//==================================================================
bool SpaceShooterClass::onPlayer(int num)
{
  if (findAlienX(num) - shipX < 24 && findAlienX(num) - shipX > -28 && findAlienY(num) - shipY < 32 &&
      findAlienY(num) - shipY > -22)
  {
    doSplode = true;
    return true;
  }
  else
  {
    return false;
  }
}
//==================================================================
bool SpaceShooterClass::exceedBoundary(int num)
{
  if (findAlienY(num) > 218)
  {
    return true;
  }
  else
  {
    return false;
  }
}
//==================================================================
void SpaceShooterClass::moveAliens()
{
  for (int i = 0; i < numAliens; i++)
  {
    if (alienLive[i])
    {
      M5m.Lcd.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, TFT_BLACK);
      drawBitmap(alienImg, alienImgW, alienImgH, findAlienX(i),
                 findAlienY(i), 2);
    }
  }
  oldAlienX = alienX;
  oldAlienY = alienY;
  alienX += changeAlienX;
  alienY += changeAlienY;
  if (changeAlienY != 0)
  {
    changeAlienY = 0;
  }
}
//---------------------------Player---------------------------------
void SpaceShooterClass::fireDaLazer()
{
  int bulletNo = -1;
  for (int i = 0; i < 4; i++)
  {
    if (fFireAge[i] == 0)
    {
      bulletNo = i;
    }
  }
  if (bulletNo != -1)
  {
    fFireAge[bulletNo] = 1;
    fFireX[bulletNo] = shipX + 13;
    fFireY[bulletNo] = shipY - 4;
    M5m.Lcd.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 3, TFT_MAGENTA);
  }
  fire = false;
}
//==================================================================
void SpaceShooterClass::keepFirinDaLazer(int bulletNo)
{
  M5m.Lcd.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_BLACK);
  fFireY[bulletNo] -= 8;
  M5m.Lcd.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_MAGENTA);
  fFireAge[bulletNo] += 1;
}
//==================================================================
void SpaceShooterClass::stopFirinDaLazer(int bulletNo)
{
  M5m.Lcd.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, TFT_BLACK);
  fFireAge[bulletNo] = 0;
}
//==================================================================
void SpaceShooterClass::moveShip()
{
  if (shipX + changeShipX < 288 && shipX + changeShipX > 6 && changeShipX != 0)
  {
    shipX += changeShipX;
  }
  if (shipY + changeShipY > 24 && shipY + changeShipY < 192 && changeShipY != 0)
  {
    shipY += changeShipY;
  }
  if (oldShipX != shipX || oldShipY != shipY)
  {
    M5m.Lcd.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
    oldShipX = shipX;
    oldShipY = shipY;
    drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
  }
}
//=========================== button functions =====================
void SpaceShooterClass::up()
{
  if (millis() - offsetT >= 50 && play)
  {
    changeShipX = 0;
    changeShipY = -6;
    offsetT = millis();
  }
}
//==================================================================
void SpaceShooterClass::down()
{
  if (millis() - offsetT >= 50 && play)
  {
    changeShipX = 0;
    changeShipY = 6;
    offsetT = millis();
  }
}
//==================================================================
void SpaceShooterClass::left()
{
  if (millis() - offsetT >= 50 && play)
  {
    changeShipX = -6;
    changeShipY = 0;
    offsetT = millis();
  }
}
//==================================================================
void SpaceShooterClass::right()
{
  if (millis() - offsetT >= 50 && play)
  {
    changeShipX = 6;
    changeShipY = 0;
    offsetT = millis();
  }
}
//==================================================================
void SpaceShooterClass::select()
{
  if (millis() - offsetF >= 500 && play)
  {
    fire = true;
    offsetF = millis();
  }
  if (!beginGame)
  {
    beginGame = true;
  }
}

//==================================================================
void SpaceShooterClass::spaceShoot_run()
{
  initVars();
  memset(alienLive, true, numAliens);
  memset(fFireX, 0, 5*sizeof(int));
  memset(fFireY, 0, 5*sizeof(int));
  memset(fFireAge, 0, 5*sizeof(int));
  M5m.Lcd.fillScreen(TFT_BLACK);
  M5m.Lcd.setTextColor(0x5E85);
  M5m.Lcd.setTextSize(3);
  // randomSeed(analogRead(2)); daar zit ws2812 op bij fri3d badge
  while (1)
  {
    if (M5m.BtnA.isPressed())
    {
      left();
    }
    if (M5m.BtnC.isPressed())
    {
      right();
    }
    if (M5m.BtnB.isPressed())
    {
      this->select();
    }
    //-------------Start Screen--------------
    if (millis() - offsetS >= 900 && !beginGame)
    {
      if (!startPrinted)
      {
        M5m.Lcd.drawString(">START<", 77, 105);
        startPrinted = true;
        offsetS = millis();
      }
      else
      {
        M5m.Lcd.fillRect(77, 105, 244, 32, TFT_BLACK);
        startPrinted = false;
        offsetS = millis();
      }
    }
    if (beginGame && beginGame2)
    {
      M5m.Lcd.fillRect(77, 105, 244, 32, TFT_BLACK);
      beginGame2 = false;
      play = true;
    }
    //-------------Player-----------------------------------------------
    if (millis() - offsetM >= shipSpeed && play)
    {
      moveShip();
      offsetM = millis();
    }
    if (oldShipX != shipX || oldShipY != shipY)
    {
      M5m.Lcd.fillRect(oldShipX, oldShipY, 28, 44, TFT_BLACK);
      oldShipX = shipX;
      oldShipY = shipY;
      drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
    }
    if (fire && play)
    {
      fireDaLazer();
    }
    if (millis() - offsetB >= 50)
    {
      for (int i = 0; i < 5; i++)
      {
        if (fFireAge[i] < 20 && fFireAge[i] > 0)
        {
          keepFirinDaLazer(i);
        }
        if (fFireAge[i] == 20)
        {
          stopFirinDaLazer(i);
        }
      }
      offsetB = millis();
    }
    if (millis() - offsetT > 50)
    {
      changeShipX = 0;
      changeShipY = 0;
    }
    //---------------Aliens--------------------------------------------
    if (millis() - offsetA >= alienSpeed && play)
    {
      moveAliens();
      offsetA = millis();
    }
    if (findAlienX(numAliensRow-1) >= M5m.Lcd.width()-26)
    {
      changeAlienX = -3;
      changeAlienY = 7;
    }
    if (alienX <= 6)
    {
      changeAlienX = 3;
      changeAlienY = 7;
    }
    alienLiveCount = 0;
    for (int i = 0; i < numAliens; i++)
    {
      if (alienLive[i])
      {
        alienLiveCount += 1;
        if (alienShot(i))
        {
          M5m.Lcd.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, TFT_BLACK);
          alienLiveCount -= 1;
          alienLive[i] = false;
          alien_shooter_score += alien_shooter_scoreInc;
        }
        if (onPlayer(i) || exceedBoundary(i))
        {
          return;
        }
      }
    }
    if (alienLiveCount == 1)
    {
      oldAlienSpeed = alienSpeed;
      if (alienSpeed > 50)
      {
        alienSpeed -= 10;
      }
      else
      {
        alienSpeed = 20;
      }
    }
    if (alienLiveCount == 0)
    {
      levelUp();
    }
    M5m.update();
  }
}

void SpaceShooterClass::Run()
{
  spaceShoot_run();
  gameOver();
}

SpaceShooterClass::SpaceShooterClass()
{
}

SpaceShooterClass::~SpaceShooterClass()
{
  M5m.Lcd.fillScreen(0);
  M5m.Lcd.setTextSize(1);
  M5m.Lcd.setTextFont(1);
  M5m.drawAppMenu(F("GAMES"), F("ESC"), F("SELECT"), F("LIST"));
  M5m.showList();
}