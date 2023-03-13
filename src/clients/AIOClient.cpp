/*
 * AIOClient
 *    Amenities for sending and receiving data from AdafruitIO
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <JSONService.h>
//                                  Local Includes
#include "AIOClient.h"
//--------------- End:    Includes ---------------------------------------------

static constexpr char AIOHost[] = "io.adafruit.com";
static constexpr uint16_t AIOPort = 80;
static constexpr int ValueResponseSize = 256;
static constexpr const char* baseURL = "/api/v2/jpasqua/feeds/";
StaticJsonDocument<128> filter;


String AIOClient::makeEndpoint(const char* feedName) {
  String endpoint = endpointRoot;

  if (!strchr(feedName, '.')) {  // It's not fully qualified
    if (dfltGroup) {
      // An empty default group is a request to use the "global" default group which
      // means no qualifier is required.
      if (dfltGroup[0] != '\0') { 
        endpoint.concat(dfltGroup);
        endpoint.concat('.');
      }
    }
    else {
      Log.warning("Default group has not been set, but unqualified name (%s) supplied", feedName);
      return String(""); // dfltGroup has not been set!
    }
  }
  endpoint.concat(feedName);
  endpoint.concat("/data");
  return endpoint;
}

bool AIOClient::init(const char* username, const char* key) {
  if (service != NULL) return true;  // Already initialized
  initialized = false;
  if (username[0] == '\0' || key[0] == '\0') {
    Log.warning("AIOClient::init: username or key is empty - can't initialize");
    return false;
  }

  ServiceDetails details(AIOHost, AIOPort, "", "", key, "X-AIO-Key");
  service = new JSONService(details);

  endpointRoot = "/api/v2/";
  endpointRoot.concat(username);
  endpointRoot.concat("/feeds/");
  filter["value"] = true;

  return true;
}

void AIOClient::setDefaultGroup(const char* dfltGroup) {
  this->dfltGroup = dfltGroup;
}

bool AIOClient::get(const char* feedName, String& into) {
  String endpoint = makeEndpoint(feedName);
  if (endpoint.isEmpty()) return false;
  endpoint += "/last";

  DynamicJsonDocument *root = service->issueGET(endpoint, ValueResponseSize, &filter);
  if (!root) {
    Log.error("The issueGet() call failed!");
    return false;
  }

  // serializeJsonPretty(*root, Serial); Log.verbose("\n");
  into = (*root)["value"].as<const char*>();
  delete root;
  // Log.verbose("AIOClient::get: %s = %s", endpoint.c_str(), into.c_str());
  return true;
}

bool AIOClient::set(const char* feedName, const char *value) {
  String endpoint = makeEndpoint(feedName);
  if (endpoint.isEmpty()) return false;

  String payload = "{\"value\": \"";
  payload.concat(value);
  payload.concat("\"}");

  DynamicJsonDocument *root = service->issuePOST(
      endpoint, ValueResponseSize, payload.c_str(), &filter);
  if (!root) {
    Log.error("AIOClient::set: issuePOST() failed!");
    return false;
  }

  // serializeJsonPretty(*root, Serial); Log.verbose("\n");
  delete root;  // It is the caller's job to delete the JSON object
  // Log.verbose("AIOClient::set: endpoint: %s, payload = %s", endpoint.c_str(), payload.c_str());
  return true;
}

bool AIOClient::set(const char* feedName, char *value) { return set(feedName, (const char*)value); }
bool AIOClient::set(const char* feedName, bool value) {
  return set(feedName, value ? "1" : "0");
}
bool AIOClient::set(const char* feedName, String value) {
  return set(feedName, (const char*)value.c_str());
}
bool AIOClient::set(const char* feedName, int value) {
  return set(feedName, (const char*)((String(value)).c_str()));
}
bool AIOClient::set(const char* feedName, unsigned int value) {
  return set(feedName, (const char*)((String(value)).c_str()));
}
bool AIOClient::set(const char* feedName, long value) {
  return set(feedName, (const char*)((String(value)).c_str()));
}
bool AIOClient::set(const char* feedName, unsigned long value) {
  return set(feedName, (const char*)((String(value)).c_str()));
}
bool AIOClient::set(const char* feedName, float value, int precision) {
  return set(feedName, (const char*)((String(value, precision)).c_str()));
}
bool AIOClient::set(const char* feedName, double value, int precision) {
  return set(feedName, (const char*)((String(value, precision)).c_str()));
}



