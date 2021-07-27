/*
 * Button.h:
 *    A touch-based button class.
 *                    
 * Consider:
 * o Provide specializations of this class that implement
 *   button drawing in addition to handling the presses.
 *   + The subclasses would know how to draw themselves directly to the
 *     display and also to a sprite with appropriate depth and then copy
 *     themselves to the display.
 *   + Anticipated subclasses are:
 *     - SimpleButton: A border and a label. It would be able to display
 *       itself using a 1bpp sprite. The border and label would be one
 *       color and the button fill would be a second color
 *     - ProgressButton: A button that displays a border, a progress bar
 *       within the border, and a percentage OR label. This would require
 *       a 4bpp sprite.
 *     - Region: A button that could draw it's border, but would delegate
 *       more complex content to a drawHelper()
 * o Add double-taps to the types of presses.
 *
 * COMPLETE:
 *
 */

#ifndef Button_h
#define Button_h

#include <functional>

class Button {
public:
  typedef enum {NormalPress, LongPress, VeryLongPress} PressType;
  typedef std::function<void(int, PressType)> ButtonCallback;
  
  static const uint32_t LongPressInterval = 500;
  static const uint32_t VeryLongPressInterval = 1000;
  
  uint16_t _x;
  uint16_t _y;
  uint16_t _w;
  uint16_t _h;

  Button();
  Button(uint16_t x, uint16_t y, uint16_t w, uint16_t h, ButtonCallback callback, uint8_t id);

  void init(uint16_t x, uint16_t y, uint16_t w, uint16_t h, ButtonCallback callback, uint8_t id);

  bool processTouch(uint16_t tx, uint16_t ty, PressType type);

  void clear(uint16_t bg);

  // Draw a simple button with a border, label, and interior background
  // The button can be drawn directly to the display or to a sprite,
  // then copied to the display to avoid flickering. If buffering is on, a 1bpp
  // buffer will be used if possible (when labelColor == borderColor), otherwise
  // a 4bpp buffer will be used.
  // 
  // @param label       The text to be displayed. It is the callers responsibility to ensure
  //                    it will fit within the bounds of the button. It will be drawn middle center
  // @param font        The font that will be used to draw the label
  // @param borderSize  The size in pixels of the border
  // @param labelColor  The color to be used to draw the label
  // @param borderColor The color to be used to draw the border
  // @param bgColor     The color to be used to draw the interior of the button
  // @param buffer      Should this be buffered offscreen then copied to the display
  void drawSimple(
      String label, uint8_t font, uint8_t borderSize,
      uint16_t labelColor, uint16_t borderColor, uint16_t bgColor,
      bool buffer = false);

  // Draw a button whose interior represents a progress bar.
  // The button can be drawn directly to the display or to a 4bpp sprite,
  // then copied to the display to avoid flickering. 
  // 
  // @param pct         The progess to be reflected in the bar (0.0-1.0).
  // @param label       The text to be displayed. It is the callers responsibility to ensure
  //                    it will fit within the bounds of the button. It will be drawn middle center.
  //                    If label is equal to showPct, then the pct will be drawn rather than the label
  // @param font        The font that will be used to draw the label
  // @param borderSize  The size in pixels of the border
  // @param labelColor  The color to be used to draw the label
  // @param borderColor The color to be used to draw the border
  // @param barColor    The color to be used to draw the progress bar
  // @param bgColor     The color to be used for the unfilled part of the progress bar
  // @param buffer      Should this be buffered offscreen then copied to the display
  void drawProgress(
        float pct, String &label, uint8_t font, uint8_t borderSize,
        uint16_t labelColor, uint16_t borderColor,
        uint16_t barColor, uint16_t bgColor, String &showPct,
        bool buffer = false);

private:
  ButtonCallback _callback;
  uint8_t _id;  
};

#endif  // Button_h