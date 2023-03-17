#ifndef DevReadingsMgr_h
#define DevReadingsMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <BPABasics.h>
#include <GenericESP.h>
//                                  WebThing Includes
//                                  Local Includes
#include "DeviceReadings.h"
//--------------- End:    Includes ---------------------------------------------


class DevReadingsMgr {
public:
  DeviceReadings latestReadings;

  void takeReadings(bool force = false) {
    if (force || millis() >= _timestampOfNextReading) {
      latestReadings.voltage = WebThing::measureVoltage();
      latestReadings.chipID = GenericESP::getChipID();
      latestReadings.heap.free = GenericESP::getFreeHeap();
      latestReadings.heap.frag = GenericESP::getHeapFragmentation();
      latestReadings.heap.maxFreeBlock = GenericESP::getMaxFreeBlockSize();
      latestReadings.timestamp = millis();
      _timestampOfNextReading = latestReadings.timestamp + TimeBetweenReads;
    }
  }

private:
  static constexpr const uint32_t TimeBetweenReads = Basics::minutesToMS(10);
  uint32_t _timestampOfNextReading = 0;
};


#endif	// DevReadingsMgr_h