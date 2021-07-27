#include "GenericESP.h"

namespace GenericESP {

#if defined(ESP8266)
  // ----- System
  uint32_t getChipID() { return ESP.getChipId(); }
  void reset() { ESP.reset(); }

  // ----- Heap Stats
  uint32_t getFreeHeap()  { return ESP.getFreeHeap(); }
  uint8_t getHeapFragmentation() { return ESP.getHeapFragmentation(); }
  uint16_t getMaxFreeBlockSize() { return ESP.getMaxFreeBlockSize(); }


#elif defined(ESP32)
  // ----- System
  void reset() { ESP.restart(); }
  uint32_t getChipID() { 
    uint32_t chipID = 0;
    for(int i = 0; i < 17; i = i+8) {
      chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return chipID;
  }

  // ----- Heap Stats
  uint32_t getFreeHeap()  { return ESP.getFreeHeap(); }
  uint8_t getHeapFragmentation() { return 100-((ESP.getMaxAllocHeap()*100)/ESP.getFreeHeap()); }
  uint16_t getMaxFreeBlockSize() { return ESP.getMaxAllocHeap(); }

#else
  #error "Must be an ESP8266 or ESP32"
#endif  
}
