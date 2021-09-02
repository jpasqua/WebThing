/*
 * DataBroker:
 *    A mechanism by which different data suppliers can publish data for
 *    consumption by other components.
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
#include <GenericESP.h>
#include <WTBasics.h>
//                                  Local Includes
#include "DataBroker.h"
//--------------- End:    Includes ---------------------------------------------


namespace DataBroker {

  typedef struct Mapper {
    char prefix;
    WTBasics::ReferenceMapper map;
  } Mapper;

  constexpr uint8_t MaxMappers = 8;
  uint8_t nMappers = 0;
  Mapper mappers[MaxMappers];

  namespace System {
    constexpr char NamespacePrefix = 'S';

    void map(const String& key, String& value) {
      if (key.equalsIgnoreCase("time")) {
        char buf[9];
        time_t theTime = now();
        sprintf(buf, "%2d|%2d|%2d", hourFormat12(theTime), minute(theTime), second(theTime));
        value += buf;
      }
      else if (key.equalsIgnoreCase("author")) value += F("Joe Pasqua");
      else if (key.equalsIgnoreCase("heap")) {
        value += F("Heap: Free=");
        value += GenericESP::getFreeHeap();
        value += ", Frag=";
        value += GenericESP::getHeapFragmentation();
        value += '%';
      }
    }
  } // ----- END: Databroker::System namespace


  Mapper* findMapper(char prefix) {
    for (int i = 0; i < nMappers; i++) {
      if (mappers[i].prefix == prefix) return &mappers[i];
    }
    return NULL;
  }

  // Upon entering this function, value is an empty String
  void map(const String& key, String& value) {
    // Keys are of the form: $P.subkey, where P is a prefix character indicating the namespace
    if (key.length() < 4 || key[0] != '$' || key[2] != '.') return;
    char prefix = key[1];
    Mapper* m = findMapper(prefix);
    if (m == NULL) {
      Log.warning("DataBroker::map: No mapper found for key %s", key.c_str());
      return;
    }

    String subkey = key.substring(3);
    m->map(subkey, value);
  }

  bool registerMapper(WTBasics::ReferenceMapper map, char prefix) {
    if (nMappers == MaxMappers) {
      Log.warning("DataBroker::registerMapper: No space remains for more mappers");
      return false;
    }
    Mapper* m = findMapper(prefix);
    if (m != NULL) {
      Log.warning("DataBroker::registerMapper: mapper for prefix %c was already registered", prefix);
      return false;
    }
    mappers[nMappers].prefix = prefix;
    mappers[nMappers].map = map;
    nMappers++;
    return true;
  }

  void begin() {
    registerMapper(System::map, System::NamespacePrefix);
  }

};