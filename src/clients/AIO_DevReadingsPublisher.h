#ifndef AIO_DevReadingsPublisher_h
#define AIO_DevReadingsPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  WebThing Includes
#include <sensors/DeviceReadings.h>
#include <sensors/DevReadingsMgr.h>
//                                  Local Includes
#include "AIOMgr.h"
//--------------- End:    Includes ---------------------------------------------


class AIO_DevReadingsPublisher : public AIOPublisher {
public:

  AIO_DevReadingsPublisher(DevReadingsMgr* drMgr)
      : _drMgr(drMgr) { }

  bool publish() override {
    // Only publish if we have a new reading, & it's time
    if (_drMgr->latestReadings.timestamp == _timestampOfLastPublishedReading ||
        millis() < _timestampOfNextPublish) return false;

    AIOMgr::busy(true);
    AIOMgr::aio->set("voltage", _drMgr->latestReadings.voltage, 2);
    // Publish other readings if they become interesting
    AIOMgr::busy(false);

    _timestampOfLastPublishedReading = _drMgr->latestReadings.timestamp;
    _timestampOfNextPublish = millis() + TimeBetweenUpdates;
    Log.verbose("AIO_DevReadingsPublisher: published at %s",
        Output::formattedDateTime(Basics::wallClockFromMillis(millis())).c_str());
    return true;
  }

private:
  static constexpr const uint32_t TimeBetweenUpdates = Basics::minutesToMS(10);

  DevReadingsMgr *_drMgr;
  uint32_t _timestampOfLastPublishedReading = 0;  // timestamp from the last reading we published
  uint32_t _timestampOfNextPublish = 0;           // Don't publish again before this time
};



#endif	// AIO_DevReadingsPublisher_h