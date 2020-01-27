# Esp8266MqttTemperature

##What it does
  * ESP8266 microcontroller board is used to 
  * take temperature and humidity measurements using a [DHT22](https://www.mouser.com/datasheet/2/737/dht-932870.pdf) sensor, using DHT library,
  * send measurements as MQTT topics (EspTopicTemp and EspTopicHumi) to a MQTT broker,
  * display measurements on an OLED display, using SSD1306Wire library,
  * display wifi ok, MQTT ok and sensor ok indicators.

##Arduino IDE version
Using version 1.6.12.

##To install the ESP8266 board (using Arduino 1.6.4+)
  * Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs": http://arduino.esp8266.com/stable/package_esp8266com_index.json
  * Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  * Select your ESP8266 in "Tools -> Board"

##Arduino IDE settings
  *  Board: NodeMCU 0.9 (ESPP-12 Module)
  *  CPU Frequency: 80 MHz
  *  Flash Size: 4M (3M SPIFFS)
  *  Upload Speed: 115200
  *  Programmer: Arduino as ISP

##Network connection
Update connection.h with SSID and password of your wifi access point.

##MQTT
Update connection.h with MQTT broker hostname of IP address.

MQTT topics are sent to via an MQTT broker to MQTT clients that subscribe to the topic.  For the broker I used an Onion and installed [mosquitto](https://mosquitto.org/).
i
The MQTT client, [MQTT Dash](https://play.google.com/store/apps/details?id=net.routix.mqttdash&hl=en_US), an Android app, is used to display the measurements in real time on a smart phone.  Set up your app with the hostname of the broker.  Once connected to the broker, create a value object using the topic name (I used EspTopicTemp for the temperature topic).
