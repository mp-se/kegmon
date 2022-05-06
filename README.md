# KegScale

Based on the ideas of these projects; 

* https://brewkegscale.wordpress.com/
* https://www.hackster.io/davidtilley/iot-home-beer-keg-scale-b603db
* https://www.instructables.com/Beer-Keg-Scales/
* https://github.com/Callwater/Beerkeg-load-cell
* https://github.com/nanab/BeerScale

# This project is a work in progress

Currently this is a beta version that implements a hardware design based on a standard HX711 and 2 x OLED 0.96" display.

it's not really well tested but the features so far;

- Wifi setup is done via SSID:kegscale and password:password , enter up to 2 SSID that will be used to connect
- Web based configuration once connected to wifi. Configuration includes, tare and automatic calculation of scaling factor.
- Can show units of weight and remaning pints (keg weight and pint weight is set in configuration)
- Showing weight and pints on display with a 1 second update frequency
- Support 2 displays and (will support 2 scales once I get more parts)
- Integration with brewfather to fetch data on brewed beers (dont like to type)

# Todo

* Allow user to select weight unit to show (does not really matter for the functionallity)
* Do a ESP32 port (might be needed to attach more units)
* Implement screen saver for OLED
* Write API tests
* Create documentation
* Implement async webserver so it's more responsive to fast requests
* Try out some 3d printed parts for the scale (or create my own)
* Create a nice 3d holder for the displays and esp that fits on my keezer.
