#include "WeatherIcons.h"
#include "images/WeatherImages.h"
#include "images/WindIcons.h"

const uint16_t *getWeatherIcon(String iconName) {
  if (iconName == "01d") return &wi_01d[0];
  if (iconName == "01n") return &wi_01n[0];
  if (iconName == "02d") return &wi_02d[0];
  if (iconName == "02n") return &wi_02n[0];
  if (iconName == "03d") return &wi_03d[0];
  if (iconName == "03n") return &wi_03d[0];
  if (iconName == "04d") return &wi_04d[0];
  if (iconName == "04n") return &wi_04d[0];
  if (iconName == "09d") return &wi_09d[0];
  if (iconName == "09n") return &wi_09d[0];
  if (iconName == "10d") return &wi_10d[0];
  if (iconName == "10n") return &wi_10n[0];
  if (iconName == "11d") return &wi_11d[0];
  if (iconName == "11n") return &wi_11d[0];
  if (iconName == "13d") return &wi_13d[0];
  if (iconName == "13n") return &wi_13d[0];
  if (iconName == "50d") return &wi_50d[0];
  if (iconName == "50n") return &wi_50d[0];
  return &wi_01d[0];
}

const uint16_t *getWindIcon(float speed) {
  if (speed > 20) return &Wind_Strong[0];
  if (speed > 5) return &Wind_Light[0];
  return &Wind_None[0];
}
