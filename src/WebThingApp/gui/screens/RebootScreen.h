/*
 * RebootScreen:
 *    Confirmation screen to trigger a reboot 
 *                    
 */

#ifndef RebootScreen_h
#define RebootScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class RebootScreen : public Screen {
public:
  RebootScreen();
  void display(bool activating = false);
  virtual void processPeriodicActivity();

private:
  static const uint16_t IconInset = 10;
  static const uint8_t RebootButtonID = 0;
  static const uint8_t CancelButtonID = 1;

  uint32_t autoCancelTime = UINT32_MAX;
};

#endif // RebootScreen_h
