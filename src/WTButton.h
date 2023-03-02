/*
 * WTButton
 *     Handle input from physical buttons
 *
 */

#ifndef WTButton_h
#define WTButton_h

#include <ArduinoLog.h>
#if defined(ESP32)
  #include <vector>
#endif

enum class PressType {None = -1, Normal, Long, VeryLong};

//
// PhysicalButton manages the input from a single physical button that is associated
// with an input pin. That pin is the unique identifier for this button.
//
class WTButton {
public:
  // ----- Types
  using Mapping = struct { uint8_t pin; uint8_t id; };
    // Just a convenience definition for clients who often map pins to some other ID.

  // ----- Constants
  static constexpr uint32_t LongInterval = 500;
  static constexpr uint32_t VeryLongInterval = 1000;

  // ----- Constructors
  WTButton(uint8_t pinNum): pin(pinNum)  {
    pinMode(pin, INPUT_PULLUP);
  }

  // ----- Member Functions

  // Determine whether the physical button has been pressed, and if so, what
  // type of press it was
  // @return A PressType value. If the returned value is PressType::None,
  //         then the button was not pressed.
  PressType wasPressed() {
    PressType pressType = PressType::None;
    bool pressed = !digitalRead(pin);

    if (pressed) {
      if (startOfPress == 0) {
        startOfPress = millis();
        endOfPress = 0;
      }
    } else {
      if (startOfPress != 0) {
        if (endOfPress == 0) {
          endOfPress = millis();
        } else if (millis() - endOfPress > DebounceTime) {
          // Process the press
          uint32_t pressDuration = (millis() - startOfPress);
          if (pressDuration >= VeryLongInterval) pressType = PressType::VeryLong;
          else if (pressDuration >= LongInterval) pressType = PressType::Long;
          else pressType = PressType::Normal;

          endOfPress = 0;
          startOfPress = 0;
        }
      }
    }
    return pressType;
  }

  // ----- Data Members
  uint8_t  pin;   // The physical pin associated with this button

private:
  // ----- Constants
  static constexpr uint32_t DebounceTime = 50;

  // ----- Data Members
  uint8_t  state = 1;
  uint32_t lastBounce = 0;
  uint32_t startOfPress = 0;
  uint32_t endOfPress = 0;
};

class WTButtonMgr  {
public:
  // ----- Types
  using Dispatcher = std::function<void(uint8_t, PressType)>;

  // ----- Constructors
  WTButtonMgr() = default;

  // ----- Member Functions
  void addButton(WTButton& pb) { buttons.push_back(pb); }
  void addButton(WTButton&& pb) { buttons.push_back(pb); }

  void setDispatcher(Dispatcher d) { dispatcher = d; }

  void process() {
    if (!dispatcher) return;

    for (WTButton& b : buttons) {
      PressType pt = b.wasPressed();
      if (pt != PressType::None) dispatcher(b.pin, pt);
    }
  }

private:
  // ----- Data Members
  std::vector<WTButton> buttons;
  Dispatcher dispatcher;
};

#endif  // WTButton_h
