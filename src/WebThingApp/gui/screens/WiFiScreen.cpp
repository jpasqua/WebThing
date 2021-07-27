/*
 * CalibrationScreen:
 *    Display a notification that the device is connecting to WiFi
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Display.h>
#include <WebThingApp/gui/Theme.h>
//                                  Local Includes
#include "WiFiScreen.h"
#include "images/WiFiLogo.h"
//--------------- End:    Includes ---------------------------------------------

using Display::tft;

WiFiScreen::WiFiScreen() {
    nButtons = 0;
    buttons = NULL;
  }

void WiFiScreen::display(bool activating) {
  (void)activating; // We don't use this parameter - avoid a warning...
  tft.fillScreen(Theme::Color_Background);
  uint16_t x = (Display::Width-WiFiLogo_Width)/2;
  uint16_t y = 30;  // A little space from the top of the screen
  tft.pushImage(x, y, WiFiLogo_Width, WiFiLogo_Height, WiFiLogo, WiFiLogo_Transparent);
  y += WiFiLogo_Height;

  Display::Font::setUsingID(Display::Font::FontID::SBO12, tft);
  tft.setTextColor(Theme::Color_WiFiBlue);
  tft.setTextDatum(MC_DATUM);
  x = Display::XCenter;
  y += (Display::Height - y)/2;
  tft.drawString(F("Connecting..."), x, y);
}

void WiFiScreen::processPeriodicActivity() {
  // Nothing to do here, we're displaying a static screen
}
