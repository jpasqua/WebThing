#ifndef CryptoPlugin_h
#define CryptoPlugin_h

/*
 * CryptoPlugin
 *    A plugin that gets crypto prices from CoinBase
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

class CryptoSettings : public BaseSettings {
public:
  // ----- Constructors and methods
  CryptoSettings();
  void fromJSON(JsonDocument &doc);
  void fromJSON(String& settings);
  void toJSON(JsonDocument &doc);
  void toJSON(String &serialized);
  void logSettings();

  // ----- Settings
  bool      enabled;
  uint8_t   nCoins;
  String*   nicknames;
  String*   coinIDs;
  uint32_t  refreshInterval;
  uint32_t  riScale;  // NOT a user-visible setting!
};

class CryptoPlugin : public Plugin {
public:
  CryptoSettings settings;

  ~CryptoPlugin();
  bool typeSpecificInit();
  void typeSpecificMapper(const String& key, String& value);
  void refresh(bool force = false);
  void getSettings(String& serializedSettings);
  void newSettings(String& serializedSettings);
  uint32_t getUIRefreshInterval();

private:
  String*   _coinVals;
  String*   _currencies;
  uint32_t  _nextRefresh = 0;
};

#endif  // CryptoPlugin_h
