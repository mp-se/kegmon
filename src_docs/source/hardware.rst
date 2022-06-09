.. _hardware:

Hardware 
--------

This is the hardware schema that I have used in my build. 

.. note::
  The software will adopt to what devices are connected and will work with 
  one, two scales, one or two displays and without a temperature sensor. 

Schema
******

.. image:: images/schema.jpg
  :width: 600
  :alt: Schema

Part list:

* U1 - Wemos D1 mini (I used the v3.0 version)
* U2 - 0.96" 128x64 I2C OLED display (with option to change i2c adress)
* U3 - 0.96" 128x64 I2C OLED display (with option to change i2c adress)
* U4 - DHT22 temp and humidity sensor
* R1 - 4.7k
* R2 - 4.7k
* SCALE1 - HX711 with 4 load cells
* SCALE2 - HX711 with 4 load cells
* CAT6 network cable
* 2 x 3D printed base for scale
* 3D printed case for displays and esp8266
* 5V power supply

R1 and R2 are just used to pull the CLK to +3.3V or the code will not detect 
that scales are missing (floating input). You can use most values between 
2k and 5k for that.  


Building the base
*****************

Print the 2 base models and 2 covers. Files can be found in the sub-model directory. 
These models are designed to be printed on my Prusa MK3+ and support a cornelius  
keg of 9, 18 or 19 l.

You can of course print or build a different mount for the load cells. Just check the 
internet and you will find several options. Mount the load cells in the base and wire 
the HX711 board to the load cells as shown in the next picture. 

.. image:: images/keg_base_loadcell.jpg
  :width: 600
  :alt: Load cells mounting

Do not shorten the cables since this will affect the function of the 
load cells. Use the wiring setup as shown here to connect the load cells to the 
HX711. 

.. image:: images/hx711.jpg
  :width: 600
  :alt: HX711

I used network cable (CAT6) for the connection between the bases and the esp8266 and this is 
what the result looked like. The cover will be glued on top of this at a later stage (preferably 
when it works correcly). I also used some transparent tape to secure the cables in the base.

.. image:: images/keg_base_wired.jpg
  :width: 600
  :alt: Wiring

Here is the second base where I have writed the cables togheter so I only need one cable to 
the esp8266.

.. image:: images/keg_base_cabling.jpg
  :width: 600
  :alt: Wiring with tape

CAT6 Wire Usage
***************

I used the following wires in the network cable to connect to the 2 bases. Note that the CLK 
wires both connected to D4 by design. 

.. list-table:: CAT Wiring
   :header-rows: 1

   * - Wire
     - Function
     - Pin
   * - Orange
     - +3.3V
     - +3.3V
   * - Orange-White
     - GND
     - GND
   * - Blue
     - CLK - Scale 1
     - D4
   * - Blue-White
     - DATA - Scale 1
     - D5  
   * - Green
     - CLK - Scale 2
     - D4
   * - Green-White
     - DATA - Scale 2
     - D3  
   * - Brown
     - Tempsensor
     - D7  
   * - Brown-White
     - Not used
     - Not used


Building the display case
*************************

This section will be written once I have completed my 3d model for the case.
