/*
 * ConfigScreen:
 *    Displays a screen with instructions when the device is going through
 *    an initil WiFi setup.
 *                    
 */

#ifndef ConfigScreen_h
#define ConfigScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class ConfigScreen : public Screen {
public:
  ConfigScreen();
  void display(bool activating = false);
  virtual void processPeriodicActivity();

  void setSSID(String &ssid);

private:
  String _ssid = "app-NNNNN";
};

#endif  // ConfigScreen_h