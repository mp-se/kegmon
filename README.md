# KegScale

This is a project that I have done for my own Keezer and i have no plans to make this an officially supported software. Feel free to use the code according to the licence agreement. 

Based on the ideas of these projects; 

* https://brewkegscale.wordpress.com/
* https://www.hackster.io/davidtilley/iot-home-beer-keg-scale-b603db
* https://www.instructables.com/Beer-Keg-Scales/
* https://github.com/Callwater/Beerkeg-load-cell
* https://github.com/nanab/BeerScale

# Hardware setup

For this project I use 2 scales based on 4 load cells and HX711 + esp8266 D1 mini + 2 OLED I2C 0.96" displays (each display needs to have a unique I2C adress)

OLED displays are both connected to SDA=D2 (gpio4) SCL=D1 (gpio5) my display have adresses 0x3c and 0x3d

HX711 scale #1 is connected to SDA=D5(gpio14) SCL=D6(gpio12)
HX711 scale #2 is connected to SDA=D3(gpio0) SCL=D4(gpio2)
DH22 temp and humidity sensor can be connected to D7(gpio13)

See: schema/schema.pdf (does not include load cell connections)

# This project is a work in progress

Features so far;

- Wifi setup is done via SSID:kegscale and password:password , enter up to 2 SSID that will be used to connect
- Can show units of weight and remaning pints (keg weight and pint weight is set in configuration for each of the two scales)
- Web based configuration once connected to wifi. Configuration includes, tare and automatic calculation of scaling factor.
- Can fetch data on brews abv, ebc, ibu from brewfather
- Showing beer name, abv, scale weight and remaning pints on display
- Support 2 displays and 2 scales
- Firmware upload function via web interface
- Temp and humidity sensor can be attached

# Todo

* Try out some 3d printed parts for the scale (or create my own)
* Create a nice 3d holder for the displays and esp that fits on my keezer.
* Rewrite code for hx711 connection so it does not hang when a scale is not connected.

* Allow user to select weight unit to show (does not really matter for the functionallity), now defaults to kg.
* Do a ESP32 port (might be needed to attach more units)
* Implement screen saver for OLED
* Implement async webserver so it's more responsive to fast requests
* Write API tests
* Create documentation
