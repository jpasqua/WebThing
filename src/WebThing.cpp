/*
 * WebThing:
 *    A generic framework for building "things" with a web configuration
 *    capability. Often used for IoT applications - hence the "thing"
 *    nomenclature. Includes a facility for deep sleep to provide for
 *    low power operation,
 *                    
 * TO DO:
 * o When the user turns on low power mode in the UI, the thing doesn't actually
 *   go into low power mode until it is rebooted. Offer a "ar you sure" screen
 *   then reboot to enter low power mode.
 * 
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Wire.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
//                                  Third Party Libraries
#include <BPABasics.h>
#include <ESP_FS.h>
#include <ArduinoLog.h>
#include <WiFiManager.h>
#include <TimeLib.h>
//                                  Local Includes
#include "WebThing.h"
#include "DataBroker.h"
#include "WebUI.h"
#include "GenericESP.h"
#include "WTButton.h"
#include "clients/TimeDB.h"
//--------------- End:    Includes ---------------------------------------------


namespace WebThing {
  /*------------------------------------------------------------------------------
   *
   * Constants
   *
   *----------------------------------------------------------------------------*/
  constexpr long     BaudRate = 115200;
  static constexpr const char* HostNameBase = "thing-";
  static constexpr const char* SettingsFileName = "/wt/settings.json";

  /*------------------------------------------------------------------------------
   *
   * Public State
   *
   *----------------------------------------------------------------------------*/

  bool apMode;
  WebThingSettings  settings;
  String versionToDisplay = Version;
  WTButtonMgr buttonMgr;

  namespace Internal {
    /*------------------------------------------------------------------------------
     *
     * Internal State
     *
     *----------------------------------------------------------------------------*/
    TimeDB timeDB;
    std::function<void()> deepSleepCallback = NULL;
    std::function<void()> afterSleepMinutesCB = NULL;
    std::function<void(const String&, const String&)> configModeCB = NULL;
    std::function<void()> configChangeCB = NULL;

    /*------------------------------------------------------------------------------
     *
     * Internal Functions
     *
     *----------------------------------------------------------------------------*/

    void prepPins(Basics::Pin SDA, Basics::Pin SCL) {
      if (settings.sleepOverridePin != Basics::UnusedPin) {
        pinMode(settings.sleepOverridePin, INPUT_PULLUP);
      }
      if (SCL != Basics::UnusedPin && SDA != Basics::UnusedPin) {
        Wire.begin(SDA, SCL);
      }
    }

    void flushSerial(Print *p, int) { p->print(CR); Serial.flush(); }

    void configModeCallback (WiFiManager *myWiFiManager) {
      String ip = WiFi.softAPIP().toString();
      String ssid = myWiFiManager->getConfigPortalSSID();
      Log.trace(F("Entered config mode: %s"), ip.c_str());
      Log.notice(F("Please connect to AP: %s to setup Wifi Configuration"), ssid.c_str());
      if (configModeCB) configModeCB(ssid, ip);
    }
      
    void prepLogging() {
      Serial.begin(BaudRate);

      // NOTE: Log level will be adjusted when settings are read, right now it is
      // whatever is in the default settings object
      Log.begin(settings.logLevel, &Serial, false);
      Log.setSuffix(flushSerial);

      delay(500);
      // Separate out from the normal garbage that starts the output
      Serial.println(); Serial.flush(); Serial.println(); Serial.flush(); 
    }

    void prepFileSystem() {
      boolean mounted = ESP_FS::begin();
      if (!mounted) {
        Log.notice(F("FS not formatted. Formatting now. This can take >= 30 seconds."));
        ESP_FS::format();
        ESP_FS::begin();
        Log.trace(F("Completed FS Formatting"));
      }
    }

    void prepNetwork() {
      Protected::mDNSStarted = false;
      if (settings.hostname.isEmpty()) {
        settings.hostname = HostNameBase + String(GenericESP::getChipID(), HEX);
      }

      if (apMode) {
        bool result = WiFi.softAP(settings.hostname.c_str(), settings.webPassword.c_str());
        if (result) {
          Log.trace("Access Point successfully established: %s", settings.hostname.c_str());
        } else {
          Log.error("Unable to eastablish access Point: %s", settings.hostname.c_str());
        }
      } else {
        WiFiManager wifiManager;
        wifiManager.setAPCallback(configModeCallback);
        
        // Don't let the config portal sit waiting forever. Timeout after 5 minutes
        // If your home experienced a power failure and has now recovered, your WebThing
        // will reboot immediately, but the connection to the internet may not yet have
        // been re-established. The WebThing would not be able to connect and then wait
        // forever in the portal. Instead, this will cause the portal to time out so
        // the device can be rebooted and try again.
        wifiManager.setConfigPortalTimeout(5*60); 

        if (!wifiManager.autoConnect(settings.hostname.c_str())) {
          Log.error(F("Autconnect failed!! Restarting..."));
          delay(3000);
          // WiFi.disconnect(true); // This removes saved credentials!!
          GenericESP::reset();
          delay(5000);
        }

        if (!MDNS.begin(settings.hostname.c_str())) {
          Log.warning(F("Unable to start mDNS"));
        } else {
          Log.warning(F("mDNS started"));
          Protected::mDNSStarted = true;
        }
      }
      // print the received signal strength:
      Log.verbose(F("Signal Strength (RSSI): %d%%"), wifiQualityAsPct());
    }

    unsigned char h2int(char c) {
      if (c >= '0' && c <='9') { return((unsigned char)c - '0'); }
      if (c >= 'a' && c <='f') { return((unsigned char)c - 'a' + 10); }
      if (c >= 'A' && c <='F') { return((unsigned char)c - 'A' + 10); }
      return(0);
    }

  } // ----- END: WebThing::Internal namespace

  namespace Protected {
    bool mDNSStarted = false;
    
    void configChanged() {
      Internal::timeDB.init(
          settings.timeZoneDBKey, settings.lat, settings.lng);
      if (Internal::configChangeCB) Internal::configChangeCB();
    }
  } // ----- END: WebThing::Protected namespace


  /*------------------------------------------------------------------------------
   *
   * setup and loop() methods
   *
   *----------------------------------------------------------------------------*/            

  void preSetup(int SDA, int SCL) {
    Internal::prepLogging();
    Log.verbose(F("WebThing:: preSetup()"));
    Internal::prepFileSystem();       // Get the filesystem ready to go
    settings.init(SettingsFileName);  // Path to the settings file
    settings.read();                  // Read settings from the filesystem
    Log.setLevel(settings.logLevel);  // Update based on the settings we just read
    Internal::prepPins(SDA, SCL);             // Set up any pins used by WebThing
    DataBroker::begin();              // Get the data exchange mechanism running
  }

  void setup(bool enterAPMode) {
    static String DefaultTitle("WebThing");
    apMode = enterAPMode;
    Log.verbose(F("WebThing:: setup(apMode = %T)"), apMode);
    Internal::prepNetwork();          // Get on the network
    Internal::timeDB.init(            // Initialize the time service... 
        settings.timeZoneDBKey,
        settings.lat,
        settings.lng);
    WebUI::init();                    // Set up the Web User Interface
    WebUI::setTitle(DefaultTitle);
    if (!apMode && !settings.timeZoneDBKey.isEmpty()) {  // ...and sync the time
      Internal::timeDB.syncTime(true);
    }
  }

  void postSetup() {
    // If we're in low power mode, that's it!! We never get to the loop
    // function, all the magic happens in setup()
    if (lowPowerModeActive()) enterDeepSleep();
  }

  void loop() {
    static uint32_t lastActionTime = 0;

    WebUI::handleClient();
    buttonMgr.process();

#if defined(ESP8266)
    if (Protected::mDNSStarted) { MDNS.update(); }
#endif

    uint32_t curMillis = millis();
    if (curMillis - lastActionTime > (settings.processingInterval * 60 * 1000L)) {
      if (Internal::afterSleepMinutesCB) Internal::afterSleepMinutesCB();
      lastActionTime = curMillis;
    }

    if (!apMode && !settings.timeZoneDBKey.isEmpty()) { Internal::timeDB.syncTime(); }
  }

  /*------------------------------------------------------------------------------
   *
   * Public Utility Functions
   *
   *----------------------------------------------------------------------------*/            

  void displayPowerOptions(bool enabled) {
    if (enabled == settings.displayPowerOptions) return;
    settings.displayPowerOptions = enabled;
    settings.write();
  }

  String formattedTime(time_t theTime, TimeFormatOptions options) {
    char buf[12]; // HH:MM:SS AM

    int h = options.use24Hour ? hour(theTime) : hourFormat12(theTime);
    const char* amPM = options.showAMPM ? (isAM(theTime) ? " AM" : " PM") : "";
    if (options.includeSeconds) sprintf(buf, "%02d:%02d:%02d%s", h, minute(theTime), second(theTime), amPM);
    else sprintf(buf, "%02d:%02d%s", h, minute(theTime), amPM);

    return String(buf);
  }

  String formattedTime(time_t theTime, bool use24Hour, bool includeSeconds) {
    return formattedInterval(
        use24Hour ? hour(theTime) : hourFormat12(theTime),
        minute(theTime), second(theTime), false, includeSeconds);
  }

  String formattedTime(bool use24Hour, bool includeSeconds) {
    return formattedTime(now(), use24Hour, includeSeconds);
  }

  String formattedInterval(int h, int m, int s, bool zeroPadHours, bool includeSeconds) {
    // Return a result of the form hh:mm:ss (seconds are optional)
    String result;
    result.reserve(5 + includeSeconds ? 3 : 0);
    if (zeroPadHours && (h < 10))  result += "0"; result += h;
    result += ':';
    if (m < 10)  result += "0"; result += m; 
    if (includeSeconds) {
      result += ':';
      if (s < 10)  result += "0"; result += s;
    }
    return result;
  }

  String formattedInterval(uint32_t seconds, bool zeroPadHours, bool includeSeconds) {
    int h = seconds / Basics::SecondsPerHour;
    int m = (seconds / Basics::SecondsPerMinute) % Basics::SecondsPerMinute;
    int s = (seconds % Basics::SecondsPerMinute);       
    return formattedInterval(h, m, s, zeroPadHours, includeSeconds);
  }

  int32_t getGMTOffset() { return apMode ? 0: Internal::timeDB.getGMTOffset(); }

  void setDisplayedVersion(const String& version) { versionToDisplay = version; }
  void setDisplayedVersion(const char* version) { versionToDisplay = version; }
  String  getDisplayedVersion() { return versionToDisplay; }

  float measureVoltage() {
    if (!settings.hasVoltageSensing) return -1;
    // NOTE: For a Wemos D1 Mini, there is a builtin voltage divider that scales
    // a 3.3V input on A0 down to the 1V range. It consists of a 220K:100K resistors
    // To get the range to be 0-5V, add an additional 220K resistor before A0 to 
    // yield a divider of 440K:100K
    unsigned long raw = analogRead(A0);
    float measuredVoltage = (raw * settings.voltageCalibFactor)/1024.0; 
    // Log.verbose(F("measuredVoltage: %FV"), measuredVoltage);
    return measuredVoltage;
  }

  void notifyBeforeDeepSleep(std::function<void()> callback) {
    Internal::deepSleepCallback = callback;
  };

  void notifyAfterSleepMinutes(std::function<void()> callback) {
    Internal::afterSleepMinutesCB = callback;
  }

  void notifyOnConfigMode(std::function<void(const String&, const String&)> callback) {
    Internal::configModeCB = callback;
  }

  void notifyConfigChange(std::function<void()> callback) {
    Internal::configChangeCB = callback;
  }

  void enterDeepSleep() {
    Log.trace(F("Going to sleep now for %d minute(s)"), settings.processingInterval);  
    if (Internal::deepSleepCallback != NULL) Internal::deepSleepCallback();
    ESP.deepSleep(settings.processingInterval * 60 * 1000000); // convert to microseconds
  }

  int8_t wifiQualityAsPct() {
    int32_t dbm = WiFi.RSSI();
    if (dbm <= -100) { return 0; }
    if (dbm >= -50) { return 100; }
    return 2 * (dbm + 100);
  }

  String ipAddrAsString() { return WiFi.localIP().toString(); }

  bool replaceEmptyHostname(const char* prefix) {
    if (settings.hostname.isEmpty()) {
      settings.hostname = (String(prefix) + String(GenericESP::getChipID(), HEX));
      return true;
    }
    return false;
  }

  bool isSleepOverrideEnabled() {
    if (settings.sleepOverridePin == Basics::UnusedPin) return false;
    return (digitalRead(settings.sleepOverridePin) == 0);
  }

  bool lowPowerModeActive()  {
    return (settings.useLowPowerMode && !isSleepOverrideEnabled());
  }

  void logHeapStatus() {
    Log.verbose(F("Heap Free Space: %d"), GenericESP::getFreeHeap());
    Log.verbose(F("Heap Fragmentation: %d"), GenericESP::getHeapFragmentation());
    Log.verbose(F("Heap Max Block Size: %d"), GenericESP::getMaxFreeBlockSize());
  }

  void genHeapStatsRow(const char* msg) {
    Log.verbose(F("HEAP,%s,%d,%d"), msg, ESP.getFreeHeap(), GenericESP::getHeapFragmentation());
  }

  String encodeAttr(const String &src) {
    String buffer = "";
    size_t srcLength = src.length();
    buffer.reserve(srcLength + 6);
    for (size_t pos = 0; pos != srcLength; ++pos) {
      char c = src[pos];
      switch (c) {
        case '&':  buffer.concat("&amp;");  break;
        case '\"': buffer.concat("&quot;"); break;
        case '\'': buffer.concat("&apos;"); break;
        case '<':  buffer.concat("&lt;");   break;
        case '>':  buffer.concat("&gt;");   break;
        default:   buffer.concat(c);        break;
      }
    }
    return buffer;
  }

  String urldecode(const String &str) { 
    String encodedString="";
    char c, code0, code1;
    int len = str.length();

    for (int i = 0; i < len; i++) {
      c = str.charAt(i);
      if (c == '+') {
        encodedString.concat(' ');
      } else if (c == '%') {
        code0 = str.charAt(++i);
        code1 = str.charAt(++i);
        c = (Internal::h2int(code0) << 4) | Internal::h2int(code1);
        encodedString.concat(c);
      } else { encodedString.concat(c);  }
      yield();
    }
   return encodedString;
  }

  String urlencode(const String &str) {
    String encodedString = "";
    char c, code0, code1;
    int len = str.length();

    for (int i = 0; i < len; i++) {
      c = str.charAt(i);
      if (c == ' ') {
        encodedString.concat('+');
      } else if (isalnum(c)) {
        encodedString.concat(c);
      } else {
        code1 = (c & 0xf) + '0';
        if ((c & 0xf) > 9) { code1 = (c & 0xf) - 10 + 'A'; }
        c = (c >> 4) & 0xf;
        code0 = c + '0';
        if (c > 9) { code0 = c - 10 + 'A'; }
        encodedString.concat('%');
        encodedString.concat(code0);
        encodedString.concat(code1);
      }
      yield();
    }
    return encodedString;
  }
}
// ----- END: WebThing namespace



