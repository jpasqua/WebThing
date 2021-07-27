#ifndef PluginMgr_h
#define PluginMgr_h

/*
 * PluginMgr.h
 *    Finds, instantiates, and manages all Plugins
 *
 * NOTES:
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <functional>
//                                  Third Party Libraries
#include <ArduinoJson.h>
#include <WebThingBasics.h>
//                                  Local Includes
#include "Plugin.h"
//--------------- End:    Includes ---------------------------------------------


class PluginMgr {
public:
  typedef std::function<Plugin*(String&)> Factory;

  static DynamicJsonDocument* getDoc(String filePathString, uint16_t maxFileSize);
  static void setFactory(Factory theFactory);

  void    loadAll(String pluginRoot);
  void    refreshAll(bool force = false);
  uint8_t getPluginCount();
  Plugin* getPlugin(uint8_t index);
  void    map(const String& key, String& value);
  void    displayPlugin(uint8_t pluginIndex);
  void    displayNextPlugin();

private:
  static const uint8_t MaxPlugins = 4;
  static Factory factory;
  
  uint8_t _nPlugins;
  Plugin* _plugins[MaxPlugins];

  int curPlugin = -1; // Used to iterate plugins for displayNextPlugin()

  bool validatePluginFiles(String pluginPath);
  void newPlugin(String pluginPath);
  uint8_t enumPlugins(String& pluginRoot, String* pluginDirNames);
};

#endif // PluginMgr_h
