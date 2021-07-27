/*
 * EnterNumberScreen:
 *    Allow the user to enter a numeric value 
 *                    
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <float.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <WebThingBasics.h>
//                                  WebThing Includes
#include <WebThingApp/gui/Display.h>
#include <WebThingApp/gui/Theme.h>
//                                  Local Includes
#include "EnterNumberScreen.h"
//--------------- End:    Includes ---------------------------------------------

using Display::tft;

static const char* DigitLabels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};

EnterNumberScreen::EnterNumberScreen() {
  nButtons = 0; // No buttons until init is called!
  buttons = new Button[TotalButtons];
}

void EnterNumberScreen::init(
    String theTitle, float initialValue, WTBasics::FloatValCB cb, bool decimalsAllowed,
    float minAllowed, float maxAllowed) {
  title = theTitle;
  minVal = minAllowed;
  maxVal = maxAllowed;
  allowDecimals = decimalsAllowed;
  newValueCB = cb;

  initialValue = constrain(initialValue, minVal, maxVal);
  if (allowDecimals) { formattedValue = String(initialValue); }
  else { formattedValue = String( (long)initialValue ); }

  auto buttonHandler =[&](int id, Button::PressType type) -> void {
    Log.verbose(F("In EnterNumberScreen Button Handler, id = %d, type = %d"), id, type);

    if (type > Button::PressType::NormalPress) {
      // Any long press is interpreted as a cancellation, which is handled the same as
      // no change to the initial value
      if (newValueCB) newValueCB(initialValue);
      return;
    }

    if (id == ValueButton) {
      if (newValueCB) newValueCB(formattedValue.toFloat());
      return;
    }

    String currentValue = formattedValue;
    if (id >= FirstDigitButton && id <= LastDigitButton) {
      if (formattedValue.toFloat() == 0) { formattedValue = DigitLabels[id-FirstDigitButton]; }
      else { formattedValue += DigitLabels[id-FirstDigitButton]; }
    } else if (id == DecimalButton) {
      if (!allowDecimals) return;
      if (formattedValue.indexOf('.') != -1) formattedValue += '.';
    } else if (id == BackspaceButton) {
      int len = formattedValue.length();
      if (len > 0) formattedValue.remove(len-1);
    } else if (id == PlusMinusButton && formattedValue.length() > 0) {
      if (formattedValue[0] == '-') formattedValue.remove(0, 1);
      else formattedValue = "-" + formattedValue;
    }
    float newVal = formattedValue.toFloat();
    if (newVal < minVal || newVal > maxVal) {
      formattedValue = currentValue;
      Log.verbose("Value out of range: %s", formattedValue.c_str());
      // TO DO: Blink the screen or something
    } else {
      display();
    }
  };

  uint16_t vW, bsW, x, w;
  
  w = (minVal < 0) + allowDecimals;
  if (w == 0) { vW = 3; bsW = 2; }
  else if (w == 1) { vW = 3; bsW = 2; }
  else { vW = 2; bsW = 1; }

  x = ValueXOrigin;
  w = vW*DigitWidth;
  buttons[ValueButton].init(
      x, ValueYOrigin, w, DigitHeight, buttonHandler, ValueButton);   

  x += w;
  w = bsW*DigitWidth; 
  buttons[BackspaceButton].init(
      x, ValueYOrigin, w, DigitHeight, buttonHandler, BackspaceButton);       

  x += w;
  if (allowDecimals) {
    w = DigitWidth; 
    buttons[DecimalButton].init(
        x, ValueYOrigin, w, DigitHeight, buttonHandler, DecimalButton);      
    x += w;
  }

  if (minVal < 0) {
    w = DigitWidth; 
    buttons[PlusMinusButton].init(
        x, ValueYOrigin, w, DigitHeight, buttonHandler, PlusMinusButton);
  }

  for (int i = FirstDigitButton; i <= LastDigitButton; i++) {
    buttons[i].init(
      DigitsXOrigin + ((i%5) * DigitWidth), DigitsYOrigin + (i/5) * DigitHeight,
      DigitWidth, DigitHeight, buttonHandler, i);        
  }

  nButtons = TotalButtons;
}

void EnterNumberScreen::display(bool activating) {
  if (activating) {
    tft.fillScreen(Theme::Color_Background);

    Display::Font::setUsingID(TitleFont, tft);
    tft.setTextColor(Theme::Color_AlertGood);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(title, TitleWidth/2, TitleHeight/2);

    // Draw all the static components -- everything except the value
    for (int i = FirstDigitButton; i <= LastDigitButton; i++) {
      buttons[i].drawSimple(
          DigitLabels[i-FirstDigitButton], DigitFont, DigitFrameSize,
          Theme::Color_NormalText, Theme::Color_Border,
          Theme::Color_Background, false);
    }

    buttons[BackspaceButton].drawSimple(
        "DEL", DigitFont, DigitFrameSize,
        Theme::Color_AlertError, Theme::Color_Border,
        Theme::Color_Background, false);

    if (minVal < 0) {
      buttons[PlusMinusButton].drawSimple(
          "+/-", DigitFont, DigitFrameSize,
          Theme::Color_NormalText, Theme::Color_Border,
          Theme::Color_Background, false);
    }

    if (allowDecimals) {
      buttons[DecimalButton].drawSimple(
          ".", DigitFont, DigitFrameSize,
          Theme::Color_NormalText, Theme::Color_Border,
          Theme::Color_Background, false);
    }
  }

  buttons[ValueButton].drawSimple(
    formattedValue, ValueFont, ValueFrameSize,
    Theme::Color_AlertGood, Theme::Color_Border,
    Theme::Color_Background);
}

void EnterNumberScreen::processPeriodicActivity() {  }








