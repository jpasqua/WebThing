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
#include "BaseSettings.h"
//--------------- End:    Includes ---------------------------------------------


namespace WebUI {
  constexpr const char* checkedOrNot[2] = {"", "checked='checked'"};

  // ----- Setup functions

  // Call only once to initialize the web interface
  void init();

  // Call at any time to update the title used in html pages as well as the
  // header for built-in pages
  // @param theTitle  The title to be displayed
  void setTitle(const String& theTitle);

  // Add a handler for a new endpoint. Unlike a call to server.on(), WebUI
  // keeps track of the mapping between paths and handlers. If you call
  // registerHandler twice for the same path, it will override the old
  // handler with the new one.
  // @param path      The URL path to look for
  // @param handler   The function to be called when the path is requested
  void registerHandler(const String& path, std::function<void(void)> handler);

  // If you'd like to know when WebThing starts and finishes processing a web request,
  // register a function here.
  // @param handler   The function to call when a web request starts/ends. When called,
  //                  the parameter will be true when the processing begins, and false
  //                  when it ends.
  void registerBusyCallback(std::function<void(bool)> handler);

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

  // Legacy version of menu handling where all of the menu item types are concatenated 
  void addMenuItems(String html);


  // ----- Periodic functions
  // Needs to be called from loop() to handle connections
  void handleClient();

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
  extern const String OKReponse;
  using ContentProvider = std::function<void(Stream&)>;
  void sendArbitraryContent(String type, int32_t length, ContentProvider cp, const String& code = OKReponse);
  void sendStringContent(String type, String payload, const String& code = OKReponse);
  void sendJSONContent(DynamicJsonDocument *doc, const String& code = OKReponse);

  // Deprecated as prep for supporitng both ESP8266 and ESP32
  WebServer* getUnderlyingServer();

  namespace Dev {
    typedef struct {
      const char* label;    // The textual label that should appear in the associated button
      const char* endpoint; // The web path to invoke when the button is pressed
      const char* color;    // The color of the button. nullptr -> default
      const char* confirm;  // A confirmation string. If not nullptr, then the dev page
                            // will display a confirmation dialog with this message
                            // before invoking the endpoint
    } ButtonDesc;

    // Provide default support for a developer page. The associated HTML template is:
    //    /data/wt/DevPage.html
    // The page allows the user (a developer in this case) to:
    // * Reboot the device
    // * Request the JSON corresponding to the device specific settings
    // * Request the JSON corresponding to the WebThing settings
    // * Enable or disable the developer menu
    //
    // @param showDevMenu   A pointer to a bool that always contains an indication of
    //                      whether the dev menu should be displayed.
    // @param devSettings   A pointer to the settings object of the underlying device.
    //                      Only used so the settings can be externalized upon request
    void init(
        bool* showDevMenu, BaseSettings* devSettings);
    
    // You may add html buttons with associated web actions to the developer page
    // by calling addButton(). The buttons will display in reverse order that they are
    // added. That is, the last button added will be shown closest to the top of the page.
    // Core WebThing items are added first (displayed lowest on the page), then the buttons
    // from WebThingAoo, then the app-specific buttons.
    // @param   buttonAction The Action that describe the button to be added to the Dev page
    void addButton(ButtonDesc&& buttonAction);
  }
}

#endif  // WebUI_h