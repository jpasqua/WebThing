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
#include <ESP8266WebServer.h>
//                                  Third Party Libraries
#include <ESPTemplateProcessor.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


namespace WebUI {
  // ----- Setup functions
  void init();
    // Call only once to start the web interface
  void setTitle(String theTitle);
    // Can be called at any time to update the title used in html pages
    // as well as the header for built-in pages
  void registerHandler(String path, std::function<void(void)> handler);
    // Add a handler for a new endpoint. This is the equivalent of calling server.on(path, handler)
    // If the path is set to "/", then the default homepage is overriden
  void addMenuItems(String html);
    // NOTES: The html should be a concatenation of items of the form:
    // <a class='w3-bar-item w3-button' href='/ACTION_NAME'><i class='fa fa-cog'></i> Action Description</a>"
    // ACTION_NAME must correspond to an endpoint that was registered using registerHandler
  
  ESPTemplateProcessor *getTemplateHandler();

  ESP8266WebServer *getUnderlyingServer();

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

  bool hasArg(String arg);
  String arg(String arg);
  
  void redirectHome();
  void closeConnection(uint16_t code, String text);
  bool authenticationOK();
}

#endif  // WebUI_h