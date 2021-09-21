#include "BMESensor.h"


void BMESensor::begin(int addr) {
  bool status;

  mock = false;
  status = bme.begin(addr);
  if (status == 0) {
    Log.error("Could not find a valid BME280 sensor, check wiring!");
    mock = true;
    mockTemp = 20;
    mockHumidity = 30;
    mockPressure = 1013;
    return;
  }

  Log.verbose("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling, filter off");
  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1, // temperature
    Adafruit_BME280::SAMPLING_X1, // pressure
    Adafruit_BME280::SAMPLING_X1, // humidity
    Adafruit_BME280::FILTER_OFF   );
}

void BMESensor::takeReadings(WeatherReadings& readings) {
  readings.timestamp = millis();
  if (mock) {
    mockReadings(readings);
    return;
  }

  bme.takeForcedMeasurement();
  readings.temp = bme.readTemperature();
  readings.humidity = bme.readHumidity();
  readings.pressure = bme.readPressure() / 100.0F;

  Log.verbose(
      "BME::Temp: %F°C, Humidity: %F%%, Pressure: %F hPa",
      readings.temp, readings.humidity, readings.pressure);    
}


uint8_t BMESensor::availableReadingTypes() {
  return
      WeatherSensor::ReadingType::Temperature |
      WeatherSensor::ReadingType::Humidity    |
      WeatherSensor::ReadingType::Pressure;
}


void BMESensor::mockReadings(WeatherReadings& readings) {
  readings.temp = mockTemp = constrain(mockTemp + ((float)random(-5, 6))/10.0, 0.0, 36.0);
  readings.humidity = mockHumidity = constrain(mockHumidity + ((float)random(-3, 4))/10.0, 0.0, 100.0);
  readings.pressure = mockPressure = constrain(mockPressure + ((float)random(-1, 2))/10.0, 1011.0, 1016.0);
}
