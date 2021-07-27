//#include <ESP8266WiFi.h>
#include <ArduinoLog.h>
#include <JSONService.h>
#include "BlynkClient.h"

static const String FailedRead = "";
static const String BlynkServer = "blynk-cloud.com";
static const uint16_t BlynkPort = 80;
static const uint8_t MaxFailures = 10;

static ServiceDetails blynkDetails(BlynkServer, BlynkPort);
static JSONService blynkService(blynkDetails);
static uint8_t nFailures = 0;

bool BlynkClient::readPin(String blynkAppID, String pin, String& value) {
    if (blynkAppID.isEmpty()) return false;
    if (nFailures > MaxFailures) return false;

    String endpoint = "/" + blynkAppID + "/get/" + pin;

    DynamicJsonDocument *root = blynkService.issueGET(endpoint, 256);
    if (!root) {
      Log.warning(F("BlynkClient::readPin(): issueGet failed"));
      nFailures++;
      return false;
    }
    //serializeJsonPretty(*root, Serial); Serial.println();

    value = (*root)[0].as<String>();
    delete root;

    return true;
}