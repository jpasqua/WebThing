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
      "<i class='fa fa-gears'></i> Dev Settings</a>");

    constexpr Action DefaultActions[] {
      {"View Settings", "/dev/settings", nullptr, nullptr},
      {"View WebThing Settings", "/dev/settings?wt=on", nullptr, nullptr},
      {"Request Reboot", "/dev/reboot", "w3-pale-red", "Perform immediate reboot?"}
    };
    uint8_t NumDefaultActions = ARRAY_SIZE(DefaultActions);

    bool* _showDevMenu;
    BaseSettings* _deviceSpecificSettings;
    const Action* appSpecificButtons = nullptr;
    uint8_t nAppSpecificButtons = 0;

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

    void concatDevButtons(String& val, const Action* actions, uint8_t nActions) {
      for (int i = 0; i < nActions; i++) {
        const Action& a = actions[i];
        if (i) val.concat(',');
        val.concat("{");
          concatProperty(val, "label", a.label, false);
          concatProperty(val, "endpoint", a.endpoint);
          if (DefaultActions[i].color) concatProperty(val, "color", a.color); 
          if (DefaultActions[i].confirm) concatProperty(val, "confirm", a.confirm);
        val.concat("}");
      }
    }

    void displayDevPage() {
      auto mapper =[](const String &key, String& val) -> void {
        if (key == "SHOW_DEV_MENU") val = checkedOrNot[*_showDevMenu];
        else if (key.equals(F("HEAP"))) { DataBroker::map("$S.heap", val); }
        else if (key == "BUTTONS") {
          if (appSpecificButtons) {
            concatDevButtons(val, appSpecificButtons, nAppSpecificButtons);
            val.concat(',');
          }
          concatDevButtons(val, DefaultActions, NumDefaultActions);
        }
      };

      WebUI::wrapWebPage("/displayDevPage", "/wt/DevPage.html", mapper);
    }

    void init(bool* showDevMenu, BaseSettings* deviceSpecificSettings)
    {
      _showDevMenu = showDevMenu;
      _deviceSpecificSettings = deviceSpecificSettings;

      addDevMenuItems(DEV_MENU_ITEMS);

      registerHandler("/dev",                 displayDevPage);
      registerHandler("/dev/reboot",          reboot);
      registerHandler("/dev/settings",        yieldSettings);
      registerHandler("/dev/updateSettings",  updateSettings);
      registerHandler("/dev/data",            getDataBrokerValue);
    }

    void addButtons(const Action* extraDevButtons, uint8_t nExtraDevButtons) {
      appSpecificButtons = extraDevButtons;
      nAppSpecificButtons = nExtraDevButtons;
    }
  } // ----- END: WebUI::Dev
} // ----- END: WebUI
