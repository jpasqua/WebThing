/*
 * WebThing:
 *    A generic ESP8266 framework for building "things" with a web configuration
 *    capability. This is designed for use by IoT applications - hence the "thing"
 *    nomenclature.
 *
 * USING THE LIBRARY:
 * o WebThing is a singleton and implemented as a namespace, not a class
 * o In the setup() function of your main app, you need to call WebThing::preSetup()
 *   first to do things like read basic settings. Call WebThing::setup() whenever
 *   appropriate to get the network/webui started. Then call WebThing::postSetup()
 *   before exiting your setup() function.
 * o In the loop() function of your main app, call WebThing::loop to give it a
 *   chance to handle network-related activities.
 * o Using the notify...() functions allows you to be notified of various events
 *   via registered callbacks.
 * o If you are using low power mode, your loop function will NEVER be called.
 *   Once you call WebThing::postSetup(), the device will enter deep sleep
 *   for settings.processingInterval minutes.
 *
 * BUILD PROCESS:
 * - WebThing uses SPIFFS to store HTML templates and settings (as a JSON file), 
 *   which imposes additional requirements when building:
 *   1. In the Arduino IDE you must ensure that you have reserved SPIFFS space
 *      Tools -> Flash Size -> (Pick a SPIFFS size)
 *   2. All of the templates must be uploaded to the ESP8266. You can use the
 *      ESP8266 Sketch Data Upload plugin for this. It can be found here:
 *      https://github.com/esp8266/arduino-esp8266fs-plugin
 *   3. Because you are likely to extend the Web UI, you may have your own
 *      templates or other files to place in SPIFFS. Put them all in a directory
 *      named 'data' within your sketch directory.
 *   4. The uploader must upload all files at once - your files and those from
 *      WebThing. That means you need to copy or link the WebThing files to your
 *      data directory. All of the WebThing files are in a sub-directory named wt
 *      Your resulting directory structure will look like:
 *      [Your Sketch Dir]
 *       |
 *       +--[data]
 *           |
 *           +- [Your File 1]
 *           +- ...
 *           +- [Your File N]
 *           |
 *           +- [wt]
 * 
 * LOW POWER MODE
 * o When low power mode is selected in the Web UI, the Web UI becomes
 *   unavailable which means that you can't switch out of low power mode
 * o To force the device out of low power mode, you may designate a pin
 *   as an override. That pin will be pulled HIGH. If it is ever seen
 *   LOW, then the low power mode web setting will be ignored.
 *
 */

#ifndef WebThing_h
#define WebThing_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
//                                  Local Includes
#include "BPABasics.h"
#include "clients/TimeDB.h"
#include "WebThingSettings.h"
#include "WTButton.h"
//--------------- End:    Includes ---------------------------------------------


namespace WebThing {
  // ----- Constants
  static constexpr const char* Version = "0.5.0";

  // ----- Public State
  extern WTButtonMgr buttonMgr;
  extern WebThingSettings  settings;
  extern bool apMode;

  // ----- setup() and loop() functions
  // Called before any setup is done by the actual thing. This does very
  // very basic things like establishing logging support.
  void preSetup(Basics::Pin SDA = Basics::UnusedPin, Basics::Pin SCL = Basics::UnusedPin);

  // Called during setup of the actual thing
  // If you want this thing to be it's own access point rather than connecting
  // to an access point, set apMode to true. Obviously in this case there will
  // be no access to services such as network time.
  void setup(bool apMode = false);

  // Called after setup of the actual thing is complete
  void postSetup();

  // Called by the loop method of the actual thing
  void loop();

  // ----- Public Utility Functions
  // --- Power-related
  float   measureVoltage(); // Returns -1 if voltage sensing is not enabled
  void    enterDeepSleep();
    // Put the device into low power mode for 'processingInterval' minutes
    // Note that to be automatically awakened after 'processingInterval' minutes
    // you must have GPIO16 connected to the RST pin to trigger the reset
  bool    lowPowerModeActive();     // Is low power mode selected in settings?
  bool    isSleepOverrideEnabled(); // Is the HW override of sleep mode engaged?
  void    displayPowerOptions(bool enabled);  // Will the power options page be a menu option?

  // --- Notification Callbacks
  void    notifyBeforeDeepSleep(std::function<void()> callback);
  void    notifyAfterSleepMinutes(std::function<void()> callback);
  void    notifyOnConfigMode(std::function<void(const String&, const String&)> callback);
  void    notifyConfigChange(std::function<void()> callback);

  // --- UI Helpers
  int8_t  wifiQualityAsPct();
  void    setDisplayedVersion(const String& version);
  void    setDisplayedVersion(const char* version);
  String  getDisplayedVersion();
  String  ipAddrAsString();

  // --- Time Helpers
  struct TimeFormatOptions {
    bool use24Hour;
    bool zeroPadHours;
    bool showAMPM;
    bool includeSeconds;
  };
  String formattedTime(time_t theTime, TimeFormatOptions options);
  String formattedTime(time_t theTime, bool use24Hour = false, bool includeSeconds = false);
  String formattedTime(bool use24Hour = false, bool includeSeconds = false);
  String formattedInterval(int h, int m, int s, bool zeroPadHours = false, bool includeSeconds = true);
  String formattedInterval(uint32_t seconds, bool zeroPadHours = true, bool includeSeconds = true);
  int32_t getGMTOffset();

  // --- HTTP/HTML Helpers
  String urldecode(const String &str);
  String urlencode(const String &str);
  String encodeAttr(const String &src);

  // --- Other
  bool replaceEmptyHostname(const char* prefix);  // If hostname in settings is empty, generate one
  void logHeapStatus();
  void genHeapStatsRow(const char* msg);

  namespace Protected {
    extern bool mDNSStarted;
    void configChanged();
  }
}

#endif // WebThing_h
