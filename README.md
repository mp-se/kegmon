
![download](https://img.shields.io/github/downloads/mp-se/kegmon/total) 
![release](https://img.shields.io/github/v/release/mp-se/kegmon?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/kegmon)
![pr](https://img.shields.io/github/issues-pr/mp-se/kegmon)
![dev_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon/pio-build.yaml?branch=dev)
![doc_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon/doc-build.yaml?branch=master)

# KegMon - Monitoring the volume in your kegs

![KegMon Logo](src_docs/source/images/kegmon_logo_s.png)

This is a project that I have done for my own Keezer, if you like it please feel free to suggest improvements. 

For docs see: https://mp-se.github.io/kegmon/index.html

# Hardware

* Supports ESP8266 D1 mini or ESP32s2 mini
* Loadcells and HX711/NAU7802 ADC converters
* DHT22 or DS18B20 temperature sensors
* OLED 0.92" display

# Features

* Can measure weight from 1 or 2 scales
* Stable scale presentation (data is filtered and processed)
* Support temperature/humidity sensor in one scale base
* Integration with Brewfather to retrive data on brew
* Integration with Brewspy to retrive data on brew and also update the remaning volume and pours
* Integration with Home Assistant
* Weights and Volumes in Metric, Imperial (both US and UK)
* Pour detection for pours over 100 ml
  