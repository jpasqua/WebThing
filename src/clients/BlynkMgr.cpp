/*
 * BlynkMgr
 *    Manage the connection to Blynk and writers who wish to publish data
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <ArduinoLog.h>
//                                  Third Party Libraries
#if defined(ESP8266)
  #include <BlynkSimpleEsp8266.h>
#elif defined(ESP32)
  #include <BlynkSimpleEsp32.h>
#endif
//                                  Local Includes
#include "BlynkMgr.h"
//--------------- End:    Includes ---------------------------------------------

namespace BlynkMgr {
  namespace Internal {
    constexpr const uint8_t MaxPublishers = 4;

    std::array<BlynkPublisher*, MaxPublishers> publishers;
    uint8_t nPublishers = 0;

    String blynkKey;
    bool connectedToBlynk;
  } // ----- END: BlyknWriter::Internal namespace

  bool registerPublisher(BlynkPublisher* p) {
    // TO DO: Ensure that this pin range does not overlap with any previously
    // registered range. If it does, the registration should fail.

    if (Internal::nPublishers == Internal::MaxPublishers) {
      Log.warning(
          "Can't register another BlynkPublisher - max # (%d) has been reached",
          Internal::MaxPublishers);
      return false;
    }
    Internal::publishers[Internal::nPublishers++] = p;
    return true;
  }

  void init(String& blynkKey) {
    Log.trace("Connecting to Blynk");
    Internal::connectedToBlynk = false;
    if (blynkKey.length() != 0) {
      Internal::blynkKey = blynkKey;
      Blynk.config(Internal::blynkKey.c_str());
      Internal::connectedToBlynk = Blynk.connect(30*1000L);
      if (Internal::connectedToBlynk) {
        Log.trace("Successfully connected to Blynk");
      } else { Log.warning("Unable to connect to Blynk"); }
    } else {
      Log.notice("No Blynk API key was specified.");
    }
  }

  bool publish() {
    if (!Internal::connectedToBlynk) return false;

    for (int i = 0; i < Internal::nPublishers; i++) {
      Internal::publishers[i]->publish();
    }

    return true;
  }

  void loop(){
    if (Internal::connectedToBlynk) Blynk.run();
  }

  void disconnect() {
    if (Internal::connectedToBlynk) {
      Blynk.run();
      Blynk.disconnect();
    }
  }
}
