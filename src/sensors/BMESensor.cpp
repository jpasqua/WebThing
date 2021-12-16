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

  Log.verbose("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling, filter off");
  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1, // temperature
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
    return;
  }

  bme.takeForcedMeasurement();
  auto newTemp = bme.readTemperature();
  if ((lastTemp != -1024.0) && abs(newTemp - lastTemp) > ((curTime-lastTimeStamp)/1000)) {
    // If the temperature changed more than 1 degree per second, it's bad - ignore it
    Log.warning(
      "new temp: %f, last temp: %f - Too much variation in %d seconds",
      newTemp, lastTemp, (curTime-lastTimeStamp)/1000);
    return;
  }

  readings.timestamp = lastTimeStamp = curTime;
  readings.temp = newTemp;
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
