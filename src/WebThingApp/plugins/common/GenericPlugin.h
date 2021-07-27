#ifndef GenericPlugin_h
#define GenericPlugin_h

/*
 * GenericPlugin
 *    A plugin that gets and displays static or global content
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  WebThing Libraries
#include <BaseSettings.h>
#include <WebThingApp/plugins/Plugin.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class GenericSettings : public BaseSettings {
public:
  // ----- Constructors and methods
  GenericSettings();
  void fromJSON(JsonDocument &doc);
  void fromJSON(String& settings);
  void toJSON(JsonDocument &doc);
  void toJSON(String &serialized);
  void logSettings();

  // ----- Settings
  bool      enabled;
  uint32_t  refreshInterval;
  uint32_t  riScale;  // NOT a user-visible setting!
};

class GenericPlugin : public Plugin {
public:
  GenericSettings settings;

  ~GenericPlugin();
  bool typeSpecificInit();
  void typeSpecificMapper(const String& key, String& value);
  void refresh(bool force = false);
  void getSettings(String& serializedSettings);
  void newSettings(String& serializedSettings);
  uint32_t getUIRefreshInterval();
};

#endif  // GenericPlugin_h
