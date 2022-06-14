#ifndef AQIBlynkPublisher_h
#define AQIBlynkPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  WebThing Includes
#include <sensors/AQIMgr.h>
//                                  Local Includes
#include "BlynkWrapper.h"
//--------------- End:    Includes ---------------------------------------------


class AQIBlynkPublisher : public BlynkPublisher {
public:
  AQIBlynkPublisher(AQIMgr* aqiMgr)
      : _aqiMgr(aqiMgr) { }

  bool publish() override {
    const AQIReadings& readings = _aqiMgr->getLastReadings();
    if (readings.timestamp == _timestampOfLastData) return false;

    BlynkClient.virtualWrite(Env010Pin, readings.env.pm10);
    BlynkClient.virtualWrite(Env025Pin, readings.env.pm25);
    BlynkClient.virtualWrite(Env100Pin, readings.env.pm100);
    BlynkClient.virtualWrite(MA30Pin, _aqiMgr->pm25env_30min.getAverage());
    BlynkClient.virtualWrite(AQIPin, _aqiMgr->derivedAQI(readings.env.pm25));

    if (timeStatus() == timeSet) {
      String dateTime = Output::formattedTime(Basics::wallClockFromMillis(readings.timestamp));
      Log.verbose("Timestamp sent to Blynk: %s", dateTime.c_str());
      BlynkClient.virtualWrite(TimestampPin, dateTime);
    }

    _timestampOfLastData = readings.timestamp;
    return true;
  }

  BlynkPinRange getPinRange() override {
    return {Env010Pin, AQIPin};
  }


private:
  static constexpr uint8_t Env010Pin    = 20;
  static constexpr uint8_t Env025Pin    = 21;
  static constexpr uint8_t Env100Pin    = 22;
  static constexpr uint8_t TimestampPin = 23;
  static constexpr uint8_t MA30Pin      = 24;
  static constexpr uint8_t AQIPin       = 25;

  AQIMgr* _aqiMgr;
  uint32_t _timestampOfLastData = 0;
};

#endif	// AQIBlynkPublisher_h
