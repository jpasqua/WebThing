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

  void    init(String _filePath);
  bool    read();
  bool    write();
  bool    clear();
  virtual void logSettings() { }

protected:
  static const uint32_t InvalidVersion;
  uint16_t maxFileSize;
  virtual void fromJSON(JsonDocument &doc) { }
  virtual void toJSON(JsonDocument &doc) { }

protected:
  // ----- State
  uint32_t version;
  String   filePath;
};
#endif // BaseSettings_h