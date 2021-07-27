#ifndef Screen_h
#define Screen_h

#include "Button.h"

class Screen {
public:
  // ----- State
  Button *buttons;
  uint8_t nButtons;
  
  // ----- Functions that must be implemented by subclasses
  virtual void display(bool force = false) = 0;
  virtual void processPeriodicActivity() = 0;

  // ----- Functions that may be overriden by subclasses
  virtual void activate() {
    startOfPress = endOfPress = 0;
    display(true);
  }

  void processInput(bool pressed, uint16_t tx, uint16_t ty) {
    if (pressed) {
      if (startOfPress == 0) {
        startOfPress = millis();
        endOfPress = 0;
      }
      lastX = tx; lastY = ty;
    } else {
      if (startOfPress != 0) {
        if (endOfPress == 0) {
          endOfPress = millis();
        } else if (millis() - endOfPress > DebounceTime) {
          // Process the press
          // Ok, we got a press/release, see which button (if any) is associated
          Button::PressType pt;
          uint32_t pressDuration = (millis() - startOfPress);
          if (pressDuration >= Button::VeryLongPressInterval) pt = Button::PressType::VeryLongPress;
          else if (pressDuration >= Button::LongPressInterval) pt = Button::PressType::LongPress;
          else pt = Button::PressType::NormalPress;

          for (int i = 0; i < nButtons; i++) {
            if (buttons[i].processTouch(lastX, lastY, pt)) break;
          }
          endOfPress = 0;
          startOfPress = 0;
        }
      }
    }
  }

private:
  static const uint32_t DebounceTime = 100;
  uint32_t startOfPress = 0;
  uint32_t endOfPress = 0;
  uint16_t lastX, lastY;
};

#endif // Screen_h