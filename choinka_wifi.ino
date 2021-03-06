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
  ws2812fx.setMode(16);
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

void nextAnim()
{
    uint8_t cloR = random(255);
    uint8_t cloG = random(255);
    uint8_t cloB = random(255);

    Serial.print("color "); Serial.println(ws2812fx.Color(cloR, cloG, cloB));
    uint8_t next_mode = (ws2812fx.getMode() + 1) % ws2812fx.getModeCount();
    ws2812fx.setMode(next_mode);
    ws2812fx.setColor(ws2812fx.Color(cloR, cloG, cloB));
    Serial.print("mode is "); Serial.print(next_mode);Serial.print(" ");Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    auto_last_change = millis();
}

void prevAnim()
{
    uint8_t cloR = random(255);
    uint8_t cloG = random(255);
    uint8_t cloB = random(255);

    Serial.print("color "); Serial.println(ws2812fx.Color(cloR, cloG, cloB));
    uint8_t next_mode = (ws2812fx.getMode() - 1) % ws2812fx.getModeCount();
    ws2812fx.setMode(next_mode);
    ws2812fx.setColor(ws2812fx.Color(cloR, cloG, cloB));
    Serial.print("mode is "); Serial.print(next_mode);Serial.print(" ");Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    auto_last_change = millis();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return ws2812fx.Color(255 - WheelPos * 3, 0,WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return ws2812fx.Color(0, WheelPos * 3,255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return ws2812fx.Color(  WheelPos * 3,255 - WheelPos * 3, 0);
}
 
void loop() {
  
  unsigned long now = millis();

  if(auto_cycle && (now - auto_last_change > 15000)) { // cycle effect mode every 15 seconds
    uint8_t cloR = random(255);
    uint8_t cloG = random(255);
    uint8_t cloB = random(255);

    Serial.print("color "); Serial.println( Wheel(cloR));
    uint8_t next_mode =  random(0, ws2812fx.getModeCount());
//    uint8_t next_mode = (ws2812fx.getMode() + 1) % ws2812fx.getModeCount();
    ws2812fx.setMode(next_mode);
//    ws2812fx.setMode(0);
//    ws2812fx.setColor(ws2812fx.Color(cloR, cloG, cloB));
    ws2812fx.setColor(Wheel(cloR));
    Serial.print("mode is "); Serial.print(next_mode);Serial.print(" ");Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    auto_last_change = now;
  }

  server.handleClient();
  ws2812fx.service();




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

    if(server.argName(i) == "k") {
      if(server.arg(i)[0] == '-') {
        Serial.println("k prev");
        prevAnim();
      } else {
        Serial.println("k next");
        nextAnim();
      }
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
