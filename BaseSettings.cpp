/*
 * BaseSettings.cpp
 *    Handle reading and writing settings information to the file system
 *    in JSON format.
 *
 * NOTES:
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
//                                  Personal Libraries
//                                  App Libraries and Includes
#include "BaseSettings.h"
//--------------- End:    Includes ---------------------------------------------


const uint32_t BaseSettings::InvalidVersion = 0x0000;

BaseSettings::BaseSettings() { }

void BaseSettings::init(String _filePath) {
  filePath = _filePath;
  Log.verbose("settings file path = %s", filePath.c_str());
}

bool BaseSettings::clear() {
  return SPIFFS.remove(filePath);
}

bool BaseSettings::read() {
  File settingsFile = SPIFFS.open(filePath, "r");
  if (!settingsFile) {
    Log.notice("No settings file exists. Creating one with default values.");
    return write();
  }

  size_t size = settingsFile.size();
  if (size > maxFileSize) {
    Log.error("Settings file size is too large (%d), using default values", size);
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  settingsFile.readBytes(buf.get(), size);
  settingsFile.close();

  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Log.warning("Failed to parse settings file, using default values: %s", error.c_str());
    return false;
  }

  uint32_t versionFound = doc["version"];
  if (versionFound != version) {
    Log.warning("Settings version mismatch. Expected %d, found %d", version, versionFound);
    Log.warning("Writing default setting values");
    write();
    return true;
  }

  fromJSON(doc);

  Log.trace("Settings successfully read");
  return true;
}

bool BaseSettings::write() {
  DynamicJsonDocument doc(maxFileSize);

  doc["version"] = version;
  toJSON(doc);

  File settingsFile = SPIFFS.open(filePath, "w");
  if (!settingsFile) {
    Log.error("Failed to open settings file for writing: %s", filePath.c_str());
    return false;
  }

  // Log.notice("JSON doc to be saved to settings file:");
  // serializeJsonPretty(doc, Serial);
  // Log.notice("");

  int sizeWritten = serializeJson(doc, settingsFile);
  settingsFile.close();
  Log.trace("Wrote %d bytes to settings file (%s)", sizeWritten, filePath.c_str());
  return true;
}
