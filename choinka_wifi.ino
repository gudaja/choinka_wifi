#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <Regexp.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "LedClass.h"
#include "WS2812FX.h"

// include the custom effects
#include "custom/BlockDissolve.h"
#include "custom/DualLarson.h"
#include "custom/Fillerup.h"
#include "custom/MultiComet.h"
#include "custom/Oscillate.h"
#include "custom/Popcorn.h"
#include "custom/Rain.h"
#include "custom/RainbowFireworks.h"
#include "custom/RainbowLarson.h"
#include "custom/RandomChase.h"
#include "custom/TriFade.h"
#include "custom/VUMeter.h"

Ticker ticker;
Ticker tickerLed;  

#define JASNOSC 16
#define Pixels 78

#define DEFAULT_COLOR GREEN
#define DEFAULT_BRIGHTNESS 15
#define DEFAULT_SPEED 1000
#define DEFAULT_MODE FX_MODE_STATIC

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

boolean auto_cycle = true;
unsigned long auto_last_change = 0;

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

WS2812FX ws2812fx = WS2812FX(Pixels, PIN, NEO_GRB + NEO_KHZ800);

extern const char index_html[];
extern const char main_js[];

String modes = "";

void tick()
{
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

ESP8266WebServer  server(80);

long aniMillis=0;
byte aniFlag=0;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();  
  Serial.println("\n Starting");

  tickerLed.attach(0.1, tickLed);

  //doida + przycisk
  pinMode(PIN_LED, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.println("WS2812FX setup");
  ws2812fx.init();
  ws2812fx.setMode(DEFAULT_MODE);
  ws2812fx.setSpeed(DEFAULT_SPEED);
  ws2812fx.setBrightness(DEFAULT_BRIGHTNESS);

  // setup the custom effects

  uint8_t modedissolve  =  ws2812fx.setCustomMode(F("blockDissolve"), blockDissolve);
  uint8_t modedualLarsonMode  = ws2812fx.setCustomMode(F("Dual Larson"), dualLarson);
  uint8_t modefillerup  = ws2812fx.setCustomMode(F("fillerup"), fillerup);
  uint8_t modemultiComet  = ws2812fx.setCustomMode(F("multiComet"), multiComet);
  uint8_t modeoscillate  = ws2812fx.setCustomMode(F("oscillate"), oscillate);
  uint8_t modepopcorn  = ws2812fx.setCustomMode(F("popcorn"), popcorn);
  uint8_t moderain  = ws2812fx.setCustomMode(F("rain"), rain);
  uint8_t moderainbowFireworks  = ws2812fx.setCustomMode(F("rainbowFireworks"), rainbowFireworks);
  uint8_t moderandomChase  = ws2812fx.setCustomMode(F("randomChase"), randomChase);
  uint8_t modetriFade  = ws2812fx.setCustomMode(F("triFade"), triFade);

      

  ws2812fx.setColor(DEFAULT_COLOR);
  ws2812fx.start();

  uint8_t num_modes = ws2812fx.getModeCount();
  Serial.print(" ModeCount: ");
  Serial.println(num_modes);

  modes.reserve(6000);
  modes_setup();  


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


  if (WiFi.status()!=WL_CONNECTED){
    Serial.println("failed to connect, finishing setup anyway");
  } else{
    tickerLed.detach();
    digitalWrite(PIN_LED, LOW);
    Serial.print("local ip: ");
    Serial.println(WiFi.localIP());
    Udp.begin(localUdpPort);
    ticker.attach(5, tick);

    server.on("/", srv_handle_index_html);
    server.on("/main.js", srv_handle_main_js);
    server.on("/modes", srv_handle_modes);
    server.on("/set", srv_handle_set);
    server.onNotFound(srv_handle_not_found);
    server.begin();
  }
}
 
void loop() {
  
  unsigned long now = millis();

  server.handleClient();
  ws2812fx.service();

  if(auto_cycle && (now - auto_last_change > 15000)) { // cycle effect mode every 15 seconds
    uint8_t cloR = random(0, 255);
    uint8_t cloG = random(0, 255);
    uint8_t cloB = random(0, 255);
    ws2812fx.setColor(ws2812fx.Color(cloR, cloG, cloB));
    uint8_t next_mode = (ws2812fx.getMode() + 1) % ws2812fx.getModeCount();
    ws2812fx.setMode(next_mode);
    Serial.print("mode is "); Serial.print(next_mode);Serial.print(" ");Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    auto_last_change = now;
  }


  if ( !(digitalRead(TRIGGER_PIN) == LOW)  || (initialConfig)) {
      server.stop();
      Serial.println("Configuration portal requested.");
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

// while (WiFi.status() == WL_CONNECTED){
//   
//  WiFiClient client = server.available();
//  if (!client) {
//    return;
//  }
// 
//  // Poczekaj na odbior danych
//  Serial.println("new client");
//  while(!client.available()){
//    delay(1);
//  }
// 
//  String request = client.readStringUntil('\r');
//  Serial.println(request);
//  client.flush();
// 
//  //Dopasuj zadanie
//
//  //kolor z zapytania
//   MatchState ms;
//   char charBuf[50];
//   request.toCharArray(charBuf, 50);
//   ms.Target(charBuf);
//
//   if(ms.Match ("(/color)", 0))
//   {
//      ms.Match ("r=(%d+)&g=(%d+)&b=(%d+)&a=(%d+)", 0);
//      char buf [100];
//      int r = 0;
//      int g = 0;
//      int b = 0;
//      int a = 0;
//      ms.GetCapture (buf, 0);
//      r = atoi(buf);
//      ms.GetCapture (buf, 1);
//      g = atoi(buf);
//      ms.GetCapture (buf, 2);
//      b = atoi(buf);
//      ms.GetCapture (buf, 3);
//      a = atoi(buf);
//
//      int rw =  map(r, 0, 255, 0, a);
//      int gw =  map(g, 0, 255, 0, a);
//      int bw =  map(b, 0, 255, 0, a);
//
//
//      Serial.println(String("wynik: r ") + String(rw) + String("g ") + String(gw) + String("b ") + String(bw));
//
////      for(uint16_t i=0; i<Pixels; i++) 
////      {
////        strip.setPixelColor(i, rw,gw,bw);
////      }
////      strip.show();
//   }
//   
//
//  //wlaczenie ulubionego koloru lampki
//  if (request.indexOf("/turnOn") > 0)  
//  {
////    for(uint16_t i=0; i<2; i++) 
////      {
////        strip.setPixelColor(i, 255,120,0);
////        strip.show();
////      }
//   
//  }
//
//  //wylaczanie lampki
//  if (request.indexOf("/turnOff") >0)  
//  {
////      for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 0);
////        strip.show();
////      }
//  }
//
//  //wlaczenie animacji
//  if (request.indexOf("/animationOn") > 0)  
//  {
//    aniMillis=millis();
//    rainbowCycle(20);
//    aniFlag=1;
//  }
//  
//  if (request.indexOf("/animationOff") >0) 
//  {
////    aniFlag=0;
////    for(uint16_t i=0; i<strip.numPixels(); i++) 
////    {
////        strip.setPixelColor(i, 0);
////        strip.show();
////    }
//  }
//  
//  //wyswietlanie koloru bialego
//  if (request.indexOf("/white") > 0)  
//  {   
////      for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 255,255,255);
////        strip.show();
////      }
//  }
//
//  //wyswietlanie koloru czerwonego
//  if (request.indexOf("/red") > 0)  
//  {   
////    for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 255,0,0);
////        strip.show();
////      }
//  }
//
//  //wyswietlanie koloru zielonego
//  if (request.indexOf("/green") > 0)  
//  { 
////   for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 0,255,0);
////        strip.show();
////      }
//  }
//
//  //wyswietlanie koloru niebieskiego
//  if (request.indexOf("/blue") > 0)  
//  { 
////   for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 0,0,255);
////        strip.show();
////      }
//  }
//
//
//  //wylaczanie animacji po okreslonym czasie
//  if (aniFlag==1)
//  {
////    if (millis()-aniMillis>3000)
////    {
////      for(uint16_t i=0; i<strip.numPixels(); i++) 
////      {
////        strip.setPixelColor(i, 0);
////        strip.show();
////        delay(10);
////      }
////      Serial.println("Wylaczam");
////      aniFlag=0;
////    }
//  }
// 
//  //Zwroc odpowiedz
//  client.println("HTTP/1.1 200 OK");
//  client.println("Content-Type: text/html");
//  client.println(""); //  do not forget this one
//  client.println("<!DOCTYPE HTML>");
//  client.println("<html>");
//  client.println("<head>");
//  client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
//  client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
//  client.println("</head>");
//  client.println("<body bgcolor = \"#dddddd\">"); 
//  client.println("<hr/><hr>");
//  client.println("<h4><center> Panel kontrolny lampki LED </center></h4>");
//  client.println("<hr/><hr>");
//  client.println("<br><br>");
//  client.println("<br><br>");
//  client.println("<center>");
//  client.println("Zaswiec");
//  client.println("<a href=\"/turnOn\"\"><button>Wlacz </button></a>");
//  client.println("<a href=\"/turnOff\"\"><button>Wylacz </button></a><br />");  
//  client.println("</center>");   
//  client.println("<br><br>");
//  client.println("<center>");
//  client.println("Animacja");
//  client.println("<a href=\"/animationOn\"\"><button>Wlacz</button></a>");
//  client.println("<a href=\"/animationOff\"\"><button>Wylacz</button></a><br />");  
//
//  client.println("</center>"); 
//  client.println("<br><br>");
//  client.println("<center>");
//  client.println("Kolor");
//  client.println("<a href=\"/red\"\"><button>Czerwony</button></a>");
//  client.println("<a href=\"/green\"\"><button>Zielony</button></a>");  
//  client.println("<a href=\"/blue\"\"><button>Niebieski</button></a>"); 
//  client.println("<a href=\"/white\"\"><button>Bialy</button></a><br />"); 
//
//  client.println("</center>"); 
//  client.println("<center>");
//   
//  client.println("<br><br>");
//  client.println("<center>");
//  
//  client.println("</html>"); 
//  delay(1);
//  Serial.println("Client disonnected");
//  Serial.println("");
// }
//
//setup(); //ponowne laczenie z siecia wifi w przypadku rozlaczenia

}

/*
 * Build <li> string for all modes.
 */
void modes_setup() {
  modes = "";
  uint8_t num_modes = ws2812fx.getModeCount();
  for(uint8_t i=0; i < num_modes; i++) {
    uint8_t m = i;
    modes += "<li><a href='#' class='m' id='";
    modes += m;
    modes += "'>";
    modes += ws2812fx.getModeName(m);
    modes += "</a></li>";
  }
}

/* #####################################################
#  Webserver Functions
##################################################### */

void srv_handle_not_found() {
  server.send(404, "text/plain", "File Not Found");
}

void srv_handle_index_html() {
  server.send_P(200,"text/html", index_html);
}

void srv_handle_main_js() {
  server.send_P(200,"application/javascript", main_js);
}

void srv_handle_modes() {
  server.send(200,"text/plain", modes);
}

void srv_handle_set() {
  for (uint8_t i=0; i < server.args(); i++){
    if(server.argName(i) == "c") {
      uint32_t tmp = (uint32_t) strtol(server.arg(i).c_str(), NULL, 16);
      if(tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        ws2812fx.setColor(tmp);
      }
    }

    if(server.argName(i) == "m") {
      uint8_t cloR = random(0, 255);
      uint8_t cloG = random(0, 255);
      uint8_t cloB = random(0, 255);
      ws2812fx.setColor(ws2812fx.Color(cloR, cloG, cloB));
      uint8_t tmp = (uint8_t) strtol(server.arg(i).c_str(), NULL, 10);
      ws2812fx.setMode(tmp % ws2812fx.getModeCount());
      Serial.print("mode is "); Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    }

    if(server.argName(i) == "b") {
      if(server.arg(i)[0] == '-') {
        ws2812fx.setBrightness(ws2812fx.getBrightness() * 0.8);
      } else if(server.arg(i)[0] == ' ') {
        ws2812fx.setBrightness(min(max(ws2812fx.getBrightness(), 5) * 1.2, 255));
      } else { // set brightness directly
        uint8_t tmp = (uint8_t) strtol(server.arg(i).c_str(), NULL, 10);
        ws2812fx.setBrightness(tmp);
      }
      Serial.print("brightness is "); Serial.println(ws2812fx.getBrightness());
    }

    if(server.argName(i) == "s") {
      if(server.arg(i)[0] == '-') {
        ws2812fx.setSpeed(max(ws2812fx.getSpeed(), 5) * 1.2);
      } else if(server.arg(i)[0] == ' ') {
        ws2812fx.setSpeed(ws2812fx.getSpeed() * 0.8);
      } else {
        uint16_t tmp = (uint16_t) strtol(server.arg(i).c_str(), NULL, 10);
        ws2812fx.setSpeed(tmp);
      }
      Serial.print("speed is "); Serial.println(ws2812fx.getSpeed());
    }

    if(server.argName(i) == "a") {
      if(server.arg(i)[0] == '-') {
        auto_cycle = false;
      } else {
        auto_cycle = true;
        auto_last_change = 0;
      }
    }
  }
  server.send(200, "text/plain", "OK");
}
