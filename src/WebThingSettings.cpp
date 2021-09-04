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


WebThingSettings::WebThingSettings() {
  version = WebThingSettings::CurrentVersion;
  maxFileSize = 1024;
}

void WebThingSettings::fromJSON(const JsonDocument &doc) {
  lat = doc[F("lat")];
  lng = doc[F("lng")];
  elevation = doc[F("elevation")];

  timeZoneDBKey = doc[F("timeZoneDBKey")].as<String>();
  googleMapsKey = doc[F("googleMapsKey")].as<String>();

  useLowPowerMode = doc[F("useLowPowerMode")];
  hasVoltageSensing = doc[F("hasVoltageSensing")];
  voltageCalibFactor = doc[F("voltageCalibFactor")];
  processingInterval = doc[F("processingInterval")];
  sleepOverridePin = doc[F("sleepOverridePin")];
  displayPowerOptions = doc[F("displayPowerOptions")];

  hostname = String(doc[F("hostname")]|"");
  webServerPort = doc[F("webServerPort")];
  useBasicAuth = doc[F("useBasicAuth")];
  webUsername = doc[F("webUsername")].as<String>();
  webPassword = doc[F("webPassword")].as<String>();

  themeColor = doc[F("themeColor")].as<String>();

  logLevel = doc[F("logLevel")];

  logSettings();
}

void WebThingSettings::toJSON(JsonDocument &doc) {
  doc[F("lat")] = lat;
  doc[F("lng")] = lng;
  doc[F("elevation")] = elevation;

  doc[F("googleMapsKey")] = googleMapsKey;
  doc[F("timeZoneDBKey")] = timeZoneDBKey;

  doc[F("hostname")] = hostname;
  doc[F("webServerPort")] = webServerPort;
  doc[F("useBasicAuth")] = useBasicAuth;
  doc[F("webUsername")] = webUsername;
  doc[F("webPassword")] = webPassword;
  doc[F("themeColor")] = themeColor;

  doc[F("voltageCalibFactor")] = voltageCalibFactor;
  doc[F("useLowPowerMode")] = useLowPowerMode;
  doc[F("hasVoltageSensing")] = hasVoltageSensing;
  doc[F("processingInterval")] = processingInterval;
  doc[F("sleepOverridePin")] = sleepOverridePin;
  doc[F("displayPowerOptions")] = displayPowerOptions;

  doc[F("logLevel")] = logLevel;
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
  Log.verbose(F("Other Settings"));
  Log.verbose(F("  logLevel = %d"), logLevel);
}

