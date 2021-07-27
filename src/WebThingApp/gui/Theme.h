#ifndef Theme_h
#define Theme_h

#include <TFT_eSPI.h>

namespace Theme {
  #include <TFT_eSPI.h> // 
  static const uint32_t Color_Online = TFT_GREEN;
  static const uint32_t Color_Offline = TFT_RED;
  static const uint32_t Color_Progress = TFT_DARKGREEN;
  static const uint32_t Color_Inactive = TFT_DARKGREY;

  static const uint32_t Color_AlertError = TFT_RED;
  static const uint32_t Color_AlertGood = TFT_GREEN;
  static const uint32_t Color_AlertWarning = TFT_ORANGE;

  static const uint32_t Color_NormalText = TFT_WHITE;
  static const uint32_t Color_DimText = 0xAD75;       // Darker than TFT_LIGHTGREY

  static const uint32_t Color_Border = TFT_WHITE;
  static const uint32_t Color_Background = TFT_BLACK;

  static const uint32_t Color_SplashBkg  = TFT_BLACK;
  static const uint32_t Color_SplashText = TFT_WHITE;

  static const uint32_t Color_WeatherTxt = TFT_PURPLE;
  static const uint32_t Color_WeatherBkg = TFT_WHITE;

  static const uint32_t Color_WiFiBlue = 0x7D1A;

  static const uint32_t Mono_Background = 0x0000;     // Background for 1bpp sprites
  static const uint32_t Mono_Foreground = 0x0001;     // Foreground for 1bpp sprites

  static const uint32_t Color_UpdatingWeather = TFT_ORANGE;
  static const uint32_t Color_UpdatingPlugins = TFT_SKYBLUE;
  static const uint32_t Color_UpdatingText = TFT_WHITE;
  static const uint32_t Color_UpdatingFill = TFT_BLACK;
}

#endif  // Theme_h