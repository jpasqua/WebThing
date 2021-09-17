/*
 * WebUI:
 *    Implements a simple WebUI for a WebThing. It contains the basic
 *    structure that can be extended to incorporate "thing-specific" UI
 *
 */

#ifndef WebUI_h
#define WebUI_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  using WebServer = ESP8266WebServer;
#elif defined(ESP32)
  #include <WebServer.h>
#else
  #error "Must be an ESP8266 or ESP32"
#endif
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <ESPTemplateProcessor.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


namespace WebUI {
  // ----- Setup functions
  void init();
    // Call only once to start the web interface
  void setTitle(const String& theTitle);
    // Can be called at any time to update the title used in html pages
    // as well as the header for built-in pages
  void registerHandler(const char* path, std::function<void(void)> handler);
    // Add a handler for a new endpoint. This is the equivalent of calling server.on(path, handler)
    // If the path is set to "/", then the default homepage is overriden
  void registerBusyCallback(std::function<void(bool)> handler);
    // If you'd like to know when WebThing starts and finishes processing a web request,
    // registera function here. When called, the parameter will be true when the
    // processing begins, and false when it ends.

  // ----- Registering Menu Items
  // The overall "hamburger" menu is composed of three sets of menu items:
  // 1. Core: These items correspond to functionality that tends to be common across use cases
  // 2. App:  These items are app-specific
  // 3. Dev:  These are developer-specific menu items and aren't usually displayed by default
  // These three sets of items can be registered independently using the functions below.
  // To support legacy use cases, they can also be pre-concatenated into a single string and
  // passed to addMenuItems().
  // In either case,  The html should be a concatenation of items of the form:
  // <a class='w3-bar-item w3-button' href='/ACTION_NAME'><i class='fa fa-cog'></i> Action Description</a>"
  // ACTION_NAME must correspond to an endpoint that was registered using registerHandler

  void addCoreMenuItems(const __FlashStringHelper* core);
  void addAppMenuItems(const __FlashStringHelper* app);
  void addDevMenuItems(const __FlashStringHelper* dev);

  void addMenuItems(String html);


  // ----- Periodic functions
  void handleClient();
    // Needs to be called from loop() to handle connections

  // ----- Page rendering functions
  // To generate a response to a request, call:
  // 1. startPage(): Sends headers and beginning of html page
  // 2. sendContent(): Sends whatever content has been created for the body of the page
  //    this can be called multiple times
  // 3. finishPage(): Sends the close of the html page and stops the connection
  void startPage(bool refresh = false);
  void sendContent(const String &content);
  void finishPage();

  void redirectHome();
  void closeConnection(uint16_t code, String text);
  bool authenticationOK();

  ESPTemplateProcessor *getTemplateHandler();

  void wrapWebAction(
      const char* actionName, std::function<void(void)> action,
      bool showStatus = true);
  void wrapWebPage(
      const char* pageName, const char* htmlTemplate,
      ESPTemplateProcessor::Mapper mapper, bool showStatus = true);


  // ---------- Helper functions that isolate your code from the underlying server object
  // ----- Request arguments
  int args();                               // get arguments count
  bool hasArg(const String& arg);           // check if argument exists
  const String arg(const String& name);     // get request argument value by name
  const String arg(int i);                  // get request argument value by number
  const String argName(int i);              // get request argument name by number

  // ----- Response Headers
  void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
  String header(const String& name);        // get request header value by name
  String header(int i);                     // get request header value by number
  String headerName(int i);                 // get request header name by number
  int headers();                            // get header count
  bool hasHeader(const String& name);       // check if header exists

  // ----- Sending Arbitrary Data
  using ContentProvider = std::function<void(Stream&)>;
  void sendArbitraryContent(String type, int32_t length, ContentProvider cp);
  void sendStringContent(String type, String payload);
  void sendJSONContent(DynamicJsonDocument *doc);

  // Deprecated as prep for supporitng both ESP8266 and ESP32
  WebServer* getUnderlyingServer();

}

#endif  // WebUI_h