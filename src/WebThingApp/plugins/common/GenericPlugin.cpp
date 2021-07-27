/*
 * GenericPlugin
 *    A plugin that gets and displays static or global content
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <ArduinoLog.h>
//                                  Third Party Libraries
//                                  Local Includes
#include "GenericPlugin.h"
//--------------- End:    Includes ---------------------------------------------



/*------------------------------------------------------------------------------
 *
 * GenericSettings Implementation
 *
 *----------------------------------------------------------------------------*/

GenericSettings::GenericSettings() {
  version = 1;
  maxFileSize = 256;

  enabled = true;
  riScale = 1 * 1000L;    // Operate in seconds
  refreshInterval = 60;   // 1 Minute
}

void GenericSettings::fromJSON(JsonDocument& doc) {
  enabled = doc[F("enabled")];
  refreshInterval = doc[F("refreshInterval")];
}

void GenericSettings::fromJSON(String& settings) {
  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, settings);
  if (error) {
    Log.warning(F("GenericSettings::fromJSON, failed to parse new settings: %s"), error.c_str());
    return;
  }
  fromJSON(doc);
}

void GenericSettings::toJSON(JsonDocument& doc) {
  doc[F("enabled")] = enabled;
  doc[F("refreshInterval")] = refreshInterval;
  doc[F("riScale")] = riScale;
}

void GenericSettings::toJSON(String& serialized) {
  DynamicJsonDocument doc(maxFileSize);
  toJSON(doc);
  serializeJson(doc, serialized);
}

void GenericSettings::logSettings() {
  Log.verbose(F("----- GenericSettings"));
  Log.verbose(F("  enabled: %T"), enabled);
  Log.verbose(F("  refreshInterval: %d"), refreshInterval);
  Log.verbose(F("  riScale: %d"), riScale);
}


/*------------------------------------------------------------------------------
 *
 * GenericPlugin Implementation
 *
 *----------------------------------------------------------------------------*/

GenericPlugin::~GenericPlugin() {
  // TO DO: free the settings object
}

void GenericPlugin::getSettings(String& serializedSettings) {
  settings.toJSON(serializedSettings);
}

void GenericPlugin::newSettings(String& serializedSettings) {
  settings.fromJSON(serializedSettings);
  settings.write();
}

uint32_t GenericPlugin::getUIRefreshInterval() { return settings.refreshInterval * settings.riScale; }

bool GenericPlugin::typeSpecificInit() {
  settings.init(_pluginDir + "/settings.json");
  settings.read();
  settings.logSettings();

  _enabled = settings.enabled;

  return true;
}

void GenericPlugin::typeSpecificMapper(const String& key, String& value) {
  (void)key;    // Unused parameter;
  (void)value;  // Unused parameter;
}

void GenericPlugin::refresh(bool force) {
  // Nothing to update - we're relying on global content
  (void)force; // Avoid compiler warning
}
