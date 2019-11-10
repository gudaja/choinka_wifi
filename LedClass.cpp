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

void LedClass::initLinijka(uint8_t wait)
{
  tick = 0;

  cloR = random(0, _jasnosc);
  cloG = random(0, _jasnosc);
  cloB = random(0, _jasnosc);

  colNegR = _jasnosc - cloR;
  colNegG = _jasnosc - cloG;
  colNegB = _jasnosc - cloB;
  
  colorWipe = _strip->Color(cloR, cloG, cloB);
  delays = wait;

  licznik = 0;
  licznik2 = 0;

  for(uint16_t i=0; i < _strip->numPixels(); i++) {
     _strip->setPixelColor(i, colorWipe);
  }
  _strip->show();
}

bool LedClass::updateLinijka()
{
  tick++;
  
  {
    for(int16_t j=0;j<_strip->numPixels();j++) {

      if(((int16_t)licznik-(int16_t)j)<5&&((int16_t)licznik-(int16_t)j)>-5)
      {
        int16_t procent = 100 - map( abs((int16_t)tick-((int16_t)j*10)),0,50,0,100);
        _strip->setPixelColor(j, _strip->Color(map( procent,0,100,cloR,colNegR), map( procent,0,100,cloG,colNegG), map( procent,0,100,cloB,colNegB)));
      }
      else
      {
        _strip->setPixelColor(j, colorWipe);
      }
    }
     _strip->show();

    if(tick%10 == 0)
    {
      licznik++;
    }
  }
  return licznik > _strip->numPixels();
}

void LedClass::initSnieg(uint8_t wait)
{
  tick = 0;
  
  colorWipe = 0;
  delays = random(1,5);;

  licznik = 0;
  licznik2 = 0;

  for(uint16_t i=0; i < _strip->numPixels(); i++) {
     _strip->setPixelColor(i, 0);
  }
  _strip->show();

  for(uint8_t i=0;i<12;i++){
    tabSnieg[i]=-1;
  }

  uint8_t adr = random(0, 12);
  tabSnieg[adr]=0;
}

bool LedClass::updateSnieg()
{
  tick++;

  if(tick%20 == 0)
  {
     if(delays==0){
      uint8_t adr = random(0, 12);
      if(tabSnieg[adr]==-1){
        tabSnieg[adr]=0;
      }
      delays = random(1,5);
    }
    
    for(uint8_t i=0;i<12;i++){
      if(tabSnieg[i]>=0 && tabSnieg[i] < 7){
        printAdr(i,tabSnieg[i]-1,0);
        printAdr(i,tabSnieg[i],_strip->Color(0, 0, _jasnosc));
      }
      else{
        tabSnieg[i]=-1;
      }
    }

    for(uint8_t i=0;i<12;i++){
      if(tabSnieg[i]>=0)
      {
        tabSnieg[i]++;
      }
    }

    delays--;

    _strip->show();
  }

  return tick > 3000;
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

void LedClass::printAdr(int8_t nrDiody,int8_t nrPierscienia,uint32_t color)
{
  if(nrPierscienia == 0)
  {
    if((nrDiody%2)==1)
    {
      if(nrDiody==1)
      {
        _strip->setPixelColor(0,color);
      }
      else
      {
        uint8_t nr = nrDiody/2;
        _strip->setPixelColor(nr,color);
      }
    }
  }
  else if(nrPierscienia < 7)
  {
      uint8_t licznikPierscienia = 0;
      if(nrPierscienia>1)
      {
        licznikPierscienia = (nrPierscienia-1)*12;
      }
      uint8_t nr = nrDiody + licznikPierscienia + 6;
      _strip->setPixelColor(nr,color);
  }
}
