.. _development:

Development
===========

Development of this project is done in VSCode with the PlatformIO extension using Arduino for ESP8266 and ESP32. 

For the documentation we use SPINX and github pages for publishing.

Compliance to coding standards PRE-COMMIT is used with their standard rules for formatting and good cpp code. Any PR needs 
to pass this validation in order to be considered for merging.

The pipeline is build based on github actions and all firmware builds and documentation publishing is done using the 
defined github actions.

For testing there is a UNIT TEST target defined which currently needs to be run on hardware. The plan is to move this to 
WOKWI and their github action to run these after a completed build. There is also a python script that can be used to validate the 
output of the available API's to ensure they deliver what is wanted. 

Future
------

The following additions is being worked on. 

* WOKWI simulator, so that the software can be run wihtout the need for hardware. 
* WOKWI CI, to run the unit test and API tests during builds.
* Update PR checks to include builds and unit testing. 

Links
-----
If you want to start contributing these are links to the tools I use.

* https://code.visualstudio.com/ (Code Editor)
* https://platformio.org/ (Arduino development)
* https://wokwi.com/ (Simulator for ESP32)
* https://marketplace.visualstudio.com/items?itemName=HookyQR.minify (Extension for making HTML files smaller)
* https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide (Platform IO extension)
* https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode (Wokwi simulator extension)
* https://pre-commit.com/ (Code validation, need to run this in Linux/WSL)
* https://cli.github.com/ (Source code management)
* https://gitforwindows.org/ (Git for Windows)

Other projects that are of interrest for development

* https://github.com/nektos/act (Run github actions locally)

