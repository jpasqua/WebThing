#ifndef BMESensor_h
#define BMESensor_h

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoLog.h>
#include "WeatherSensor.h"
#include "WeatherReadings.h"

class BMESensor : public WeatherSensor {
public:
  BMESensor() = default;
  void begin(int addr=0x76);  // Either 0x76 or 0x77

  virtual uint8_t availableReadingTypes() override; 
  virtual void takeReadings(WeatherReadings& readings) override;

private:
  Adafruit_BME280 bme;

  // Support for mocked operation
  void mockReadings(WeatherReadings& readings);
  bool  mock;
  float mockTemp = 20;
  float mockHumidity = 30;
  float mockPressure = 1013;
};

#endif  // BMESensor_h
