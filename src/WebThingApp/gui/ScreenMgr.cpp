/*
 * ScreenMgr
 *    Manages the screens used in a WebThing app
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <TimeLib.h>
//                                  Local Includes
#include "Display.h"
#include "ScreenMgr.h"
#include "Theme.h"
#include "FlexScreen.h"
//--------------- End:    Includes ---------------------------------------------

using Display::tft;

namespace ScreenMgr {
  namespace State {
    static const uint8_t MaxScreens = 20;

    struct {
      String name;
      Screen* screen;
    } map[MaxScreens];

    uint8_t nScreens = 0;
    Screen* curScreen;
    Screen* homeScreen;

    BrightnessSchedule* schedule;
  };

  namespace Internal {
    static const uint16_t InfoIconSize = 32;
    static const uint16_t InfoIconBorderSize = 5;
    static const uint16_t InfoIconX = (320 - InfoIconSize);
    static const uint16_t InfoIconY = 0;
    uint16_t *savedPixels = NULL;
    bool infoIconIsDisplayed = false;

    void drawInfoIcon(uint16_t x, uint16_t y, uint16_t borderColor, uint16_t fillColor, uint16_t textColor) {
      uint16_t centerX = x+(InfoIconSize/2);
      uint16_t centerY = y+(InfoIconSize/2);
      tft.fillCircle(centerX, centerY, InfoIconSize/2-1, borderColor);
      tft.fillCircle(centerX, centerY, (InfoIconSize/2-1)-InfoIconBorderSize, fillColor);
      tft.setTextDatum(MC_DATUM);
      // tft.setFreeFont(&FreeSerifBoldItalic9pt7b);
      tft.setFreeFont(&FreeSansBold9pt7b);
      tft.setTextColor(textColor);
      tft.drawString("i", centerX, centerY);
    }

    void initInfoIcon() {
      // In theory it would be better to allocate/deallocate this as needed, but it causes
      // a lot more fragmentation and potentially a crash.
      savedPixels = (uint16_t *)malloc(InfoIconSize*InfoIconSize*sizeof(uint16_t));  // This is BIG!
    }

    void showUpdating(uint16_t accentColor) {
      if (infoIconIsDisplayed) return;
      tft.readRect(InfoIconX, InfoIconY, InfoIconSize, InfoIconSize, savedPixels);
      drawInfoIcon(
        InfoIconX, InfoIconY, accentColor,
        Theme::Color_UpdatingFill, Theme::Color_UpdatingText);
      infoIconIsDisplayed = true;
    }

    void hideUpdating() {
      if (!infoIconIsDisplayed) return;
      tft.pushRect(InfoIconX, InfoIconY, InfoIconSize, InfoIconSize, savedPixels);
      infoIconIsDisplayed = false;
    }

    void processSchedules() {
      static uint32_t eveningExecutedOnDay = UINT32_MAX;
      static uint32_t morningExecutedOnDay = UINT32_MAX;
      static uint32_t nextScheduleCheck = 0;

      if (State::schedule->active) {
        uint16_t morning = State::schedule->morning.hr * 100 + State::schedule->morning.min;
        uint16_t evening = State::schedule->evening.hr * 100 + State::schedule->evening.min;
        uint32_t curMillis = millis();
        uint32_t today = day();
        if (curMillis > nextScheduleCheck) {
          nextScheduleCheck = curMillis + (60-second()) * 1000L;
          uint16_t curTime = hour() * 100 + minute();
          if (curTime >= evening || curTime < morning) {
            if (eveningExecutedOnDay != today) {
              Display::setBrightness(State::schedule->evening.brightness);
              eveningExecutedOnDay = today;
            }
          } else if (morningExecutedOnDay != today) {
            Display::setBrightness(State::schedule->morning.brightness);
            morningExecutedOnDay = today;
          }
        }
      }
    }
  };  // ----- END: ScreenMgr::Internal

  void setup(BrightnessSchedule* theSchedule, bool flip, Display::CalibrationData* cd) {
    Display::begin(flip, cd);
    State::schedule = theSchedule;
    Internal::initInfoIcon();
  }

  void loop() {
    if (State::curScreen != NULL) {
      State::curScreen->processPeriodicActivity();
      uint16_t tx = 0, ty = 0;
      bool pressed = Display::tft.getTouch(&tx, &ty);
      State::curScreen->processInput(pressed, tx, ty);
    }
    Internal::processSchedules();
  }


  bool registerScreen(String screenName, Screen* theScreen) {
    if (State::nScreens == State::MaxScreens) {
      Log.warning("No space for more screens while registering: %s", screenName.c_str());
      return false;
    }
    if (find(screenName) != NULL) {
      Log.warning("Trying to register a screen with a name that is in use: %s", screenName.c_str());
      return false;
    }
    State::map[State::nScreens].name = screenName;
    State::map[State::nScreens].screen = theScreen;
    State::nScreens++;
    return true;
  }

  void setAsHomeScreen(Screen* screen) {
    State::homeScreen = screen;
  }

  Screen* find(String name) {
    for (int i = 0; i < State::nScreens; i++) {
      if (State::map[i].name.equals(name)) return (State::map[i].screen);
    }
    return NULL;
  }

  void display(String name) {
    Screen* screen = find(name);
    if (screen == NULL) {
      Log.warning("Trying to display screen %s, but none is registered", name.c_str());
      return;
    }
    display(screen);
  }

  void display(Screen* screen) {
    State::curScreen = screen;
    screen->activate();
  }

  void displayHomeScreen() {
    if (State::homeScreen == NULL) {
      Log.warning("Trying to display a home screen when none has been set");
      return;
    }
    display(State::homeScreen);
  }

  FlexScreen* createFlexScreen(
      JsonDocument &doc,
      uint32_t refreshInterval,
      const WTBasics::ReferenceMapper &mapper)
  {
    FlexScreen* flexScreen = new FlexScreen();
    JsonObjectConst descriptor = doc.as<JsonObjectConst>();
    if (!flexScreen->init(descriptor, refreshInterval, mapper)) {
      delete flexScreen;
      return NULL;
    }

    registerScreen(flexScreen->getScreenID(), flexScreen);
    return (flexScreen);
  }

  void showUpdatingIcon(uint16_t accentColor) { Internal::showUpdating(accentColor); }
  void hideUpdatingIcon() { Internal::hideUpdating(); }

};