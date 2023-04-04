/*
 * AIOMgr
 *    Manage the connection to AdafruitIO and writers who wish to publish data
 *
 */

#ifndef AIOMgr_h
#define AIOMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <functional>
//                                  Third Party Libraries
//                                  Local Includes
#include "AIOClient.h"
//--------------- End:    Includes ---------------------------------------------

class AIOPublisher {
public:
  virtual bool publish() = 0;
};

namespace AIOMgr {
  extern AIOClient* aio;

  extern void init(String& username, String& key);

  // Register a publisher which is responsible for publishing data to AIO
  // via the PublishCallback.
  // Returns:
  // true  -> The publisher was registered
  // false -> No space for more publishers
  extern bool registerPublisher(AIOPublisher* p);

  // Ask all registered publishers to send their data to AIO
  // They may or may not publish anything depending on whether they
  // have any updated information to report.
  // Returns:
  // true  -> All publishers were given an opportunity to write their data
  // false -> We don't have a connection to AIO
  extern bool publish();

  // Must be called to flush any pending data to the AIO service.
  extern void flush();

  // Provide a callback which can be invoked when AIOMgr (or publishers)
  // are going to be busy for a human-perceptible period of time
  extern void setBusyCB(std::function<void(bool)> busyCB);

  // Publishers may call this to indicate that they will be busy for a while, or have
  // stopped being busy after previously indicating that they were
  extern void busy(bool isBusy);
}

#endif // AIOMgr_h




