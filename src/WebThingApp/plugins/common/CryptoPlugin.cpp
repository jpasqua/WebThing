/*
 * CryptoPlugin
 *    A plugin that gets crypto prices from CoinBase
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <ArduinoLog.h>
//                                  Third Party Libraries
//                                  WebThing Libraries
#include <WebThingApp/clients/CoinbaseClient.h>
#include <WebThingApp/gui/Theme.h>
#include <WebThingApp/gui/ScreenMgr.h>
//                                  Local Includes
#include "CryptoPlugin.h"
//--------------- End:    Includes ---------------------------------------------



/*------------------------------------------------------------------------------
 *
 * CryptoSettings Implementation
 *
 *----------------------------------------------------------------------------*/

CryptoSettings::CryptoSettings() {
  version = 1;
  maxFileSize = 2048;

  enabled = true;
  nCoins = 0;
  nicknames = NULL;
  coinIDs = NULL;
  riScale = 60 * 1000L;   // Operate in minutes
  refreshInterval = 10;   // 10 Minutes
}

void CryptoSettings::fromJSON(JsonDocument &doc) {
  JsonArrayConst ids = doc[F("coinIDs")];
  JsonArrayConst names = doc[F("nicknames")];
  nCoins = ids.size();  
  coinIDs = new String[nCoins];
  nicknames = new String[nCoins];

  for (int i = 0; i < nCoins; i++) {
    coinIDs[i] = ids[i].as<String>();
    nicknames[i] = names[i].as<String>();
  }

  enabled = doc[F("enabled")];
  refreshInterval = doc[F("refreshInterval")];
  riScale = doc[F("riScale")];
}

void CryptoSettings::fromJSON(String& settings) {
  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, settings);
  if (error) {
    Log.warning(F("CryptoSettings::fromJSON, failed to parse new settings: %s"), error.c_str());
    return;
  }
  fromJSON(doc);
}

void CryptoSettings::toJSON(JsonDocument &doc) {
  JsonArray coinIDArray = doc.createNestedArray(F("coinIDs"));
  JsonArray nnArray = doc.createNestedArray(F("nicknames"));
  for (int i = 0; i < nCoins; i++) {
    coinIDArray.add(coinIDs[i]);
    nnArray.add(nicknames[i]);
  }

  doc[F("refreshInterval")] = refreshInterval;
  doc[F("riScale")] = riScale;
  doc[F("enabled")] = enabled;
}

void CryptoSettings::toJSON(String& serialized) {
  DynamicJsonDocument doc(maxFileSize);
  toJSON(doc);
  serializeJson(doc, serialized);
}

void CryptoSettings::logSettings() {
  Log.verbose(F("----- CryptoSettings"));
  for (int i = 0; i < nCoins; i++) {
    Log.verbose(F("  %s (%s)"), nicknames[i].c_str(), coinIDs[i].c_str());
  }
  Log.verbose(F("  enabled: %T"), enabled);
  Log.verbose(F("  refreshInterval: %d"), refreshInterval);
  Log.verbose(F("  riScale: %d"), riScale);
}


/*------------------------------------------------------------------------------
 *
 * CryptoPlugin Implementation
 *
 *----------------------------------------------------------------------------*/

CryptoPlugin::~CryptoPlugin() {
  // TO DO: free the settings object
}

void CryptoPlugin::getSettings(String& serializedSettings) {
  settings.toJSON(serializedSettings);
}

void CryptoPlugin::newSettings(String& serializedSettings) {
  settings.fromJSON(serializedSettings);
  settings.write();
}

uint32_t CryptoPlugin::getUIRefreshInterval() { return settings.refreshInterval * settings.riScale; }

bool CryptoPlugin::typeSpecificInit() {
  settings.init(_pluginDir + "/settings.json");
  settings.read();
  settings.logSettings();

  _enabled = settings.enabled;
  _coinVals = new String[settings.nCoins];
  _currencies = new String[settings.nCoins];

  return true;
}

void CryptoPlugin::typeSpecificMapper(const String& key, String& value) {
  // Get index of desired item
  int index = key.substring(2).toInt();
  if (index > 0 && index <= settings.nCoins) {
    if (key.startsWith("NN")) {
      value = settings.nicknames[index-1];
    } else if (key.startsWith("PR")) {
      value = _coinVals[index-1];
    } else if (key.startsWith("CU")) {
      value = _currencies[index-1];
    }
  }
}


void CryptoPlugin::refresh(bool force) {
  if (!force && (_nextRefresh > millis())) return;
  ScreenMgr::showUpdatingIcon(Theme::Color_UpdatingPlugins);
  for (int i = 0; i < settings.nCoins; i++) {
    CoinbaseClient::getPrice(settings.coinIDs[i], _currencies[i], _coinVals[i]);
  }
  _nextRefresh = millis() + (settings.refreshInterval * settings.riScale);
  ScreenMgr::hideUpdatingIcon();
}
