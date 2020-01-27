#ifndef SENSOR_H
#define SENSOR_H

// DHT sensor reads temperature and humidity
#define DHTPIN     D2     // Pin gpio 2 in sensor
#define DHTTYPE    DHT22  // DHT 22 Change this if you have a DHT11
float temperature;
float humidity;
bool b_sensor_ok = false;

#endif /* SENSOR_H */
