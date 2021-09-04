// TimeDB.h
//    A class that gets the time from timezonedb.com and sets TimeLib based on tha
//    data.
//


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <JSONService.h>
//                                  Local Header Files
#include "TimeDB.h"
//--------------- End:    Includes ---------------------------------------------


/*------------------------------------------------------------------------------
 *
 * Constants
 *
 *----------------------------------------------------------------------------*/

static constexpr uint32_t ClockSyncInterval = 60 * 60 * 1000L;   // Once an hour
static constexpr uint32_t ThrottleInterval = 60 * 1000L;         // Once an minute


/*------------------------------------------------------------------------------
 *
 * Constructors and initialization
 *
 *----------------------------------------------------------------------------*/

void TimeDB::init(const String& key, float lat, float lon) {
  _apiKey = key;
  _lat = String(lat, 6);
  _lon = String(lon, 6);
  _valid = !(_apiKey.isEmpty() || _lat.isEmpty() || _lon.isEmpty());

  if (!_valid) {
    Log.verbose(F("TimeDB::getTime: API Key or lat/lng have not been supplied"));
  }

  _timeOfLastTimeRefresh = 0;      // OK to refresh sooner than usual

  if (_service == NULL) {
    ServiceDetails details;
    details.server = "api.timezonedb.com";
    details.port = 80;
    _service = new JSONService(details);
  }
}

/*------------------------------------------------------------------------------
 *
 * Public Member Functions
 *
 *----------------------------------------------------------------------------*/



time_t TimeDB::getTime() {
  if (!_valid || throttle()) return FailedRead;

  time_t theTime;
  for (int i = 0; i < 10; i++) {
   if ((theTime = tryGettingTime()) != FailedRead) return theTime;
   delay(2000);
  }
  Log.warning(F("Unable to connect to / read from timezonedb"));
  return FailedRead;
}

time_t TimeDB::syncTime(bool force) {
  if (force || updateNeeded()) {
    time_t curTime = getTime();
    _timeOfLastTimeRefresh = millis();   // Failures can take a long time, so read millis() again
    if (curTime != FailedRead) { setTime(curTime); }
  }
  return now();
}


/*------------------------------------------------------------------------------
 *
 * Private Member Functions
 *
 *----------------------------------------------------------------------------*/

time_t TimeDB::tryGettingTime() {
  static const String TimeEndpoint =
      "/v2.1/get-time-zone?key=" + _apiKey +
      "&format=json&by=position&lat=" + _lat + "&lng=" + _lon;
  constexpr uint32_t TimeEndpointJSONSize = JSON_OBJECT_SIZE(13) + 512;

  DynamicJsonDocument *root = _service->issueGET(TimeEndpoint, TimeEndpointJSONSize);
  if (!root) {
    Log.warning(F("issueGET failed for timezonedb"));
    return FailedRead;
  }
  //serializeJsonPretty(*root, Serial); Serial.println();

  _gmtOffset = (*root)["gmtOffset"];
  time_t timestamp = (*root)["timestamp"];
  delete root;

  return (timestamp ? timestamp : FailedRead);
}

bool TimeDB::throttle() {
  static uint32_t timeOfLastAPICall = UINT32_MAX;
  uint32_t curMillis = millis();
  if ((timeOfLastAPICall != UINT32_MAX) && ((curMillis - timeOfLastAPICall) < ThrottleInterval)) {
    Log.verbose(F("Throttling timezonedb gets: last = %d"), timeOfLastAPICall);
    return true;
  }
  timeOfLastAPICall = curMillis;
  return false;
}

bool TimeDB::updateNeeded() {
  if (timeStatus() == timeNotSet) return true;
  if (millis() - _timeOfLastTimeRefresh > ClockSyncInterval) return true;

  // If we haven't updated since 2AM, an update may be needed to account for DST
  int32_t minuteSinceLastRefresh = (millis() - _timeOfLastTimeRefresh)/(1000*60);
  return ((hour() == 2) && (minute() < minuteSinceLastRefresh));
}
