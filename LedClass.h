#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedClass
{
  public:

    LedClass(Adafruit_NeoPixel* strip)
    {
      _strip = strip;
    }

    void initColorWipe(uint32_t c, uint8_t wait);
    bool updateColorWipe();


    void initTheaterChase(uint32_t c, uint8_t wait);
    bool updateTheaterChase();


  private:
    uint32_t tick;
    
    uint32_t colorWipe;
    uint32_t delayMs;

    uint32_t licznik;

    Adafruit_NeoPixel* _strip;
};
