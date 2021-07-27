/*
 * EnterNumberScreen:
 *    Allow the user to enter a numeric value using a numberpad
 *                    
 * NOTES:
 * o The value of the numberpad is always provided as a float even if
 *   decimalsAllowed is false (see below)
 * o The numberpad can be configured using parameters to the init function:
 *   + decimalsAllowed: If true, users will be allowed to enter floats, otherwise
 *     they will only be able to enter whole numbers
 *   + minAllowed: The minimum acceptable value. If this is less than 0, a '+/-'
 *     button will be provided to change the sign of the entered value
 *   + maxAllowed: The maximum acceptable value
 */

#ifndef EnterNumberScreen_h
#define EnterNumberScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <float.h>
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Display.h>
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class EnterNumberScreen : public Screen {
public:
  EnterNumberScreen();
  void display(bool activating = false);
  virtual void processPeriodicActivity();

  void init(
      String theTitle, float initialValue, WTBasics::FloatValCB cb, bool decimalsAllowed = false,
      float minAllowed = 0.0, float maxAllowed = FLT_MAX);

private:
  String title;
  float minVal, maxVal;
  float _initialValue;
  bool allowDecimals;
  String formattedValue;
  WTBasics::FloatValCB newValueCB;

  // -- Button IDs
  static const uint8_t FirstDigitButton = 0;
  static const uint8_t LastDigitButton = 9;
  static const uint8_t DecimalButton = 10;
  static const uint8_t BackspaceButton = DecimalButton+1;
  static const uint8_t PlusMinusButton = BackspaceButton+1;
  static const uint8_t ValueButton = PlusMinusButton+1;
  static const uint8_t TotalButtons = ValueButton+1;

  // Fonts
  static const auto ValueFont = Display::Font::FontID::SB12;
  static const uint16_t ValueFontHeight = 29;

  static const auto DigitFont = Display::Font::FontID::SB9;
  static const uint16_t DigitFontHeight = 22;

  // Field Locations
  static const auto TitleFont = Display::Font::FontID::SB12;
  static const uint16_t TitleFontHeight = 29;   // TitleFont->yAdvance;
  static const uint16_t TitleXOrigin = 0;
  static const uint16_t TitleYOrigin = 0;
  static const uint16_t TitleHeight = TitleFontHeight;
  static const uint16_t TitleWidth = Display::Width;

  static const uint16_t DigitWidth = 60;
  static const uint16_t DigitHeight = 60;

  static const uint16_t ValueXOrigin = 5;
  static const uint16_t ValueYOrigin = TitleHeight + 5;
  static const uint16_t ValueWidth = 100;
  static const uint16_t ValueHeight = 60;

  static const uint16_t DigitsXOrigin = ValueXOrigin;
  static const uint16_t DigitsYOrigin = ValueYOrigin + ValueHeight + 5;

  static const uint8_t ValueFrameSize = 2;
  static const uint8_t DigitFrameSize = 1;
};

#endif  // EnterNumberScreen_h







