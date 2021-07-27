/*
 * Display:
 *    A set of functions on top of the base display driver (TFT_eSPI) which
 *    provide some conveniences (and efficiencies).
 *                    
 * NOTES:
 * o The Font functions are provided to work around an issue in TFT_eSPI.
 *   + It assumes that it will be used an application that only includes the header once. 
 *   + This is true for a "normal" Arduino application with all files in the top
 *     level sketch folder. These are effectively concatenated at build time into
 *     a single compilation unit.
 *   + With more sophisticated file structures, the font data gets included multiple
 *     times cause the ap size to balloon. 
 */

#ifndef Display_h
#define	Display_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
//                                  Local Includes
#include "Screen.h"
#include "FlexScreen.h"//--------------- End:    Includes ---------------------------------------------


namespace Display {
  //----- Types
  class CalibrationData {
  public:
    static const int nCalReadings = 5;
    uint16_t readings[nCalReadings];
  };


  // ----- Constants
  static const uint16_t Width = 320;
  static const uint16_t Height = 240;
  static const uint16_t XCenter = Width/2;
  static const uint16_t YCenter = Height/2;


  // ----- Data
  extern TFT_eSPI tft;
  extern TFT_eSprite* sprite;


  // ----- Functions
  void begin(bool flipDisplay, CalibrationData* calibrationData);
  void calibrate(CalibrationData* newCalibrationData);

  void setBrightness(uint8_t b);
  uint8_t getBrightness();

  uint32_t getSizeOfScreenShotAsBMP();
  void streamScreenShotAsBMP(Stream &s);


  // ----- Font-related data and Functions
  namespace Font {
    // ----- Data
    enum FontID {
      M9,  MB9,  MO9,  MBO9,
      S9,  SB9,  SO9,  SBO9,
      S12, SB12, SO12, SBO12,
      S18, SB18, SO18, SBO18,
      S24, SB24, SO24, SBO24,
      D20, D72,  D100
    };

    // ----- Functions
    void setUsingID(uint8_t fontID, TFT_eSPI& t);
    void setUsingID(uint8_t fontID, TFT_eSprite *s);
    int8_t idFromName(String fontName);
    uint8_t getHeight(uint8_t fontID);
  }

};

#endif	// Display_h