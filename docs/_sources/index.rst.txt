.. KegScale documentation master file, created by
   sphinx-quickstart on Tue Jun  7 09:29:30 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to KegScale's documentation!
====================================

.. note::
  Reflects test version v1.0.0, Last updated 2022-06-11

This project is intended for my own use, I've have not really focused on making this reusable for others. 

However I have documented my build process and share it here. The design is created for 2 kegs but it 
will work if you only use one (make sure to use the pins for scale 1 in that case)

Im still building my own so the project is regarded as work in progress.

The project contains the following parts:

* Software that manage the scale and present the readings.
* Hardware design based on standard HX711 AD converter with load cells.
* 3D models for the base and display case.

Features
++++++++

* Measure the weight of up to two kegs and estimate the number of pints (or pours) left in the keg. This 
  is done using the weight of a keg and the weight of a pint of beer. 
* Import beer information from brewfather (name, abv, ebc and ibu)
* Show the remaning pints (or pours) on OLED displays, one per keg
* Configuration is done via web interface
* Calibration of scales are done via web interface

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


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   hardware
   software
   3d


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

