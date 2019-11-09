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
