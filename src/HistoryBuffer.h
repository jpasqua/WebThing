/*
 * HistoryBuffer
 *     Keeps track of an ordered series of data and provides functionality
 *     to externalize as JSON and internalize from JSON
 *
 * NOTES:
 * o The type you specify as a template parameter to HistoryBuffer must
 *   implement internalize() and externalize() methods.
 * o When using the HistoryBuffer template class, you specify (statically) the
 *   max number of elements and their type. These elements are allocated as
 *   part of the HistoryBuffer object (not separately in the heap).
 * o You can add elements to the history up to the size limit, and once
 *   that limit is reached, adding a new hisotry element will cause the
 *   oldest to be removed.
 * o Timestamps: If your data includes timestamps, they should be in 
 *   "wall clock" time rather than millis() since that gets reset on
 *   every boot. It is usually best to use times in GMT
 *
 */

#ifndef HistoryBuffer_h
#define HistoryBuffer_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <CircularBuffer.h>
//                                  WebThing Includes
#include <ESP_FS.h>
//                                  Local Includes
#include "Serializable.h"
//--------------- End:    Includes ---------------------------------------------


template<typename ItemType, int Size>
class HistoryBuffer {
private:
	static constexpr size_t MaxHistoryFileSize = 8192;
  static_assert(std::is_base_of<Serializable, ItemType>::value, "HistoryBuffer Item must derive from Serializable");
  

public:
  bool store(Stream& writeStream)
  {
    writePreamble(writeStream);

    // Write the items
    for (size_t i = 0; i < _historyItems.size(); i++) {
      if (i) writeStream.print(',');
      const ItemType& item = _historyItems.peekAt(i);
      item.externalize(writeStream);
    }

    writePostscript(writeStream);

    Log.verbose("\n\nHistoryBuffer written to stream");
    return true;
  }

  bool store(const String& historyFilePath) {
    File historyFile = ESP_FS::open(historyFilePath, "w");

    if (!historyFile) {
      Log.error(F("Failed to open history file for writing: %s"), historyFilePath.c_str());
      return false;
    }

    bool success = store(historyFile);
    historyFile.close();

    if (success) Log.verbose("HistoryBuffer written written to file");
    else Log.warning("Error saving history to %s", historyFilePath.c_str());
    
    return success;
  }

  bool load(Stream& readStream) {
    _historyItems.clear();  // Start from scratch...

    DynamicJsonDocument doc(MaxHistoryFileSize);
    auto error = deserializeJson(doc, readStream);
    if (error) {
      Log.warning(F("HistoryBuffer::load: Parse error: %s"), error.c_str());
      return false;
    }

    JsonArrayConst historyArray = doc[F("history")];

    ItemType item;
    for (JsonObjectConst jsonItem : historyArray) {
      item.internalize(jsonItem);
      _historyItems.push(item);
    }

    return true;
  }

  bool load(const JsonObjectConst& historyElement) {
    JsonArrayConst historyArray = historyElement[F("history")];

    ItemType item;
    for (JsonObjectConst jsonItem : historyArray) {
      item.internalize(jsonItem);
      _historyItems.push(item);
    }

    return true;
  }

  bool load(const String& historyFilePath) {
    size_t size = 0;
    File historyFile = ESP_FS::open(historyFilePath, "r");

    if (!historyFile) {
      Log.error(F("Failed to open history file for read: %s"), historyFilePath.c_str());
      return false;
    } else {
      size = historyFile.size();
      if (size > MaxHistoryFileSize) {
        Log.warning(F("HistoryBuffer file is too big: %d"), size);
        historyFile.close();
        return false;
      }    
    }

    bool success = load(historyFile);
    historyFile.close();

    if (success) Log.verbose("HistoryBuffer data loaded");
    else Log.warning("Error loading history from %s", historyFilePath.c_str());
    
    return success;
  }

  inline bool push(const ItemType& item) {
    return _historyItems.push(std::move(item));
  }

  inline const ItemType& peekAt(size_t index) const {
    return _historyItems.peekAt(index);
  }

  inline void clear() { _historyItems.clear(); }

  constexpr size_t size() const { return _historyItems.size(); } 

private:
	CircularBuffer<ItemType, Size> _historyItems;

  void writePreamble(Stream& writeStream) {
    writeStream.print("{ \"history\": [");
  }

  void writePostscript(Stream& writeStream) {
    writeStream.println("]}");
    writeStream.flush();
  }
};

#endif  // HistoryBuffer_h