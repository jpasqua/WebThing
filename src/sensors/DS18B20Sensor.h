#ifndef DS18B20_h
#define DS18B20_h

#include <OneWire.h>
#include <DallasTemperature.h>
#include "WeatherSensor.h"
#include "WeatherReadings.h"


class DS18B20Sensor : public WeatherSensor {
public:
  static constexpr float ReadingError = -1000.0;

  void begin(int pin) {
    ow = new OneWire(pin);
    dt = new DallasTemperature(ow);
    dt->begin();
  }

  virtual uint8_t availableReadingTypes() override {
    return WeatherSensor::ReadingType::Temperature;
  }

  virtual void takeReadings(WeatherReadings& readings) override {
    if (_includeTypes & WeatherSensor::ReadingType::Temperature) {
      dt->requestTemperatures();            // Send the command to get temperatures
      float tempC = dt->getTempCByIndex(0); // Read the temperature back
      if (tempC == DEVICE_DISCONNECTED_C) {
        Log.warning("DS18B20: DEVICE_DISCONNECTED_C. Unable to take reading");
      } else {
        readings.temp = tempC;
        Log.verbose("DS18B20::Temp: %F°C", readings.temp);
      }
    }
  }

private:
  OneWire* ow;
  DallasTemperature* dt;
};

#endif  // DS18B20_h
