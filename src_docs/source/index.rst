.. KegMon documentation master file, created by
   sphinx-quickstart on Tue Jun  7 09:29:30 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to KegMon - Keezer Level Monitor
========================================

.. note::
  Reflects test version v0.5.0, Last updated 2023-01-27

This project is intended for my own use, I've have not really focused on making this reusable for others so the 
documentation is not that extensive. 

However I have documented my build process and share it here. The design is created for 2 kegs but it 
will work if you only use one (make sure to use the pins for scale 1 in that case). 

The software will detect how many hx711 are connected and if there is a tempsensor. 

The project contains the following parts:
* Software that manage the scale and present the readings
* Hardware design based on standard HX711 AD converter with load cells
* 3D models for the base
* 3D models for display case (work in progress)

Features
++++++++

* Measure the weight & volume of up to two kegs 
* Eestimate the number of glasses are left in the keg
* Level detection filters out faulty readings from the HX scales
* Define the size of a pour by selecting the beer glass volume
* Volume of beer is determined by the FG and weight of the beer (total - keg)
* Import beer information from brewfather (name, abv, ebc and ibu, fg)
* Import beer information from brewspy (name, abv) - fg will come in when the brewspy API has implemented it
* Show the remaning glasses on OLED displays, one display per keg
* Show the remaning glasses on web interface
* Configuration is done via web interface including wifi setup
* Calibration of scales are done via web interface, no need for building your own firmware
* Stores statistics about the scales since start to determine the stabilty of the load cells
* Update firmware via web interface
* Interact with brew-spy's tap list (volume and pour notification)
* Display volumes in cl or fl. oz (uk & us)
* Display weights in kg or lbs
* Show a graph over scale changes and pours (last 40 data points)
* Show the scale readings over time (history view) for up to 6 hours

Todo list
++++++++++
* Validate stability of the hardware design and load cells
* Switch to larger OLED displays (0.96 -> 1.3), this requires a different display driver (but I have a hard time to find the larger displays with the options to change I2C adress). 
* Temperature compensation

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

