#ifndef WeatherSensor_h
#define WeatherSensor_h

#include "WeatherReadings.h"

class WeatherSensor {
public:
  enum ReadingType {Temperature = 1 << 0, Humidity = 1 << 1, Pressure = 1 << 2};

  // Define which readings this sensor should record. It can be any combination of
  // the defined ReadingType's. before this function is called, the default is to
  // record all ReadingType's.
  // @param   types   A bitwise or of the ReadingType's we are being asked to record
  void includeReadingTypes(uint8_t types) { _includeTypes = types; }

  // Return the capabilities of the sensor; that is, what ReadingType's it can take
  // @return A bitwise or of the ReadingType's this device can deliver.
  virtual uint8_t availableReadingTypes() = 0;

  // Take whatever sensor readings the device is capable of, but only record the
  // types of readings specified by includeReadingTypes(). Note that the device
  // need not take a reading for a particular type if it will not be used.
  virtual void takeReadings(WeatherReadings& readings) = 0;

  static void calculateDerivedValues(WeatherReadings& readings, int elevation);
  static void applyCorrections(WeatherReadings& readings, float t, float h);

protected:
  uint8_t _includeTypes = Temperature | Humidity | Pressure;
};


#endif	// WeatherSensor_h