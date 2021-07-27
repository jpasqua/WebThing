/*
 * Button:
 *    A class that manages a rectangular region as a button and handles touches
 *    and drawing the button itself
 *                    
 * NOTES:
 * o A button can be displayed as a simple rectangle with a textual label, an
 *   optional border, and background color.
 * o Buttons may also be displayed as a progress bar.
 * o Buttons may be drawn directly to the screen or they can be buffered using
 *   sprites.
 * o If they are drawn with sprites, the minimum allowable depth will be used
 *   depending on how many distinct colors are being displayed.
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  Local Includes
#include "WebThingApp/gui/Display.h"
#include "WebThingApp/gui/Theme.h"
#include "Button.h"
//--------------- End:    Includes ---------------------------------------------


using Display::tft;
using Display::sprite;

Button::Button() { }
Button::Button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, ButtonCallback callback, uint8_t id) {
  init(x, y, w, h, callback, id);
}

void Button::init(uint16_t x, uint16_t y, uint16_t w, uint16_t h, ButtonCallback callback, uint8_t id) {
  _x = x; _y = y; _w = w; _h = h; _callback = callback; _id = id;
}

bool Button::processTouch(uint16_t tx, uint16_t ty, PressType type) {
  if ((tx >= _x) && (tx < _x+_w) && (ty >= _y) && (ty < _y+_h)) { _callback(_id, type); return true; }
  return false;
}

void Button::clear(uint16_t bg) {
  tft.fillRect(_x, _y, _w, _h, bg);
}

void Button::drawSimple(
      String label, uint8_t font, uint8_t borderSize,
      uint16_t labelColor, uint16_t borderColor, uint16_t bgColor,
      bool buffer) {
  if (buffer) {
    if (labelColor == borderColor) {  // We can do this with 1bpp
      sprite->setColorDepth(1);
      sprite->createSprite(_w, _h);
      sprite->fillSprite(Theme::Mono_Background);   // Initialize the sprite

      for (int i = 0; i < borderSize; i++) {      // Draw the frame
        sprite->drawRect(i, i, _w-(2*i), _h-(2*i), Theme::Mono_Foreground);
      }
      Display::Font::setUsingID(font, sprite);
      sprite->setTextColor(Theme::Mono_Foreground);
      sprite->setTextDatum(MC_DATUM);
      sprite->drawString(label, (_w/2), (_h/2));
      
      sprite->setBitmapColor(labelColor, bgColor);
      sprite->pushSprite(_x, _y);
      sprite->deleteSprite();
    } else {                          // We need to go to 4bpp
      static const uint8_t BackgroundIndex = 0;
      static const uint8_t LabelIndex = 1;
      static const uint8_t BorderIndex = 2;
      uint16_t cmap[16];
      cmap[BackgroundIndex] = bgColor;
      cmap[LabelIndex] = labelColor;
      cmap[BorderIndex] = borderColor;

      sprite->setColorDepth(4);
      sprite->createSprite(_w, _h);
      sprite->createPalette(cmap);
      sprite->fillSprite(BackgroundIndex);   // Initialize the sprite

      for (int i = 0; i < borderSize; i++) {      // Draw the frame
        sprite->drawRect(i, i, _w-(2*i), _h-(2*i), BorderIndex);
      }
      Display::Font::setUsingID(font, sprite);
      sprite->setTextColor(LabelIndex);
      sprite->setTextDatum(MC_DATUM);
      sprite->drawString(label, (_w/2), (_h/2));
      
      sprite->pushSprite(_x, _y);
      sprite->deleteSprite();
    }
  } else {
    tft.fillRect(_x+borderSize, _y+borderSize, _w-borderSize*2, _h-borderSize*2, bgColor);
    for (int i = 0; i < borderSize; i++) {    // Draw the frame
      tft.drawRect(_x+i, _y+i, _w-(2*i), _h-(2*i), borderColor);
    }

    Display::Font::setUsingID(font, tft);
    tft.setTextColor(labelColor);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(label, _x + (_w/2), _y+(_h/2));
  }
}

void Button::drawProgress(
      float pct, String &label, uint8_t font, uint8_t borderSize,
      uint16_t labelColor, uint16_t borderColor,
      uint16_t barColor, uint16_t bgColor, String &showPct,
      bool buffer) {
  String note = (label == showPct) ? String((int)(pct*100)) + "%" : label;
  if (buffer) {
    static const uint8_t BackgroundIndex = 0;
    static const uint8_t BarIndex = 1;
    static const uint8_t TextIndex = 2;
    static const uint8_t BorderIndex = 3;
    uint16_t cmap[16];
    cmap[BackgroundIndex] = bgColor;
    cmap[BarIndex] = barColor;
    cmap[TextIndex] = labelColor;
    cmap[BorderIndex] = borderColor;

    sprite->setColorDepth(4);
    sprite->createSprite(_w, _h);
    sprite->createPalette(cmap);
    sprite->fillSprite(BackgroundIndex);

    // Draw the frame
    for (int i = 0; i < borderSize; i++) {
      sprite->drawRect(0+i, 0+i, _w-(2*i), _h-(2*i), BorderIndex);
    }

    // Draw the bar
    sprite->fillRect(borderSize, borderSize, pct*(_w-2*borderSize), (_h-2*borderSize), BarIndex);

    // Draw the overlay text
    Display::Font::setUsingID(font, sprite);
    sprite->setTextColor(TextIndex);
    sprite->setTextDatum(MC_DATUM);
    sprite->drawString(note, _w/2, _h/2);

    // Push to the display and cleanup
    sprite->pushSprite(_x, _y);
    sprite->deleteSprite();
  } else {
    // Draw the frame
    for (int i = 0; i < borderSize; i++) {
      tft.drawRect(_x+i, _y+i, _w-(2*i), _h-(2*i), borderColor);
    }

    // Draw the bar
    tft.fillRect(_x+borderSize, _y+borderSize, pct*(_w-2*borderSize), (_h-2*borderSize), barColor);

    // Draw the overlay text
    Display::Font::setUsingID(font, tft);
    tft.setTextColor(labelColor);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(note, _x+_w/2, _y+_h/2);
  }
}
