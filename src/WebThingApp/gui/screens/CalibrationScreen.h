/*
 * CalibrationScreen:
 *    Provides a way to calibrate the touch sensor on the screen
 *
 */

#ifndef CalibrationScreen_h
#define CalibrationScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class CalibrationScreen : public Screen {
public:
  typedef std::function<void(Display::CalibrationData* newCalData)> CalibrationDataListener;

  CalibrationScreen(CalibrationDataListener listener);
  void display(bool activating = false) ;
  virtual void processPeriodicActivity();

private:
  enum {pre, post, complete} state;
  CalibrationDataListener _listener;

  void init();
};

#endif  // CalibrationScreen_h