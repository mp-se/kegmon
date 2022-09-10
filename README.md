
![download](https://img.shields.io/github/downloads/mp-se/kegscale/total) 
![release](https://img.shields.io/github/v/release/mp-se/kegscale?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/kegscale)
![pr](https://img.shields.io/github/issues-pr/mp-se/kegscale)
![dev_build](https://img.shields.io/github/workflow/status/mp-se/kegscale/PlatformIO%20CI/dev?label=dev%20build)
![doc_build](https://img.shields.io/github/workflow/status/mp-se/kegscale/Sphinx%20Build/dev?label=doc%20build)

# KegScale

This is a project that I have done for my own Keezer, if you like it please feel free to suggest improvements. Feel free to use the code according to the licence agreement. 

Based on the ideas of these projects (there are many that have implemented the same idea);

* https://www.hackster.io/davidtilley/iot-home-beer-keg-scale-b603db
* https://www.instructables.com/Beer-Keg-Scales/
* https://brewkegscale.wordpress.com/
* https://github.com/Callwater/Beerkeg-load-cell
* https://github.com/nanab/BeerScale

For docs see: https://mp-se.github.io/kegscale/index.html

# Update 2022 August

After running my first build for a few weeks I noticed that the weight was not really stable so the HX711 scales are not that accurate. I've build in a few functions to measure the stability of the scales and use
average values instead of the last reading. This has improved stability for one of my two scales so I'm also looking into hardware design to see what can be improved. Ideas so far:

* Use average values over a longer time
* Try out HX chips from other manufacturers.
* Move HX chip from scale to outside the fride
* Use larger weights for calibration (currently using 1 kg)
