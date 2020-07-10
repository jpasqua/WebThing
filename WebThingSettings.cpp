/*
 * WebThingSettings.cpp
 *    Handle reading and writing settings information to the file system
 *    in JSON format.
 *
 * NOTES:
 *
 * TO DO:
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
//                                  Local Includes
#include "WebThingSettings.h"
//--------------- End:    Includes ---------------------------------------------


const uint32_t  WebThingSettings::CurrentVersion = 0x0002;
const int8_t    WebThingSettings::NoPinAssigned = -1;
const int8_t    WebThingSettings::UseBuiltinLED = -2;

WebThingSettings::WebThingSettings() {
  version = WebThingSettings::CurrentVersion;
  maxFileSize = 1024;
}

void WebThingSettings::fromJSON(JsonDocument &doc) {
  lat = doc["lat"];
  lng = doc["lng"];
  elevation = doc["elevation"];

  timeZoneDBKey = doc["timeZoneDBKey"].as<String>();
  googleMapsKey = doc["googleMapsKey"].as<String>();

  useLowPowerMode = doc["useLowPowerMode"];
  hasVoltageSensing = doc["hasVoltageSensing"];
  voltageCalibFactor = doc["voltageCalibFactor"];
  processingInterval = doc["processingInterval"];
  sleepOverridePin = doc["sleepOverridePin"];
  displayPowerOptions = doc["displayPowerOptions"];

  hostname = doc["hostname"].as<String>();
  webServerPort = doc["webServerPort"];
  useBasicAuth = doc["useBasicAuth"];
  webUsername = doc["webUsername"].as<String>();
  webPassword = doc["webPassword"].as<String>();

  themeColor = doc["themeColor"].as<String>();

  logLevel = doc["logLevel"];
  indicatorLEDPin = doc["indicatorLEDPin"];
  indicatorLEDInverted = doc["indicatorLEDInverted"];

  logSettings();
}

void WebThingSettings::toJSON(JsonDocument &doc) {
  doc["lat"] = lat;
  doc["lng"] = lng;
  doc["elevation"] = elevation;

  doc["googleMapsKey"] = googleMapsKey;
  doc["timeZoneDBKey"] = timeZoneDBKey;

  doc["hostname"] = hostname;
  doc["webServerPort"] = webServerPort;
  doc["useBasicAuth"] = useBasicAuth;
  doc["webUsername"] = webUsername;
  doc["webPassword"] = webPassword;
  doc["themeColor"] = themeColor;

  doc["voltageCalibFactor"] = voltageCalibFactor;
  doc["useLowPowerMode"] = useLowPowerMode;
  doc["hasVoltageSensing"] = hasVoltageSensing;
  doc["processingInterval"] = processingInterval;
  doc["sleepOverridePin"] = sleepOverridePin;
  doc["displayPowerOptions"] = displayPowerOptions;

  doc["logLevel"] = logLevel;
  doc["indicatorLEDPin"] = indicatorLEDPin;
  doc["indicatorLEDInverted"] = indicatorLEDInverted;
}

void WebThingSettings::logSettings() {
  Log.verbose(F("Location Settings"));
  Log.verbose(F("  lat = %F"), lat);
  Log.verbose(F("  lng = %F"), lng);
  Log.verbose(F("  elevation = %d"), elevation);
  Log.verbose(F("Power Settings"));
  Log.verbose(F("  useLowPowerMode = %T"), useLowPowerMode);
  Log.verbose(F("  hasVoltageSensing = %T"), hasVoltageSensing);
  Log.verbose(F("  processingInterval = %d"), processingInterval);
  Log.verbose(F("  voltageCalibFactor = %F"), voltageCalibFactor);
  Log.verbose(F("  sleepOverridePin = %d"), sleepOverridePin);
  Log.verbose(F("  displayPowerOptions = %T"), displayPowerOptions);
  Log.verbose(F("API Keys"));
  Log.verbose(F("  googleMapsKey = %s"), googleMapsKey.c_str());
  Log.verbose(F("  timeZoneDBKey = %s"), timeZoneDBKey.c_str());
  Log.verbose(F("Web Server Settings"));
  Log.verbose(F("  hostname = %s"), hostname.c_str());
  Log.verbose(F("  webServerPort = %d"), webServerPort);
  Log.verbose(F("  useBasicAuth = %T"), useBasicAuth);
  Log.verbose(F("  webUsername = %s"), webUsername.c_str());
  Log.verbose(F("  webPassword = %s"), webPassword.c_str());
  Log.verbose(F("  themeColor = %s"), themeColor.c_str());
  Log.verbose(F("Indicator LED"));
  Log.verbose(F("  indicatorLEDPin = %d"), indicatorLEDPin);
  Log.verbose(F("  indicatorLEDInverted = %T"), indicatorLEDInverted);
  Log.verbose(F("Other Settings"));
  Log.verbose(F("  logLevel = %d"), logLevel);
}

