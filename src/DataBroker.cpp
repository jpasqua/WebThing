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

  namespace Mappings {
    using Mapper = struct {
      char prefix;
      WTBasics::ReferenceMapper map;
    };

    constexpr uint8_t MaxMappers = 8;
    uint8_t nMappers = 0;
    std::array<Mapper, MaxMappers> mappers;

    Mapper* findMapper(char prefix) {
      for (auto& mapper: mappers) {
        if (mapper.prefix == prefix) return &mapper;
      }
      return NULL;
    }

    bool addMapper(WTBasics::ReferenceMapper map, char prefix) {
      if (nMappers == MaxMappers) {
        Log.warning("DataBroker::registerMapper: No space remains for more mappers");
        return false;
      }

      Mapper* m = findMapper(prefix);
      if (m != NULL) {
        Log.warning("DataBroker::registerMapper: mapper for prefix %c was already registered", prefix);
        return false;
      }
      mappers[nMappers++] = {prefix, map};

      return true;
    }

    void performMapping(char prefix, const String& key, String& value) {
      Mapper* m = findMapper(prefix);
      if (m == NULL) {
        Log.warning("DataBroker::map: No mapper found for key %s", key.c_str());
        return;
      }
      m->map(key, value);
    }
  } // ----- END: Databroker::Mappings namespace


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



  // Upon entering this function, value is an empty String
  void map(const String& key, String& value) {
    // Keys are of the form: $P.subkey, where P is a prefix character indicating the namespace
    if (key.length() < 4 || key[0] != '$' || key[2] != '.') return;
    char prefix = key[1];
    const String subkey(&(key.c_str()[3]));
    Mappings::performMapping(prefix, subkey, value);
  }

  bool registerMapper(WTBasics::ReferenceMapper map, char prefix) {
    return Mappings::addMapper(map, prefix);
  }

  void begin() {
    registerMapper(System::map, System::NamespacePrefix);
  }

};