#ifndef WeatherIcons_h
#define WeatherIcons_h

#include <Arduino.h>

#define WI_Width (72)
#define WI_Height (72)
#define WI_Transparent (0x9999)
extern const uint16_t *getWeatherIcon(String iconName);

#define WindIcon_Width (72)
#define WindIcon_Height (72)
#define WindIcon_Transparent (0x9999)
extern const uint16_t *getWindIcon(float speed);

#endif // WeatherIcons_h