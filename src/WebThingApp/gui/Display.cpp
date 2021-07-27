/*
 * Display:
 *    A set of functions on top of the base display driver (TFT_eSPI) which
 *    provide some conveniences (and efficiencies).
 *                    
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#if defined(ESP32)
  #include <analogWrite.h>
  #define PWMRANGE 255
#endif
#include <ArduinoLog.h>
#include <WebThingBasics.h>
//                                  Local Includes
#include "Display.h"
#include "FlexScreen.h"
#include "fonts/DSEG7_Classic_Bold_22.h"
#include "fonts/DSEG7_Classic_Bold_72.h"
#include "fonts/DSEG7_Classic_Bold_100.h"
//--------------- End:    Includes ---------------------------------------------


namespace Display {
  // ----- Internal data and functions
  namespace Internal {
    uint8_t brightness = 0;     // 0-100
  }

  // ----- Public state
  TFT_eSPI tft;
  TFT_eSprite *sprite = new TFT_eSprite(&tft);


  void begin(bool flipDisplay, CalibrationData* calibrationData) {
    if (TFT_LED != -1) pinMode(TFT_LED, OUTPUT);
    setBrightness(80);
    tft.begin();
    tft.setSwapBytes(true);
    tft.setRotation(flipDisplay ? 3 : 1);
    tft.fillScreen(TFT_BLACK);
    calibrate(calibrationData);
  }

  void calibrate(CalibrationData* calibrationData) {
    uint16_t sum = 0;
    for (int i = 0; i < CalibrationData::nCalReadings; i++) {
      sum += calibrationData->readings[i];
    }
    if (sum) {
      // We've got some values, so pass it in to the tft
      Log.trace(F("Display::init: Applying calibration data from settings"));
      tft.setTouch(calibrationData->readings);
    } else {
      // All zeroes in the calibration data means that it isn't set. 
      Log.trace(F("Display::calibrate: No calibration data provided"));
    }
  }

  void setBrightness(uint8_t b) {
    if (TFT_LED == -1) return;
    if (b == Internal::brightness) return;
    Internal::brightness = b;
    int analogValue = map(Internal::brightness, 0, 100, 0, PWMRANGE);
    analogWrite(TFT_LED, analogValue);
  }

  uint8_t getBrightness() { return Internal::brightness; }

  uint32_t getSizeOfScreenShotAsBMP() {
    return (2ul * tft.width() * tft.height() + 54); // pix data + 54 byte hdr
  }

  void streamScreenShotAsBMP(Stream &s) {
    // Adapted form https://forum.arduino.cc/index.php?topic=406416.0
    byte hiByte, loByte;
    uint16_t i, j = 0;

    uint8_t bmFlHdr[14] = {
      'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0
    };
    // 54 = std total "old" Windows BMP file header size = 14 + 40
    
    uint8_t bmInHdr[40] = {
      40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 0
    };   
    // 40 = info header size
    //  1 = num of color planes
    // 16 = bits per pixel
    // all other header info = 0, including RI_RGB (no compr), DPI resolution

    uint32_t w = tft.width();
    uint32_t h = tft.height();
    unsigned long bmpSize = 2ul * h * w + 54; // pix data + 54 byte hdr
    
    bmFlHdr[ 2] = (uint8_t)(bmpSize      ); // all ints stored little-endian
    bmFlHdr[ 3] = (uint8_t)(bmpSize >>  8); // i.e., LSB first
    bmFlHdr[ 4] = (uint8_t)(bmpSize >> 16);
    bmFlHdr[ 5] = (uint8_t)(bmpSize >> 24);

    bmInHdr[ 4] = (uint8_t)(w      );
    bmInHdr[ 5] = (uint8_t)(w >>  8);
    bmInHdr[ 6] = (uint8_t)(w >> 16);
    bmInHdr[ 7] = (uint8_t)(w >> 24);
    bmInHdr[ 8] = (uint8_t)(h      );
    bmInHdr[ 9] = (uint8_t)(h >>  8);
    bmInHdr[10] = (uint8_t)(h >> 16);
    bmInHdr[11] = (uint8_t)(h >> 24);

    s.write(bmFlHdr, sizeof(bmFlHdr));
    s.write(bmInHdr, sizeof(bmInHdr));

    for (i = h; i > 0; i--) {
      byte buf[w*2];
      byte *ptr = &buf[0];
      for (j = 0; j < w; j++) {
        uint16_t rgb = tft.readPixel(j,i);  // Get pixel in rgb565 format
        
        hiByte = (rgb & 0xFF00) >> 8;   // High Byte
        loByte = (rgb & 0x00FF);        // Low Byte
        
        // RGB565 to RGB555 conversion... 555 is default for uncompressed BMP
        loByte = (hiByte << 7) | ((loByte & 0xC0) >> 1) | (loByte & 0x1f);
        hiByte = (hiByte >> 1);
        
        *ptr++ = loByte;
        *ptr++ = hiByte;
      }
      s.write(buf, w*2);
    }
  }


  // ----- Font-related data and Functions
  namespace Font {
    // ----- Private Data
    const struct  {
      const char *name;
      const GFXfont *font;
    } GFXFonts[] = {
      // ORDER MUST MATCH Display::Font::FontID enum
      {"M9",    &FreeMono9pt7b},
      {"MB9",   &FreeMonoBold9pt7b},
      {"MO9",   &FreeMonoOblique9pt7b},
      {"MBO9",  &FreeMonoBoldOblique9pt7b},

      {"S9",    &FreeSans9pt7b},
      {"SB9",   &FreeSansBold9pt7b},
      {"SO9",   &FreeSansOblique9pt7b},
      {"SBO9",  &FreeSansBoldOblique9pt7b},

      {"S12",   &FreeSans12pt7b},
      {"SB12",  &FreeSansBold12pt7b},
      {"SO12",  &FreeSansOblique12pt7b},
      {"SBO12", &FreeSansBoldOblique12pt7b},

      {"S18",   &FreeSans18pt7b},
      {"SB18",  &FreeSansBold18pt7b},
      {"SO18",  &FreeSansOblique18pt7b},
      {"SBO18", &FreeSansBoldOblique18pt7b},

      {"S24",   &FreeSans24pt7b},
      {"SB24",  &FreeSansBold24pt7b},
      {"SO24",  &FreeSansOblique24pt7b},
      {"SBO24", &FreeSansBoldOblique24pt7b},

      {"D20",   &DSEG7_Classic_Bold_20},
      {"D72",   &DSEG7_Classic_Bold_72},
      {"D100",  &DSEG7_Classic_Bold_100}
    };
    const uint8_t nGFXFonts = ARRAY_SIZE(GFXFonts);

    void setUsingID(uint8_t fontID, TFT_eSPI& t) { t.setFreeFont(GFXFonts[fontID].font); }
    void setUsingID(uint8_t fontID, TFT_eSprite *s) { s->setFreeFont(GFXFonts[fontID].font); }

    int8_t idFromName(String fontName) {
      for (int i = 0; i < nGFXFonts; i++) {
        if (fontName == GFXFonts[i].name) return i;
      }
      return -1;
    }

    uint8_t getHeight(uint8_t fontID) { return GFXFonts[fontID].font->yAdvance; }
  }

};