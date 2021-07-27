/*
 * WeatherScreen:
 *    Display weather info for the selected city and a running clock. 
 *                    
 */

#ifndef WeatherScreen_h
#define WeatherScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  WebThing Includes
#include <WebThingApp/clients/OWMClient.h>
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class WeatherScreen : public Screen {
public:
  WeatherScreen(OWMClient* weatherClient, bool* metric, bool* use24);
  void display(bool force = false);
  virtual void processPeriodicActivity();

private:
  uint32_t lastDT = UINT32_MAX;
  uint32_t lastClockUpdate = 0;
  OWMClient* owmClient;
  bool* useMetric;
  bool* use24Hour;

  void displaySingleWeather(int index);
  void showTime();
};

#endif  // WeatherScreen_h