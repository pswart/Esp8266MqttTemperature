/*
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

/**
 * @file Esp8266MqttTemperature.h
 * @author Paul Swart
 * @date 26 January 2020
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Here typically goes a more extensive explanation of what the header
 * defines. Doxygens tags are words preceeded by either a backslash @\
 * or by an at symbol @@.
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */

#include <DHT.h>
#include <DHT_U.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "connection.h"
#include "sensor.h"

// For a connection via I2C using Wire include
#include <Wire.h>        // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, D3, D5);

// --------------------------------------------------------------------------------
// #defines
// --------------------------------------------------------------------------------
#define BAUD_RATE 115200

// --------------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------------
const unsigned int PIN_DIGITAL_WIFI_OK = D0;
const unsigned int PIN_DIGITAL_MQTT_OK = D1;
const unsigned int PIN_DIGITAL_SENS_OK = D2;

// Variables
WiFiClient espClient;
PubSubClient client(espClient);
long g_last_msg = 0;
// Initialize DHT
DHT dht(DHTPIN, DHTTYPE);

// --------------------------------------------------------------------------------
// Display initialising
// --------------------------------------------------------------------------------
void display_init() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,  1, String("W - ") + WIFI_SSID);
  display.drawString(0, 22, String("M - ") + MQTT_BROKER);
  display.display();
  }

// --------------------------------------------------------------------------------
// Display connected wifi
// --------------------------------------------------------------------------------
void display_conn_wifi() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,  1, String("W Ok ") + WIFI_SSID);
  display.drawString(0, 22, String("M - ") + MQTT_BROKER);
  display.display();
  }

// --------------------------------------------------------------------------------
// Display temperature and humidity
// --------------------------------------------------------------------------------
void display_measurements() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  if (b_wifi_ok)
    display.drawString(90, 1, "W Ok");
  else
    display.drawString(90,  1, "W --");
  if (b_mqtt_ok)
    display.drawString(90, 22, "M Ok");
  else
    display.drawString(90, 22, "M --");

  if (isnan(temperature) || isnan(humidity))
    display.drawString(90, 43, "S --");
  else
    display.drawString(90, 43, "S Ok");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0,  2, String(temperature, 1) + "°C");
  display.drawString(0, 36, String(humidity, 1) + "%");
  display.display();
  }

// --------------------------------------------------------------------------------
// ESP8266 setup
// --------------------------------------------------------------------------------
void setup() {
  // Initialise BUILTIN_LED pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // Initialise wifi ok pin as output
  pinMode(PIN_DIGITAL_WIFI_OK, OUTPUT);
  // Initialise mqtt ok pin as output
  pinMode(PIN_DIGITAL_MQTT_OK, OUTPUT);
  // Initialise sensor ok pin as output
  pinMode(PIN_DIGITAL_SENS_OK, OUTPUT);
  
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display_init();
  
  Serial.begin(BAUD_RATE);

  // Set up connection to wifi access point
  setup_wifi();
  
  // Initialise mqtt server
  client.setServer(MQTT_BROKER, MQTT_PORT);
  // Register callback function
  client.setCallback(callback);

  dht.begin();  // start up DHT
}

// --------------------------------------------------------------------------------
// Connect to a wifi network
// --------------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  digitalWrite(PIN_DIGITAL_WIFI_OK, LOW);
  b_wifi_ok = false;
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(PIN_DIGITAL_WIFI_OK, HIGH);
  b_wifi_ok = true;
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  display_conn_wifi();
  // Wait for display to be viewed
  delay(RECONNECT_PERIOD_MS);
}

// --------------------------------------------------------------------------------
// MQTT message reception callback function
// --------------------------------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    // Turn the LED on (Note that LOW is the voltage level
    digitalWrite(BUILTIN_LED, LOW);
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    // Turn the LED off by making the voltage HIGH
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

// --------------------------------------------------------------------------------
// Connect to MQTT broker
// --------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  b_mqtt_ok = false;
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      //Switch on MQTT ok LED
      digitalWrite(PIN_DIGITAL_MQTT_OK, HIGH);
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
      b_mqtt_ok = true;
      publish_measurements();
    } else {
      //Switch off MQTT ok LED
      digitalWrite(PIN_DIGITAL_MQTT_OK, LOW);
      b_mqtt_ok = false;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 4 seconds");

      display_measurements();

      // Wait before retrying
      delay(RECONNECT_PERIOD_MS);
    }
  }
}

// --------------------------------------------------------------------------------
// Publish temperature measurement as MQTT topic.
// --------------------------------------------------------------------------------
void publish_measurements() {
  client.publish(TOPIC_TEMP, String(temperature).c_str(), true);
  client.publish(TOPIC_HUMI, String(humidity).c_str(), true);
  if (isnan(temperature) || isnan(humidity)) {
    client.publish(TOPIC_NAN, String("True").c_str(), true); 
  }
  else {
    client.publish(TOPIC_NAN, String("False").c_str(), true);
  }
}

// --------------------------------------------------------------------------------
// Main loop
// --------------------------------------------------------------------------------
void loop() {
  // Read humidity
  humidity = dht.readHumidity();
  // Read temperature
  temperature = dht.readTemperature();
  // Display measurements
  display_measurements();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - g_last_msg > PUBLISH_PERIOD_MS) {
    g_last_msg = now;
    // publish temperature as MQTT topic
    publish_measurements();
  }
}
