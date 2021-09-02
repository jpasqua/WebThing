/*
 * DataBroker:
 *    A centralized point to get data from all available sources
 *
 */

#ifndef DataBroker_h
#define DataBroker_h


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <WTBasics.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

namespace DataBroker {
  void begin();
  void map(const String& key, String& value);
  bool registerMapper(WTBasics::ReferenceMapper map, char prefix);
};

#endif  // DataBroker_h