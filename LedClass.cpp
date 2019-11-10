#include "LedClass.h"


void LedClass::initColorWipe(uint32_t c, uint8_t wait)
{
  tick = 0;
  
  colorWipe = c;
  delays = wait;

  licznik = 0;
}

bool LedClass::updateColorWipe()
{
      tick++;

      _strip->setPixelColor(licznik, colorWipe);
      _strip->show();

      licznik++;

      return tick >= _strip->numPixels();
}


void LedClass::initTheaterChase(uint32_t c, uint8_t wait)
{
  tick = 0;
  
  colorWipe = c;
  delays = wait;

  licznik = 0;
}

bool LedClass::updateTheaterChase()
{
  tick++;

  if(tick%25 == 0)
  {
      for (uint16_t i=0; i < _strip->numPixels(); i=i+3) {
         _strip->setPixelColor(i+licznik, 0);        //turn every third pixel off
      }
      
      for (uint16_t i=0; i <  _strip->numPixels(); i=i+3) {
         _strip->setPixelColor(i+licznik+1, colorWipe);        //turn every third pixel off
      }

      _strip->show();

      licznik++;

      if(licznik >= 3)
      {
        licznik = 0;
      }
  }

  return tick > delays * 100;
}

void LedClass::initRainbow(uint8_t wait)
{
  tick = 0;
  
  colorWipe = 0;
  delays = wait;

  licznik = 0;
}

bool LedClass::updateRainbow()
{
  tick++;
  
  if(tick%2 == 0)
  {
      uint16_t i;
      for(i=0; i< _strip->numPixels(); i++) {
         _strip->setPixelColor(i, Wheel((i+licznik) & 255));
      }
       _strip->show();

      licznik++;

  }
  return licznik > 256;
}

void LedClass::initRainbowCycle(uint8_t wait)
{
  tick = 0;
  
  colorWipe = 0;
  delays = wait;

  licznik = 0;
}

bool LedClass::updateRainbowCycle()
{
  tick++;
  
  if(tick%2 == 0)
  {
      uint16_t i;
      for(i=0; i< _strip->numPixels(); i++) {
         _strip->setPixelColor(i, Wheel(((i * 256 / _strip->numPixels()) + licznik) & 255));
      }
       _strip->show();

      licznik++;

  }
  return licznik > 256;
}


void LedClass::initTheaterChaseRainbow(uint8_t wait)
{
  tick = 0;
  
  colorWipe = 0;
  delays = wait;

  licznik = 0;
  licznik2 = 0;
}

bool LedClass::updateTheaterChaseRainbow()
{
  tick++;
  
  if(tick%25 == 0)
  {
      for (uint16_t i=0; i < _strip->numPixels(); i=i+3) {
         _strip->setPixelColor(i+licznik, 0);        //turn every third pixel off
      }
      
      for (uint16_t i=0; i <  _strip->numPixels(); i=i+3) {
         _strip->setPixelColor(i+licznik+1,  Wheel( (i+licznik2) % 255));
      }
       _strip->show();

      licznik++;
      licznik2+=10;

      if(licznik >= 3)
      {
        licznik = 0;
      }
  }
  return licznik2 > 256;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t LedClass::Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    map(4,0,255,0,_jasnosc);
    return _strip->Color(  map(255 - WheelPos * 3,0,255,0,_jasnosc), 0, map(WheelPos * 3,0,255,0,_jasnosc));
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return _strip->Color(0, map(WheelPos * 3,0,255,0,_jasnosc),map(255 - WheelPos * 3,0,255,0,_jasnosc));
  }
  WheelPos -= 170;
  return _strip->Color(  map(WheelPos * 3,0,255,0,_jasnosc),   map(255 - WheelPos * 3,0,255,0,_jasnosc), 0);
}
