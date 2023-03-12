.. _software:

Software
--------

Index
*****

This is the web page accessed by accessing the device via a web-browser. This section will show the general device information.

.. image:: images/index.png
  :width: 600
  :alt: Index

* **Weights/Volumes**: Shows weighs for scale 1 and 2.

* **Glasses**: Shows glasses (pours) left for scale 1 and 2. 

* **Last Pour**: How much the last pour was. Is lost when rebooting.

* **Temperature**: Temperature reading (not shown if there is no sensor).

* **Current version**: Shows the current software version including git revision or beta tag.

* **Host name**: Name of the device on the network.

* **Platform**: Which hardware platform is used (ESP8266 or ESP32S2).

* **Device ID**: ESP chip ID. Used for identifying the device.

* **SSID**: Name of network we are connected to.

Beers
*****

This is where the beer on tap is configured. 

.. image:: images/beer.png
  :width: 600
  :alt: Beer configuration

* **Empty keg weight**: Weight of the empty keg, used to determine how much beer is in the keg.

* **Glass volume**: Choose the volume of the beer glasses, used to determine how many pours are left. 

* **Beer name**, **Beer ABV**, **Beer EBC**, **Beer IBU**: Information about the beer, used to show on user interfaces.

* **Beer FG**: Used in formula for calculating the beer volume. FG has a slight impact on the weight / volume unit. If not defined 1 will be used for calculations.


Configuration
*************

This is where the software is configured. 

.. image:: images/config.png
  :width: 600
  :alt: Configuration

* **Device name**: Set the name of the device on the network, your network needs to support mDNS.

* **Temperature format**: Select format for temperatures

* **Weight unit**: Select format for weights

* **Volume unit**: Select format for volumes

* **Display layout**: Choose the layout/information on the OLED displays.

* **Temp sensor**: Choose the what temp sensor is used.

* **Scale sensor**: Choose the what ADC is used. Note that these require different wiring and connections.

.. image:: images/config2.png
  :width: 600
  :alt: Configuration (2)

* **HA MQTT**: Setting for MQTT server that is used by Home Assistant.

* **Brewfather user key**, **Brewfather API key**: Brewfather User key/API key.

* **Brewspy token**: Brewspy tokens for keg 1 & keg 2.

.. image:: images/config3.png
  :width: 600
  :alt: Configuration (3)

Here you can customise some of the thresholds and parameters that are used in the software. 

Calibration
***********

This is the page where you can calibrate your scales in 3 simple steps.. 

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

.. image:: images/stability2.png
  :width: 600
  :alt: Statistics

If you keep the browser open you can also see the history of the values (raw, kalman & stable). This can help to show
how your scale varies over time. Data is only stored in the browser so any refresh or page change will delete the graphs.

Firmware update
***************

Here you can upload a new firmware version without the need to hook the device to a serial port 
on your computer.

.. image:: images/upload.png
  :width: 600
  :alt: Upload firmware

Backup & Recovery
*****************

.. image:: images/backup.png
  :width: 600
  :alt: Backup configuration
