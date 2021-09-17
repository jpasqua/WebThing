/*
 * THPReadings
 *     Store values for Temperature, Humidty, and Pressure readings and calculate
 *     various other derived values from them.
 *
 * NOTES:
 * o This class can internalize itself from JSON and externalize itself to
 *   JSON as defined in the Serializable interface.
 * o Only the core T, H, & P values are externalized since the others can
 *   be re-rederived from them.
 *
 */

#ifndef THPReadings_h
#define THPReadings_h

#include <Serializable.h>
#include <ArduinoJson.h>

class THPReadings : public Serializable {
public:
  float     temp;             // A value < -500 implies no data is available
  float     humidity;         // A value < 0 implies no data is available
  float     pressure;         // A value < 0 implies no data is available
  double    dewPointTemp;     // A value < -500 implies no data is available
  float     dewPointSpread;   // A value < -500 implies no data is available
  float     heatIndex;        // A value < -500 implies no data is available
  uint32_t  timestamp;        // When was this reading taken?

  THPReadings() = default;

  THPReadings(float t, float h, float p) : 
      temp(t), humidity(h), pressure(p)
  {
    calculateDerivedValues();
    timestamp = millis();
  }
  
  void calculateDerivedValues() {
    // Dewpoint
    double a = 17.271;
    double b = 237.7;
    double tempcalc = (a * temp) / (b + temp) + log(humidity*0.01);
    dewPointTemp = (b * tempcalc) / (a - tempcalc);

    // Dewpoint Spread (difference between actual temp and dewpoint)
    dewPointSpread = temp - dewPointTemp;

    // Heat Index. Only applies above 26.7 °C
    if (temp > 26.7) {
      double c1 = -8.784, c2 = 1.611, c3 = 2.338, c4 = -0.146, c5= -1.230e-2, c6=-1.642e-2, c7=2.211e-3, c8=7.254e-4, c9=-2.582e-6  ;
      double T = temp;
      double R = humidity;
      
      double A = (( c5 * T) + c2) * T + c1;
      double B = ((c7 * T) + c4) * T + c3;
      double C = ((c9 * T) + c8) * T + c6;
      heatIndex = (C * R + B) * R + A; 
    } else {
      heatIndex = temp;
    }
  }

  /*------------------------------------------------------------------------------
   *
   * Implementation of the Serializable interface
   *
   *----------------------------------------------------------------------------*/

  /*
      JSON externalization is of the form:
      {
        "temp": 18.8,
        "humidity": 20,
        "pressure": 1020,
        "timestamp": 2345678
      }
  */

  virtual void internalize(const JsonObjectConst &obj) {
    temp = obj["temp"];
    humidity = obj["humidity"];
    pressure = obj["pressure"];
    timestamp = obj["timestamp"];
    calculateDerivedValues();
  }

  virtual void externalize(Stream& writeStream) const {
    StaticJsonDocument<128> doc; // Size provided by https://arduinojson.org/v6/assistant
    doc["temp"] = temp;
    doc["humidity"] = humidity;
    doc["pressure"] = pressure;
    serializeJson(doc, writeStream);
  }

};

#endif // THPReadings.h