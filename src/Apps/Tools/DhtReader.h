#pragma once
#include "M5StackMod.h"

class DhtReaderClass
{
public:
  DhtReaderClass();
  ~DhtReaderClass();

  void Run();

private:
  unsigned long past = 0, sht_wait = 0, startTime = 0, start;
  float temperature = 0, humidity = 0, oldtemperature = 0, oldhumidity = 0;
  uint8_t _address;
  int pin = 17; // TODO !!!
  bool SHT = true, sw = true;
  int t_dr = 48, h_dr = 5;

  void DrawWidgets();
  void getSHTData(uint8_t _address);
  void getDHTData();
};
