//
// TarWriter
//   Enables the creation of tar files that are streamed out to a given
//   supplied output stream
//
// This code was inspired by from: https://github.com/bebuch/tar
// I don't believe any of that code remains, but I've included the
// original copyright and credits at the end of the file.
//

#ifndef ESPTarWriter_h
#define ESPTarWriter_h

#include <Stream.h>
#include <ESP_FS.h>
#include <ArduinoLog.h>
#include <TimeLib.h>

#pragma pack(push, 1)  // Ensure that the structure is packed tightly without any padding

struct TarHeader {
  char name[100];          // File name
  char mode[8];            // File mode (permissions)
  char uid[8];             // User ID of the owner
  char gid[8];             // Group ID of the owner
  char size[12];           // File size in bytes (octal representation)
  char mtime[12];          // Last modification time (octal representation)
  char checksum[8];        // Checksum for header block
  char typeflag;           // Type of file entry
  char linkname[100];      // Target name of symbolic link
  char magic[6];           // Magic value to identify tar format ("ustar" or "ustar\0")
  char version[2];         // Tar format version
  char uname[32];          // User name of owner
  char gname[32];          // Group name of owner
  char devmajor[8];        // Major device number (if applicable)
  char devminor[8];        // Minor device number (if applicable)
  char prefix[155];        // Prefix for file name (if too long for name field)
  char pad[12];            // Fill out to 512 bytes
};

#pragma pack(pop)  // Restore default struct padding

constexpr char magic[] = {'u', 's', 't', 'a', 'r'};
constexpr char mode[] = {'0', '0', '0', '6', '4', '4'};

class TarWriter {
private:
  void fillHeader(TarHeader& header, String& name, size_t size) {
    memset(&header, 0, sizeof(header));
    memcpy(&header.name[0], name.c_str(), name.length());
    memcpy(&header.mode[0], &mode[0], sizeof(mode));
    // header.uid can remain zero-filled
    // header.gid can remain zero-filled
    setOctal(&header.size[0], size, sizeof(header.size));
    setOctal(&header.mtime[0], now(), sizeof(header.mtime));
    header.typeflag = '0';
    // header.linkname can remain zero-filled
    memcpy(&header.magic[0], &magic[0], sizeof(magic));
    header.version[0] = header.version[1] = '0';
    // header.uname can remain zero-filled
    // header.gname can remain zero-filled
    // header.devmajor can remain zero-filled
    // header.devminor can remain zero-filled
    // header.prefix can remain zero-filled
    // header.pad can remain zero-filled

    // Now compute and store the checksum
    setOctal(&header.checksum[0], computeChecksum(header), sizeof(header.checksum)-1);
    header.checksum[sizeof(header.checksum)-1] = ' ';
  }

  void addFile(String& filename, std::function< void(void) > const& writeFileContent, size_t size) {
    TarHeader header;
    fillHeader(header, filename, size);

    _out.write(&header.name[0], sizeof(header));

    writeFileContent();

    uint16_t padding = (512 - (size % 512)) % 512;
    for (uint16_t i = 0; i < padding; i++) _out.write(0);
  }

  inline unsigned long computeChecksum(TarHeader& header){
    char* buffer = &header.name[0];

    // Pretend the checksum field was full of spaces
    unsigned sum = sizeof(header.checksum) * ((unsigned)' ');
    for (uint16_t i = 0; i < sizeof(header); i++) sum += buffer[i];
    return sum;
  }

  void setOctal(char *buffer, unsigned long val, size_t size) {
    size--;  // Leave room for a null
    for (int i = size-1; i >= 0; i--) {
      unsigned long digit = val % 8;
      buffer[i] = digit + '0';
      val /= 8;
    }
  }

  void streamFileContent(Stream& inputStream) {
    const size_t bufferSize = 512; // Adjust the buffer size as needed
    char buffer[bufferSize];

    size_t bytesRead;
    while ((bytesRead = inputStream.readBytes(buffer, bufferSize))) {
      size_t bytesWritten = _out.write(buffer, bytesRead);
      if (bytesRead != bytesWritten) {
        Log.warning("streamFileContent: not all bytes of file were written");
      }
    }
  }

  Print& _out;

public:
  TarWriter(Print& outputStream): _out(outputStream) {}

  ~TarWriter() { _out.flush(); }

  void streamTarFile(const String& directoryPath) {
    Log.verbose("streamTarFile: Creating tar stream of %s", directoryPath.c_str());

    ESP_FS::DirEnumerator* de = ESP_FS::newEnumerator();
    if (!de->begin("/")) {
      Log.warning("streamTarFile: Unable to enumerate /");
      delete de;
      return;
    }

    String filePath;
    Log.verbose("About to begin enumeration");
    while (de->next(filePath)) {
      Log.verbose("Processing %s", filePath.c_str());
      File inputFile = ESP_FS::open(filePath, "r");
      if (!inputFile) {
        Log.warning("Unable to open: %s, ignoring it", filePath.c_str());
        continue;
      }

      size_t size = inputFile.size();
      Log.trace("streamTarFile: Adding %s", filePath.c_str());
      auto writeFileContent = [this, &inputFile](void) { streamFileContent(inputFile); };
      addFile(filePath, writeFileContent, size);
      inputFile.close();
    }
    delete de;
    _out.flush();
  }


};


#endif

// Inspiration and ideas from:
//
//-----------------------------------------------------------------------------
// Copyright (c) 2013-2018 Benjamin Buch
//
// https://github.com/bebuch/tar
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
