.. _software:

Software
--------

THe software is tailored towards my personal needs and external service 
that I use. The software has two interfaces, one via the OLED displays and 
one via the web-browser.

The scale seams to be much more stable now that I added a kalman filter as the first step,
this eliminates the big changes in values. It takes a few seconds for the level to stabilize but
that's not a big issue. 

Installation
************

If there is an interest let me know and I will add the project to brewflasher for easier flashing.

WIFI
****

To be documented. 


OLED Screens
************

The OLD screens will show the name of the beer, abv and alternate between weight and pours. The first 
screen will display values for keg 1 and the second for keg 2.


Index
*****

This is the web page accessed by accessing the device via a web-browser. This 
section will show the general device information.

.. image:: images/index.png
  :width: 600
  :alt: Index

* **Temperature**

Temperature reading (not shown if there is no sensor).

* **Humidity**

Humidity reading (not shown if there is no sensor).

* **Weights/Volumes**

Shows weighs for scale 1 and 2.

* **Glasses**

Shows glasses (pours) left for scale 1 and 2. 

* **Current version**

Shows the current software version including git revision.

* **Host name**

Name of the device on the network.

* **Device ID**

ESP8266 chip ID. Used for identifying the device.

* **SSID**

Name of network we are connected to.

Beers
*****

This is where the software is configured. 

.. image:: images/beer.png
  :width: 600
  :alt: Beer configuration

* **Empty keg weight**

Weight of the empty keg, used to determine how much beer is in the keg.

* **Glass volume**

Choose the volume of the beer glasses, used to determine how many pours are left. 

* **Beer name**, **Beer ABV**, **Beer EBC**, **Beer IBU**

Information about the beer, used to show on user interfaces.

* **Beer FG**

Used in formula for calculating the beer volume. FG has a slight impact on the weight / volume unit. If not defined 1 will be used for calculations.


Configuration
*************

This is where the software is configured. 

.. image:: images/config.png
  :width: 600
  :alt: Configuration

* **Device name**

Set the name of the device on the network, your network needs to support mDNS.

* **Temperature format**

Select format for temperatures

* **Weight unit**

Select format for weights

* **Volume unit**

Select format for volumes

* **Display layout**

Choose the layout/information on the OLED displays.

* **Brewfather user key**, **Brewfather API key**

Brewfather User key/API key.

* **Brewspy token**

Brewspy tokens for keg 1 & keg 2.

Calibration
***********

This is the page where you can calibrate your scales. 

.. image:: images/calibration.png
  :width: 600
  :alt: Calibration

* **STEP 1 - Tare scale**

The first step is to tare the scale. First select the scale from the dropdown list that you 
want to operate on. Make sure the scale is empty.

* **STEP 2 - Calculate factor**

The second step is to calculate the factor used to calculate the weight. Place a thing with a 
known weight on the scale and enter the weight of that object. The software will then calculate
the factor for estimating the weight. 

.. note::
  I need to do more testing in order to have recommendations on what is a good option for this.

* **STEP 3 - Validate**

The third step is to validate that everything works, place anohter thing with a know weight and 
check the measured value. If you are satisfied then you are done.

History
*******

Shows the last level changes detected by the software. There is a second graph for the pours.

.. image:: images/history.png
  :width: 600
  :alt: Level changes

.. image:: images/history2.png
  :width: 600
  :alt: Pours


Stability
*********

Information page that can determine the stability of your hardware build.

.. image:: images/stability.png
  :width: 600
  :alt: Statistics

I have found that some of my hardware builds was not really stable so I added this 
feature to check the scale build before doing the final assembly. 

Firmware update
***************

Here you can upload a new firmware version without the need to hook the device to a serial port 
on your computer.

.. image:: images/upload.png
  :width: 600
  :alt: Upload firmware

