#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedClass
{
  public:

    static const uint16_t Pixels = 78;
    static const float fadeRate;

    LedClass(Adafruit_NeoPixel* strip,uint32_t jasnosc)
    {
      _strip = strip;
      _jasnosc = jasnosc;
    }

    void initColorWipe(uint32_t c, uint8_t wait);
    bool updateColorWipe();

    void initTheaterChase(uint32_t c, uint8_t wait);
    bool updateTheaterChase();

    void initRainbow(uint8_t wait);
    bool updateRainbow();
    
    void initRainbowCycle(uint8_t wait);
    bool updateRainbowCycle();
    
    void initTheaterChaseRainbow(uint8_t wait);
    bool updateTheaterChaseRainbow();

    void initLinijka(uint8_t wait);
    bool updateLinijka();

    void initSnieg(uint8_t wait);
    bool updateSnieg();

    void initColorTwinkle(uint8_t wait);
    bool updateColorTwinkle();

  private:
    uint32_t tick;
    
    uint32_t colorWipe;
    uint32_t delays;

    uint32_t licznik;
    uint32_t licznik2;

    Adafruit_NeoPixel* _strip;

    uint32_t Wheel(byte WheelPos);
    
    void printAdr(int8_t nrDiody,int8_t nrPierscienia,uint32_t color);

    uint32_t _jasnosc;

    uint8_t cloR;
    uint8_t cloG;
    uint8_t cloB;

    uint8_t colNegR;
    uint8_t colNegG;
    uint8_t colNegB;

    int8_t tabSnieg[12];

    float redStates[Pixels];
    float blueStates[Pixels];
    float greenStates[Pixels];
    
};
