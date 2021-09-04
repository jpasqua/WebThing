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

  bool remove(const String& path);

  class DirEnumerator {
  public:
    virtual ~DirEnumerator() {};  // Must provide body for pure virtual destructor!
    virtual bool begin(const String& path) = 0;
    virtual bool next(String& name) = 0;  // NOTE: name is an "out" parameter
  };

  DirEnumerator* newEnumerator();
};

#endif  // ESP_FS_h