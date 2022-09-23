
![download](https://img.shields.io/github/downloads/mp-se/kegmon/total) 
![release](https://img.shields.io/github/v/release/mp-se/kegmon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/kegmon)
![pr](https://img.shields.io/github/issues-pr/mp-se/kegmon)
![dev_build](https://img.shields.io/github/workflow/status/mp-se/kegmon/PlatformIO%20CI/dev?label=dev%20build)
![doc_build](https://img.shields.io/github/workflow/status/mp-se/kegmon/Sphinx%20Build/dev?label=doc%20build)

# KegMon

This is a project that I have done for my own Keezer, if you like it please feel free to suggest improvements. Feel free to use the code according to the licence agreement. 

Based on the ideas of these projects (there are many that have implemented the same idea);

* https://www.hackster.io/davidtilley/iot-home-beer-keg-scale-b603db
* https://www.instructables.com/Beer-Keg-Scales/
* https://brewkegscale.wordpress.com/
* https://github.com/Callwater/Beerkeg-load-cell
* https://github.com/nanab/BeerScale

For docs see: https://mp-se.github.io/kegmon/index.html (Not yet updated to match v0.4)

# Update 2022 September

After a lot of differnet testing and building prototypes I think I have a good base to build on. Averaging the reads over a period of time seams to provide stable readings even though it takes a while for the pour detection to recognize a stable level.

I tried to use a kalman filter but that didt respond well to sudden changes in level so I ended up with looking at the last reads for stability and using statistics/averaging for determining a stable level in the keg. It's probably not as accuracte as having a flow meter
but it's good enough to have a rough view on how many pints are left in the keg. 

Now for some long term testing to determine stability and accuracy, just need to have a few pints.....

I think most of the features I wanted to add has been added, just need to check that they work as planned.

Happy building