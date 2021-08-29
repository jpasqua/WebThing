// TimeDB.h
//    A class that gets the time from timezonedb.com and sets TimeLib based on tha
//    data.
//
//

#ifndef TimeDB_h
#define TimeDB_h

#include <JSONService.h>

class TimeDB
{
  public:
    static const time_t FailedRead;
    
    /*
     * Initialize (or re-initialize) the TimeDB object.
     *
     * @param key   A valid TimeZoneDB API Key
     * @param lat   The latitude for which we want the time
     * @param lon   The latitude for which we want the time
     */
    void init(String& key, float lat, float lon);

    /*
     * Make a request to the TimeZoneDB service to get the current time.
     * This function will throttle rapidly repeated requests to protect
     * against clients running amok. This function *does not* sync the time
     * with TimeLib.
     *
     * @return The current time or FailedRead if an error occurred.
     */
    time_t getTime();

    /*
     * Get the current time and sync it with TimeLib. This function may be called
     * frequently. It will only call out to TimeZoneDB if:
     * (a) A minimum period of time has passed since the last call.
     * (b) The force parameter is set to true
     * (c) It is required to deal with potential day light savings time changes
     *
     * @param force A value of true forces an update
     * @return The current time
     */
    time_t syncTime(bool force=false);

    /*
     * Get the gmtOffset for the lat/lon set with init(). This value is not
     * valid until the time has been fetched using getTime or syncTime
     *
     * @return The GMT offset in seconds of the given lat/lon
     */

    inline int32_t getGMTOffset() {return _gmtOffset; }

  private:
    static const uint32_t ClockSyncInterval;
    static const uint32_t ThrottleInterval;
    static const char* servername;

    bool        _valid = false;
    int32_t     _gmtOffset = 0;
    String      _apiKey;
    String      _lat;
    String      _lon;
    uint32_t    _timeOfLastTimeRefresh;
    JSONService *_service = NULL;

    time_t tryGettingTime();
    bool   throttle();
    bool   updateNeeded();
};

#endif  // TimeDB_h
