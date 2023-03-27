#ifndef AQIReadings_h
#define AQIReadings_h

struct ParticleReadings {
  uint16_t pm10;
  uint16_t pm25;
  uint16_t pm100;
};

typedef struct {
  uint32_t timestamp  = 0;
  // The following fields come directly from the sensor
  ParticleReadings standard = {0, 0, 0};
  ParticleReadings env = {0, 0, 0};
  uint16_t particles_03um = 0;  // 0.3um Particle Count
  uint16_t particles_05um = 0;  // 0.5um Particle Count
  uint16_t particles_10um = 0;  // 1.0um Particle Count
  uint16_t particles_25um = 0;  // 2.5um Particle Count
  uint16_t particles_50um = 0;  // 5.0um Particle Count
  uint16_t particles_100um = 0; // 10.0um Particle Count
} AQIReadings;

#endif  // AQIReadings_h
