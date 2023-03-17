#ifndef DeviceReadings_h
#define DeviceReadings_h

struct DeviceReadings {
  float voltage;
  uint32_t chipID;
  struct {
    uint32_t free;
    uint8_t frag;
    uint16_t maxFreeBlock;
  } heap;
  uint32_t timestamp;
};

#endif // DeviceReadings_h