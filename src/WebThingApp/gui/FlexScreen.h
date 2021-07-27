/*
 * FlexScreen:
 *    Display values driven by a screen layout definition 
 *
 * NOTES:
 * o FlexScreen is central to the Plugin system for WebThing applications
 * o It creates a Screen driven by a definition given in a JSON document
 * o That document specifies the layout of the elements on the screen
 *   and the data that should be used to fill those elements
 * o The data is accessed via the WebThing DataBroker. The JSON document
 *   provides the keys to be used and FlexScreen uses the DataBroker to
 *   get values associated with the keys at runtime.
 * o There is only one user interaction defined for a FlexScreen. Touching
 *   anywhere on the screen invoked the ButtonDelegate supplied when the
 *   FlexScreen is instantiated.
 */

#ifndef FlexScreen_h
#define FlexScreen_h


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <WebThingBasics.h>
//                                  WebThing Includes
//                                  Local Includes
#include "Screen.h"
#include "Button.h"
//--------------- End:    Includes ---------------------------------------------


class FlexItem {
public:
  typedef enum {INT, FLOAT, STRING, BOOL, CLOCK, STATUS} Type;

  void fromJSON(JsonObjectConst& item);

  uint16_t _x, _y;    // Location of the field
  uint16_t _w, _h;    // Size of the field
  uint16_t _xOff;     // x offset of text within field
  uint16_t _yOff;     // y offset of text within field
  int8_t   _gfxFont;  // The ID of the GFXFont to use. If negative, use a built-in font
  uint8_t  _font;     // Font to use if no GFXFont was given
  uint16_t _color;    // Color to use
  String   _format;   // Format string to use when displaying the value
  uint8_t  _datum;    // Justification of the output

  uint8_t _strokeWidth; 

  String _key;        // The key that will be used to get the value
  Type _dataType;

  void display(uint16_t bkg, WTBasics::ReferenceMapper mapper);
};


class FlexScreen : public Screen {
public:
  static void setButtonDelegate(Button::ButtonCallback delegate) { _buttonDelegate = delegate; }

  // ----- Functions that are specific to FlexScreen
  virtual ~FlexScreen();

  bool init(
      JsonObjectConst& screen,
      uint32_t refreshInterval,
      const WTBasics::ReferenceMapper &mapper);
  String getScreenID() { return _screenID; }

  // ----- Functions defined in Screen class
  void display(bool activating = false);
  virtual void processPeriodicActivity();


private:
  static   Button::ButtonCallback _buttonDelegate;

  FlexItem* _items;             // An array of items on the screen
  uint8_t   _nItems;            // Number of items
  uint16_t  _bkg;               // Background color of the screen
  String    _screenID;          // UUID of the screen. Not used in the UI, but human readable
                                // is helpful for testing / debugging
  uint32_t _refreshInterval;    // How often to refresh the display
  WTBasics::ReferenceMapper _mapper; // Maps a key from thee screen definition to a value
  uint32_t  lastDisplayTime;    // Last time the display() function ran
  uint32_t  lastClockTime;
  FlexItem* _clock;

  bool fromJSON(JsonObjectConst& screen);
};

#endif  // FlexScreen_h
