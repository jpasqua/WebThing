/*
 * ForecastScreen:
 *    Display the 5-day forecast. Since we are arranging the readings in two columns
 *    with the 3 forecast cells each, we've got an extra spot. Make the first spot
 *    the current readings and the next 5 spots the 5-day forecast.
 *
 */

#ifndef ForecastScreen_h
#define ForecastScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <WebThingApp/clients/OWMClient.h>
//                                  WebThing Includes
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class ForecastScreen : public Screen {
public:
  ForecastScreen(OWMClient* weatherClient, bool* use24hrTime, bool* metric);
  void display(bool force = false);
  virtual void processPeriodicActivity();

private:
  OWMClient* owmClient;
  bool* use24Hour;
  bool* useMetric;

  void displaySingleForecast(Forecast *f, uint16_t x, uint16_t y);
  void showTime(bool clear = false);
};

#endif  // ForecastScreen_h