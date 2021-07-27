/*
 * ForecastScreen:
 *    Display the 5-day forecast. Since we are arranging the readings in two columns
 *    with the 3 forecast cells each, we've got an extra spot. Make the first spot
 *    the current readings and the next 5 spots the 5-day forecast.
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <WebThingBasics.h>
#include <WebThing.h>
#include <WebThingApp/clients/OWMClient.h>
//                                  WebThing Includes
#include <WebThingApp/gui/Display.h>
#include <WebThingApp/gui/Theme.h>
#include <WebThingApp/gui/ScreenMgr.h>
//                                  Local Includes
#include "ForecastScreen.h"
#include "WeatherIcons.h"
//--------------- End:    Includes ---------------------------------------------


using Display::tft;

/*------------------------------------------------------------------------------
 *
 * CONSTANTS
 *
 *----------------------------------------------------------------------------*/

static const auto ReadingsFont = Display::Font::FontID::SB9;
static const uint16_t ReadingsFontHeight = 22;  // ReadingsFont->yAdvance;

static const uint16_t TileWidth = Display::Width/2;
static const uint16_t TileHeight = Display::Height/3;
static const uint16_t TextVPad = (TileHeight - (2*ReadingsFontHeight))/2;

static int16_t TinyFont = 2;  // A small 5x7 font

/*------------------------------------------------------------------------------
 *
 * Constructors and Public methods
 *
 *----------------------------------------------------------------------------*/

ForecastScreen::ForecastScreen(OWMClient** weatherClient, bool* use24hrTime, bool* metric) {
  owmClient = weatherClient;
  use24Hour = use24hrTime;
  useMetric = metric;

  auto buttonHandler =[&](int id, Button::PressType type) -> void {
    Log.verbose(F("In ForecastScreen Button Handler, id = %d, type = %d"), id, type);
    if (type > Button::PressType::NormalPress) ScreenMgr::display("Weather");
    ScreenMgr::displayHomeScreen();
  };

  buttons = new Button[(nButtons = 1)];
  buttons[0].init(0, 0, Display::Width, Display::Height, buttonHandler, 0);
}


void ForecastScreen::display(bool activating) {
  (void)activating; // We don't use this parameter - avoid a warning...

  if (!(*owmClient)) return; // Make sure we've got an OpenWeatherMap client

  tft.fillScreen(Theme::Color_WeatherBkg);
  uint16_t x = 0, y = 0;

  // The first element of the forecast display is really the current temperature
  Forecast current;
  current.dt = (*owmClient)->weather.dt + WebThing::getGMTOffset();
  current.hiTemp = (*owmClient)->weather.readings.temp;
  current.loTemp = Forecast::NoReading;
  current.icon = (*owmClient)->weather.description.icon;
  displaySingleForecast(&current, x, y);
  y += TileHeight;

  Forecast *f = (*owmClient)->getForecast();
  for (int i = 0; i < OWMClient::ForecastElements; i++) {
    // It's possible that the current temperature is higher than what was
    // forecast. If so, update the forecast with the known higher temp
    if (i == 0 && day(f[i].dt) == day(current.dt)) {
      if (f[i].hiTemp < current.hiTemp) {
        f[i].dt = current.dt;
        f[i].hiTemp = current.hiTemp;
        f[i].icon = current.icon;
      }
    }
    displaySingleForecast(&f[i], x, y);
    if (i == 1) x += TileWidth;
    y = (y + TileHeight) % Display::Height;
  }
}

void ForecastScreen::processPeriodicActivity() {
  // Nothing to do here...
}

void ForecastScreen::displaySingleForecast(Forecast *f, uint16_t x, uint16_t y) {
  tft.pushImage(
      x, y+((TileHeight-WI_Height)/2),
      WI_Width, WI_Height,
      getWeatherIcon(f->icon),
      WI_Transparent);
  x += WI_Width;

  tft.setTextColor(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);

  String reading = "";
  String units = *useMetric ? "C" : "F";

  if (f->loTemp != Forecast::NoReading) reading = String((int)(f->loTemp+0.5)) + " / ";
  reading += String((int)(f->hiTemp+0.5)) + units;

  y += TextVPad;
  tft.setTextColor(TFT_BLACK);
  Display::Font::setUsingID(ReadingsFont, tft);
  tft.drawString(reading, x, y);

  y += ReadingsFontHeight;
  reading = dayShortStr(weekday(f->dt));
  int h = hour(f->dt);
  bool pm = isPM(f->dt);
  if (!*use24Hour) {
    if (pm) h -= 12;
    if (h == 0) h = 12;
    reading += " " + String(h);
  } else {
    reading += " ";
    if (h < 10) reading += "0";
    reading += String(h);
  }

  x += tft.drawString(reading, x, y) + 1;
  if (*use24Hour) {
    reading = ":00";
  } else {
    reading = (pm ? "PM" : "AM");
  }
  tft.drawString(reading, x+1, y+1, TinyFont);
}
