#ifndef ScreenMgr_h
#define ScreenMgr_h

/*
 * ScreenMgr
 *    Manages the screens used in a WebThing app
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <functional>
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <WebThingBasics.h>
//                                  Local Includes
#include "Display.h"
#include "FlexScreen.h"//--------------- End:    Includes ---------------------------------------------


namespace ScreenMgr {
  typedef struct {
    bool active;                        // Is the scheduling system active?
    struct {
      uint8_t hr;                       // Hour that the period starts (24 hour time)
      uint8_t min;                      // Minute that the period starts
      uint8_t brightness;               // The brightness level during this period
    } morning, evening;
  } BrightnessSchedule;

  void setup(BrightnessSchedule* theSchedule, bool flip, Display::CalibrationData* cd);
  void loop();

  // ----- Screen Management functions
  bool registerScreen(String screenName, Screen* theScreen);
  void setAsHomeScreen(Screen* screen);
  Screen* find(String name);

  // ----- Screen Display functions
  void display(String name);
  void display(Screen* screen);
  void displayHomeScreen();
  /**
   * Overlay the current screen with an icon to indicate that a potentially
   * long-running update is in progress. this lets the user know that the UI
   * will be unresponsive in this period. Calling showUpdatingIcon()
   * when the icon is already displayed is safe and does nothing.
   * @param   accentColor   An accent color to indicate what's happening
   */
  void showUpdatingIcon(uint16_t accentColor);

  /**
   * Remove the "updating icon" from the current screen and restore the original
   * screen content. Calling hideUpdatingIcon() when no icon is displayed
   * is safe and does nothing.
   */
  void hideUpdatingIcon();

  // ----- Plugin-related functions
  FlexScreen* createFlexScreen(
      JsonDocument &doc,
      uint32_t refreshInterval,
      const WTBasics::ReferenceMapper &mapper);


};

#endif  // ScreenMgr_h