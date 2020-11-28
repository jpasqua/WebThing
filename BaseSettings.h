/*
 * BaseSettings.h
 *    Base class for Settings objects
 *
 */

#ifndef BaseSettings_h
#define BaseSettings_h

#include <ArduinoJson.h>
#if defined(ESP8266)
#elif defined(ESP32)
  #include <FS.h>
  #include <SPIFFS.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif

class BaseSettings {
public:
  // ----- Constructors and methods -----
  BaseSettings();

  void    init(String _filePath);
  bool    clear();
  bool    read();
  bool    write();

  /*
   * Provide a JSON document tht represent the current state of the settings
   * including the version #.
   * NOTE: It is the caller's responsibility to clear/delete the returned
   *       JSON document.
   *
   * return   A pointer to a newly allocated DynamicJsonDocument which
   *          holds the settings as JSON.
   */
  DynamicJsonDocument *asJSON();
  
  virtual void logSettings() { }

protected:
  static const uint32_t InvalidVersion;
  uint16_t maxFileSize;
  virtual void fromJSON(JsonDocument &doc) = 0;
  virtual void toJSON(JsonDocument &doc) = 0;

protected:
  // ----- State
  uint32_t version;
  String   filePath;
};
#endif // BaseSettings_h