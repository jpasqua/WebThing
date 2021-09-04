/*
 * WebUI:
 *    Implements a simple WebUI for a WebThing. It contains the basic
 *    structure that can be extended to incorporate "thing-specific" UI
 *                    
 * NOTES:
 *   
 * TO DO:
 * o Whenever config changes are made, we need to figure out what we need to 
 *   update AND notify the client of WebThing that it may need to make some
 *   updates as well.We need to add a notify callback for config changes. 
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#if defined(ESP8266)
  #include <FS.h>                   // Required by ESP8266WiFi.h!
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  using WebServer = ESP8266WebServer;
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <WiFiManager.h>
#include <ESPTemplateProcessor.h>
//                                  Local Includes
#include "WebThing.h"
#include "WebUI.h"
#include "ESP_FS.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespacea
namespace WebUI {
  WebServer*            server;
  String                title;
  String                additionalMenuItems = "";
  PGM_P                 coreMenuItems = nullptr;
  PGM_P                 appMenuItems = nullptr;
  PGM_P                 devMenuItems = nullptr;
  ESPTemplateProcessor  *templateHandler;
  static const String   checkedOrNot[2] = {"", "checked='checked'"};

  // ----- BEGIN: WebUI::Internal
  namespace Internal {
    String EmptyString = "";

    std::function<void(void)> homeHandler = NULL;

    bool authentication() {
      if (WebThing::settings.useBasicAuth              &&
          WebThing::settings.webUsername.length() >= 1 &&
          WebThing::settings.webPassword.length() >= 1) {
        return server->authenticate(
          WebThing::settings.webUsername.c_str(),
          WebThing::settings.webPassword.c_str());
      } 
      return true; // Authentication not required
    }

    void sendPageHeader(bool refresh = false) {
      auto mapper =[refresh](String &key) -> String {
        if (key.equals(F("TITLE")))               return title;
        if (key.equals(F("THEME_COLOR")))         return WebThing::settings.themeColor;
        if (key.equals(F("REFRESH")) && refresh)  return "<meta http-equiv='refresh' content='90'>";
        if (key.equals(F("PWR_VSBL")))            return WebThing::settings.displayPowerOptions ? "inline" : "none";
        if (key.equals(F("CORE_MENU_ITEMS"))) {
          if (coreMenuItems) return String(coreMenuItems);
          else if (!additionalMenuItems.isEmpty()) return (String(additionalMenuItems));
        }
        if (key.equals(F("APP_MENU_ITEMS")) && appMenuItems) return String(appMenuItems);
        if (key.equals(F("DEV_MENU_ITEMS")) && devMenuItems) return String(devMenuItems);
        return EmptyString;
      };

      templateHandler->send("/wt/Header.html", mapper);
    }

    void sendFooter() {
      auto mapper =[](String &key) -> String {
        if (key.equals(F("VERSION"))) return WebThing::getDisplayedVersion();
        if (key.equals(F("RSSI")))  return (String(WebThing::wifiQualityAsPct()) + "%");
        if (key.equals(F("SLEEP_OVERRIDE")) && WebThing::settings.useLowPowerMode) {
          String val = "<span style='float:right;'><i class='fa fa-bed' aria-hidden='true'></i> Sleep Mode Override: ";
          if (WebThing::isSleepOverrideEnabled()) val.concat("On");
          else val.concat("Off");
          val.concat("</span>");
          return val;
        }
        return EmptyString;
        };

      templateHandler->send("/wt/Footer.html", mapper);
    }

    void sendStringOptions(String selectedVal, String &OptionList, String &extras) {
      String theOptions = extras + OptionList;
      String target = ">"+selectedVal+"<";
      theOptions.replace(target, " selected"+target);
      sendContent(theOptions);
    }

    void sendIntOptions(int selectedVal, String &OptionList, String &extras) {
      String theOptions = extras + OptionList;
      String target = "'" + String(selectedVal) + "'";  // e.g. '7'
      theOptions.replace(target, target + " selected");
      sendContent(theOptions);
    }

    void handleNotFound() {
      Log.verbose("WebUI::handleNotFound: URI = %s", server->uri().c_str());
      redirectHome();
    }
  }
  // ----- END: WebUI::Internal

  // ----- BEGIN: WebUI::Endpoints
  namespace Endpoints {
    void handleWifiReset() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace(F("Web Request: Handle WiFi Reset"));
      WebUI::redirectHome();
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      ESP.restart();
    }

    void handleSystemReset() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.notice(F("Reset System Configuration"));
      if (WebThing::settings.clear()) {
        WebUI::redirectHome();
        ESP.restart();
      }
    }

    void setLogLevel() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace(F("Web Request: Set Log Level"));
      WebThing::settings.logLevel = server->arg("logLevel").toInt();
      Log.setLevel(WebThing::settings.logLevel);
      Log.verbose(F("New Log Level: %d"), WebThing::settings.logLevel);
      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

    void updatePwrConfig() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace(F("Web Request: Handle Update Config"));

      // ----- Power Settings
      WebThing::settings.useLowPowerMode = server->hasArg("useLowPowerMode");
      WebThing::settings.hasVoltageSensing = server->hasArg("hasVoltageSensing");
      int interval = server->arg("processingInterval").toInt();
      WebThing::settings.processingInterval = (interval < 1) ? 1 : interval;
      if (WebThing::settings.processingInterval < 1) WebThing::settings.processingInterval = 1;
      WebThing::settings.sleepOverridePin = server->arg("sleepOverridePin").toInt();
      WebThing::settings.voltageCalibFactor = server->arg("voltageCalibFactor").toFloat();

      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

    void updateConfig() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace(F("Web Request: Handle Update Config"));

      // ----- Location Settings
      WebThing::settings.lat = server->arg("lat").toFloat();
      WebThing::settings.lng = server->arg("lng").toFloat();
      WebThing::settings.elevation = server->arg("elevation").toInt();

      // ----- API Keys
      WebThing::settings.googleMapsKey = server->arg("googleMapsKey");
      WebThing::settings.timeZoneDBKey = server->arg("timeZoneDBKey");

      // ----- Webserver Settings
      WebThing::settings.hostname = server->arg("hostname");
      WebThing::settings.webServerPort = server->arg("webServerPort").toInt();
      WebThing::settings.useBasicAuth = server->hasArg("useBasicAuth");
      WebThing::settings.webUsername = server->arg("webUsername");
      WebThing::settings.webPassword = server->arg("webPassword");
      WebThing::settings.themeColor = server->arg("themeColor");

      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

  }

  // ----- BEGIN: WebUI::Pages
  namespace Pages {

    void displayHomePage() {
      Log.trace(F("Web Request: Home Page"));
      if (Internal::homeHandler) { Internal::homeHandler(); return; }

      startPage();
      server->sendContent("<h1>WebThing Home</h1>");
      finishPage();
    }

    void displayLogLevel() {
      Log.trace(F("Web Request: Choose Log Level"));
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String llTarget = "SL" + String(WebThing::settings.logLevel);

      auto mapper =[&llTarget](String &key) -> String {
        if (key == llTarget) return "selected";
        return Internal::EmptyString;
      };

      startPage();
      templateHandler->send("/wt/LogLevel.html", mapper);
      finishPage();
    }

    void displayPowerConfig() {
      Log.trace(F("Web Request: Power Config"));
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String piTarget     = "SI" + String(WebThing::settings.processingInterval);
      String sopPinTarget = "SP" + String(WebThing::settings.sleepOverridePin);

      auto mapper =[&piTarget, &sopPinTarget](String &key) -> String {
        if (key == sopPinTarget)  return "selected";
        if (key == piTarget)      return "selected";
        if (key.equals(F("ULPM")))        return checkedOrNot[WebThing::settings.useLowPowerMode];
        if (key.equals(F("VSENSE")))      return checkedOrNot[WebThing::settings.hasVoltageSensing];
        if (key.equals(F("VCF")))         return WebThing::settings.vcfAsString();
        return                    Internal::EmptyString;
      };

      startPage();
      templateHandler->send("/wt/PowerForm.html", mapper);
      finishPage();
    }

    void displayConfig() {
      Log.trace(F("Web Request: Handle Configure"));
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String themeTarget = "SL" + WebThing::settings.themeColor;

      auto mapper =[&themeTarget](String &key) -> String {
        if (key.equals(F("LAT")))          return WebThing::settings.latAsString();
        if (key.equals(F("LNG")))          return WebThing::settings.lngAsString();
        if (key.equals(F("ELEV")))         return String(WebThing::settings.elevation);
        if (key.equals(F("GMAPS_KEY")))    return WebThing::settings.googleMapsKey;
        if (key.equals(F("TZDB_KEY")))     return WebThing::settings.timeZoneDBKey;
        if (key.equals(F("HOSTNAME")))     return WebThing::settings.hostname;
        if (key.equals(F("SERVER_PORT")))  return String(WebThing::settings.webServerPort);
        if (key.equals(F("BASIC_AUTH")))   return checkedOrNot[WebThing::settings.useBasicAuth];
        if (key.equals(F("WEB_UNAME")))    return WebThing::settings.webUsername;
        if (key.equals(F("WEB_PASS")))     return WebThing::settings.webPassword;  
        if (key == themeTarget)       return "selected";
        return Internal::EmptyString;
      };

      startPage();
      templateHandler->send("/wt/ConfigForm.html", mapper);
      finishPage();
    }
  } 
  // ----- END: WebUI::Pages


  /*------------------------------------------------------------------------------
   *
   * Functions typically clled at setup() time
   *
   *----------------------------------------------------------------------------*/

  void init() {
    server = new WebServer(WebThing::settings.webServerPort);
    templateHandler = new ESPTemplateProcessor(server);

    server->on("/",               Pages::displayHomePage);
    server->on("/config",         Pages::displayConfig);
    server->on("/configPwr",      Pages::displayPowerConfig);
    server->on("/configLogLevel", Pages::displayLogLevel);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 
    server->serveStatic("/favicon.ico", *ESP_FS::getFS(), "/wt/favicon.ico");
#pragma GCC diagnostic pop      

    server->on("/updateconfig",   Endpoints::updateConfig);
    server->on("/updatePwrConfig",Endpoints::updatePwrConfig);
    server->on("/setLogLevel",    Endpoints::setLogLevel);
    server->on("/systemreset",    Endpoints::handleSystemReset);
    server->on("/forgetwifi",     Endpoints::handleWifiReset);
    
    server->onNotFound(Internal::handleNotFound);

    server->begin();
    if (WebThing::Protected::mDNSStarted) {
      MDNS.addService("http", "tcp", WebThing::settings.webServerPort); // Advertise the web service
      Log.notice(
        "Server started at: http://%s:%d/", 
        WiFi.localIP().toString().c_str(),
        WebThing::settings.webServerPort);
    }
  }

  void setTitle(const String& theTitle) { title = WebThing::encodeAttr(theTitle); }

  void addMenuItems(String html) { additionalMenuItems = html; }
  void addCoreMenuItems(PGM_P core) {  coreMenuItems = core; }
  void addAppMenuItems(PGM_P app) { appMenuItems = app; }
  void addDevMenuItems(PGM_P dev) { devMenuItems = dev; }

  void registerHandler(const char* path, std::function<void(void)> handler) {
    if (path[0] == '/' && path[1] == '\0') {
      Internal::homeHandler = handler;
    } else {
      server->on(path, handler);
    }
  }

  ESPTemplateProcessor *getTemplateHandler() { return templateHandler; }

  // Deprecated as prep for supporitng both ESP8266 and ESP32
  // Get away from exposing the underlying server object
  WebServer *getUnderlyingServer() { return server; }
  

  /*------------------------------------------------------------------------------
   *
   * Functions typically clled inside of loop()
   *
   *----------------------------------------------------------------------------*/

  void handleClient() { server->handleClient(); }

  /*------------------------------------------------------------------------------
   *
   * Functions used to compose and send pages
   *
   *----------------------------------------------------------------------------*/

  void startPage(bool refresh) {
    server->sendHeader("Cache-Control", "no-cache, no-store");
    server->sendHeader("Pragma", "no-cache");
    server->sendHeader("Expires", "-1");
    server->setContentLength(CONTENT_LENGTH_UNKNOWN);
    server->send(200, "text/html", "");
    Internal::sendPageHeader(refresh);
  }

  bool authenticationOK() {
    if (!WebUI::Internal::authentication()) { server->requestAuthentication(); return false; }
    return true;
  }

  int args()  { return server->args(); }
  bool hasArg(const String& arg) { return server->hasArg(arg); }
  const String arg(const String& name) { return server->arg(name); }
  const String arg(int i)  { return server->arg(i); }
  const String argName(int i)  { return server->argName(i); }

  void collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
    server->collectHeaders(headerKeys, headerKeysCount);
  }
  String header(const String& name) { return server-> header(name); }
  String header(int i) {return server->header(i); }
  String headerName(int i) { return server->headerName(i); }
  int headers() { return server->headers(); }
  bool hasHeader(const String& name) { return server->hasHeader(name); }

  void sendContent(const String &content) {
    static const int ChunkSize = 512;
    int length = content.length();
    if (length < ChunkSize) { server->sendContent(content); return; }
    for (int index = 0; index < length; index += ChunkSize) {
      String chunk = content.substring(index, index+ChunkSize);
      server->sendContent(chunk);
    }
  }

  void redirectHome() {
    // Send them back to the Root Directory
    server->sendHeader("Location", String("/"), true);
    server->sendHeader("Cache-Control", "no-cache, no-store");
    server->sendHeader("Pragma", "no-cache");
    server->sendHeader("Expires", "-1");
    server->send(302, "text/plain", "");
    server->client().stop();
  }

  void finishPage() {
    Internal::sendFooter();
    server->sendContent("");
    server->client().flush();
    server->client().stop();
  }

  void closeConnection(uint16_t code, String text) {
    server->send(code, "text/plain", text);
    server->client().stop();
  }

  void sendJSONContent(DynamicJsonDocument *doc) {
    auto cp = [doc](Stream &s) { serializeJsonPretty(*doc, s); };
    sendArbitraryContent("application/json", measureJsonPretty(*doc), cp);
  }

  void sendArbitraryContent(String type, int32_t length, ContentProvider cp) {
    auto client = server->client();
    client.println(F("HTTP/1.0 200 OK"));
    client.print(F("Content-Type: ")); client.println(type);
    client.println(F("Connection: close"));
    if (length > 0) {
      client.print(F("Content-Length: "));
      client.println(length);
    }
    client.println();

    cp(client);     // Send the arbitrary data
    client.stop();  // Disconnect
  }

  void sendStringContent(String type, String payload) {
    int length = payload.length();
    auto cp = [length, &payload](Stream &s) { s.write(payload.c_str(), length); };
    sendArbitraryContent(type, length, cp);
  }
}
// ----- END: WebUI
