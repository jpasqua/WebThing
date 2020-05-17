/*
 * WebThingSettings.h
 *    Defines the values that can be set through the web UI and sets their initial values
 *
 * NOTES:
 * o Adding a setting is a multi-step process:
 *   1. Add a member variable to store the new setting in the class definition below
 *      Give it a default value here, or in the constructor if it needs to be computed.
 *   2. Update toJSON(), fromJSON, and logSettings() to reflect the new setting
 *   3. Assuming the setting is configureable through a UI (the WebUI and/or others),
 *      add the interface for it. For WebUIs, this is typically in the handleConfigure()
 *      function and the handleUpdateConfig() function.
 * 
 */

#ifndef WebThingSettings_h
#define WebThingSettings_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  Local Includes
#include "BaseSettings.h"
//--------------- End:    Includes ---------------------------------------------


class WebThingSettings: public BaseSettings {
public:
  // ----- Constants
  static const int8_t   NoPinAssigned;
  static const int8_t   UseBuiltinLED;

  // ----- Constructors and methods
  WebThingSettings();
  void fromJSON(JsonDocument &doc);
  void toJSON(JsonDocument &doc);
  void logSettings();

  // ----- Location Settings
  float  lat = 37.448237;
  float  lng = -122.180620;
  int    elevation = 21;                // Units: meters
  String latAsString() { return String(lat, 6); }
  String lngAsString() { return String(lng, 6); }

  // ----- Power
  bool    useLowPowerMode = false;
  int     processingInterval = 10;
  int8_t  sleepOverridePin = -1;        // -1 -> No Pin Assigned, >=0 -> GPIO Pin
  bool    hasVoltageSensing;            // Voltage sensing on pin A0
  float   voltageCalibFactor = 5.28;    // Calibrate the battery voltage
  String  vcfAsString() { return String(voltageCalibFactor, 2); }
  bool    displayPowerOptions = true;   // Whether or not these options are shown in the UI

  // ----- API Keys
  String timeZoneDBKey = "";
  String googleMapsKey = "";

  // ----- Webserver Settings
  String  hostname = "";                // The hostname for the WebThing which will be broadcast using mDNS
  int     webServerPort = 80;           // The port you can access this device on over HTTP
  bool    useBasicAuth = true;          // true = require athentication to change config settings / false = no auth
  String  webUsername = "admin";        // User account for the Web Interface
  String  webPassword = "password";     // Password for the Web Interface
  String  themeColor = "light-green";   // Theme color of the web interface. Can be updated through the UI

  // ----- Developer Settings
  int logLevel = 6;                     // 6 is LOG_LEVEL_VERBOSE

  // ----- Indicator LED
  int8_t  indicatorLEDPin = -2;         // -1 -> No LED, -2 -> Built_in, >=0 -> GPIO Pin
  bool    indicatorLEDInverted = true;

private:
  // ----- Constants
  static const uint32_t CurrentVersion;
};

#endif // WebThingSettings_h