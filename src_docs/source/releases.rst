.. _releases:

Releases 
########

v1.0.0-alfa2
============

* Added influx integration for scale data (debugging)
* Removed chart views since this made the build to large for esp8266

v1.0.0-alfa1
============

* Created new user interface in VueJS
* Changed all API's to use JSON

v0.9.0
======

* Added new board esp32s3 mini
* Added option to fetch temperature from brewpi-esp installation
* Removed mobile dashboard, replaced with KegMonApp (see: https://github.com/mp-se/kegmonapp)
* Added support for ESP32s3 mini
* Fixed memory leak in temp sensor code

v0.8.0
======

* Added possiblity to run on wokwi simulator (work in progress, scales, file system not yet working)
* Added option for setting pins for external devices (display, scales and sensor)
* Added support for BME280 sensor + pressure information
* Refactor documentation to show the hardware options better
* Updating dependecies
* DS18B20 temperature sensor is not the default
* Updated code to support newer versions of ArduinoJSON

v0.7.1
======

* Fixed some issues with logging changes in levels so that the logfile is not filled with data connected to startup events
* Fixed stability issue that could cause crashes
* Updated display layout (One) to show details on display 2 if attached

v0.7.0
======

* Added support for LED 20x4 displays
* Added a few new layouts including support for one single display
* Refactored the display logic to make it easier to change settings and share formatting among multiple rendering options. 

v0.6.0
======

.. note::
    Updating from the 0.5 async version requires a manual reset after upload is finished to apply the new software.

.. warning::
    Issues with connecting to DHT22 sensor on ESP32S2/S3 variant over long cables. Uncertain what causes this. Recommendation is to use DS18B20 instead on ESP32S2/S3.

* BUG: Configuration lost when JSON document became to large, increased buffer to 3kb (from 2kb)

* Added support for NAU7802 AD converter from sparkfun. Uses I2C bus. Will require ESP32S2/S3 for two scales and different wiring. This is EXPERIMENTAL!
* Added error message to restore configuration feature in case the ID did not match.
* Added pull-up resistor to temperature sensor data line (applies to both DS18B20 and DHT22)
* Added web based serial console accessed via http://device/serial or ws://device/serialws (only works with async webserver)
* Added mobile dashboard accessed via http://device/dashboard
* Added updated schema and pcb for the HX711 edition.
* Added 3d model for case that works with pcb
* Moved to async webserver as standard
* Added support for ESP32 s2 mini board
* Upgraded Arduino frameworks to latest stable + fixed problem in HX711 library due to Arduino update.
* Added backup & recovery function
* Created own code for multireset feature (for entering wifi setup)
* Refactored Wifi manager library since its not longer maintained by original author. Added logging output.

v0.5.0
======

* Experimental Async webserver support, this improves the speed of the web UI quite a lot but can be prone to crashes
* Level detection has been refactored and I added a Kalman filter to remove the worst interference.
* Added option to do temperature compensation of load cells in software (formula still to be concluded). Requires a temperature sensor close to the load cells.
* Added integration with Home Assistant (via MQTT)
* Added graphs in web interface that shows the stability (requires the web browser to be open, data storage and rendering is done in the browser)
* Added some advanced configuration options to define params used by the level detection algorithm.
* Refactored logoutput from WifiManager to be consistent with other logs.

v0.4.0
======

* Prepared support for 1.3" displays (other driver), but havent found any displays that can change I2C adress.
* Logging of pours and graph to show history
* Better stability & pour detection
* Integration with brewspy both for level updating and beer information (waiting for API to expose FG)
* Some redesign of the UI
* Hardware design has been updated to have separate CLK pints to the HX scales, I found that this would make readings more unstable. 
