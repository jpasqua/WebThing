#ifndef Plugin_h
#define Plugin_h

/*
 * Plugin.h
 *    A method for adding enw functionality to an app.
 *
 * NOTES:
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <ArduinoJson.h>
//                                  WebThing Libraries
#include <WebThingBasics.h>
#include <WebThingApp/gui/FlexScreen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class Plugin {
public:
  // Implemented by subclasses
  virtual ~Plugin() { }
  virtual bool typeSpecificInit() = 0;
  virtual void typeSpecificMapper(const String& key, String &value) = 0;
	virtual void refresh(bool force = false) = 0;
  virtual void getSettings(String &settings) = 0;
  virtual void newSettings(String &settings) = 0;
  virtual uint32_t getUIRefreshInterval() = 0;

  // Implemented by Plugin class
  bool init(const String& name, const String& piNamespace, const String& pluginDir);
    // Called immediately after the Plugin's constructor. Will invoke
    // typeSpecificInit() after _name and _pluginDir are set
  void getForm(String& form);
    // Return a serialized version of the JSON doc representing the screen
  String getName() { return _name; }
    // Return the UI name of the plugin
  FlexScreen* getFlexScreen() { return _flexScreen; }
    // Return the associated screen
  String getNamespace() { return _namespace; }
    // Return the namespace for the associated data provider (if any)
  bool enabled() { return _enabled; }
    // Is this plugin enabled?

protected:
  String   _name;
    // Read from the plugin.json file and set in the init() function
  String _pluginDir;
    // The directory from which the plugin was loaded. Set in the init() function
  bool _enabled;
    // Is this plugin enabled. This may always be true, may be a setting, or may be
    // set based on some operational parameters
  FlexScreen* _flexScreen;
    // The associated FlexScreen for the plugin
  String _namespace;
    // If this is a custom plugin with an associated data source, the data can be made available
    // through the DataBroker mechanism. This is the namespace that will be used to access the data

private:
  static const uint32_t MaxScreenDescriptorSize = 7*1024;
  WTBasics::ReferenceMapper _mapper;
};

#endif // Plugin_h
