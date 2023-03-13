/*
 * AIOMgr
 *    Manage the connection to AIO and writers who wish to publish data
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <ArduinoLog.h>
//                                  Third Party Libraries
//                                  Local Includes
#include "AIOMgr.h"
//--------------- End:    Includes ---------------------------------------------

namespace AIOMgr {
  namespace Internal {
    constexpr uint8_t MaxPublishers = 4;

    bool    initialized = false;
    uint8_t nPublishers = 0;
    std::array<AIOPublisher*, MaxPublishers> publishers;
  } // ----- END: AIOMgr::Internal namespace

  AIOClient* aio = NULL;

  bool registerPublisher(AIOPublisher* p) {
    // TO DO: Ensure that this pin range does not overlap with any previously
    // registered range. If it does, the registration should fail.

    if (Internal::nPublishers == Internal::MaxPublishers) {
      Log.warning(
          "Can't register another AIOPublisher - max # (%d) has been reached",
          Internal::MaxPublishers);
      return false;
    }
    Internal::publishers[Internal::nPublishers++] = p;
    return true;
  }

  void init(String& username, String& key) {
    if (!Internal::initialized) {
      aio = new AIOClient();
      Internal::initialized = aio->init(username.c_str(), key.c_str());
    }
  }

  bool publish() {
    if (!Internal::initialized) return false;

    for (int i = 0; i < Internal::nPublishers; i++) {
      Internal::publishers[i]->publish();
    }

    return true;
  }

  void flush() { }

}


