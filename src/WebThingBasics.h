#ifndef Basics_h
#define Basics_h
#define Basics_h_Version 0.3


#include <Arduino.h>
#include <functional>

// ----- Low-level Macros
#define ALLOC(t,n) (t *) malloc((n)*sizeof(t))
#define REALLOC(p, t,n) (t *) realloc(p, (n)*sizeof(t))
#define FREE(p) { if (p) free(p); }
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

namespace WTBasics {
  // ----- Basic Types
  typedef std::function<String(String&)> StringMapper;
  typedef std::function<void(const String&, String&)> ReferenceMapper;
  typedef std::function<void(float)> FloatValCB;

  // ----- Time Related Constants
  static const uint32_t  MillisPerSecond = 1000L;
  static const uint32_t  SecondsPerMinute = 60;
  static const uint32_t  MinutesPerHour = 60;
  static const uint32_t  SecondsPerHour = (SecondsPerMinute * MinutesPerHour);
  static const uint32_t  MillisPerHour = (SecondsPerHour * MillisPerSecond);


  // String Utilities
  inline void setStringContent(String& target, const char* newContent) {
    target.clear();
    if (newContent) target.concat(newContent);
  }
  inline void resetString(String& target) {
    if (target.length()) target.setCharAt(0, 0); // In case c_str() will be used
    target.clear();
  }


  // ----- Unit conversions

  // Temperature
  inline float c_to_f(float c) { return (c * 9.0f/5.0f) + 32.0f; }
  inline float f_to_c(float f) { return (f - 32.0f) * 9.0f/5.0f; }
  inline float k_to_c(float k) { return k - 273.15f; }
  inline float c_to_k(float c) { return c + 273.15f; }

  // Barometric pressure
  inline float hpa_to_inhg(float hpa) { return hpa * 0.02953f; }
  inline float inhg_to_hpa(float inhg) { return inhg / 0.02953f; }

  // Speed
  inline float mph_to_kph(float mph) { return mph * 1.60934f; }
  inline float kph_to_mph(float kph) { return kph / 1.60934f; }

  // Length
  inline float in_to_cm(float in) { return in * 2.54f; }
  inline float cm_to_in(float cm) { return cm / 2.54f; }
  inline float in_to_ft(float in) { return in * 12.0f; }
  inline float ft_to_in(float ft) { return ft / 12.0f; }
  inline float m_to_km(float m) { return m * 1.60934f; }
  inline float km_to_m(float km) { return km / 1.60934f; }

  static String EmptyString("");
};
#endif  // Basics_h
