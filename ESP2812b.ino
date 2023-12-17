#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>


#define PIN 12
#define CNT 250
Adafruit_NeoPixel strip(CNT, PIN, NEO_GRB + NEO_KHZ800);

// Setup connection stuff
const char* ssid = "ssid";
const char* pass = "pass";
ESP8266WebServer server(80);
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long frameTime = 65535;
const long timeoutTime = 2000;

class Endpoints {
  public:
    void index() {
      server.send(200, "text/html", "yeet");
    }
    
    void solid_color() {
      String sColor = server.arg("c");
      uint32_t color = strtol(&sColor[0], NULL, 16);
      Serial.println(color, HEX);
      for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
        strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      }
      strip.show();
      server.send(200, "text/html", sColor);
      frameTime = 65535;
    }

    void pattern() {
      String json = server.arg("json");
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, json);
      
      if(error) {
        Serial.print(F("deserialization failed: "));
        Serial.println(error.f_str());
        server.send(200, "application/json", error.c_str());
      }
      
      int cycle = 0;
      for(int i = 0; i < strip.numPixels() - 1; ++i) {
        uint32_t c = strtol(doc[cycle].as<const char*>(), NULL, 16);
        ++cycle;
        if(cycle == doc.size())
          cycle = 0;

        Serial.printf("led: %d : color: %d\n", i, c);
        strip.setPixelColor(i, c);
      }
      strip.show();
      server.send(200, "application/json", json);
      frameTime = 65535;
      return;
    }

    void jsonTestArea() {
      String json = server.arg("json");
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, json);
      
      if(error) {
        Serial.print(F("deserialization failed: "));
        Serial.println(error.f_str());
        server.send(200, "application/json", error.c_str());
      }

      Serial.println(doc.size());
      for(int i = 0; i < doc.size(); ++i) {
        int r = doc[i]["r"];
        int g = doc[i]["g"];
        int b = doc[i]["b"];
        Serial.print("(");
        Serial.print(r);
        Serial.print(",");
        Serial.print(g);
        Serial.print(",");
        Serial.print(b);
        Serial.println(")");
      }
      server.send(200, "application/json", json);
    }

    void chaser() {
      chaserBase = server.arg("json");
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, chaserBase);
      
      if(error) {
        Serial.print(F("deserialization failed: "));
        Serial.println(error.f_str());
        server.send(200, "application/json", error.c_str());
      }

      int cycle = 0;
      for(int i = 0; i < strip.numPixels() - 1; ++i) {
        int r = doc["p"][cycle]["r"];
        int g = doc["p"][cycle]["g"];
        int b = doc["p"][cycle]["b"];

        cycle = (cycle == doc["p"].size()) ? 0 : cycle + 1;
        strip.setPixelColor(i, strip.Color(r,g,b));
      }
      strip.show();
      server.send(200, "application/json", chaserBase);
      lastFrame = 1;
      totalFrames = doc["p"].size();
      frameTime = doc["time"];
      Serial.printf("Frame time: %d\n", frameTime);
      return;
    }

    void nextChaser() {
      StaticJsonDocument<2048> doc;
      DeserializationError error = deserializeJson(doc, chaserBase);
      if (error) {
        return;
      }

      int cycle = lastFrame;

      int ar = doc["p"][cycle]["r"];
      int ag = doc["p"][cycle]["g"];
      int ab = doc["p"][cycle]["b"];
      Serial.printf("Cycle: %d  |  Starter Color: %d\n", cycle, strip.Color(ar,ag,ab));

      for(int i = 0; i < strip.numPixels() - 1; ++i) {
        int r = doc["p"][cycle]["r"];
        int g = doc["p"][cycle]["g"];
        int b = doc["p"][cycle]["b"];
        
        cycle = (cycle + 1 == doc["p"].size()) ? 0 : cycle + 1;
        strip.setPixelColor(i, strip.Color(r,g,b));
      }
      strip.show();
      lastFrame = (lastFrame + 1 == totalFrames) ? 0 : lastFrame + 1;
      return;
    }

    void sparkle() {
      //not yet implemented
    }

    void nextSparkle() {
      
    }

    void nextFrame() {
      lastFrame = lastFrame > totalFrames ? 0 : lastFrame;
      String json = frames[lastFrame];
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, json);
      if(error) {
        Serial.print(F("deserialization failed: "));
        Serial.println(error.f_str());
        server.send(200, "application/json", error.c_str());
      }

      int cycle = 0;
      for(int i = 0; i < strip.numPixels() - 1; ++i) {
        int r = doc[cycle]["r"];
        int g = doc[cycle]["g"];
        int b = doc[cycle]["b"];
        ++cycle;
        if(cycle == doc.size())
          cycle = 0;

        Serial.printf("led: %d : color: %d\n", i, strip.Color(r,g,b));
        strip.setPixelColor(i, strip.Color(r,g,b));
      }
      strip.show();
      server.send(200, "application/json", json);
      return;
    }

    void setupEndpoints() {
      server.on(index_uri, std::bind(&Endpoints::index, this));
      server.on(fill_uri, std::bind(&Endpoints::solid_color, this));
      server.on(pattern_uri, std::bind(&Endpoints::pattern, this));
      server.on(chaser_uri, std::bind(&Endpoints::chaser, this));
      server.on(json_uri, std::bind(&Endpoints::jsonTestArea, this));
      server.begin();    
    }

    private:
      String index_uri = "/";
      String fill_uri = "/solid";
      String pattern_uri = "/pattern";
      String json_uri = "/json";
      String chaser_uri = "/chaser";
      String frames[10];
      String chaserBase;
      uint8_t totalFrames = 0;
      uint8_t lastFrame = 255;
};

void colorWipe(uint32_t color, uint32_t time=0) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    if(time > 0) {
      delay(time);
      strip.show();
    }
  }
  if(time == 0)
    strip.show();
}

Endpoints ep;

void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.show();
  strip.setBrightness(100);
  Serial.begin(115200);
  Serial.println("Connecting");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    colorWipe(strip.Color(0,0,100));
    delay(250);
    Serial.print(".");
    colorWipe(strip.Color(0,0,0));
  }

  ep.setupEndpoints();

  // Indicator animation
  colorWipe(strip.Color(0,100,0), 5);
  delay(50);
  colorWipe(strip.Color(0,0,0));
  delay(25);
  colorWipe(strip.Color(0,100,0));
  delay(25);
  colorWipe(strip.Color(0,0,0));
  delay(25);

  Serial.println();
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  currentTime = millis();
  if(currentTime - previousTime >= frameTime && frameTime != 65535) {
    previousTime = currentTime;
    //ep.nextFrame(); not implemented
    ep.nextChaser();
  }
}
