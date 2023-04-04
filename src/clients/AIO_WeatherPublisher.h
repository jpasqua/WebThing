#ifndef AIO_WeatherPublisher_h
#define AIO_WeatherPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  WebThing Includes
#include <sensors/WeatherReadings.h>
#include <sensors/WeatherMgr.h>
//                                  Local Includes
#include "AIOMgr.h"
//--------------- End:    Includes ---------------------------------------------


class AIO_WeatherPublisher : public AIOPublisher {
public:

  AIO_WeatherPublisher(WeatherMgr* weatherMgr)
      : _weatherMgr(weatherMgr) { }

  bool publish() override {
    const WeatherReadings& readings = _weatherMgr->getLastReadings();
    // Only publish if we have a new reading, & it's time
    if (readings.timestamp == _timestampOfLastData ||
        millis() < _timestampOfNextPublish) return false;

    AIOMgr::busy(true);
    if (!isnan(readings.temp)) {
      AIOMgr::aio->set("temp", Output::temp(readings.temp), 2);
    }

    if (!isnan(readings.humidity)) {
      AIOMgr::aio->set("humidity", (int)readings.humidity);
    }

    if (!isnan(readings.pressure)) {
      AIOMgr::aio->set("barometer", Output::baro(readings.pressure), 2);
    }

    if (timeStatus() == timeSet) {
      String dateTime = Output::formattedDateTime(Basics::wallClockFromMillis(readings.timestamp));
      AIOMgr::aio->set("wthrtime", dateTime);
    }
    AIOMgr::busy(false);

    _timestampOfLastData = readings.timestamp;
    _timestampOfNextPublish = millis() + TimeBetweenUpdates;

    Log.verbose("AIO_WeatherPublisher: published at %s",
        Output::formattedDateTime(Basics::wallClockFromMillis(millis())).c_str());
    return true;
  }


private:
  static constexpr uint32_t TimeBetweenUpdates = Basics::minutesToMS(5);

  WeatherMgr* _weatherMgr;
  uint32_t _timestampOfLastData = 0;
  uint32_t _timestampOfNextPublish = 0;
};



#endif	// AIO_WeatherPublisher_h