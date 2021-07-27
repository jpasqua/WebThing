/*
 * ESP_FS
 *    Implementation of the ESP_FS interface
 *
 */

#include "ESP_FS.h"

#if defined(ESP32)
  #include <SPIFFS.h>

  namespace ESP_FS {
    FS* getFS() { return &SPIFFS; }
    bool begin() { return SPIFFS.begin(); }
    bool format() { return SPIFFS.format(); }

    File open(const char* path, const char* mode) { return SPIFFS.open(path, mode); }
    File open(const String& path, const char* mode) { return SPIFFS.open(path, mode); }

    bool exists(const char* path) { return SPIFFS.exists(path); }
    bool exists(const String& path) { return SPIFFS.exists(path); }

    static File enumRoot;

    bool beginFileList(String& path) {
      enumRoot = SPIFFS.open(path);
      return (enumRoot.isDirectory());
    }

    bool getNextFileName(String& name) {
      File f;
      do { f = enumRoot.openNextFile(); } while (f && f.isDirectory());
      if (!f) return false;
      name = f.name();
      return true;
    }

    bool remove(const String& path) {
      return SPIFFS.remove(path);
    }
  };

#elif defined(ESP8266)
  namespace ESP_FS {
    // SPIFFS is being deprecated on ESP8266 which causes warnings at compile time.
    // I have a task to move off of SPIFFS to LittleFS, but in the mean time, I
    // don't want to keep seeing the warnings so I wrapped the SPIFFS calls with
    // pragma's to silence the warnings

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
      FS* getFS() { return &SPIFFS; }
      bool begin() { return SPIFFS.begin(); }
      bool format() { return SPIFFS.format(); }
  
      File open(const char* path, const char* mode) { return SPIFFS.open(path, mode); }
      File open(const String& path, const char* mode) { return SPIFFS.open(path, mode); }

      bool exists(const char* path) { return SPIFFS.exists(path); }
      bool exists(const String& path) { return SPIFFS.exists(path); }

      Dir openDir(const char* path) { return SPIFFS.openDir(path); }
      Dir openDir(const String& path) { return SPIFFS.openDir(path); }

      bool remove(const String& path) {
        return SPIFFS.remove(path);
      }
    #pragma GCC diagnostic pop

    static Dir enumRoot;

    bool beginFileList(String& path) {
      enumRoot = ESP_FS::openDir(path);
      return true;  // Always returns a directory object, even if it is empty
    }
    
    bool getNextFileName(String &name) {
      if (!enumRoot.next()) return false;
      name = enumRoot.fileName();
      return true;
    }
  };

#endif  
