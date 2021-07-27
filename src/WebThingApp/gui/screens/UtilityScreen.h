/*
 * UtilityScreen:
 *    Display info about the GrillMon including things like the server name,
 *    wifi address, heap stats, etc. Also allow brightness adjustment 
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <WebThingApp/gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class UtilityScreen : public Screen {
public:
  typedef std::function<void(bool)> RefreshCallback;

  UtilityScreen(
      PluginMgr* mgr,
      const String appName, const String* appVersion,
      const String subheading, const String* subcontent,
      RefreshCallback refreshCallback);
  void display(bool activating = false);
  virtual void processPeriodicActivity();


private:
  static const auto HeaderFont = Display::Font::FontID::SB12;
  static const uint16_t HeaderFontHeight = 29;

  static const auto ButtonFont = Display::Font::FontID::SB9;
  static const uint16_t ButtonFontHeight = 22;

  static const uint16_t ButtonFrameSize = 2;
  static const uint16_t ButtonHeight = 40;
  static const uint16_t ButtonInset = 1;
  static const uint16_t HalfWidth = (Display::Width-(2*ButtonInset))/2;
  static const uint16_t ThirdWidth = (Display::Width-(2*ButtonInset))/3;

  static const uint16_t PI_YOrigin = 60;

  static const uint8_t FirstPluginIndex = 0;
  static const uint8_t MaxPlugins = 4;
  static const uint8_t DimButtonIndex = 4;
  static const uint8_t MediumButtonIndex = 5;
  static const uint8_t BrightButtonIndex = 6;
  static const uint8_t RefreshButtonIndex = 7;
  static const uint8_t CalButtonIndex = 8;
  static const uint8_t HomeButtonIndex = 9;
  static const uint8_t TotalButtons = 10;

  static const uint16_t WifiBarsWidth = 13;
  static const uint16_t WifiBarsHeight = 16;

  PluginMgr* pluginMgr;
  const String _appName;
  const String* _appVersion;
  const String _subHeading;
  const String* _subContent;
  RefreshCallback _updateAllData;

  void drawButton(String label, int i, uint16_t textColor, bool clear = false);

  // (x, y) represents the bottom left corner of the wifi strength bars
  void drawWifiStrength(uint16_t x, uint16_t y, uint32_t color);

};