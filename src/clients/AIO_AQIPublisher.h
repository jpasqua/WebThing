#ifndef AIO_AQIPublisher_h
#define AIO_AQIPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  WebThing Includes
#include <sensors/AQIMgr.h>
//                                  Local Includes
#include "AIOMgr.h"
//--------------- End:    Includes ---------------------------------------------


class AIO_AQIPublisher : public AIOPublisher {
public:
  AIO_AQIPublisher(AQIMgr* aqiMgr)
      : _aqiMgr(aqiMgr) { }

  bool publish() override {
    const AQIReadings& readings = _aqiMgr->getLastReadings();
    // Only publish if we have a new reading, & it's time
    if (readings.timestamp == _timestampOfLastData ||
        millis() < _timestampOfNextPublish) return false;

    AIOMgr::busy(true);
    AIOMgr::aio->set("env010", readings.env.pm10);
    AIOMgr::aio->set("env025", readings.env.pm25);
    AIOMgr::aio->set("env100", readings.env.pm100);
    AIOMgr::aio->set("aqi", _aqiMgr->derivedAQI(readings.env.pm25));

    if (timeStatus() == timeSet) {
      String dateTime = Output::formattedDateTime(Basics::wallClockFromMillis(readings.timestamp));
      AIOMgr::aio->set("aqitime", dateTime);
    }
    AIOMgr::busy(false);

    _timestampOfLastData = readings.timestamp;
    _timestampOfNextPublish = millis() + TimeBetweenUpdates;
    Log.verbose("AIO_AQIPublisher: published at %s",
        Output::formattedDateTime(Basics::wallClockFromMillis(millis())).c_str());
    return true;
  }

private:
  static constexpr uint32_t TimeBetweenUpdates = Basics::minutesToMS(5);
  AQIMgr* _aqiMgr;
  uint32_t _timestampOfLastData = 0;
  uint32_t _timestampOfNextPublish = 0;
};

#endif	// AIO_AQIPublisher_h
