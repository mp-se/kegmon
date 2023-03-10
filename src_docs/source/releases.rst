.. _releases:

Releases 
########

v0.6.0 beta 2
=============

* NOTE! Updating from the 0.5 async version requires a manual reset after upload is finished to apply the new software.

* Added support for NAU7802 AD converter from sparkfun. Uses I2C bus. Will require ESP32S2 for two scales. This is EXPERIMENTAL!
* Added error message to restore configuration feature in case the ID did not match.

v0.6.0 beta 1
=============

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
