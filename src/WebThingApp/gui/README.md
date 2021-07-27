# WebThingApp/gui

![](../../../doc/images/WebThing_Logo_256.png)  

GUI Support for building apps using the [WebThing](https://github.com/jpasqua/WebThing) framework. This is part of the overall WebThingApp framework.

## Introduction

The WebThingApp gui library consists of three conceptual layers:

* **Display**: At the lowest layer we have the Display namespace.
	* It is primarily a shim on top of the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library which provides all of the graphics functionality.
	* It provides lower level functions such as setting the screen brightness, and calibarating the touch panel.
	* It also provides a wrapper over the TFT_eSPI font mechansim to reduce memory consumption.
* **Screen**: Above that we have the `Screen` class and a number of common subclasses.
	* A `Screen` instance displays information and handles user input. For example, there is a `WeatherScreen` that provides weather information based on data from [OpenWeatherMap.org](https://openweathermap.org).
	* The `Screen` instances provided in this library may be used by an application or not depending on their applicability.
	* Applications may create their own sublclasses of `Screen` to provide application-specific functionality and UI.
* **ScreenMgr**: As the name implies, ScreenMgr manages multiple screens that are used as part of an application.
	* It keeps track of all registered screens.
	* It is used by the application or other `Screen` instances to cause a screen to be displayed; For example, `ScreenMgr::display(weatherScreen)`
	* It provides other utlity functions related to screen management

## File Organization

The gui library is part of the WebThingApp framework andis organized as follows:

````
WebThing
└── src
    ├── ...
    ├── ...
    ├── WebThingApp
    │   ├── ...
    │   ├── gui
    │   │   ├── [Interfaces and Implementations of the GUI]
    │   │   ├── fonts
    │   │   │   └── [Custom fonts used by the GUI]
    │   │   └── screens
    │   │       ├── [Interfaces and Implementations of common Screens]
    │   │       └── images
    │   │           └── [Images (.h files) used by the screens]
    │   └── ...
    ├── ...
    ├── ...
````

## Common `Screen` Subclasses

This library ships with the following `Screen` subclasses whcih can be used without modification:

* `CalibrationScreen`: Provides a process for the user to calibrate the touch screen. The application would typically want to save the calibration data, so it can provide a change-listener callback which will be invoked by `CalibrationScreen` any time the process is completed.
* `ConfigScreen`: When a WebThing comes online for the first time it typically has no WiFi connection information and must be configured by the user. If this is the case, the app may display the `ConfigScreen` to instruct the user to do so.
* `EnterNumberScreen`: Displays a number pad that allows the user to enter numerical data.
* `ForecastScreen`: Provides a 5-day forecast based on data from [OpenWeatherMap.org](https://openweathermap.org). It is typically used in conjunction with `WeatherScreen`.
* `RebootScreen`: Some `WebThing` applications may provide expert or developer interfaces. These may wish to invoke a `RebootScreen` which asks the user to confirm a reboot and upon confirmation, reboots the device.
* `UtilityScreen`: This screen provides information about the "thing" such as the name, version number, the web address, and the wifi signal strength. It also provides buttons that can be used to:
  * Jump directly to a named plugin screen (up to 4)
  * Adjust the screen brightness
  * Ask the application to refresh data from whatever sources it is monitoring
  * Enter the touch screen calibration process (see CalibrationScreen).
  * Return tot he home screen.
* `WeatherScreen`: Provides current weather information based on data from [OpenWeatherMap.org](https://openweathermap.org). It is typically used in conjunction with `ForecastScreen `.
* `WiFiScreen`: During the startup process of a `WebThing`, one of the earliest steps is to connect to WiFi. This screen can be displayed to tell the user that the connection is being established.

## Creating new subclasses of `Screen`

The `Screen` class has three functions that you need to write in your subclass:

* The constructor: Do whatever setup is necessary for your object. If your Screen has button areas that can be touched by the user to trigger an activity, this is where you would typically define them.
* The `display()` function: This is the heart of the `Screen`. It is responsible for displaying all data and graphics.
	* It can optimize the redrawing of elements as it sees fit based on whether information has changed, *unless* the `force` parameter is set in which case it must redraw the entire screen. This happens, for example, when the user moves from one screen to another.
	* This function will use the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library to actually draw the visual elements. Please refer to the documentation for that library to learn how to use it. You can also look at the common `Screen` subclasses as examples.
* The `processPeriodActivity()` function: A `Screen` may be static or it may need to be updated periodically. When a `Screen` is being displayed, the `processPeriodActivity` function will be called each time through the Arduino loop. The function can decide if it is time to refresh part or all of the `Screen` and act accordingly. For example, if the `Screen` is display a clock with seconds, it could update each second. For a clock with minutes it would update each minute. For a `Screen` that displays stock market information, it would only update if new information has been retrieved.
* The `ButtonCallback`: If you defined buttons in your constructor you will provide a callback which is invoked if the button is touched. It decides what to do. A trivial example is a **Home** button. When the user touches the button, your callback should invoke `ScreenMgr` and request that the home screen be displayed.
* 
## Plugins and FlexScreen

WebThing applications can be extended using the [Plugin](https://github.com/jpasqua/WebThingPlugins) mechanism. A plugin can define a `Screen` without new code using the `FlexScreen` mechanism, which:

 * Creates a `Screen` based on a definition given in a JSON document
 * That document specifies the layout of the elements on the screen and the data that should be used to fill those elements
 * The data is accessed via the `WebThing DataBroker`. The JSON document provides the keys to be used and `FlexScreen` uses the `DataBroker` to get values associated with the keys at runtime.
 * There is only one user interaction defined for a `FlexScreen`. Touching anywhere on the display invokes the `ButtonDelegate` supplied when the `FlexScreen` is instantiated.

So, a developer need never use `FlexScreen` directly. The `Plugin` system will use it as needed if plugins are loaded.
