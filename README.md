# ESP2812b
ESP2812b is a simple API for controlling NeoPixels and other WS2812b/WS2811 addressable LED strips with short JSON files.

## Libraries Used
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [Adafruit's NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)

## Endpoints
- /solid
    - Sets all LEDs to the same color
    - `/solid?c=FF0000`

- /pattern
    - Sets LEDs to repeating set of array of colors
    - `/pattern?json=["FF0000", "009900"]`

- /chaser
    - Similar behavior to /pattern with the addition of a light chaser effect
    - `/chaser?json={"p": ["FF0000", "00FF00", "0000FF"], "time": 300}`

- /sparkle (Not Implemented)
    - Displays a pattern onto the strip with random flashes of a specified color
    - `/sparkle?json={"p": ["FF0000", "009900"], "time": 100, "spark_color": "FFFFFF"}`

- /json
    - Test area to ensure json parsing is working as intended
    - `/json?json={"any": "valid json works", "here": [6,42]}`

#### All colors are hex RGB values, all times are in milliseconds

