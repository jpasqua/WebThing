

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//#include <ESP8266WiFi.h>
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <JSONService.h>
//                                  Local Includes
#include "OWMClient.h"
//--------------- End:    Includes ---------------------------------------------

static const String OWMServer = "api.openweathermap.org";
static const uint16_t OWMPort = 80;

static const String ProxyOWMServer = "192.168.1.99";
static const uint16_t ProxyOWMPort = 59641;

//static ServiceDetails owmDetails(ProxyOWMServer, ProxyOWMPort);
static ServiceDetails owmDetails(OWMServer, OWMPort);
static JSONService owmService(owmDetails);

OWMClient::OWMClient(String key, int cityID, bool useMetric, String language) :
    _key(key), _cityID(cityID), _useMetric(useMetric), _lang(language) { }

void OWMClient::update() {
  String endpoint = "/data/2.5/group?id=" + _cityID + "&units=" + (_useMetric?"metric":"imperial") + "&cnt=1&APPID=" + _key + "&lang=" + _lang;
  DynamicJsonDocument *root = owmService.issueGET(endpoint, 1024);
  if (!root) {
    Log.warning(F("Failed to update weather info"));
    weather.cached = true;
    weather.error = "Unable to get data from service";
    weather.dt = 0;
    return;
  }
  //serializeJsonPretty(*root, Serial); Serial.println();

  weather.cached = false;
  weather.error = "";

  /* ----- Sample response -----
  {
    "cnt": 1,
    "list": [
      {
        "coord": { "lon": -122.18, "lat": 37.45 },
        "sys": {
          "country": "US",
          "timezone": -25200,
          "sunrise": 1586784938,
          "sunset": 1586832124
        },
        "weather": [
          {
            "id": 800,
            "main": "Clear",
            "description": "clear sky",
            "icon": "01d"
          }
        ],
        "main": {
          "temp": 63.12, "feels_like": 60.17,
          "temp_min": 57, "temp_max": 68,
          "pressure": 1021, "humidity": 59
        },
        "visibility": 16093,
        "wind": { "speed": 4.7 },
        "clouds": { "all": 5 },
        "dt": 1586805383,
        "id": 5372223,
        "name": "Menlo Park"
      }
    ]
  }
  ** -----END: Sample Response */

  JsonObject city = (*root)["list"][0];
  weather.location.lat = city["coord"]["lat"];
  weather.location.lon = city["coord"]["lon"];
  weather.location.country = city["sys"]["country"].as<String>();
  weather.location.city = city["name"].as<String>();
  weather.location.cityID = city["id"];

  weather.time.tz = city["sys"]["timezone"];
  weather.time.sunrise = city["sys"]["sunrise"];
  weather.time.sunset = city["sys"]["sunset"];

  weather.description.basic = city["weather"][0]["main"].as<String>();
  weather.description.longer = city["weather"][0]["description"].as<String>();
  weather.description.icon = city["weather"][0]["icon"].as<String>();
  weather.description.code = city["weather"][0]["id"];

  weather.readings.temp = city["main"]["temp"];
  weather.readings.feelsLike = city["main"]["feels_like"];
  weather.readings.minTemp = city["main"]["temp_min"];
  weather.readings.maxTemp = city["main"]["temp_max"];
  weather.readings.pressure = city["main"]["pressure"];
  weather.readings.humidity = city["main"]["humidity"];
  weather.readings.windSpeed = city["wind"]["speed"];
  weather.readings.visibility = city["visibility"];
  weather.readings.cloudiness = city["clouds"]["all"];

  weather.dt = city["dt"];
  delete root;
}

void OWMClient::updateForecast(int32_t gmtOffset) {
  StaticJsonDocument<128> filter;
  filter["list"][0]["dt"] = true;
  filter["list"][0]["main"]["temp"] = true;
  filter["list"][0]["weather"][0]["icon"] = true;
  String endpoint = "/data/2.5/forecast?id=" + _cityID + "&units=" + (_useMetric?"metric":"imperial") + "&APPID=" + _key + "&lang=" + _lang;
  DynamicJsonDocument *root = owmService.issueGET(endpoint, 8192, &filter);
  if (!root) {
    Log.warning(F("Failed to retreive forecast"));
    return;
  }
  // serializeJsonPretty(*root, Serial); Serial.println();

  // The 5 forecast elements correspond to the 5-day forecast

  JsonArray list = (*root)["list"];

  int operationalDay = -1;

  uint32_t curDT;
  String   curIcon;
  float    curMin = 1000.0f;
  float    curMax = -1000.0f;
  int      dayOfCurDT;
  uint32_t timeOfMaxTemp = 0;
  int forecastIndex = 0;

  // Iterate through the JsonArray this way rather than referencing values by
  // index in a loop. This is much more efficient since the underlying JsonArray
  // is actually a linked list (see ArduinoJson doc for the recommendation)
  for (JsonObject f : list) {
    curDT = f["dt"];
    curDT += gmtOffset;
    dayOfCurDT = day(curDT);

    if (operationalDay == -1) { operationalDay = dayOfCurDT; }

    if (dayOfCurDT != operationalDay) {
      forecast[forecastIndex].dt = timeOfMaxTemp;
      forecast[forecastIndex].loTemp = curMin;
      forecast[forecastIndex].hiTemp = curMax;
      forecast[forecastIndex].icon = curIcon;
      if (++forecastIndex == ForecastElements) break;
      operationalDay = dayOfCurDT;
      curMin = 1000;
      curMax = -1000;
    }
    float curTemp = f["main"]["temp"];
    if (curTemp > curMax) {
      curMax = curTemp;
      timeOfMaxTemp = curDT;
      curIcon = f["weather"][0]["icon"].as<String>();
    }
    if (curTemp < curMin) {
      curMin = curTemp;
    }
  }
  if (forecastIndex < ForecastElements) {
    forecast[forecastIndex].dt = timeOfMaxTemp;
    forecast[forecastIndex].loTemp = curMin;
    forecast[forecastIndex].hiTemp = curMax;
    forecast[forecastIndex].icon = curIcon;
  }

  dumpForecast();
  delete root;
}

void OWMClient::dumpForecast() {
  for (int i = 0; i < ForecastElements; i++) {
    forecast[i].dumpToLog();
  }
}











