/*
MIT License

Copyright (c) 2023-2024 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#ifndef SRC_DISPLAYOUT_HPP_
#define SRC_DISPLAYOUT_HPP_

#include <cstdio>
#include <kegconfig.hpp>
#include <main.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>

enum DisplayIterator {
  ShowWeight = 0,
  ShowGlasses = 1,
  ShowPour = 2,
  ShowTemp = 3
};

extern WifiConnection myWifi;

class DisplayLayout {
 private:
  DisplayIterator _iter = DisplayIterator::ShowWeight;
  uint32_t _loopMillis = 0;
  char _buf[30] = "";

  String getFormattedBeerName(UnitIndex idx) {
    snprintf(&_buf[0], sizeof(_buf), "%d:%s", idx + 1,
             myConfig.getBeerName(idx));
    return String(_buf);
  }

  String getFormattedBeerABV(UnitIndex idx) {
    snprintf(&_buf[0], sizeof(_buf), "%.1f%%", myConfig.getBeerABV(idx));
    return String(_buf);
  }

  String getFormattedBeerWeight(float beerWeight) {
    convertFloatToString(beerWeight, &_buf[0], myConfig.getWeightPrecision());
    String s(_buf);
    s.trim();
    s = "Beer " + s + " " + String(myConfig.getWeightUnit());
    return s;
  }

  String getFormattedBeerVolume(float beerVolume) {
    convertFloatToString(beerVolume, &_buf[0], myConfig.getVolumePrecision());
    String s(_buf);
    s.trim();
    s = "Beer " + s + " " + String(myConfig.getVolumeUnit());
    return s;
  }

  String getFormattedGlasses(float glass) {
    snprintf(&_buf[0], sizeof(_buf), "%.1f glasses", glass);
    return String(_buf);
  }

  String getFormattedPour(float pour) {
    snprintf(&_buf[0], sizeof(_buf), "%.0f pour", pour * 100);
    return String(_buf);
  }

  String getFormattedTemp(float tempC) {
    if (isnan(tempC)) return String("No temperature");

    if (myConfig.getTempFormat() == 'F') tempC = convertCtoF(tempC);

    convertFloatToString(tempC, &_buf[0], 2);
    String s(_buf);
    s.trim();
    s += " " + String(myConfig.getTempFormat());
    return s;
  }

  String getFormattedStableLevel(bool stable) {
    if (stable) return String("Stable level");

    return String("Searching level");
  }

  String getFormattedWifiName() { return myConfig.getWifiSSID(0); }

  String getFormattedIP() { return myWifi.getIPAddress(); }

  void showDefault(UnitIndex idx, bool isScaleConnected, float beerWeight,
                   float glasses, float pour, float temp, bool stableLevel);
  void showGraph(UnitIndex idx, bool isScaleConnected, float beerVolume,
                 float pour);
  void showGraphOne(UnitIndex idx, bool isScaleConnected, float beerVolume,
                    float pour, float beerWeight);
  void showHardwareStats(UnitIndex idx, bool isScaleConnected);

 public:
  DisplayLayout();
  void loop();

  void showWifiPortal();
  void showStartupDevices(bool hasScale1, bool hasScale2, bool hasTemp);
  void showCurrent(UnitIndex idx, bool isScaleConnected, float beerWeight,
                   float beerVolume, float glasses, float pour, float temp,
                   bool stableLevel);
};

extern DisplayLayout myDisplayLayout;

#endif  // SRC_DISPLAYOUT_HPP_

// EOF
