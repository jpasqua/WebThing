#include "BMESensor.h"


void BMESensor::begin(int addr) {
  bool status;

  if (addr == 0) { mock = true; return; }
  else mock = false;
  
  status = bme.begin(addr);
  if (status == 0) {
    Log.error("Could not find a valid BME280 sensor, check wiring!");
    mock = true;
    mockTemp = 20;
    mockHumidity = 30;
    mockPressure = 1013;
    return;
  }

  Log.verbose("BMESensor: forced mode, oversampling: 16x temp / 1x humi / 1x baro, filter off");
  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X16, // temperature
    Adafruit_BME280::SAMPLING_X1, // pressure
    Adafruit_BME280::SAMPLING_X1, // humidity
    Adafruit_BME280::FILTER_OFF   );
}

void BMESensor::takeReadings(WeatherReadings& readings) {
  static uint32_t lastTimeStamp = 0;
  static float lastTemp = -1024.0;

  uint32_t curTime = millis();
  if (mock) {
    readings.timestamp = curTime;
    mockReadings(readings);
    Log.verbose(
        "BME.Mock::Temp: %F°C, Humidity: %F%%, Pressure: %F hPa",
        readings.temp, readings.humidity, readings.pressure);    
    return;
  }

  bme.takeForcedMeasurement();
  auto newTemp = bme.readTemperature();
  if ((lastTemp != -1024.0) && abs(newTemp - lastTemp) > 1) {
    Log.warning(
      "new temp: %f, last temp: %f - Too much variation, re-reading",
      newTemp, lastTemp);
    // This may be due to a dip in voltage caused by other activity, try again
    bme.takeForcedMeasurement();
    newTemp = bme.readTemperature();
  }
  lastTemp = newTemp;

  readings.timestamp = lastTimeStamp = curTime;
  if (_includeTypes & WeatherSensor::ReadingType::Temperature) readings.temp = newTemp;
  if (_includeTypes & WeatherSensor::ReadingType::Humidity) readings.humidity = bme.readHumidity();;
  if (_includeTypes & WeatherSensor::ReadingType::Pressure) readings.pressure = bme.readPressure() / 100.0F;;

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
