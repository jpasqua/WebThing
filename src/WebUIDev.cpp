/*
 * WebUIDev:
 *    Adds functionality for Developer related features accessible via
 *    the '/dev' location or, if enabled, through the Dev menu item
 *                    
 * NOTES:
 *   
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <BPABasics.h>
//                                  Local Includes
#include "DataBroker.h"
#include "WebThing.h"
#include "WebUI.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespace
namespace WebUI {

  namespace Dev {
    const __FlashStringHelper* DEV_MENU_ITEMS = FPSTR(
      "<a class='w3-bar-item w3-button' href='/dev'>"
      "<i class='fa fa-gears'></i> Developer</a>");

    bool* _showDevMenu;
    BaseSettings* _deviceSpecificSettings;
    std::vector<ButtonDesc> buttonActions;

    void reboot() {
      if (!authenticationOK()) { return; }
      redirectHome();
      ESP.restart();
    }

    void getDataBrokerValue() {
      auto action = []() {
        String key = "$" + WebUI::arg(F("key"));
        String value;
        DataBroker::map(key, value);
        String result = key + ": " + value;
        WebUI::sendStringContent("text", result);
      };

      WebUI::wrapWebAction("/dev/data", action);
    }

    void updateSettings() {
      auto action = []() {
        *_showDevMenu = hasArg("showDevMenu");
        addDevMenuItems(*_showDevMenu ? DEV_MENU_ITEMS : nullptr);
        _deviceSpecificSettings->write();
        redirectHome();
      };
      wrapWebAction("/updateSettings", action, false);
    }

    void yieldSettings() {
      auto action = []() {
        DynamicJsonDocument *doc = (hasArg("wt")) ?
            WebThing::settings.asJSON() : _deviceSpecificSettings->asJSON();

        sendJSONContent(doc);
        doc->clear();   // TO DO: Is this needed?
        delete doc;
      };
      wrapWebAction("/updateSettings", action, false);
    }

    void concatProperty(
        String& val, const char* propName, const char* propVal, bool leadingComma = true) {
      if (leadingComma) val.concat(',');
      val.concat(propName); val.concat(": \""); val.concat(propVal); val.concat('"');
    }

    void concatDevButtons(String& val) {
      bool first = true;
      size_t nButtons = buttonActions.size();
      for (int i = nButtons-1; i >= 0; i--) {
        const ButtonDesc& b = buttonActions[i];
        if (!first) val.concat(',');
        else first = false;
        val.concat("{");
          concatProperty(val, "label", b.label, false);
          concatProperty(val, "endpoint", b.endpoint);
          if (b.color) concatProperty(val, "color", b.color); 
          if (b.confirm) concatProperty(val, "confirm", b.confirm);
        val.concat("}");
      }
    }

    void displayDevPage() {
      auto mapper =[](const String &key, String& val) -> void {
        if (key == "SHOW_DEV_MENU") val = checkedOrNot[*_showDevMenu];
        else if (key.equals(F("HEAP"))) { DataBroker::map("$S.heap", val); }
        else if (key == "BUTTONS") concatDevButtons(val);
      };

      WebUI::wrapWebPage("/displayDevPage", "/wt/DevPage.html", mapper);
    }

    void init(bool* showDevMenu, BaseSettings* deviceSpecificSettings)
    {
      _showDevMenu = showDevMenu;
      _deviceSpecificSettings = deviceSpecificSettings;

      addDevMenuItems(DEV_MENU_ITEMS);

      addButton({"Request Reboot", "/dev/reboot", "w3-pale-red", "Request reboot?"});
      addButton({"View WebThing Settings", "/dev/settings?wt=on", nullptr, nullptr});
      addButton({"View Settings", "/dev/settings", nullptr, nullptr});
      addButton({"File System", "/fslist", nullptr, nullptr});

      registerHandler("/dev",                 displayDevPage);
      registerHandler("/dev/reboot",          reboot);
      registerHandler("/dev/settings",        yieldSettings);
      registerHandler("/dev/updateSettings",  updateSettings);
      registerHandler("/dev/data",            getDataBrokerValue);
    }

    void addButton(ButtonDesc&& buttonAction) {
      buttonActions.push_back(buttonAction);
    }

  } // ----- END: WebUI::Dev
} // ----- END: WebUI
