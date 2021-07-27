/*
 * UtilityScreen:
 *    Display info about the GrillMon including things like the server name,
 *    wifi address, heap stats, etc. Also allow brightness adjustment 
 *                    
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
//                                  WebThing Includes
#include <WebThing.h>
#include <WebThingApp/plugins/PluginMgr.h>
#include <WebThingApp/gui/Theme.h>
#include <WebThingApp/gui/ScreenMgr.h>
#include <WebThingApp/gui/Display.h>
//                                  Local Includes
#include "UtilityScreen.h"
//--------------- End:    Includes ---------------------------------------------

using Display::tft;

static const char *pis_label[6] = {"Dim", "Medium", "Bright", "Refresh", "Calibrate", "Home"};
static const uint16_t pis_colors[6] = {
  Theme::Color_Inactive,   Theme::Color_DimText,    Theme::Color_NormalText,
  Theme::Color_NormalText, Theme::Color_NormalText, Theme::Color_AlertGood};
static int16_t SubFont = 2; // A small 5x7 font

UtilityScreen::UtilityScreen(
    PluginMgr* mgr,
    const String appName, const String* appVersion,
    const String subHeading, const String* subContent,
    RefreshCallback refreshCallback) :
  pluginMgr(mgr),
  _appName(appName), _appVersion(appVersion),
  _subHeading(subHeading), _subContent(subContent),
  _updateAllData(refreshCallback)
{

  auto buttonHandler =[&](int id, Button::PressType type) -> void {
    Log.verbose(F("In UtilityScreen Button Handler, id = %d, type = %d"), id, type);

    if (id < MaxPlugins) {
      pluginMgr->displayPlugin(id);
      return;
    }

    if (id <= BrightButtonIndex) {
      Display::setBrightness(id == DimButtonIndex ? 20 : (id == MediumButtonIndex ? 50 : 90));
      return;
    }

    if (id == RefreshButtonIndex) { _updateAllData(true); return;}
    if (id == CalButtonIndex) { ScreenMgr::display("Calibration"); return; }
    if (id == HomeButtonIndex) { ScreenMgr::displayHomeScreen(); return; }
  };


  buttons = new Button[(nButtons = TotalButtons)];
  for (int i = 0; i < MaxPlugins; i++) {
    buttons[i].init(
      ButtonInset + ((i%2) * HalfWidth), PI_YOrigin + (i/2) * ButtonHeight, HalfWidth, ButtonHeight, buttonHandler, i);        
  }

  int x = ButtonInset;
  buttons[DimButtonIndex].init(
      x, 160, ThirdWidth, ButtonHeight, buttonHandler, DimButtonIndex);     x+= ThirdWidth;
  buttons[MediumButtonIndex].init(
      x, 160, ThirdWidth, ButtonHeight, buttonHandler, MediumButtonIndex);  x+= ThirdWidth;
  buttons[BrightButtonIndex].init(
      x, 160, ThirdWidth, ButtonHeight, buttonHandler, BrightButtonIndex);
  x = ButtonInset;
  buttons[RefreshButtonIndex].init(
      x, 200, ThirdWidth, ButtonHeight, buttonHandler, RefreshButtonIndex); x+= ThirdWidth;
  buttons[CalButtonIndex].init(
      x, 200, ThirdWidth, ButtonHeight, buttonHandler, CalButtonIndex);     x+= ThirdWidth;
  buttons[HomeButtonIndex].init(
      x, 200, ThirdWidth, ButtonHeight, buttonHandler, HomeButtonIndex);
}

void UtilityScreen::display(bool activating) {
  if (activating) tft.fillScreen(Theme::Color_Background);

  int y = 0;
  tft.setTextColor(Theme::Color_AlertGood);
  Display::Font::setUsingID(HeaderFont, tft);
  tft.setTextDatum(TC_DATUM);
  String appInfo = _appName + "v" + *_appVersion;
  tft.drawString(appInfo, Display::XCenter, 0);
  drawWifiStrength(Display::Width-WifiBarsWidth-3, ButtonHeight-12, Theme::Color_NormalText);
  y += HeaderFontHeight;

  Display::Font::setUsingID(ButtonFont, tft);
  tft.setTextColor(Theme::Color_NormalText);
  tft.setTextDatum(TC_DATUM);
  String address = WebThing::settings.hostname + " (" + WebThing::ipAddrAsString() + ")";
  tft.drawString(address, Display::XCenter, y);

  tft.setTextColor(Theme::Color_NormalText);
  tft.setTextDatum(BC_DATUM);
  String sub = _subHeading + *_subContent;
  tft.drawString(sub, Display::XCenter, PI_YOrigin-1, SubFont);

  String name;
  uint16_t textColor = Theme::Color_NormalText;
  uint8_t nPlugins = min(pluginMgr->getPluginCount(), MaxPlugins);

  for (int i = 0; i < TotalButtons; i++) {
    if (i < nPlugins) {
      Plugin *p = pluginMgr->getPlugin(i);
      if (!p->enabled()) textColor = Theme::Color_Inactive;
      name = p->getName();
      textColor = Theme::Color_WiFiBlue;
    } else if (i < MaxPlugins) {
      name = "Unused";
      textColor = Theme::Color_Inactive;
    } else {
      name = pis_label[i-MaxPlugins];
      textColor = pis_colors[i-MaxPlugins];
    }
    drawButton(name, i, textColor, activating);
  }
}

void UtilityScreen::processPeriodicActivity() {  }

void UtilityScreen::drawButton(String label, int i, uint16_t textColor, bool clear) {
  if (clear) buttons[i].clear(Theme::Color_Background);
  buttons[i].drawSimple(
      label, ButtonFont, ButtonFrameSize,
      textColor, Theme::Color_Border, Theme::Color_Background);
}

// (x, y) represents the bottom left corner of the wifi strength bars
void UtilityScreen::drawWifiStrength(uint16_t x, uint16_t y, uint32_t color) {
  int8_t quality = WebThing::wifiQualityAsPct();
  for (int i = 0; i < 4; i++) {
    int h = (quality > (25 * i)) ? 4*(i+1) : 1;
    tft.drawRect(x+(i*4), y-h+1, 1, h, color);
  }
}