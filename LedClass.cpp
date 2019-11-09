#include "LedClass.h"


void LedClass::initColorWipe(uint32_t c, uint8_t wait)
{
  tick = 0;
  
  colorWipe = c;
  delayMs = wait;

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
  delayMs = wait;

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

      licznik++;

      if(licznik >= 3)
      {
        licznik = 0;
      }
  }

  return tick > 25000;
  
//  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
//    for (int q=0; q < 3; q++) {
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, c);    //turn every third pixel on
//      }
//      strip.show();
//
//      delay(wait);
//
//      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
//        strip.setPixelColor(i+q, 0);        //turn every third pixel off
//      }
//    }
//  }
}
