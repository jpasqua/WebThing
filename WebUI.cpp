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
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <WiFiManager.h>
#include <ESPTemplateProcessor.h>
//                                  Local Includes
#include "WebThing.h"
#include "WebUI.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespacea
namespace WebUI {
  ESP8266WebServer      *server;
  String                title;
  String                additionalMenuItems = "";
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

    String sendPageHeader(bool refresh = false) {
      auto mapper =[refresh](String &key) -> String {
        if (key == "TITLE")               return title;
        if (key == "THEME_COLOR")         return WebThing::settings.themeColor;
        if (key == "REFRESH" && refresh)  return "<meta http-equiv='refresh' content='90'>";
        if (key == "PWR_VSBL")            return WebThing::settings.displayPowerOptions ? "inline" : "none";
        if (key == "ADDED_MENU_ITEMS" && !additionalMenuItems.isEmpty()) return additionalMenuItems + "<hr>";
        return EmptyString;
      };

      templateHandler->send("/wt/Header.html", mapper);
    }

    void sendFooter() {
      auto mapper =[](String &key) -> String {
        if (key == "VERSION") return WebThing::getDisplayedVersion();
        if (key == "RSSI")  return (String(WebThing::wifiQualityAsPct()) + "%");
        if (key == "SLEEP_OVERRIDE" && WebThing::settings.useLowPowerMode) {
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
  }
  // ----- END: WebUI::Internal

  // ----- BEGIN: WebUI::Endpoints
  namespace Endpoints {
    void handleWifiReset() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace("Web Request: Handle WiFi Reset");
      WebUI::redirectHome();
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      ESP.restart();
    }

    void handleSystemReset() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.notice("Reset System Configuration");
      if (WebThing::settings.clear()) {
        WebUI::redirectHome();
        ESP.restart();
      }
    }

    void setLogLevel() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace("Web Request: Set Log Level");
      WebThing::settings.logLevel = server->arg("logLevel").toInt();
      Log.setLevel(WebThing::settings.logLevel);
      Log.verbose("New Log Level: %d", WebThing::settings.logLevel);
      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

    void updatePwrConfig() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace("Web Request: Handle Update Config");

      // ----- Power Settings
      WebThing::settings.useLowPowerMode = server->hasArg("useLowPowerMode");
      WebThing::settings.hasVoltageSensing = server->hasArg("hasVoltageSensing");
      WebThing::settings.processingInterval = server->arg("processingInterval").toInt();
      if (WebThing::settings.processingInterval < 1) WebThing::settings.processingInterval = 1;
      WebThing::settings.sleepOverridePin = server->arg("sleepOverridePin").toInt();
      WebThing::settings.voltageCalibFactor = server->arg("voltageCalibFactor").toFloat();

      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

    void updateConfig() {
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }
      Log.trace("Web Request: Handle Update Config");

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

      // ----- Sensor
      WebThing::settings.indicatorLEDInverted = server->hasArg("indicatorLEDInverted");
      WebThing::settings.indicatorLEDPin = server->arg("indicatorLEDPin").toInt();

      WebThing::settings.write();
      WebThing::Protected::configChanged();
      WebUI::redirectHome();
    }

  }

  // ----- BEGIN: WebUI::Pages
  namespace Pages {

    void displayHomePage() {
      Log.trace("Web Request: Home Page");
      if (Internal::homeHandler) { Internal::homeHandler(); return; }

      startPage();
      server->sendContent("<h1>WebThing Home</h1>");
      finishPage();
    }

    void displayLogLevel() {
      Log.trace("Web Request: Choose Log Level");
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String llTarget = "SL" + String(WebThing::settings.logLevel);

      auto mapper =[llTarget](String &key) -> String {
        if (key == llTarget) return "selected";
        return Internal::EmptyString;
      };

      startPage();
      templateHandler->send("/wt/LogLevel.html", mapper);
      finishPage();
    }

    void displayPowerConfig() {
      Log.trace("Web Request: Power Config");
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String piTarget     = "SI" + String(WebThing::settings.processingInterval);
      String sopPinTarget = "SP" + String(WebThing::settings.sleepOverridePin);

      auto mapper =[piTarget, sopPinTarget](String &key) -> String {
        if (key == sopPinTarget)  return "selected";
        if (key == piTarget)      return "selected";
        if (key == "ULPM")        return checkedOrNot[WebThing::settings.useLowPowerMode];
        if (key == "VSENSE")      return checkedOrNot[WebThing::settings.hasVoltageSensing];
        if (key == "VCF")         return WebThing::settings.vcfAsString();
        return                    Internal::EmptyString;
      };

      startPage();
      templateHandler->send("/wt/PowerForm.html", mapper);
      finishPage();
    }

    void displayConfig() {
      Log.trace("Web Request: Handle Configure");
      if (!WebUI::Internal::authentication()) { return server->requestAuthentication(); }

      String themeTarget = "SL" + WebThing::settings.themeColor;
      String ledPinTarget = "SP" + String(WebThing::settings.indicatorLEDPin);

      auto mapper =[themeTarget, ledPinTarget](String &key) -> String {
        if (key == "LAT")             return WebThing::settings.latAsString();
        if (key == "LNG")             return WebThing::settings.lngAsString();
        if (key == "ELEV")            return String(WebThing::settings.elevation);
        if (key == "GMAPS_KEY")       return WebThing::settings.googleMapsKey;
        if (key == "TZDB_KEY")        return WebThing::settings.timeZoneDBKey;
        if (key == "HOSTNAME")        return WebThing::settings.hostname;
        if (key == "SERVER_PORT")     return String(WebThing::settings.webServerPort);
        if (key == "BASIC_AUTH")      return checkedOrNot[WebThing::settings.useBasicAuth];
        if (key == "WEB_UNAME")       return WebThing::settings.webUsername;
        if (key == "WEB_PASS")        return WebThing::settings.webPassword;  
        if (key == "LED_INVERTED")    return checkedOrNot[WebThing::settings.indicatorLEDInverted];
        if (key == themeTarget)       return "selected";
        if (key == ledPinTarget)      return "selected";
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
    server = new ESP8266WebServer(WebThing::settings.webServerPort);
    templateHandler = new ESPTemplateProcessor(server);

    server->on("/config",         Pages::displayConfig);
    server->on("/configPwr",      Pages::displayPowerConfig);
    server->on("/configLogLevel", Pages::displayLogLevel);
    server->on("/systemreset",    Endpoints::handleSystemReset);
    server->on("/",               Pages::displayHomePage);

    server->on("/updateconfig",   Endpoints::updateConfig);
    server->on("/updatePwrConfig",Endpoints::updatePwrConfig);
    server->on("/setLogLevel",    Endpoints::setLogLevel);

    server->on("/forgetwifi",     Endpoints::handleWifiReset);
    server->onNotFound(redirectHome);

    server->begin();
    if (WebThing::Protected::mDNSStarted) {
      MDNS.addService("http", "tcp", WebThing::settings.webServerPort); // Advertise the web service
      Log.notice(
        "Server started at: http://%s:%d/", 
        WiFi.localIP().toString().c_str(),
        WebThing::settings.webServerPort);
    }
  }

  void setTitle(String theTitle) { title = WebThing::encodeAttr(theTitle); }

  void addMenuItems(String html) { additionalMenuItems = html; }

  void registerHandler(String path, std::function<void(void)> handler) {
    if (path.length() == 1 && path.charAt(0) == '/') {
      Internal::homeHandler = handler;
    } else {
      server->on(path, handler);
    }
  }

  ESPTemplateProcessor *getTemplateHandler() { return templateHandler; }
  ESP8266WebServer *getUnderlyingServer() { return server; }
  

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

  bool hasArg(String arg) { return server->hasArg(arg); }
  String arg(String arg) { return server->arg(arg); }

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

  void sendArbitraryContent(String type, uint32_t length, ContentProvider cp) {
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
}
// ----- END: WebUI
