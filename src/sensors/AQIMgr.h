/*
 * AQIMgr
 *    Read data realited to the Air Quality Index from an underlying device and
 *    store historical information and averages.
 *
 */

#ifndef AQIMgr_h
#define AQIMgr_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <TimeLib.h>
#include <Indicators.h>
#include <MovingAverage.h>
#include <Indicators.h>
#include <Serializable.h>
#include <HistoryBuffer.h>
#include <HistoryBuffers.h>
//                                  WebThing Includes
//                                  Local Includes
#include "AQIReadings.h"
#include "PMS5003.h"
//--------------- End:    Includes ---------------------------------------------




class AQIMgr {
public:
  // ----- Types
  enum HistoryRange {Range_1Hour, Range_1Day, Range_1Week};

  // ----- Constants -----

  // ----- Constructors & Destructor -----
  AQIMgr();

  // ----- Methods -----
  // Creates a communication path to the underlying sensor
  // @param streamToSensor  Either a HW or SW serial connection to the sensor
  // @param indicator       An object which can present feedback to the user 
  bool init(Stream* streamToSensor, Indicator* indicator);

  // This function must be called periodically to give AQIMgr (and the
  // underlying sensor) a chance to do some work. It is typically called
  // each time through the Arduino loop() function. It will return very quickly
  // if there is nothing to do - which is most of the time.
  void loop();


  // --- Getting and using sensor readings ---
  // Returns the timestamp of the last reading
  inline uint32_t lastReadingTime() { return data.timestamp; }

  // Returns the last set of values read from the sensor. This may be the same
  // as the answer as given on a previous call if the sesnor has not read new
  // values  in the interval.
  const AQIReadings& getLastReadings() { return data; }

  // Does the calculation to provide an AQI value from a sensor reading
  // @param  reading   An env.pm25 value
  // @return The AQI value for this reading
  uint16_t derivedAQI(uint16_t reading);

  // There are omewhat standard colors used along with the various air quality
  // lelvels (good, moderate, etc.). Return a color for the given quality level.
  // @param  quality   The env.pm25 value for which we want the associated color
  // @return A 24bit RGB color specification
  uint32_t colorForQuality(uint16_t quality);

  size_t sizeOfRange(HistoryRange r) const;
  uint16_t aqiFromHistory(HistoryRange r, size_t index) const;
  void getTimeRange(HistoryRange r, time_t& start, time_t&end) const;


  // --- Getting data in JSON form ---
  // Output the accumulated history of readings
  // @param  range  Which range of data we're interested in.
  // @param  s      The stream where the JSON data should be written
  void emitHistoryAsJson(HistoryRange r, Stream& s);

  // Output the accumulated history of readings for each range
  // @param  s      The stream where the JSON data should be written
  void emitHistoryAsJson(Stream& s);

  // Generate a JSON description corresponding to the supplied AQI value. An
  // example of the JSON that might be generated is:
  // {
  //   "timestamp": 1604162511,
  //   "aqi": 51,
  //   "shortDesc": "Moderate",
  //   "longDesc": "Air quality is acceptable. <remainder omitted for brevity>",
  //   "color": 16776960
  // }
  // @param  quality    The AQI for which we want the description
  // @param  timestamp  When the AQI reading was taken
  // @param  result     A String to which the description will be appended
  void aqiAsJSON(uint16_t quality, time_t timestamp, String& result);


  // There are 6 quality brackets: Good, Moderate, Unhealthy, Very Unhealthy, Hazardous
  // aqiBracket() returns the (zero-based) index of the bracket associated with the
  // specified quality.
  uint8_t aqiBracket(uint16_t quality);

  
  // ----- Data Members -----
  // --- Moving averages of the env.pm25 for a various time ranges ---
  MovingAverage pm25env_10min;
  MovingAverage pm25env_30min;
  MovingAverage pm25env_1hr;
  MovingAverage pm25env_6hr;


private:
  // ----- Types -----
  class SavedReadings : public Serializable {
  public:
    SavedReadings() = default;
    SavedReadings(ParticleReadings& pr, uint16_t quality, time_t ts)
        : Serializable(ts), env(pr), aqi(quality) { }

    virtual void internalize(const JsonObjectConst &obj);
    virtual void externalize(Stream& writeStream) const;

    ParticleReadings env;
    uint16_t aqi;
  };

  enum State {awake, retrying, waking, asleep};


  // ----- Constants -----
  static const uint32_t ColorForState[];

  // ----- Constructors & Destructor -----
  static constexpr size_t MaxHistoryFileSize = 9000;
  
  // ----- Methods -----
  void enterState(State);
  void takeNoteOfNewData(AQIReadings& newSample);

  // --- Utility Functions ---
  void logData(AQIReadings& data);
  void logAvgs();


  // ----- Data Members -----
  PMS5003* aqi;
  State state;
  uint32_t enteredStateAt;
  Indicator* _indicator;
  AQIReadings data;

  HistoryBuffer<SavedReadings, 12> readings_5min; // The last hour's worth of readings at 5 minute intervals
  HistoryBuffer<SavedReadings, 24> readings_1hr;  // The last day's worth of readings at 1 hour intervals
  HistoryBuffer<SavedReadings, 28> readings_6hr;  // The last week's worth of readings at 6 hour intervals
  HistoryBuffers<3> buffers;
  bool historyBufferIsDirty = false;
};

#endif  // AQIMgr_h