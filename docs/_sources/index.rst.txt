.. KegMon documentation master file, created by
   sphinx-quickstart on Tue Jun  7 09:29:30 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to KegMon - Keezer Level Monitor
----------------------------------------

.. note::
  Reflects test version **v0.6.0 beta 1**, Last updated 2023-03-05

Introduction
============

The software is tailored towards my personal needs and external services that I use, but suggestions are always 
welcome. The software has two interfaces, one via the OLED displays and one via the web-browser.

The cheap load cells are quite unpredicteble so it's hard to get a fully accurate system, but they are really
cheap. In order to compensate for this I have build in the possibility to add filters and clean up the read values, 
these filters include:

* raw average (makes an average over the last 10 readings)
* kalman (smooths out the peaks readings, but slows down level detection)
* formula adjustment (this is not yet active, but its possible to add an equation to adjust the weight, for instance compensate for temperature)

Here are two views on the data change over time, the temperature in my keezer is between 4 and 5 degress Celcius. My two 
scales behave diffrently even though the load cells are from the same batch. I would guess that the peak in the first 
graph is due to interference from the compressor when it starts to cool.

.. image:: images/temp_variation.png
  :width: 400
  :alt: Scale variation

.. image:: images/temp_variation2.png
  :width: 400
  :alt: temp_variation

Since the level will vary slightly based on the temperature (on my scale +0.04 to -0.01 kg), I have added a level detection algorithm
that uses the average level over time and done adjust unless the delta is larger than the defined threasholds (can be defined).

I have prepared for adding a temperature correction option but I'm still working defining a formula for that weight compensation. 

In order to trigger a level change then both the raw values and the kalman value needs to be aligned and the minium level change 
needed to detect a pour is by default set to 10 cl. It can take up to 60 seconds for a new level to be detected and a pour registered.

The design is created for 2 kegs but it will work if you only use one (make sure to use the pins for scale 1 in that case). 

The OLD screens will show the name of the beer, abv and alternate between weight and pours. The first 
screen will display values for keg 1 and the second for keg 2.

The values presented on the screen is the raw values so that you can see directly if the level changes. 

The software will detect how many hx711 are connected and if there is a tempsensor. 

The project contains the following parts:
* Software that manage the scale and present the readings
* Hardware design based on standard HX711 AD converter with load cells and temperature sensor
* 3D models for the base
* 3D models for display case (work in progress)

TODO list
=========
* Complete compensation formula for temperature compensation
* Test alternatives to HX711 board
* Create PCB 
* Finalize 3D model for case

Credits to
==========
Thanks to the following projects.

* https://github.com/mp-se/gravitymon
* https://github.com/graphitemaster/incbin
* https://github.com/khoih-prog/ESP_WiFiManager
* https://github.com/thijse/Arduino-Log
* https://github.com/bblanchon/ArduinoJson
* https://getbootstrap.com
* https://github.com/bogde/HX711
* https://github.com/ThingPulse/esp8266-oled-ssd1306
* https://modelviewer.dev/
* https://github.com/RobTillaart/Statistic
* https://github.com/denyssene/SimpleKalmanFilter


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   
   functionality
   releases
   intro
   hardware
   3dmodels
   installation
   configuration
   license
   q_and_a


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

