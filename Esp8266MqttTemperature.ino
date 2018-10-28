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
 * @date 21 July 2018
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Here typically goes a more extensive explanation of what the header
 * defines. Doxygens tags are words preceeded by either a backslash @\
 * or by an at symbol @@.
 * @see http://www.stack.nl/~dimitri/doxygen/docblocks.html
 * @see http://www.stack.nl/~dimitri/doxygen/commands.html
 */


#include <math.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "connection.h"

// --------------------------------------------------------------------------------
// #defines
// --------------------------------------------------------------------------------
#define ADC_MAX           255
#define BAUD_RATE         115200
#define PUBLISH_PERIOD_MS 2000
#define R1_OHM            5600.0
#define VCC_VOLT          3.3
#define TOPIC_TEMP        "EspTopicTemp"
#define TOPIC_BLA         "EspTopicBla"

// --------------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------------
const unsigned int PIN_DIGITAL_WIFI_OK = D0;
const unsigned int PIN_DIGITAL_MQTT_OK = D1;
const unsigned int PIN_DIGITAL_SENS_OK = D2;
const unsigned int PIN_ANALOG_TEMP     = A0;

WiFiClient espClient;
PubSubClient client(espClient);
long g_last_msg = 0;
char g_msg[32];

// --------------------------------------------------------------------------------
// ESP8266 setup
// --------------------------------------------------------------------------------
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);          // Initialise BUILTIN_LED pin as output
  pinMode(PIN_DIGITAL_WIFI_OK, OUTPUT);  // Initialise wifi ok pin as output
  pinMode(PIN_DIGITAL_MQTT_OK, OUTPUT);  // Initialise mqtt ok pin as output
  pinMode(PIN_DIGITAL_SENS_OK, OUTPUT);  // Initialise sensor ok pin as output
  pinMode(PIN_ANALOG_TEMP, INPUT);       // Initialise sensor ok pin as input
  Serial.begin(BAUD_RATE);
  setup_wifi();
  client.setServer(MQTT_BROKER, MQTT_PORT);  // Initialise mqtt server
  client.setCallback(callback);              // Register callback function
}

// --------------------------------------------------------------------------------
// Connect to a wifi network
// --------------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  digitalWrite(PIN_DIGITAL_WIFI_OK, LOW);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(PIN_DIGITAL_WIFI_OK, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

// --------------------------------------------------------------------------------
// Connect to MQTT broker
// --------------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      digitalWrite(PIN_DIGITAL_MQTT_OK, HIGH);  //Switch on MQTT ok LED
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      digitalWrite(PIN_DIGITAL_MQTT_OK, LOW);  //Switch off MQTT ok LED
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// --------------------------------------------------------------------------------
// Convert analog to digital converter number to voltage.
// From the analogue to digital converter number (ADC), the voltage is calculated.
// ADC is the number that the ESP8266 produces when the analog voltage is
// digitised.  If the ESP8266 reads a voltage of 3.3V, it outputs ADC_MAX.
// Returns: float voltage in V.
// --------------------------------------------------------------------------------
float volt_from_adc(unsigned int adc_number) {
  float volt = (float)((float)adc_number / (float)ADC_MAX);
  Serial.print("ADC number: ");
  Serial.println(adc_number);
  Serial.print("Voltage from ADC: ");
  Serial.println(volt);
  return volt;
}

// --------------------------------------------------------------------------------
// Convert voltage to resistance.
// The ESP8266 measures the voltage across the thermistor. The 5.6 kΩ resistor and
// the thermistor make a voltage divider.
// Returns: float resistance in Ω.
// --------------------------------------------------------------------------------
float res_from_volt(float voltage) {
  float res = (float)((float)((float)voltage * (float)R1_OHM) / (float)((float)VCC_VOLT - (float)voltage));
  Serial.print("Resistance from voltage: ");
  Serial.println(res);
  return res;
}

// --------------------------------------------------------------------------------
// Convert resistance to temperature.
// The thermistor’s resistance varies with temperature.  The thermistor
// manufacturer gives the measured resistance (in Ω) at a range of temperatures
// (in ºC).  Temperature as a function of temperature is per 2nd order polynomial
// trendline, suggested by MsExcel, based on graph of measurements taken with an
// uncalibrated temperature sensor/display unit as temperature reference, and a
// multimeter measuring across the thermistor.
// Returns: float temperature in ºC
// --------------------------------------------------------------------------------
float temp_from_res(float res_ohm) {
  // Temperature as a function of temperature is per 2nd order polynomial trendline
  // generated by MsExcel, based on graph of measurements taken on 20 Oct 2018.
  float temp = -0.0000003 * res_ohm * res_ohm - 0.007 * res_ohm + 53.14;
  Serial.print("Temperature from resistance: ");
  Serial.println(temp);
  return temp;
}

// --------------------------------------------------------------------------------
// Publish temperature measurement as MQTT topic.
// --------------------------------------------------------------------------------
void publish_temp() {
  Serial.print("ADC read: ");
  Serial.println(analogRead(PIN_ANALOG_TEMP));  
  float temp = temp_from_res(res_from_volt(volt_from_adc(analogRead(PIN_ANALOG_TEMP))));
  client.publish(TOPIC_TEMP, String(temp).c_str(), true);
  client.publish(TOPIC_BLA, "publishing");
  Serial.print("Temperature: ");
  Serial.println(temp);
}

// --------------------------------------------------------------------------------
// Main loop
// --------------------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - g_last_msg > PUBLISH_PERIOD_MS) {
    g_last_msg = now;
    publish_temp();  // publish temperature as MQTT topic
  }
}
