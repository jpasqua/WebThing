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
#include <ESP_FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
//                                  Personal Libraries
//                                  App Libraries and Includes
#include "BaseSettings.h"
//--------------- End:    Includes ---------------------------------------------


BaseSettings::BaseSettings() { }

void BaseSettings::init(const String& _filePath) {
  filePath = _filePath;
  Log.verbose(F("settings file path = %s"), filePath.c_str());
}

bool BaseSettings::clear() {
  return(ESP_FS::remove(filePath));
}

bool BaseSettings::read() {
  File settingsFile = ESP_FS::open(filePath, "r");
  if (!settingsFile) {
    Log.notice(F("No settings file exists. Creating one with default values."));
    return write();
  }

  size_t size = settingsFile.size();
  if (size > maxFileSize) {
    Log.error(F("Settings file size is too large (%d), using default values"), size);
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  settingsFile.readBytes(buf.get(), size);
  settingsFile.close();

  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Log.warning(F("Failed to parse settings file, using default values: %s"), error.c_str());
    return false;
  }

  uint32_t versionFound = doc["version"];
  if (versionFound != version) {
    Log.warning(F("Settings version mismatch. Expected %d, found %d"), version, versionFound);
    Log.warning(F("Writing default setting values"));
    write();
    return true;
  }

  fromJSON(doc);

  Log.trace(F("Settings successfully read"));
  return true;
}

void BaseSettings::toJSON(Stream& s) {
  DynamicJsonDocument doc(maxFileSize);
  toJSON(doc);
  serializeJson(doc, s);
}

void BaseSettings::toJSON(String& serialString) {
  DynamicJsonDocument doc(maxFileSize);
  toJSON(doc);
  serializeJson(doc, serialString);
}

void BaseSettings::fromJSON(const String& json) {
  DynamicJsonDocument doc(maxFileSize);
  auto error = deserializeJson(doc, json);
  if (error) {
    Log.warning(F("Error (%s) while parsing: %s"), error.c_str(), json.c_str());
  }
  fromJSON(doc);
}

DynamicJsonDocument *BaseSettings::asJSON() {
  DynamicJsonDocument *doc = new DynamicJsonDocument(maxFileSize);

  (*doc)["version"] = version;
  toJSON((*doc));
  return doc;
}

bool BaseSettings::write() {
  DynamicJsonDocument doc(maxFileSize);

  doc["version"] = version;
  toJSON(doc);

  File settingsFile = ESP_FS::open(filePath, "w");
  if (!settingsFile) {
    Log.error(F("Failed to open settings file for writing: %s"), filePath.c_str());
    return false;
  }

  Log.notice(F("JSON doc to be saved to settings file:"));
  serializeJsonPretty(doc, Serial);
  Log.notice(F(""));

  int sizeWritten = serializeJson(doc, settingsFile);
  settingsFile.close();
  Log.trace(F("Wrote %d bytes to settings file (%s)"), sizeWritten, filePath.c_str());
  return true;
}