#ifndef ESP_FS_h
#define ESP_FS_h

#include <FS.h>

namespace ESP_FS {
  FS* getFS();
  bool begin();
  bool format();

  File open(const char* path, const char* mode);
  File open(const String& path, const char* mode);

  bool exists(const char* path);
  bool exists(const String& path);

  bool beginFileList(String& path);
  bool getNextFileName(String& name);

  bool remove(const String& path);
};

#endif  // ESP_FS_h