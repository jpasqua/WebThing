
/* WeatherMgr
 *     Provide a uniform interface to one or more weather sensing devices
 *     such as a BME280, a DHT22, and/or a DS18B20
 *
 * NOTES:
 * o At the moment we only manage a BME280. The itnerface will need to be generalized
 *   to support other devices.
 * o There may be times when multiple devices can take the same reading. For example,
 *   you may have a BME280 and a DS18B20, and use the Humidity and pressure values
 *   from the BME280, but prefer the temp value from the DS18B20.
 *
 */

#ifndef WeatherMgr_h
#define WeatherMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <BPABasics.h>
#include <HistoryBuffers.h>
//                                  WebThing Includes
#include <WebThing.h>
//                                  Local Includes
#include "BMESensor.h"
#include "WeatherReadings.h"
//--------------- End:    Includes ---------------------------------------------


class WeatherMgr {
public:
  // ----- Types
  enum HistoryRange {Range_1Hour, Range_1Day, Range_1Week};

  // ----- Constants -----
  static constexpr uint32_t HistoryWriteInterval = minutesToTime_t(10) * 1000L;
  static constexpr const char* HistoryFilePath = "/wthrhist.json";

  // ----- Constructors & Destructor -----
  WeatherMgr() = default;

  // ----- Methods -----
  void init(
      float tempCorrection, float humidityCorrection, int32_t elevation,
      std::function<void(bool)> busyCallback = nullptr)
  {
    _tempCorrection = tempCorrection;
    _humiCorrection = humidityCorrection;
    _elevation = elevation;
    _busyCallback = busyCallback;

    buffers.setBuffer(0, {&readings_5min, "hour", minutesToTime_t(5)});
    buffers.setBuffer(1, {&readings_1hr, "day", hoursToTime_t(1)});
    buffers.setBuffer(2, {&readings_6hr, "week", hoursToTime_t(6)});
    buffers.load(HistoryFilePath);
  }

  void addSensor(WeatherSensor* sensor) {
    _sensors.push_back(sensor);
  }

  // --- Getting and using sensor readings ---
  // Returns the timestamp of the last reading
  inline uint32_t lastReadingTime() { return lastReadings.timestamp; }

  // Returns the last set of values read from the sensor. This may be the same
  // as the answer as given on a previous call if the sensor has not read new
  // values  in the interval.
  const WeatherReadings& getLastReadings() { return lastReadings; }


  // --- Getting data in JSON form ---
  // Output the accumulated history of readings
  // @param  range  Which range of data we're interested in.
  // @param  s      The stream where the JSON data should be written
  void emitHistoryAsJson(HistoryRange r, Stream& s) {
    switch (r) {
      case Range_1Hour: readings_5min.store(s); break;
      case Range_1Day: readings_1hr.store(s); break;
      case Range_1Week: readings_6hr.store(s); break;
    }
  }

  void emitHistoryAsJson(Stream& s) {
    buffers.store(s);
  }

  void takeReadings(bool force = false) {
    // We do three things here:
    // 1. If it is time, take a new set of readings from the sensor
    // 2. Push the reading into the appropriate set of history buffers (if any)
    // 3. If it is time, write the history out to the file system

    // 1. If it is time, take a new set of readings from the sensors
    static uint32_t nextReading = 0;
    uint32_t curMillis = millis();
    if (!(force || curMillis > nextReading)) return;

    if (_busyCallback) _busyCallback(true);
    for (WeatherSensor* sensor : _sensors) {
      sensor->takeReadings(lastReadings);
    }
    WeatherSensor::applyCorrections(lastReadings, _tempCorrection, _humiCorrection);
    WeatherSensor::calculateDerivedValues(lastReadings, _elevation);
    lastReadings.timestamp = curMillis;
    nextReading = curMillis + _readingInterval;

    // 2. Push the reading into the appropriate set of history buffers (if any)
    time_t gmtTimestamp = Basics::wallClockFromMillis(lastReadings.timestamp) - WebThing::getGMTOffset();
    SavedReadings readings(lastReadings.temp, gmtTimestamp);
    historyBufferIsDirty |= buffers.conditionalPushAll(readings);

    // 3. If it is time, write the history out to the file system
    static uint32_t nextWrite = 0;
    if (historyBufferIsDirty && (curMillis >= nextWrite)) {
      buffers.store(HistoryFilePath);
      nextWrite = curMillis + HistoryWriteInterval;
      historyBufferIsDirty = false;
    }
    _busyCallback(false);
  }

  void setAttributes(float tempCorrection, float humidityCorrection, int32_t elevation) {
    _tempCorrection = tempCorrection;
    _humiCorrection = humidityCorrection;
    _elevation = elevation;
  }

  size_t sizeOfRange(HistoryRange r) const { return buffers.sizeOfBuffer(r); }

  uint16_t tempFromHistory(HistoryRange r, size_t index) const {
    return static_cast<const SavedReadings&>(buffers.peekAt(r, index)).temp;
  }

  void getTimeRange(HistoryRange r, time_t& start, time_t&end) const {
    buffers.getTimeRange(r, start, end);
  }

private:
  // ----- Types -----
  class SavedReadings : public Serializable {
  public:
    SavedReadings() = default;
    SavedReadings(float t, uint32_t ts) : Serializable(ts), temp(t) { }

    virtual void externalize(Stream& writeStream) const {
      StaticJsonDocument<32> doc; // Size provided by https://arduinojson.org/v6/assistant
      doc["ts"] = timestamp;
      doc["t"] = ((float)((int)(temp * 10))) / 10;  // Limit to 1 decimal place
      serializeJson(doc, writeStream);
    }

    void internalize(const JsonObjectConst &obj) {
      timestamp = obj["ts"];
      temp = obj["t"];
    }

    float temp;
  };


  // ----- Data Members -----
  std::vector<WeatherSensor*> _sensors;
  float _tempCorrection;
  float _humiCorrection;
  int32_t _elevation;
  std::function<void(bool)> _busyCallback;
  WeatherReadings lastReadings;

  uint32_t _readingInterval = 60 * 1000L;               // How often do we take a reading

  HistoryBuffer<SavedReadings, 12> readings_5min; // The last hour's worth of readings at 5 minute intervals
  HistoryBuffer<SavedReadings, 24> readings_1hr;  // The last day's worth of readings at 1 hour intervals
  HistoryBuffer<SavedReadings, 28> readings_6hr;  // The last week's worth of readings at 6 hour intervals
  HistoryBuffers<3> buffers;
  bool historyBufferIsDirty = false;
};
#endif // WeatherMgr_h