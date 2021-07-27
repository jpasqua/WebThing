/*
 * Plugin
 *    A method for adding new functionality to an app.
 *
 * NOTES:
 * o A plugin consists of multiple parts:
 *   + A subclass of Plugin that can is specific to some data or activity
 *   + A screen definition which can be materialized by FlexScreen. This takes
 *     the form of a JSON file which describes the screen layout and look
 *   + A source for data to be provided to the plugin (e.g. a news source,
 *     a feed of weather info, data from some other IoT device)
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
//                                  WebThing Includes
#include <ESP_FS.h>
#include <DataBroker.h>
#include <WebThingApp/gui/ScreenMgr.h>
//                                  Local Includes
#include "PluginMgr.h"
//--------------- End:    Includes ---------------------------------------------



bool Plugin::init(const String& name, const String& piNamespace, const String& pluginDir) {
  _name = name;
  _namespace = piNamespace;
  _pluginDir = pluginDir;
  if (!typeSpecificInit()) return false;

  _mapper = [&](const String& key, String &value) -> void {
    if (key.isEmpty()) return;
    if (key[0] == '$') { DataBroker::map(key, value); return; }
    else typeSpecificMapper(key, value);;
  };

  // Create the FlexScreen UI
  DynamicJsonDocument* doc = PluginMgr::getDoc(_pluginDir + "/screen.json", MaxScreenDescriptorSize);
  if (doc == NULL) return false;
  _flexScreen = ScreenMgr::createFlexScreen(*doc, getUIRefreshInterval(), _mapper);
  delete doc;
  return !(_flexScreen == NULL);
}

void Plugin::getForm(String& form) {
  String fullPath = _pluginDir+"/form.json";
  File f = ESP_FS::open(fullPath.c_str(), "r");
  if (!f) { form = "{ }"; }

  size_t size = f.size();
  form.reserve(size);

  while (f.available()) {
    String line = f.readString();
    if (!line.isEmpty()) form += line;
  }
  f.close();
}