/*
 * RebootScreen:
 *    Confirmation screen to trigger a reboot 
 *                    
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
//                                  WebThing Includes
#include <WebThingApp/gui/Display.h>
#include <WebThingApp/gui/Theme.h>
#include <WebThingApp/gui/ScreenMgr.h>
//                                  Local Includes
#include "RebootScreen.h"
#include "images/CancelBitmap.h"
#include "images/RebootBitmap.h"
//--------------- End:    Includes ---------------------------------------------

using Display::tft;

RebootScreen::RebootScreen() {
    auto buttonHandler =[&](int id, Button::PressType type) -> void {
      Log.verbose(F("In RebootScreen ButtonHandler, id = %d"), id);
      if (id == RebootButtonID && type > Button::PressType::NormalPress) { ESP.restart(); }
      if (id == CancelButtonID) ScreenMgr::displayHomeScreen();
    };

    buttons = new Button[(nButtons = 2)];
    buttons[0].init(0, 0,                Display::Width, Display::Height/2, buttonHandler, RebootButtonID);
    buttons[1].init(0, Display::Height/2, Display::Width, Display::Height/2, buttonHandler, CancelButtonID);
  }

void RebootScreen::display(bool activating) {
  (void)activating; // We don't use this parameter - avoid a warning...
  tft.fillScreen(Theme::Color_Background);

  Display::Font::setUsingID(Display::Font::FontID::SB12, tft);
  tft.setTextDatum(MC_DATUM);

  uint16_t xc = (Display::Width + (IconInset+RebootIcon_Width))/2;
  uint16_t yc = (Display::Height)/4;
  tft.drawRect(
      buttons[RebootButtonID]._x, buttons[RebootButtonID]._y,
      buttons[RebootButtonID]._w, buttons[RebootButtonID]._h, Theme::Color_AlertError);
  tft.drawBitmap(
      IconInset, (Display::Height/2-RebootIcon_Height)/2, RebootIcon,
      RebootIcon_Width, RebootIcon_Height,
      Theme::Color_AlertError);
  tft.setTextColor(Theme::Color_AlertError);
  tft.drawString(F("Reboot"), xc, yc);

  xc = (Display::Width + (IconInset+CancelIcon_Width))/2;
  yc = (Display::Height*3)/4;
  tft.drawRect(
      buttons[CancelButtonID]._x, buttons[CancelButtonID]._y,
      buttons[CancelButtonID]._w, buttons[CancelButtonID]._h, Theme::Color_AlertGood);
  tft.drawBitmap(
      IconInset, yc-(CancelIcon_Height/2), CancelIcon,
      CancelIcon_Width, CancelIcon_Height,
      Theme::Color_AlertGood);
  tft.setTextColor(Theme::Color_AlertGood);
  tft.drawString(F("Cancel"), xc, yc);
  autoCancelTime = millis() + 60 * 1000L; // If nothing has happened in a minute, cancel
}

void RebootScreen::processPeriodicActivity() {
  if (millis() >= autoCancelTime) ScreenMgr::displayHomeScreen();
}



