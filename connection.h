#ifndef CONNECTION_H
#define CONNECTION_H

// Swifi
#define WIFI_SSID           "SwartNET"
#define WIFI_PWD            "onsgaanbraai"
#define RECONNECT_PERIOD_MS 2000
bool b_wifi_ok = false;

// MQTT
#define MQTT_BROKER       "192.168.0.108"
#define MQTT_PORT         1883
#define PUBLISH_PERIOD_MS 5000
#define TOPIC_TEMP        "EspTopicTemp"
#define TOPIC_HUMI        "EspTopicHumi"
#define TOPIC_NAN         "EspTopicNan"
bool b_mqtt_ok = false;


#endif /* CONNECTION_H */
