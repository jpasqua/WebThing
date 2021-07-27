#ifndef GenericESP_h
#define GenericESP_h

#include <Arduino.h>

namespace GenericESP {

  // ----- System
  uint32_t getChipID();
  void reset();

  // ----- Heap Stats
  uint32_t getFreeHeap();
  uint8_t getHeapFragmentation();
  uint16_t getMaxFreeBlockSize();
}

#endif // GenericESP_h