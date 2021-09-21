#ifndef WeatherUtils_h
#define WeatherUtils_h

#include "WeatherMgr.h"

namespace WeatherUtils {
  void configureAvailableSensors(WeatherMgr& mgr) {

    #if defined(BME280_READINGS)
      #include "BMESensor.h"
      BMESensor* bme = new BMESensor();
      bme->begin(BME_I2C_ADDR);
      bme->includeReadingTypes(BME280_READINGS);
      mgr.addSensor(bme);
    #endif  // BME280_READINGS


    #if defined(DHT22_READINGS)
      #include "DHT22Sensor.h"
      DHT22* dht22 = new DHT22();
      dht22->begin(DHT22_PIN);
      dht->includeReadingTypes(DHT22_READINGS);
      mgr.addSensor(dht22);
    #endif  // DHT22_READINGS


    #if defined(DS18B20_READINGS)
      #include "DS18B20Sensor.h"
      DS18B20* ds18b20 = new DS18B20();
      ds18b20->begin(DS18B20_PIN);
      ds18b20->includeReadingTypes(DS18B20_READINGS);
      mgr.addSensor(ds18b20);
    #endif  // DS18B20_READINGS
  }
}
#endif // WeatherUtils_h