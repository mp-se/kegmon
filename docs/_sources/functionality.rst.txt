.. _functionality:

Functionality
==============

The main features
-----------------

* **Lots of hardware options**

  One firmware that support multiple hardware opions. You can select displays, temperature sensors and ADC. Can be configured in the web UI.

* **Modern web based interface**

  Modern HTML5 based web interface with easy access to configuration and current status.

* **3 step easy scale calibration**

  3 easy steps to calibrate your scale all from the web interface. Tare the scale, calculate factor based on known weight and finally verify.

* **3D printable bases for kegs**

  3D models for keg bases or uses any of the other load cell bases that can be found. 

* **Hardware design for up to 2 scales**

  Software support one or two scale with attached displays for showing statistics.

* **Stable level detection**

  Software based detection for finding a stable level of each keg.

* **Pour detection**

  Can detect when a beer is poured. Needs to be more than 10 cl and there needs to be a stable level of approx 40 seconds after the pour.

* **Estimating remaning glasses/pints**

  Based on weight and final gravity the remaning volume can be calculated. The weight of the empty keg can be entered so that this is removed from the masurement. 

* **Integration with Home Assistant**

  Data can be sent to home assistant to show current beers, last pour, remaning glasses etc.

* **Integration with BrewSpy**

  Can fetch data from BrewSpy on what brews are current and update the level when changed. 

* **Integration with BrewFather**

  Can fetch information on last brews for easy import of data.

* **Load cell temperature compensation**

  Formula can be added to do temperature compensation for each loadcell.

* **Easy software updates**

  Upload new software via the web UI.

* **Temperature & Humidity sensor**

  Place a temperature sensor close to the base, this can be used to do temperature adjustments of the load cells using a formula. Supports either a DS18B20, BME280 or a DHT22.

* **Backup and Restore of settings**

  All the configuration of the device can be exported and stored as a text file (json). This can be used to restore settings in case of 
  hardware problems. You can also create a template of your settings and use that on multiple devices. Just make sure to change the "id" fields 
  to match your device id (can be found on the index page). 
 
* **Celsius or Fahrenheit**

  You can switch between different temperature formats. KegMon will always use *C* for it's internal calculations and 
  convert to F when displayed.

* **Kg or Lbs**

  You can switch between different weight formats. KegMon will always use *kg* for it's internal calculations and 
  convert to Lbs when displayed.

* **cl, UK fl. oz or US fl. oz**

  You can switch between different volume formats. KegMon will always use *cl* for it's internal calculations and 
  convert to other formats when displayed.

* **Customize pins and the hardware design**

  You can customize the pin configuration if you want to change the hardware setup.
