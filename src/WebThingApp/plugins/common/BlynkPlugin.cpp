/*
 * BlynkPlugin
 *    A plugin that gets and displays content from Blynk
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <ArduinoLog.h>
//                                  Third Party Libraries
//                                  WebThing Libraries
#include <WebThingApp/clients/BlynkClient.h>
#include <WebThingApp/gui/Theme.h>
#include <WebThingApp/gui/ScreenMgr.h>
//                                  App Libraries and Includes
#include "BlynkPlugin.h"
//--------------- End:    Includes ---------------------------------------------



/*------------------------------------------------------------------------------
 *
 * BlynkSettings Implementation
 *
 *----------------------------------------------------------------------------*/

BlynkSettings::BlynkSettings() {
  version = 1;
  maxFileSize = 2048;

  enabled = true;
  nPins = 0;
  nBlynkIDs = 0;
  blynkIDs = NULL;
  nicknames = NULL;
  pins = NULL;
  riScale = 60 * 1000L;   // Operate in minutes
  refreshInterval = 10;   // 10 Minutes
}

void BlynkSettings::fromJSON(JsonDocument &doc) {
  JsonArrayConst ids = doc[F("blynkIDs")];
  JsonArrayConst names = doc[F("nicknames")];
  nBlynkIDs = ids.size();  
  blynkIDs = new String[nBlynkIDs];
  nicknames = new String[nBlynkIDs];

  for (int i = 0; i < nBlynkIDs; i++) {
    blynkIDs[i] = ids[i].as<String>();
    nicknames[i] = names[i].as<String>();
  }

  JsonArrayConst pinArray = doc[F("pins")];
  nPins = pinArray.size();  
  pins = new String[nPins];

  for (int i = 0; i < nPins; i++) {
    pins[i] = pinArray[i].as<String>();
  }

  enabled = doc[F("enabled")];
  refreshInterval = doc[F("refreshInterval")];
  riScale = doc[F("riScale")];
}

void BlynkSettings::fromJSON(String& settings) {
  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, settings);
  if (error) {
    Log.warning(F("BlynkSettings::fromJSON, failed to parse new settings: %s"), error.c_str());
    return;
  }
  fromJSON(doc);
}

void BlynkSettings::toJSON(JsonDocument &doc) {
  JsonArray blynkIDArray = doc.createNestedArray(F("blynkIDs"));
  JsonArray nnArray = doc.createNestedArray(F("nicknames"));
  for (int i = 0; i < nBlynkIDs; i++) {
    blynkIDArray.add(blynkIDs[i]);
    nnArray.add(nicknames[i]);
  }

  JsonArray pinArray = doc.createNestedArray(F("pins"));
  for (int i = 0; i < nPins; i++) {
    pinArray.add(pins[i]);
  }

  doc[F("refreshInterval")] = refreshInterval;
  doc[F("riScale")] = riScale;
  doc[F("enabled")] = enabled;
}

void BlynkSettings::toJSON(String& serialized) {
  DynamicJsonDocument doc(maxFileSize);
  toJSON(doc);
  serializeJson(doc, serialized);
}

void BlynkSettings::logSettings() {
  Log.verbose(F("----- BlynkSettings"));
  for (int i = 0; i < nBlynkIDs; i++) {
    Log.verbose(F("  %s (%s)"), nicknames[i].c_str(), blynkIDs[i].c_str());
  }
  for (int i = 0; i < nPins; i++) {
    Log.verbose(F("  Pin %d: %s"), i, pins[i].c_str());
  }
  Log.verbose(F("  enabled: %T"), enabled);
  Log.verbose(F("  refreshInterval: %d"), refreshInterval);
  Log.verbose(F("  riScale: %d"), riScale);
}


/*------------------------------------------------------------------------------
 *
 * BlynkPlugin Implementation
 *
 *----------------------------------------------------------------------------*/

BlynkPlugin::~BlynkPlugin() {
  // TO DO: free the settings object
}

void BlynkPlugin::getSettings(String& serializedSettings) {
  settings.toJSON(serializedSettings);
}

void BlynkPlugin::newSettings(String& serializedSettings) {
  settings.fromJSON(serializedSettings);
  settings.write();
}

uint32_t BlynkPlugin::getUIRefreshInterval() { return settings.refreshInterval * settings.riScale; }

bool BlynkPlugin::typeSpecificInit() {
  settings.init(_pluginDir + "/settings.json");
  settings.read();
  settings.logSettings();

  _enabled = settings.enabled;
  _pinVals = new String[settings.nPins];


  return true;
}

void BlynkPlugin::typeSpecificMapper(const String& key, String& value) {
  for (int i = 0; i < settings.nPins; i++) {
    if (key == settings.pins[i]) { value = _pinVals[i]; return; }
  }
  if (key.startsWith("NN")) {
    int whichNN = key.substring(2).toInt();
    if (whichNN > 0 && whichNN <= settings.nBlynkIDs) { value = settings.nicknames[whichNN-1]; }
  }
}

void BlynkPlugin::refresh(bool force) {
  if (!force && (_nextRefresh > millis())) return;
  ScreenMgr::showUpdatingIcon(Theme::Color_UpdatingPlugins);
  for (int i = 0; i < settings.nPins; i++) {
    int index = settings.pins[i].indexOf('/');
    int blynkIndex = settings.pins[i].substring(0, index).toInt();
    String rawPin = settings.pins[i].substring(index+1);
    BlynkClient::readPin(settings.blynkIDs[blynkIndex], rawPin, _pinVals[i]);
  }
  _nextRefresh = millis() + (settings.refreshInterval * settings.riScale);
  ScreenMgr::hideUpdatingIcon();
}
