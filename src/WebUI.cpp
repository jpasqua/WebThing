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
  #include <ESP8266HTTPClient.h>
  using WebServer = ESP8266WebServer;
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <map>
  #include <WiFi.h>
  #include <WebServer.h>
  #include <HTTPClient.h>
  #include <ESPmDNS.h>
  #include <detail/mimetable.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <BPABasics.h>
#include <WiFiManager.h>
#include <ESP_FS.h>
#include <BPABasics.h>
#include <ESPTemplateProcessor.h>
//                                  Local Includes
#include "WebThing.h"
#include "WebUI.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespace
namespace WebUI {

  WebServer* server;
  String     title;
  String     additionalMenuItems = "";
  const __FlashStringHelper*  coreMenuItems = nullptr;
  const __FlashStringHelper*  appMenuItems = nullptr;
  const __FlashStringHelper*  devMenuItems = nullptr;
  ESPTemplateProcessor  *templateHandler;

  // ----- BEGIN: WebUI::Internal
  namespace Internal {
    String EmptyString = "";
    String uploadPath = "";

    std::function<void(bool)> busyCallback = nullptr; 

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
      auto mapper =[refresh](const String &key, String& val) -> void {
        if (key.equals(F("TITLE"))) val = title;
        else if (key.equals(F("THEME_COLOR"))) val = WebThing::settings.themeColor;
        else if (key.equals(F("REFRESH")) && refresh) val = "<meta http-equiv='refresh' content='90'>";
        else if (key.equals(F("PWR_VSBL"))) val = WebThing::settings.displayPowerOptions ? "inline" : "none";
        else if (key.equals(F("CORE_MENU_ITEMS"))) {
          if (coreMenuItems) val = coreMenuItems;
          else if (!additionalMenuItems.isEmpty()) val = additionalMenuItems;
        }
        else if (key.equals(F("APP_MENU_ITEMS")) && appMenuItems) val = appMenuItems;
        else if (key.equals(F("DEV_MENU_ITEMS")) && devMenuItems) val = devMenuItems;
      };

      templateHandler->send("/wt/Header.html", mapper);
    }

    void sendFooter() {
      auto mapper =[](const String &key, String& val) -> void {
        if (key.equals(F("VERSION"))) val = WebThing::getDisplayedVersion();
        else if (key.equals(F("RSSI"))) { val.concat(WebThing::wifiQualityAsPct()); val.concat('%'); }
        else if (key.equals(F("SLEEP_OVERRIDE")) && WebThing::settings.useLowPowerMode) {
          val = "<span style='float:right;'><i class='fa fa-bed' aria-hidden='true'></i> Sleep Mode Override: ";
          if (WebThing::isSleepOverrideEnabled()) val.concat("On");
          else val.concat("Off");
          val.concat("</span>");
        }
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

    void updateAdvSettings() {
      auto action = []() {
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
      };

      wrapWebAction("updatePwrConfig", action, true);
    }

    void updateConfig() {
      auto action = []() {
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
      };

      wrapWebAction("updateConfig", action, true);
    }

    void completeUpload() {
      String dest = server->arg("targetName");
      // Log.verbose("completeUpload: target: %s", dest.c_str());
      if (ESP_FS::move(Internal::uploadPath.c_str(), dest.c_str())) {
        Log.verbose("%s was moved to %s", Internal::uploadPath.c_str(), dest.c_str());
      } else {
        Log.verbose("Failed moving %s to %s", Internal::uploadPath.c_str(), dest.c_str());
      }
      Internal::uploadPath.clear();
      redirectHome();
    }

    void handleUpload() {
      static File fsUploadFile;
      HTTPUpload& upload = server->upload();

      if (upload.status == UPLOAD_FILE_START) {
        // We're just getting started. Get the file name and open/create the file
        Internal::uploadPath = upload.filename;
        if (Internal::uploadPath.startsWith("/")) { Internal::uploadPath = "/tmp"+Internal::uploadPath; }
        else { Internal::uploadPath = "/tmp/"+Internal::uploadPath; }
        Log.trace("handleUpload: Final uploadTarget = %s", Internal::uploadPath.c_str());
        fsUploadFile = ESP_FS::open(Internal::uploadPath, "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        // There is data available, write it
        if (fsUploadFile) {
          fsUploadFile.write(upload.buf, upload.currentSize);
          Log.trace("handleUpload: Received %d bytes", upload.currentSize);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        // We're done. Close the file and send a response
        if (fsUploadFile) {
          fsUploadFile.close();
          Log.trace("Upload succeeded, file size: %d", upload.totalSize);
          server->sendHeader("Location", "/");  // Redirect the client to the home page
          server->send(303, "text/plain", "Redirecting...");
        } else {
          server->send(500, "text/plain", "500: couldn't create file");
        }
        server->client().stop();
      }
    }

#if defined(ESP32)
    String getContentType(String& filename) {
      for (mime::Entry e : mime::mimeTable) {
        if (filename.endsWith(e.endsWith)) { return e.mimeType; }
      }
      return mime::mimeTable[mime::maxType-1].mimeType;
    } 
#elif defined(ESP8266)
    String getContentType(String& filename) { return mime::getContentType(filename); } 
#endif

    void displayFileContent() {
      auto action = []() {

        String filename = server->arg("file");
        if (filename.isEmpty()) {
          server->send(404, "text/plain", "404: Requested filename is empty");
          server->client().stop();
        }

        File f = ESP_FS::open(filename, "r");
        if (!f) {
          server->send(404, "text/plain", "404: Requested file does not exist");
          server->client().stop();
        }

        String contentType = getContentType(filename);
        if (server->streamFile(f, contentType) != f.size()) {
          Log.warning("Sent less data than expected!");
        }
        server->client().stop();
        f.close();
      };

      wrapWebAction("fileList", action, true);
    }
    
    struct {
      const char* typeName;
      const char* header;
    } typeMap[] = {
      {"json",   "application/json; charset=utf-8"},
      {"html",   "text/html; charset=utf-8"},
      {"text",   "text/plain; charset=utf-8"},
      {"binary", "application/octet-stream"}      // Default type must be last
    };

    // Map an abbreviated mime-type into an HTTP response header
    // E.g. json -> application/json; charset=utf-8
    const char* mapType(const char* type) {
      for (int i = 0; i < countof(typeMap); i++) {
        if (strcmp(type, typeMap[i].typeName) == 0) return typeMap[i].header;
      }
      return typeMap[countof(typeMap)-1].header;
    }

    //
    // Perform a GET on the "srcURL" arg and stream back the results.
    // Set the Content-Type header to a vlue corresponding to the "type" arg
    // Sample invocation:
    //   pass?srcURL=http://newsapi.org/v2/top-headlines?sources=abc-news%26apiKey=KEY&type=json
    //
    void pass() { 
      String srcURL = server->arg("srcURL");
      const char* type = mapType(server->arg("type").c_str());

      WiFiClient client;
      HTTPClient httpSrc;  // Must be declared after client for correct destruction

      httpSrc.begin(client, srcURL);
      int httpCode = httpSrc.GET();

      if (httpCode <= 0) {
        Log.warning("[HTTP] GET... failed, error: %s\n", httpSrc.errorToString(httpCode).c_str());
        httpSrc.end();
        return;
      }

      // HTTP header has been sent and source server response header has been handled
      // Log.trace("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        int len = httpSrc.getSize(); // -1 when source server sends no Content-Length header
        uint8_t buffer[512];

        WiFiClient* src = &client;
        auto dest = server->client();

        // Send headers
        dest.write("HTTP/1.1 200 OK\r\n");
        dest.write("Content-Type: "); dest.write(type);
        dest.write("Content-Type: "); dest.write(type);
        dest.write("Cache-Control: max-age=3600");
        dest.write("\r\n\r\n");

        // Read from the source and write to the dest
        while (httpSrc.connected() && (len > 0 || len == -1)) {
          int bytesRead = src->readBytes(buffer, std::min((size_t)len, sizeof(buffer)));
          // Log.verbose("bytesRead: %d", bytesRead);
          if (!bytesRead) {
            Log.warning("Timeout while reading from %s", srcURL.c_str());
            break;
          }
          else dest.write(buffer, bytesRead);
          if (len > 0) { len -= bytesRead; }
        }
        dest.stop();
        Log.trace("Finished passing through %s", srcURL.c_str());
      }

      httpSrc.end();
    }

    void handleFileList() {
      auto action = []() {

        ESP_FS::DirEnumerator* de = ESP_FS::newEnumerator();
        if (!de->begin("/")) {
          Log.warning("Unable to enumerate /");
          delete de;
          server->send(404, "text/plain", "404: Unable to enumerate /");
          server->client().stop();
          return;
        }

        startPage();

        String path;
        String content(128);
        server->sendContent("<h1>File System Listing</h1><ul style='list-style-type:none;'>");
        while (de->next(path)) {
          content.clear();
          // Log.verbose("Found file: %s", path.c_str());
          String content = "<li><a href='uploadPage?targetName=";
          content += path;
          content += "'><i class='fa fa-upload'></i></a>&nbsp;<a href='/content?file=";
          content += path;
          content += "'>";
          content += path;
          content += "</a></li>";
          server->sendContent(content);
        }
        delete de;

        server->sendContent("</ul>");

        finishPage();
      };

      wrapWebAction("fileList", action, true);
    }

  } // ----- END: WebUI::Endpoints



  // ----- BEGIN: WebUI::Pages
  namespace Pages {

    void displayHomePage() {
      auto action = []() {
        startPage();
        server->sendContent("<h1>WebThing Home</h1>");
        finishPage();
      };

      wrapWebAction("/", action, true);
    }

    void displayUploadPage() {
      String target = server->arg("targetName");
      String accept = "";
      int length = target.length();
      if (length) {
        int dotIndex = target.lastIndexOf('.');
        if (dotIndex != -1 && dotIndex != length-1) {
          // We have a file extension
          accept = "accept='";
          accept += target.substring(dotIndex);
          accept += "'";
        }
      }

      auto mapper =[&target, &accept](const String &key, String& val) -> void {
        if (key == "TARGET_NAME") val = target;
        if (key == "ACCEPT") val = accept;
      };

      wrapWebPage("/uploadPage", "/wt/UploadPage.html", mapper);
    }

    void displayAdvSettings() {
      String piTarget     = "SI" + String(WebThing::settings.processingInterval);
      String sopPinTarget = "SP" + String(WebThing::settings.sleepOverridePin);

      auto mapper =[&piTarget, &sopPinTarget](const String &key, String& val) -> void {
        if (key == sopPinTarget) val = "selected";
        else if (key == piTarget) val = "selected";
        else if (key.equals(F("ULPM"))) val = checkedOrNot[WebThing::settings.useLowPowerMode];
        else if (key.equals(F("VSENSE"))) val = checkedOrNot[WebThing::settings.hasVoltageSensing];
        else if (key.equals(F("VCF"))) val = WebThing::settings.vcfAsString();
        else if (key.equals(F("PWR_VSBL"))) val = WebThing::settings.displayPowerOptions ? "inline" : "none";
      };

      if (Internal::busyCallback) Internal::busyCallback(false);
      wrapWebPage("/displayPowerConfig", "/wt/AdvSettings.html", mapper);
    }

    void displayConfig() {
      String themeTarget = "SL" + WebThing::settings.themeColor;

      auto mapper =[&themeTarget](const String &key, String& val) -> void {
        if (key.equals(F("LAT")))               val = WebThing::settings.latAsString();
        else if (key.equals(F("LNG")))          val = WebThing::settings.lngAsString();
        else if (key.equals(F("ELEV")))         val.concat(WebThing::settings.elevation);
        else if (key.equals(F("GMAPS_KEY")))    val = WebThing::settings.googleMapsKey;
        else if (key.equals(F("TZDB_KEY")))     val = WebThing::settings.timeZoneDBKey;
        else if (key.equals(F("HOSTNAME")))     val = WebThing::settings.hostname;
        else if (key.equals(F("SERVER_PORT")))  val.concat(WebThing::settings.webServerPort);
        else if (key.equals(F("BASIC_AUTH")))   val = checkedOrNot[WebThing::settings.useBasicAuth];
        else if (key.equals(F("WEB_UNAME")))    val = WebThing::settings.webUsername;
        else if (key.equals(F("WEB_PASS")))     val = WebThing::settings.webPassword;  
        else if (key == themeTarget)            val = "selected";
      };

      wrapWebPage("/displayConfig", "/wt/ConfigForm.html", mapper);
    }
  } 
  // ----- END: WebUI::Pages


  /*------------------------------------------------------------------------------
   *
   * Functions typically called at setup() time
   *
   *----------------------------------------------------------------------------*/

  void init() {
    server = new WebServer(WebThing::settings.webServerPort);
    templateHandler = new ESPTemplateProcessor(server);

    registerHandler("/",               Pages::displayHomePage);
    registerHandler("/config",         Pages::displayConfig);
    registerHandler("/advSettings",    Pages::displayAdvSettings);
    registerHandler("/uploadPage",     Pages::displayUploadPage);

    registerStatic("/favicon.ico", "/wt/favicon.ico");
    registerStatic("/favicon-16x16.png", "/wt/favicon-16x16.png");
    registerStatic("/favicon-32x32.png", "/wt/favicon-32x32.png");
    registerStatic("/apple-touch-icon.png", "/wt/favicon.ico");

    registerHandler("/updateconfig",   Endpoints::updateConfig);
    registerHandler("/updatePwrConfig",Endpoints::updateAdvSettings);
    registerHandler("/systemreset",    Endpoints::handleSystemReset);
    registerHandler("/forgetwifi",     Endpoints::handleWifiReset);
    registerHandler("/fslist",         Endpoints::handleFileList);
    registerHandler("/content",        Endpoints::displayFileContent);
    registerHandler("/pass",           Endpoints::pass);
    
    server->onNotFound(Internal::handleNotFound);

    server->on( // Handle file uploads
      "/upload", HTTP_POST, Endpoints::completeUpload, Endpoints::handleUpload );

    Dev::init();

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
  void addCoreMenuItems(const __FlashStringHelper* core) { coreMenuItems = core; }
  void addAppMenuItems(const __FlashStringHelper* app) { appMenuItems = app; }
  void addDevMenuItems(const __FlashStringHelper* dev) { devMenuItems = dev; }


  std::map<String, std::function<void(void)>> handlers;

  using Handler = std::function<void(void)>;

  void indirectHandler() {
    auto handler = handlers.find(server->uri());
    if (handler == handlers.end()) {
      Internal::handleNotFound();
      return;
    }
    handler->second();
  }

  void registerHandler(const String& path, Handler handler) {
    if (handlers.find(path) == handlers.end()) {
      server->on(path, indirectHandler);
    } else {
      Log.verbose("Replacing URL handler for %s", path.c_str());
    }
    handlers[path] = handler;
  }

  void registerStatic(const char* uri, const char* filePath) {
    server->serveStatic(uri, *ESP_FS::getFS(), filePath);
  }

  void registerBusyCallback(std::function<void(bool)> bc) {
    Internal::busyCallback = bc;
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

  void wrapWebAction(const char* actionName, std::function<void(void)> action, bool showStatus) {
    Log.trace(F("Handling %s"), actionName);
    if (!WebUI::authenticationOK()) { return; }

    if (showStatus && Internal::busyCallback) Internal::busyCallback(true);
    action();
    if (showStatus && Internal::busyCallback) Internal::busyCallback(false);
  }

  void wrapWebPage(
      const char* pageName, const char* htmlTemplate,
      ESPTemplateProcessor::Mapper mapper,
      bool showStatus)
  {
    Log.trace(F("Handling %s"), pageName);
    if (!WebUI::authenticationOK()) { return; }

    if (showStatus && Internal::busyCallback) Internal::busyCallback(true);
    WebUI::startPage();
    templateHandler->send(htmlTemplate, mapper);
    WebUI::finishPage();
    if (showStatus && Internal::busyCallback) Internal::busyCallback(false);
  }

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

  const String OKReponse = "200 OK";

  void sendJSONContent(DynamicJsonDocument *doc, const String& code) {
    auto cp = [doc](Stream &s) { serializeJsonPretty(*doc, s); };
    sendArbitraryContent("application/json", measureJsonPretty(*doc), cp, code);
  }

  void sendArbitraryContent(String type, int32_t length, ContentProvider cp, const String& code) {
    auto client = server->client();
    client.print("HTTP/1.0 "); client.println(code);
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

  void sendStringContent(String type, String payload, const String& code) {
    int length = payload.length();
    auto cp = [length, &payload](Stream &s) { s.write(payload.c_str(), length); };
    sendArbitraryContent(type, length, cp, code);
  }
}
// ----- END: WebUI
