#include <ArduinoLog.h>
#include <WebUI.h>
#include "EmptyThing.h"

void setup() {
  WebThing::preSetup();
  WebThing::setDisplayedVersion("ET 0.1");

  prepIO();

  WebThing::setup();
  WebThing::notifyBeforeDeepSleep(flushBeforeSleep);
  WebThing::notifyConfigChange(configChange);
  WebUI::setTitle("EmptyThing (" + WebThing::settings.hostname + ")");
  WebUI::registerHandler("/", displayHomePage);
  WebUI::registerHandler("/takeReadings", displayReadings);
  WebUI::addMenuItems(
    "<a class='w3-bar-item w3-button' href='/takeReadings'><i class='fa fa-thermometer-three-quarters'></i> Take readings</a>"
    );

  WebThing::postSetup();
}

void loop() { 
  WebThing::loop();
}


/*------------------------------------------------------------------------------
 *
 * Internal Functions
 *
 *----------------------------------------------------------------------------*/

void displayHomePage() {
  WebUI::startPage();
  WebUI::sendContent("<h1>EmptyThing</h1>");
  WebUI::finishPage();
}

void displayReadings() {
  WebUI::startPage(true);
  WebUI::sendContent("<h1>The Readings</h1>");
  WebUI::finishPage();
}

void prepIO() {
  Log.verbose("prepIO(): set pinModes, etc...");
}

void flushBeforeSleep() {
  Log.verbose("flushBeforeSleep(): About to go to sleep...");
}

void configChange() {
  WebUI::setTitle("EmptyThing (" + WebThing::settings.hostname + ")");
}


