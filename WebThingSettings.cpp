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
  Log.verbose("Location Settings");
  Log.verbose("  lat = %F", lat);
  Log.verbose("  lng = %F", lng);
  Log.verbose("  elevation = %d", elevation);
  Log.verbose("Power Settings");
  Log.verbose("  useLowPowerMode = %T", useLowPowerMode);
  Log.verbose("  hasVoltageSensing = %T", hasVoltageSensing);
  Log.verbose("  processingInterval = %d", processingInterval);
  Log.verbose("  voltageCalibFactor = %F", voltageCalibFactor);
  Log.verbose("  sleepOverridePin = %d", sleepOverridePin);
  Log.verbose("  displayPowerOptions = %T", displayPowerOptions);
  Log.verbose("API Keys");
  Log.verbose("  googleMapsKey = %s", googleMapsKey.c_str());
  Log.verbose("  timeZoneDBKey = %s", timeZoneDBKey.c_str());
  Log.verbose("Web Server Settings");
  Log.verbose("  hostname = %s", hostname.c_str());
  Log.verbose("  webServerPort = %d", webServerPort);
  Log.verbose("  useBasicAuth = %T", useBasicAuth);
  Log.verbose("  webUsername = %s", webUsername.c_str());
  Log.verbose("  webPassword = %s", webPassword.c_str());
  Log.verbose("  themeColor = %s", themeColor.c_str());
  Log.verbose("Indicator LED");
  Log.verbose("  indicatorLEDPin = %d", indicatorLEDPin);
  Log.verbose("  indicatorLEDInverted = %T", indicatorLEDInverted);
  Log.verbose("Other Settings");
  Log.verbose("  logLevel = %d", logLevel);
}

