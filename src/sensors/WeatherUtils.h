#ifndef WeatherUtils_h
#define WeatherUtils_h

#include "WeatherMgr.h"
#include "BMESensor.h"
#include "DHT22Sensor.h"
#include "DS18B20Sensor.h"

namespace WeatherUtils {
  void configureAvailableSensors(WeatherMgr& mgr) {

    #if defined(BME280_READINGS)
      BMESensor* bme = new BMESensor();
      bme->begin(BME_I2C_ADDR);
      bme->includeReadingTypes(BME280_READINGS);
      mgr.addSensor(bme);
    #endif  // BME280_READINGS


    #if defined(DHT22_READINGS)
      DHT22Sensor* dht22 = new DHT22Sensor();
      dht22->begin(DHT22_PIN);
      dht22->includeReadingTypes(DHT22_READINGS);
      mgr.addSensor(dht22);
    #endif  // DHT22_READINGS


    #if defined(DS18B20_READINGS)
      DS18B20Sensor* ds18b20 = new DS18B20Sensor();
      ds18b20->begin(DS18B20_PIN);
      ds18b20->includeReadingTypes(DS18B20_READINGS);
      mgr.addSensor(ds18b20);
    #endif  // DS18B20_READINGS

    #if !defined(BME280_READINGS) && !defined(DHT22_READINGS) && !defined(DS18B20_READINGS)
      (void)mgr;  // avoid compiler warning
    #endif
  }
}
#endif // WeatherUtils_h