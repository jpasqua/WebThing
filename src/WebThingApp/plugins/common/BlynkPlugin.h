#ifndef BlynkPlugin_h
#define BlynkPlugin_h

/*
 * BlynkPlugin
 *    A plugin that gets and displays content from Blynk
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <BaseSettings.h>
//                                  WebThing Libraries
#include <WebThingApp/plugins/Plugin.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class BlynkSettings : public BaseSettings {
public:
  // ----- Constructors and methods
  BlynkSettings();
  void fromJSON(JsonDocument &doc);
  void fromJSON(String& settings);
  void toJSON(JsonDocument &doc);
  void toJSON(String &serialized);
  void logSettings();

  // ----- Settings
  bool      enabled;
  uint8_t   nBlynkIDs;
  String*   blynkIDs;
  String*   nicknames;
  uint8_t   nPins;
  String*   pins;
  uint32_t  refreshInterval;
  uint32_t  riScale;  // NOT a user-visible setting!
};

class BlynkPlugin : public Plugin {
public:
  BlynkSettings settings;

  ~BlynkPlugin();
  bool typeSpecificInit();
  void typeSpecificMapper(const String& key, String& value);
  void refresh(bool force = false);
  void getSettings(String& serializedSettings);
  void newSettings(String& serializedSettings);
  uint32_t getUIRefreshInterval();

private:
  String*   _pinVals;
  uint32_t  _nextRefresh = 0;
};

#endif  // BlynkPlugin_h
