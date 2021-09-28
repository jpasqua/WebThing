#ifndef WeatherBlynkPublisher_h
#define WeatherBlynkPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  WebThing Includes
#include <sensors/WeatherReadings.h>
#include <sensors/WeatherMgr.h>
//                                  Local Includes
#include "BlynkClient.h"
//--------------- End:    Includes ---------------------------------------------


class WeatherBlynkPublisher : public BlynkPublisher {
public:

  WeatherBlynkPublisher(WeatherMgr* weatherMgr)
      : _weatherMgr(weatherMgr) { }

  bool publish() override {
    const WeatherReadings& readings = _weatherMgr->getLastReadings();
    if (readings.timestamp == _timestampOfLastData) return false;

    if (!isnan(readings.temp)) {
      Log.verbose("Temp: %F, converted: %F", readings.temp, Output::temp(readings.temp));
      Blynk.virtualWrite(TempPin, Output::temp(readings.temp));
      Blynk.virtualWrite(HeatIndexPin, Output::temp(readings.heatIndex));
      Blynk.virtualWrite(DewPointPin, Output::temp(readings.dewPointTemp));
      Blynk.virtualWrite(DewSpreadPin, Output::tempSpread(readings.dewPointSpread));
    }

    if (!isnan(readings.humidity)) {
      Blynk.virtualWrite(HumidityPin, readings.humidity);
    }

    if (!isnan(readings.pressure)) {
      Blynk.virtualWrite(PressurePin,    Output::baro(readings.pressure));
      Blynk.virtualWrite(RelPressurePin, Output::baro(readings.relPressure));
    }

    if (timeStatus() == timeSet) {
      String dateTime = Output::formattedTime(Basics::wallClockFromMillis(readings.timestamp));
      Log.verbose("Timestamp sent to Blynk: %s", dateTime.c_str());
      Blynk.virtualWrite(TimestampPin, dateTime);
    }

    _timestampOfLastData = readings.timestamp;
    return true;
  }

  BlynkPinRange getPinRange() override {
    return {TempPin, TimestampPin};
  }


private:
  static constexpr uint8_t TempPin         = 0;
  static constexpr uint8_t HumidityPin     = 1;
  static constexpr uint8_t PressurePin     = 2;
  static constexpr uint8_t RelPressurePin  = 3;
  static constexpr uint8_t DewPointPin     = 4;
  static constexpr uint8_t DewSpreadPin    = 5;
  static constexpr uint8_t HeatIndexPin    = 6;
  static constexpr uint8_t TimestampPin    = 8;

  WeatherMgr* _weatherMgr;
  uint32_t _timestampOfLastData = 0;
};



#endif	// WeatherBlynkPublisher_h