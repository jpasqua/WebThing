#ifndef BlynkClient_h
#define BlynkClient_h

#define NO_GLOBAL_INSTANCES
#define NO_GLOBAL_BLYNK

#if defined(ESP8266)
  #include <BlynkSimpleEsp8266.h>
#elif defined(ESP32)
  #include <BlynkSimpleEsp32.h>
#endif

#undef NO_GLOBAL_INSTANCES
#undef NO_GLOBAL_BLYNK

#endif	// BlynkClient_h