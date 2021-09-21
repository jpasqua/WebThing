#ifndef DHT22Sensor_h
#define DHT22Sensor_h

#include <DHT.h>
#include "Readings.h"

class DHT22Sensor {
public:
  DHT22Sensor() = default;

  void begin(uint8_t pin) {
    _dht = new DHT(pin, DHT22);
    _dht->begin();
  }

  virtual uint8_t availableReadingTypes() override {
    return WeatherSensor::ReadingType::Temperature | WeatherSensor::ReadingType::Humidity;
  }

  void takeReadings(WeatherReadings& readings) override {
    readings.timestamp = millis();
    readings.temp = _dht->readTemperature();
    readings.humidity = _dht->readHumidity();

    Log.verbose("DHT22 Temp: %F°C, Humidity: %F%%", readings.temp, readings.humidity);
  }

private:
  DHT*  _dht;
};

#endif  // DHT22Sensor_h
