
/* WeatherMgr
 *     Provide a uniform interface to one or more weather sensing devices
 *     such as a BME280, a DHT22, and/or a DS18B20
 *
 */

#ifndef WeatherMgr_h
#define WeatherMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <math.h>
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
  class SavedReadings : public Serializable {
  public:
    SavedReadings(time_t ts) : Serializable(ts), temp(0.0), humidity(0), _availableReadings(0) {}
    SavedReadings() : SavedReadings(0) {}

    void setTemp(float t) {
      temp = t;
      _availableReadings |= WeatherSensor::ReadingType::Temperature;
    }

    void setHumi(uint8_t h) {
      humidity = h;
      _availableReadings |= WeatherSensor::ReadingType::Humidity;
    }

    virtual void externalize(Stream& writeStream) const {
      StaticJsonDocument<96> doc; // Size provided by https://arduinojson.org/v6/assistant
      doc["ts"] = timestamp;
      if (_availableReadings & WeatherSensor::ReadingType::Temperature) {
        doc["t"] = ((float)((int)(temp * 10))) / 10;  // Limit to 1 decimal place
      }
      if (_availableReadings & WeatherSensor::ReadingType::Humidity) {
        doc["h"] = humidity;
      }
      serializeJson(doc, writeStream);
    }

    void internalize(const JsonObjectConst &obj) {
      timestamp = obj["ts"];
      _availableReadings = 0;
      if (obj.containsKey("t")) {
        temp = obj["t"];
        _availableReadings |= WeatherSensor::ReadingType::Temperature;
      }
      if (obj.containsKey("h")) {
        humidity = obj["h"];
        _availableReadings |= WeatherSensor::ReadingType::Humidity;
      }
    }

    float temp;
    uint8_t humidity;

  private:
    uint8_t _availableReadings;
  };

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

    buffers.describe({12, "hour", minutesToTime_t(5)});
    buffers.describe({24, "day", hoursToTime_t(1)});
    buffers.describe({28, "week", hoursToTime_t(6)});
    buffers.load(HistoryFilePath);
  }

  void addSensor(WeatherSensor* sensor) {
    _sensors.push_back(sensor);
    _availableReadings |= sensor->availableReadingTypes();
  }

  uint8_t availableReadingTypes() const { return _availableReadings; }
  bool hasTemp() { return _availableReadings & WeatherSensor::ReadingType::Temperature; }
  bool hasHumi() { return _availableReadings & WeatherSensor::ReadingType::Humidity; }
  bool hasBaro() { return _availableReadings & WeatherSensor::ReadingType::Pressure; }

  // --- Getting and using sensor readings ---
  // Returns the timestamp of the last reading
  inline uint32_t lastReadingTime() { return lastReadings.timestamp; }

  // Returns the last set of values read from the sensor. This may be the same
  // as the answer as given on a previous call if the sensor has not read new
  // values  in the interval.
  const WeatherReadings& getLastReadings() { return lastReadings; }


  // --- Getting data in JSON form ---
  // Output the accumulated history of readings for a particular range
  // @param  range  Which range of data we're interested in.
  // @param  s      The stream where the JSON data should be written
  void emitHistoryAsJson(HistoryRange r, Stream& s) {
    buffers[r].store(s);
  }

  // Output the accumulated history of readings for all ranges
  // @param  range  Which range of data we're interested in.
  // @param  s      The stream where the JSON data should be written
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
    SavedReadings readings(Basics::wallClockFromMillis(lastReadings.timestamp) - WebThing::getGMTOffset());
    if (_availableReadings & WeatherSensor::ReadingType::Temperature) readings.setTemp(lastReadings.temp);
    if (_availableReadings & WeatherSensor::ReadingType::Humidity) readings.setHumi(lastReadings.humidity);
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

  size_t sizeOfRange(HistoryRange r) const { return buffers[r].size(); }

  HistoryBuffers<SavedReadings, 3> buffers;

private:
  // ----- Types -----


  // ----- Data Members -----
  std::vector<WeatherSensor*> _sensors;
  float _tempCorrection;
  float _humiCorrection;
  int32_t _elevation;
  std::function<void(bool)> _busyCallback;
  WeatherReadings lastReadings;

  uint32_t _readingInterval = 60 * 1000L;               // How often do we take a reading
  bool historyBufferIsDirty = false;
  uint8_t _availableReadings = 0;
};
#endif // WeatherMgr_h