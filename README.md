# WebThing
A generic ESP8266 framework for building *things* with a web configuration capability. They are called *things* as in Internet of *Things*.

The basic idea here is that you want some network-connected *thing* that performs one or more activities such as:

* Taking sensor readings (e.g. weather conditions)
* Controlling something in the physical world (e.g. a power switch)
* Providing an interface for data (e.g. an internet-connected clock, stock ticker, sensor status, or status of another service). The interface may be given via an attached display, a web interface, or some other means,

In all cases, the *thing* needs a way of getting connected to WiFi and configuring other basic settings. `WebThing` provides this capability and it also provides an extensible framework to add new web content including new pages and new menu items.

From the user perspective, the process begins with powering-on a *thing* for the first time. At this point it doesn't know which WiFi network to join so it will create it's own network named `thing-nnnnnn` (`nnnnnn` will be different for each device). The user should go to WiFi preferences on their computer or phone and they will see a new SSID of this form. They should connect to this network and in a few seconds, they will be placed in a "captive portal" which will allow them to choose their preferred WiFi network and specify credentials to join it. For more details, please see the [WiFiManager](https://github.com/tzapu/WiFiManager) project which is incorporated by `WebThing`.

Once the *thing* is configured it will join the specified network, but how does the user know its address? It depends. If the *thing* you are building has an interface, it can display its hostname or IP address. If not, the user can scan their network looking for a hostname of the form `thing-nnnnnn` (just like the SSID above), or for an ESP device. Most WiFi routers have the ability to show the attached devices.

## Dependencies

### Libraries
The following third party libraries are used by this library:

* [Arduino-Log](https://github.com/thijse/Arduino-Log)
* [ArduinoJson (v6)](https://github.com/bblanchon/ArduinoJson)
* [WiFiManager](https://github.com/tzapu/WiFiManager)
* [TimeLib](https://github.com/PaulStoffregen/Time.git)
* [ESPTemplateProcessor](https://github.com/jpasqua/ESPTemplateProcessor)

### Services

The following internet services are used by this library:

* [Google Maps](https://developers.google.com/maps/documentation/javascript/get-api-key) [optional]: Used for geocoding and reverse geocoding. Though not absolutely necessary, it does make using the system a bit more convenient. The user may enter an address rather than Lat/Lng coordinates.
* [TimeZoneDB](https://timezonedb.com/api): Used to get local time and time zone data. This is used to set the device's internal clock and keep it in sync.

## Configuration via the Web UI
Now that the user knows the *thing's* address, they can navigate to it in a web browser where they will be presented with a web page which has a header, main content area, and footer. On the left side of the header they will see a "hamburger" menu icon, which when pressed, will open up a left side bar consisting of *thing*-specific menu items followed by core `WebThing` items. The core menu items are:

* **General Settings**: This item displays a page that allows the user to enter basic settings. See [General Settings](#general-settings)
below:
* **Power Settings**: This item displays a page that allows the user to configure settings related to low power operation. See [Power Settings](#power-settings) below.
* **Choose Log Level**: This item should only be used by developers to configure how much information is provided during logging. See [Arduino-Log](https://github.com/thijse/Arduino-Log) for an explanation of the levels.
* **Reset Settings**: Resets all settings to their default values.
* **Forget WiFi**: Forgets the WiFi configuration which will cause the user to go through the initial setup again.

Of course a particular *thing* may add menu items that are specific to it. The *thing* may also provide content on the "home page". By default, the home page doesn't display anything interesting. A *thing* such as an internet-connected clock may have no additional menu items and no additional web content. It would just use the WebThing framework to get on the network and be provided with updated time information. Other *things* may have extensive settings of their own and additional web content.


### Settings
#### General Settings

* Location: Latitude, Longitude, Elevation. The user may enter an address and press the `Geocode` button to have the lat/lng/elevation determined automatically. This requires a Google Maps API Key (see below)
* API Keys:
	* [Google Maps](https://developers.google.com/maps/documentation/javascript/get-api-key): Fill in the api key you acquired from for the Google Maps service
	* [TimeZoneDB](https://timezonedb.com/api): Fill in the api key you acquired from for the TimezoneDB service.
		* NTP is perhaps a more natural choice for a time service, but it does not incoporate a mechanism for time zone determination. TimeZoneDB allows WebThing to get the time, time zone, and local time without need for another mechanism.
* Web Server Settings: The hostname of your *thing*, the web port to use, credentials, and the color theme for the Web UI.

#### Power Settings
* Use Low Power Mode: When checked, the *thing* will enter low power mode after setup
* Processing Interval: How often should the *thing* wake itself up to perform periodic tasks
* Sleep Override Pin: Select a HW pin (or no pin) that can be used to override low power mode.
* Voltage Sensing: When checked, this indicates that the device can read the input voltage on A0
* Voltage Calibration Factor: When sensing the voltage on A0, multiply by this factor to scale for the HW voltage divider used in your *thing*.

#### Implementer Settings
* Log Level: Sets the level at which messages will be logged. See [Arduino-Log](https://github.com/thijse/Arduino-Log) for a description of the options.
* Indicator LED: Configures whether there is an indicator LED on the *thing*, and if so, which pin is used. The user may select a specific pin, no pin, or the built-in LED. A related setting indicates whether the LED state is inverted (i.e. use HIGH to to turn the LED off and LOW to turn it on).
 
## Using the Library

### Incorporating the Library into your code

#### Basic Flow
The `setup()` and `loop()` functions of your app will need to incorporate calls into WebThing as follows:

* In the `setup()` function of your main app you must:
    * Call `WebThing::preSetup()` first to do things like read basic settings.
    * Call `WebThing::setup()` whenever you are ready for the network and Web UI to be started.
    * Use the `WebThing::notify...()` functions to register callbacks for various events such as when the system is about to go into low power mode.
    * Call `WebThing::postSetup()` before exiting your `setup()` function.
* In your `loop()` function
	* Perform your normal `loop()` activities
	* Call `WebThing::loop()` to give it a
  chance to handle network-related activities.

See `examples/EmptyThing.ino` to show the basic flow through an Arduino project that uses this framework.

#### Implementing Your Own Settings

WebThing stores its settings data in SPIFFS as a JSON file and it provides a mechanism for you to store your own settings as well. Do so by creating a sublcass of `BaseSettings` which contains member data representing your settings. It must implement the following member functions:

* `fromJSON`: Accepts a reference to a `JsonDocument` and populates the member variables with data from the JSON structure.
* `toJSON`: Accepts a reference to a `JsonDocument` and populates it based on the member variables.
* `logSettings`: This is for logging purposes only and can be a no-op. Log the data from your settings in any way you deem appropriate. WebThing uses the [Arduino-Log](https://github.com/thijse/Arduino-Log) framework, but you need not use it here. If you do, what is logged will be determined by the `logLevel` setting. 

**Note:** WebThing is a singleton and implemented as a namespace, not a class.

### Low Power Mode

When low power mode is selected in the Web UI, the device will put itself to sleep automatically when `WebThing::postSetup()` is called. This means that if you are using low power mode, your `loop()` function will **NEVER** be called. Once you call `WebThing::postSetup()`, the device will enter deep sleep for `settings.processingInterval` minutes. Waking up really just amounts to resetting the device, which will start again at `setup()`.

For example, let's say you are implementing a weather station that should wake up every 10 minutes, take readings, send those readings to a service, and then go back to sleep. Your code must take the readings and send them out in your `setup()` function before calling `WebThing::postSetup()`. If your *thing* may be operated in low power mode or normal mode, it must also be prepared to take readings in your `loop()` function.

 **NOTE**: When in low power mode the *thing* cannot provide a user interface (Web UI or GUI). This means that there is not a convenient way to get out of low power mode. You may designate a pin as a low power mode override. The pin is selected from the Power Saving menu item. Pulling this pin low will override power saving mode.

### Build Process
WebThing uses SPIFFS to store HTML templates and settings, which imposes additional requirements when building:

1. In the Arduino IDE you must ensure that you have reserved SPIFFS space. Do this by selecting `Tools -> Flash Size -> (Pick a SPIFFS size)`
2. All of the templates must be uploaded to the ESP8266. Use the [ESP8266 Sketch Data Upload plugin](https://github.com/esp8266/arduino-esp8266fs-plugin) for this. Any time you change a template, you must upload the data to the ESP8266.
3. Because you are likely to extend the Web UI, you may have your own templates or other files to place in SPIFFS. Put them all in a directory named `data` within your sketch directory.
<a name="link-wt"></a>
4. The uploader must upload all files at once - your files and those from the WebThing library. That means you need to copy or link the WebThing files to your data directory. All of the WebThing files are in a sub-directory of the data directory named `wt`. Your resulting directory structure will look like this:

```
Your_Sketch_Dir
    /data
        YourTemplate1.html
        ...
        YourTemplateN.html
        /wt
            WebThing_Template1.html
            ...
            WebThing_TemplateN.html
```

## Examples

### EmptyThing

As of 2020-05-25 there is only one example: EmptyThing. It is basically an empty project (hence the name) which demonstrates the structure and sequence of calls required to use *WebThing*. It also shows how to add an item to the main menu of the Web UI.

Like any *WebThing*, you must have a `data` directory which includes the `wt` subdirectory from *WebThing*. EmptyThing requires no files of its own, so the only thing in the `data` directory will be the `wt` subdirectory. This means you can just copy the whole `data` directory from *WebThing* to your EmptyThing example. See notes [above](#link-wt).

The resulting directory structure will look like this:

```
EmptyThing
    /data
        /wt
            WebThing_Template1.html
            ...
            WebThing_TemplateN.html
```


## Acknowledgements

* The [printer-monitor](https://github.com/Qrome/printer-monitor) project by [Qrome](https://github.com/Qrome) provided inspiration, direction, and the model for the Web UI.
* The [Solar Weather Station](https://github.com/3KUdelta/Solar_WiFi_Weather_Station) project by [3KU_Delta](https://github.com/3KUdelta) also provided valuable insights for the structure of low power mode.