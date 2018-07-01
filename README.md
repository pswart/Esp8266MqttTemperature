# Esp8266MqttTemperature
MQTT via Ethernet connected thermometer using ESP8266 with thermistor to sense temperature.

Using Arduino IDE version 1.6.12.

To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

Arduino IDE settings:
  -  Board: NodeMCU 0.9 (ESPP-12 Module)
  -  CPU Frequency: 80 MHz
  -  Flash Size: 4M (3M SPIFFS)
  -  Upload Speed: 115200
  -  Programmer: Arduino as ISP

Update connection.h with your wifi SSID and password, and MQTT broker host name.
