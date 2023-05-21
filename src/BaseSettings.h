/*
 * BaseSettings.h
 *    Base class for Settings objects
 *
 */

#ifndef BaseSettings_h
#define BaseSettings_h

#include <ArduinoJson.h>

class BaseSettings {
public:
  // ----- Constructors and methods -----
  BaseSettings();

  void    init(const String& _filePath);
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
  
  // Must be implemented by subclasses
  virtual void fromJSON(const JsonDocument &doc) = 0;
  virtual void toJSON(JsonDocument &doc) = 0;

  // May be implemented by subclasses
  virtual void logSettings() { };

  // Implemented in terms of functions given above
  void fromJSON(const String& json);
  void toJSON(Stream& s);
  void toJSON(String& s);

protected:
  static const uint32_t InvalidVersion = 0x0000;
  uint16_t maxFileSize;

protected:
  // ----- State
  uint32_t version;
  String   filePath;
};
#endif // BaseSettings_h