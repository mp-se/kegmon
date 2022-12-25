
![download](https://img.shields.io/github/downloads/mp-se/kegmon/total) 
![release](https://img.shields.io/github/v/release/mp-se/kegmon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/kegmon)
![pr](https://img.shields.io/github/issues-pr/mp-se/kegmon)
![dev_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon/pio-build.yaml?branch=dev)
![doc_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon/doc-build.yaml?branch=master)

# KegMon - Monitoring the volume in your kegs

![KegMon Logo](src_docs/source/images/kegmon_logo_s.png)

This is a project that I have done for my own Keezer, if you like it please feel free to suggest improvements. 

For docs see: https://mp-se.github.io/kegmon/index.html (Not yet updated to match v0.4)

# Features

* Based on ESP8266 D1 mini, HX711 sensors and 2 OLED 0.92" displays
* Can measure weight from 1 or 2 scales
* Stable scale presentation (data is filtered and processed)
* Support temperature/humidity sensor in one scale base
* Integration with Brewfather to retrive data on brew
* Integration with Brewspy to retrive data on brew and also update the remaning volume and pours
* Display data in weight or volume
* Weights and Volumes in Metric, Imperial (both US and UK)
* Pour detection for pours over 100 ml

# Future considerations

* Considering switching to ESP32 for more processing power.
* Switching to larger display, 3-4" TFT (will require ESP32)
* PCB and 3d printed case design
  