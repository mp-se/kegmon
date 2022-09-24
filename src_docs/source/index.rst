.. KegMon documentation master file, created by
   sphinx-quickstart on Tue Jun  7 09:29:30 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to KegMon's documentation!
====================================

.. note::
  Reflects test version v0.4.0, Last updated 2022-09-24

  Im still building my own so the project is regarded as work in progress.

Main challenges so far:
* Stable level detection, I'm trying out a few methods to detect a stable level and tuning the algorithms I have implemented. It's plausible that this is dependant on the hardware build and components. My second build seams to be more stable (at least in room temperature)
* The esp8266 is really slow when interacting with the web ui. I'm considering switching to an esp32 instead with more power and/or an async web server. 

This project is intended for my own use, I've have not really focused on making this reusable for others so the 
documentation is not that extensive. 

However I have documented my build process and share it here. The design is created for 2 kegs but it 
will work if you only use one (make sure to use the pins for scale 1 in that case). 

The software will detect how many hx711 are connected and if there is a tempsensor. 

The project contains the following parts:
* Software that manage the scale and present the readings.
* Hardware design based on standard HX711 AD converter with load cells.
* 3D models for the base and display case.

Features
++++++++

* Measure the weight & volume of up to two kegs and estimate the number of glasses are left in the keg
* Define the size of a pour by selecting the beer glass size
* Volume of beer is determined by the FG and weight of the beer (total - keg)
* Import beer information from brewfather (name, abv, ebc and ibu, fg)
* Import beer information from brewspy (name, abv) - fg will come in when the brewspy API has implemented it
* Show the remaning glasses on OLED displays, one per keg
* Show the remaning glasses on web interface
* Configuration is done via web interface
* Calibration of scales are done via web interface, no need for building your own firmware
* Stores statistics about the scales since start to determine the stabilty of the load cells
* Update firmware via web interface
* Interact with brew-spy's tap list (volume and pour notification)
* Display volumes in cl or fl. oz (uk & us)
* Display weights in kg or lbs
* Show a graph over scale changes and pours (last 40 data points)

Future
++++++
* Validate stability of the hardware design and load cells
* Switch to larger OLED displays (0.96 -> 1.3), this requires a different display driver (currently compile time option). 

Credits to
----------
Thanks to the following projects.

* https://github.com/mp-se/gravitymon
* https://github.com/graphitemaster/incbin
* https://github.com/khoih-prog/ESP_DoubleResetDetector
* https://github.com/khoih-prog/ESP_WiFiManager
* https://github.com/thijse/Arduino-Log
* https://github.com/bblanchon/ArduinoJson
* https://getbootstrap.com
* https://github.com/lorol/LITTLEFS
* https://github.com/bogde/HX711
* https://github.com/ThingPulse/esp8266-oled-ssd1306
* https://modelviewer.dev/
* https://github.com/RobTillaart/Statistic
* https://github.com/denyssene/SimpleKalmanFilter


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   software
   hardware
   3d


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

