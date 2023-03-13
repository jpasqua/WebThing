#include <ArduinoLog.h>
#include "WeatherSensor.h"

void WeatherSensor::calculateDerivedValues(WeatherReadings& readings, int elevation) {
  if (!isnan(readings.pressure)) {
    // Relative Pressure
    float SLpressure_hPa = (((readings.pressure * 100.0)/pow((1-((float)(elevation))/44330), 5.255))/100.0);
    readings.relPressure=(int)(SLpressure_hPa+.5);
    Log.verbose("Relative Pressure: %d hPa", readings.relPressure);
  }

  if (!isnan(readings.temp) && !isnan(readings.humidity)) {
    // Calculate dewpoint
    double a = 17.271;
    double b = 237.7;
    double tempcalc = (a * readings.temp) / (b + readings.temp) + log(readings.humidity*0.01);
    readings.dewPointTemp = (b * tempcalc) / (a - tempcalc);

    // Calculate dewpoint spread (difference between actual temp and dewpoint -> the smaller the number: rain or fog
    readings.dewPointSpread = readings.temp - readings.dewPointTemp;

    // Calculate Heat Index (readings.heatIndex in °C) --> Valid for temps above 26.7 °C
    if (readings.temp > 26.7) {
      double c1 = -8.784, c2 = 1.611, c3 = 2.338, c4 = -0.146, c5= -1.230e-2, c6=-1.642e-2, c7=2.211e-3, c8=7.254e-4, c9=-2.582e-6  ;
      double T = readings.temp;
      double R = readings.humidity;
      
      double A = (( c5 * T) + c2) * T + c1;
      double B = ((c7 * T) + c4) * T + c3;
      double C = ((c9 * T) + c8) * T + c6;
      readings.heatIndex = (C * R + B) * R + A;
    } else {
      readings.heatIndex = readings.temp;
    }
  }
}

void WeatherSensor::applyCorrections(WeatherReadings& readings, float t, float h) {
  readings.temp += t;
  readings.humidity = constrain(readings.humidity + h, 0.0, 100.0);
}
