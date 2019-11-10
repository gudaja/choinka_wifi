#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <Regexp.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "LedClass.h"

Ticker ticker;

Ticker tickerLed;  

Ticker tickerWs;

#define JASNOSC 16
#define Pixels 78


const char* ssid = "lukasz"; //nazwa ssid sieci
const char* password = "tajne123"; //haslo

#define PIN 14 //pin do podlaczenia diod adresowanych

const int PIN_LED = BUILTIN_LED;
const int TRIGGER_PIN = 4;

IPAddress IP_br(255, 255, 255,255);
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "esp8266 led";  // a reply string to send back
bool initialConfig = false;


Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, PIN, NEO_GRB + NEO_KHZ800);

LedClass ledDriver = LedClass(&strip,JASNOSC);

void tick()
{
  //toggle state
    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(IP_br, localUdpPort);
    Udp.write(replyPacket);
    Udp.endPacket();
}

void tickLed()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void initWipe()
{
        uint8_t cloR = random(0, JASNOSC);
        uint8_t cloG = random(0, JASNOSC);
        uint8_t cloB = random(0, JASNOSC);

        ledDriver.initColorWipe(strip.Color(cloR, cloG, cloB),10);
}

void initTheater()
{
        uint8_t cloR = random(0, JASNOSC);
        uint8_t cloG = random(0, JASNOSC);
        uint8_t cloB = random(0, JASNOSC);

        ledDriver.initTheaterChase(strip.Color(cloR, cloG, cloB),5);
}

void tickWs()
{
  if(ledDriver.updateTheaterChaseRainbow())
  {
    ledDriver.initTheaterChaseRainbow(10);
  }
}

WiFiServer server(80);

long aniMillis=0;
byte aniFlag=0;


void setup() {

  pinMode(PIN_LED, OUTPUT);
  tickerLed.attach(0.1, tickLed);
  Serial.begin(115200);
  delay(10);
  Serial.println("\n Starting");

  //Zerowanie diod adresowanych
  strip.begin();
  for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 20,0,0);
        strip.show();
      }

  if (WiFi.SSID()==""){
    Serial.println("We haven't got any access point credentials, so get them now");   
    initialConfig = true;
  }
  else{
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
    unsigned long startedAt = millis();
    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
  }
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    tickerLed.detach();
    digitalWrite(PIN_LED, LOW);
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
    Udp.begin(localUdpPort);
    ticker.attach(5, tick);
    server.begin();
  }

//    initTheater();
ledDriver.initTheaterChaseRainbow(10);
tickerWs.attach(0.01, tickWs);
 
//  //Laczenie z siecia wifi
//  Serial.println();
//  Serial.println();
//  Serial.print("Laczenie z: ");
//  Serial.println(ssid);

 
//  WiFi.begin(ssid, password);
// 
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.println("WiFi polaczone");
//  digitalWrite(diodaWifi, HIGH);
// 
//  // Start serwera
//  server.begin();
//  Serial.println("Serwer uruchomiony");
// 
//  // Wyswietlanie adresu IP
//  Serial.print("Uzyj tego adresu URL by polaczyc sie z lampka: ");
//  Serial.print("http://");
//  Serial.print(WiFi.localIP());
//  Serial.println("/");
//
//   Udp.begin(localUdpPort);
//   ticker.attach(5, tick);
}
 
void loop() {

    if ( !(digitalRead(TRIGGER_PIN) == LOW)  || (initialConfig)) {
      server.stop();
     Serial.println("Configuration portal requested.");
//     digitalWrite(PIN_LED, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.
    tickerLed.attach(0.6, tickLed);
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    
    if (!wifiManager.startConfigPortal()) {
      Serial.println("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }

    Serial.println("#################### wyjscie ###############################################");
    
    digitalWrite(PIN_LED, HIGH); // Turn led off as we are not in configuration mode.
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }

 while (WiFi.status() == WL_CONNECTED){
   
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Poczekaj na odbior danych
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  //Dopasuj zadanie

  //kolor z zapytania
   MatchState ms;
   char charBuf[50];
   request.toCharArray(charBuf, 50);
   ms.Target(charBuf);

   if(ms.Match ("(/color)", 0))
   {
      ms.Match ("r=(%d+)&g=(%d+)&b=(%d+)&a=(%d+)", 0);
      char buf [100];
      int r = 0;
      int g = 0;
      int b = 0;
      int a = 0;
      ms.GetCapture (buf, 0);
      r = atoi(buf);
      ms.GetCapture (buf, 1);
      g = atoi(buf);
      ms.GetCapture (buf, 2);
      b = atoi(buf);
      ms.GetCapture (buf, 3);
      a = atoi(buf);

      int rw =  map(r, 0, 255, 0, a);
      int gw =  map(g, 0, 255, 0, a);
      int bw =  map(b, 0, 255, 0, a);


      Serial.println(String("wynik: r ") + String(rw) + String("g ") + String(gw) + String("b ") + String(bw));

      for(uint16_t i=0; i<Pixels; i++) 
      {
        strip.setPixelColor(i, rw,gw,bw);
      }
      strip.show();
   }
   

  //wlaczenie ulubionego koloru lampki
  if (request.indexOf("/turnOn") > 0)  
  {
    for(uint16_t i=0; i<2; i++) 
      {
        strip.setPixelColor(i, 255,120,0);
        strip.show();
      }
   
  }

  //wylaczanie lampki
  if (request.indexOf("/turnOff") >0)  
  {
      for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 0);
        strip.show();
      }
  }

  //wlaczenie animacji
  if (request.indexOf("/animationOn") > 0)  
  {
    aniMillis=millis();
    rainbowCycle(20);
    aniFlag=1;
  }
  
  if (request.indexOf("/animationOff") >0) 
  {
    aniFlag=0;
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
        strip.setPixelColor(i, 0);
        strip.show();
    }
  }
  
  //wyswietlanie koloru bialego
  if (request.indexOf("/white") > 0)  
  {   
      for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 255,255,255);
        strip.show();
      }
  }

  //wyswietlanie koloru czerwonego
  if (request.indexOf("/red") > 0)  
  {   
    for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 255,0,0);
        strip.show();
      }
  }

  //wyswietlanie koloru zielonego
  if (request.indexOf("/green") > 0)  
  { 
   for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 0,255,0);
        strip.show();
      }
  }

  //wyswietlanie koloru niebieskiego
  if (request.indexOf("/blue") > 0)  
  { 
   for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 0,0,255);
        strip.show();
      }
  }


  //wylaczanie animacji po okreslonym czasie
  if (aniFlag==1)
  {
    if (millis()-aniMillis>3000)
    {
      for(uint16_t i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, 0);
        strip.show();
        delay(10);
      }
      Serial.println("Wylaczam");
      aniFlag=0;
    }
  }
 
  //Zwroc odpowiedz
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
  client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
  client.println("</head>");
  client.println("<body bgcolor = \"#dddddd\">"); 
  client.println("<hr/><hr>");
  client.println("<h4><center> Panel kontrolny lampki LED </center></h4>");
  client.println("<hr/><hr>");
  client.println("<br><br>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("Zaswiec");
  client.println("<a href=\"/turnOn\"\"><button>Wlacz </button></a>");
  client.println("<a href=\"/turnOff\"\"><button>Wylacz </button></a><br />");  
  client.println("</center>");   
  client.println("<br><br>");
  client.println("<center>");
  client.println("Animacja");
  client.println("<a href=\"/animationOn\"\"><button>Wlacz</button></a>");
  client.println("<a href=\"/animationOff\"\"><button>Wylacz</button></a><br />");  

  client.println("</center>"); 
  client.println("<br><br>");
  client.println("<center>");
  client.println("Kolor");
  client.println("<a href=\"/red\"\"><button>Czerwony</button></a>");
  client.println("<a href=\"/green\"\"><button>Zielony</button></a>");  
  client.println("<a href=\"/blue\"\"><button>Niebieski</button></a>"); 
  client.println("<a href=\"/white\"\"><button>Bialy</button></a><br />"); 

  client.println("</center>"); 
  client.println("<center>");
   
  client.println("<br><br>");
  client.println("<center>");
  
  client.println("</html>"); 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 }

setup(); //ponowne laczenie z siecia wifi w przypadku rozlaczenia

}


 // Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Przyklad z biblioteki Adafruit neopixel dla animacji teczy
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
