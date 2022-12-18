#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define PIN 5
#define CNT 102

Adafruit_NeoPixel strip(CNT, PIN, NEO_RGB + NEO_KHZ800);
const String html = "<html lang=\"en\"><meta charset=\"utf-8\"/><head> <style>*{font-family: 'Montserrat', sans-serif; font-size: 62.5%; padding: 0; margin: 0;}body{background: #102027;}h1{font-family: 'Montserrat', sans-serif; font-weight: 300; font-size: 1.7rem; padding: 0 29rem 1rem 0; color: #ffffff80;}h2{font-size: 2rem; font-weight: 300; color: #ffffff;}.plsCenterMe{display: flex; flex-direction: column; justify-content: top; align-items: top; height: 100vh;}.lighterSection{display: flex; flex-direction: column; justify-content: space-evenly; align-items: top; height: auto; padding: 0 1rem; background: #1a2a2f; border-color: #ffffff0a; border-style: solid; border-width: 1px; border-radius: 1px; box-shadow: #0000005f 1px 1px;}input#btn{font-family: 'Montserrat', sans-serif; font-weight: 400; width: auto; background-color: #102027; border: solid; border-color: #1a2a2f; border-radius: 5px; color: white; padding: .5rem; text-align: center; text-decoration: none; display: inline-block; margin-right: 10px; transition: .15s; font-size: 2rem;}input#btn:hover{background-color: #5b666b;}input#txt{height: 25%; font-size: 2rem;}.flex{display: -webkit-box; display: -moz-box; display: -ms-flexbox; display: -webkit-flex; display: flex;}.flex-child{-webkit-box-flex: 1 1 auto; -moz-box-flex: 1 1 auto; -webkit-flex: 1 1 auto; -ms-flex: 1 1 auto; flex: 1 1 auto; margin-right: 10px;}.inline{display: inline-block; margin-right: 10px;}#cblock{font-family: 'Montserrat', sans-serif; text-align: center; font-weight: 300; font-size: 1.68rem; padding: 0 150rem 6rem 0; color: #ffffff30; position: absolute; bottom: 0;}#cblock-content{position: absolute; text-align: center; word-break: break-all;}.slider{-webkit-appearance: none; appearance: none; width: 100%; height: 15px; background: #ffffff30; outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s; padding: 2.5rem; border-radius: 5px; border-color: #102027;}.slider:hover{opacity: 1;}.slider::-webkit-slider-thumb{-webkit-appearance: none; appearance: none; width: 25px; height: 25px; background: #AA1510; cursor: pointer; border-radius: 50%;}.slider::-moz-range-thumb{width: 25px; height: 25px; background: #AA1510; cursor: pointer; border-radius: 50%;}</style> <title>rpi rgb control</title> <link href=\"https://fonts.googleapis.com/css2?family=Montserrat:wght@300&display=swap\" rel=\"stylesheet\"></head><body> <div class=\"plsCenterMe\"> <div class=\"lighterSection\"> <h2>rgb strip control</h2> <input type=\"button\" id=\"btn\" value=\"Set value\" onclick=\"window.location=`/solid_color?r=${document.getElementById('red').value}&g=${document.getElementById('green').value}&b=${document.getElementById('blue').value}&bright=${document.getElementById('bright').value}`\"> <h2>red</h2> <input type=\"range\" min=\"0\" max=\"255\" value=\"127\" class=\"slider\" id=\"red\"> <h2>green</h2> <input type=\"range\" min=\"0\" max=\"255\" value=\"127\" class=\"slider\" id=\"green\"> <h2>blue</h2> <input type=\"range\" min=\"0\" max=\"255\" value=\"127\" class=\"slider\" id=\"blue\"> <h2>brightness</h2> <input type=\"range\" min=\"0\" max=\"100\" value=\"100\" class=\"slider\" id=\"bright\"> </div></div><script>const queryString=window.location.search;const urlParams=new URLSearchParams(queryString);if(urlParams !=null){document.getElementById('red').value=urlParams.get('r');document.getElementById('green').value=urlParams.get('g');document.getElementById('blue').value=urlParams.get('b');document.getElementById('bright').value=urlParams.get('bright');}</script> </body></html>";
const char* ssid = "Corvair";
const char* pass = "N3w 50luti0n5";

ESP8266WebServer server(80);
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

class Endpoints {
  public:
    void patternGen (String s, char delim) {
      int numPatterns = 0;
      int positions[100] = { 0 };
      for(int i = 0; i < s.length(); ++i) {
        if(s[i] == delim) {
          positions[numPatterns] = i; 
          ++numPatterns;
        }
      }
      positions[numPatterns] = s.length();
      ++numPatterns;
      patterns[0] = s.substring(0,positions[0]); // -> 1,1,1
      for(int i = 1; i <= numPatterns; ++i) {
        patterns[i] = s.substring(positions[i-1]+1,positions[i]);
      }
      totalPattern = numPatterns;
    }

    void colorGen() {
      for(int i = 0; i <= totalPattern; ++i) {
        int index = 0;
        String r = "";
        String g = "";
        String b = "";
        int rIndex = 0;
        int gIndex = 0;
        int bIndex = 0;
        for(int j = 0; j < patterns[i].length(); ++j) {
          if (patterns[i][j] == ',') {
            switch(index) {
              case 0:
                rIndex = j;
                ++index;
                break;

              case 1:
                gIndex = j;
                ++index;
                break;
            }
            bIndex = patterns[i].length();
          }
        }

        r = patterns[i].substring(0,rIndex);
        g = patterns[i].substring(rIndex + 1, gIndex);
        b = patterns[i].substring(gIndex + 1, bIndex);
        Serial.print("colGen i - ");
        Serial.println(i);
        reds[i] = r.toInt();
        greens[i] = g.toInt();
        blues[i] = b.toInt();
        Serial.print("colors[i] - ");
        Serial.println(strip.Color(reds[i], greens[i], blues[i]));
      }
    }

    void index() {
      server.send(200, "text/html", html);
    }
    
    void solid_color() {
      String redStr = server.arg("r");
      String greenStr = server.arg("g");
      String blueStr = server.arg("b");
      String brightStr = server.arg("bright");
      int red = redStr.toInt() * (brightStr.toInt() / 100);
      int green = greenStr.toInt() * (brightStr.toInt() / 100);
      int blue = blueStr.toInt() * (brightStr.toInt() / 100);
      uint32_t color = strip.Color(red, green, blue);
      for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
        strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      }
      strip.show();
      server.send(200, "text/html", html);
    }

    void pattern() {
      server.send(200, "text/html", html);
      String pattern = server.arg("p");
      std::vector<uint32_t> colors;
      patternGen(pattern, '_');
      colorGen();
      Serial.println("pattern generated");
      for(int i = 0; i < strip.numPixels() - 1; ++i) {
        Serial.println(i);
        for(int j = 0; j < totalPattern; ++j) {
          Serial.print("\t");
          Serial.println(j);
          strip.setPixelColor(i, strip.Color(reds[j],greens[j],blues[j]));
        }
      }
    }

    void setupEndpoints() {
      server.on(index_uri, std::bind(&Endpoints::index, this));
      server.on(fill_uri, std::bind(&Endpoints::solid_color, this));
      server.on(pattern_uri, std::bind(&Endpoints::pattern, this));
      server.begin();    
    }

    private:
      String index_uri = "/index";
      String fill_uri = "/solid_color";
      String pattern_uri = "/pattern_color";
      String patterns[100] = { " " };
      uint8_t reds[100] = { 0 };
      uint8_t greens[100] = { 0 };
      uint8_t blues[100] = { 0 };
      uint8_t totalPattern = 0;
};

void setup() {
  Endpoints ep;
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Connecting");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ep.setupEndpoints();
  
  Serial.println();
  Serial.println(WiFi.localIP());
  
  strip.begin();
  strip.show();
  strip.setBrightness(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}
