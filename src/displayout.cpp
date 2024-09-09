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
#include <display.hpp>
#include <displayout.hpp>
#include <levels.hpp>

constexpr auto DISPLAY_ITER_TIME = 4000;

DisplayLayout::DisplayLayout() { _loopMillis = millis(); }

void DisplayLayout::loop() {
  if (abs((int32_t)(millis() - _loopMillis)) > DISPLAY_ITER_TIME) {
    _loopMillis = millis();

    switch (_iter) {
      case DisplayIterator::ShowWeight:
        _iter = DisplayIterator::ShowGlasses;
        break;
      case DisplayIterator::ShowGlasses:
        _iter = DisplayIterator::ShowPour;
        break;
      case DisplayIterator::ShowPour:
        _iter = DisplayIterator::ShowWeight;
        break;
      case DisplayIterator::ShowTemp:
        _iter = DisplayIterator::ShowTemp;
        break;
    }
  }
}

void DisplayLayout::showWifiPortal() {
  myDisplay.clear(UnitIndex::U1);
  myDisplay.setFont(UnitIndex::U1, FontSize::FONT_16);
  myDisplay.printLineCentered(UnitIndex::U1, 0, "WIFI Portal Active");
  myDisplay.printLineCentered(UnitIndex::U1, 1, "192.168.4.1");
  myDisplay.show(UnitIndex::U1);
}

void DisplayLayout::showStartupDevices(bool hasScale1, bool hasScale2,
                                       bool hasTemp) {
  char buf[30];

  myDisplay.clear(UnitIndex::U1);
  myDisplay.setFont(UnitIndex::U1, FontSize::FONT_10);
  snprintf(&buf[0], sizeof(buf), "Scale 1: %s", hasScale1 ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 0, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Scale 2: %s", hasScale2 ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 1, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Temp : %s", hasTemp ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 2, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Version: %s (%s)", CFG_APPVER,
#if defined(ENABLE_INFLUX_DEBUG)
           "Yes");
#else
           "No");
#endif
  myDisplay.printLine(UnitIndex::U1, 3, &buf[0]);
  myDisplay.show(UnitIndex::U1);
}

void DisplayLayout::showDefault(UnitIndex idx, bool isScaleConnected,
                                float beerWeight, float glasses, float pour,
                                float tempC, bool stableLevel) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_16);

  myDisplay.printPosition(idx, -1, 0, getFormattedBeerName(idx));

  if (isScaleConnected) {
    myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 1,
                            getFormattedBeerABV(idx));

    switch (_iter) {
      case DisplayIterator::ShowWeight:
        myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                                getFormattedBeerWeight(beerWeight));
        break;

      case DisplayIterator::ShowGlasses:
        myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                                getFormattedGlasses(glasses));
        break;

      case DisplayIterator::ShowPour:
        myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                                getFormattedPour(pour));
        break;

      case DisplayIterator::ShowTemp:
        myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                                getFormattedTemp(tempC));
        break;
    }

  } else {
    myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                            "No scale");
  }

  myDisplay.setFont(idx, FontSize::FONT_10);

  switch (_iter) {
    case DisplayIterator::ShowTemp:
    case DisplayIterator::ShowWeight:
      myDisplay.printPosition(
          idx, -1,
          myDisplay.getDisplayHeight(idx) - myDisplay.getFontHeight(idx),
          getFormattedStableLevel(stableLevel));
      break;
    case DisplayIterator::ShowGlasses:
      if (idx == UnitIndex::U1) {
        myDisplay.printPosition(
            idx, -1,
            myDisplay.getDisplayHeight(idx) - myDisplay.getFontHeight(idx),
            getFormattedWifiName());
      }
      break;
    case DisplayIterator::ShowPour:
      if (idx == UnitIndex::U1) {
        myDisplay.printPosition(
            idx, -1,
            myDisplay.getDisplayHeight(idx) - myDisplay.getFontHeight(idx),
            getFormattedIP());
      }
      break;
  }

  myDisplay.show(idx);
}

void DisplayLayout::showGraph(UnitIndex idx, bool isScaleConnected,
                              float beerVolume, float pour) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_16);

  myDisplay.printPosition(idx, -1, 0, getFormattedBeerName(idx));
  myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 1,
                          getFormattedPour(pour));

  float keg = myConfig.getKegVolume(idx);

  if (isScaleConnected) {
    myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 2,
                            getFormattedBeerVolume(beerVolume));
    myDisplay.drawProgressBar(idx, myDisplay.getFontHeight(idx) * 3,
                              keg / beerVolume);
  } else {
    myDisplay.printPosition(idx, -1, myDisplay.getFontHeight(idx) * 3,
                            "No scale");
  }

  myDisplay.show(idx);
}

void DisplayLayout::showGraphOne(UnitIndex idx, bool isScaleConnected,
                                 float beerVolume, float pour,
                                 float beerWeight) {
  // Handle printout to display #1 which will show all the graphs
  if (idx == UnitIndex::U1) {
    myDisplay.clear(UnitIndex::U1);
    myDisplay.setFont(UnitIndex::U1, FontSize::FONT_16);
    myDisplay.printPosition(UnitIndex::U1, -1, 0, getFormattedBeerName(idx));

    float keg1 = myConfig.getKegVolume(UnitIndex::U1);

    if (isScaleConnected) {
      myDisplay.drawProgressBar(UnitIndex::U1,
                                myDisplay.getFontHeight(UnitIndex::U1) * 1,
                                keg1 / beerVolume);
    } else {
      myDisplay.printPosition(UnitIndex::U1, -1,
                              myDisplay.getFontHeight(UnitIndex::U1) * 1,
                              "No scale");
    }
  } else {
    myDisplay.printPosition(UnitIndex::U1, -1,
                            myDisplay.getFontHeight(UnitIndex::U1) * 2,
                            getFormattedBeerName(idx));

    float keg2 = myConfig.getKegVolume(UnitIndex::U2);

    if (isScaleConnected) {
      myDisplay.drawProgressBar(UnitIndex::U1,
                                myDisplay.getFontHeight(UnitIndex::U1) * 3,
                                keg2 / beerVolume);
    } else {
      myDisplay.printPosition(UnitIndex::U1, -1,
                              myDisplay.getFontHeight(UnitIndex::U1) * 3,
                              "No scale");
    }
    myDisplay.show(UnitIndex::U1);
  }

  // Handle printout to display #2 which will show all the details (iterating
  // between the two displays)
  if (isScaleConnected) {
    myDisplay.setFont(UnitIndex::U2, FontSize::FONT_16);

    switch (_iter) {
      case DisplayIterator::ShowTemp:
      case DisplayIterator::ShowWeight:
        if (idx == UnitIndex::U1) {
          myDisplay.clear(UnitIndex::U2);

          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(UnitIndex::U2) * 0,
                                  getFormattedBeerName(idx));
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(UnitIndex::U2) * 1,
                                  getFormattedPour(pour));
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(idx) * 2,
                                  getFormattedBeerWeight(beerWeight));

          myDisplay.setFont(UnitIndex::U2, FontSize::FONT_10);
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getDisplayHeight(UnitIndex::U2) -
                                      myDisplay.getFontHeight(UnitIndex::U2),
                                  getFormattedWifiName());
        }
        break;
      case DisplayIterator::ShowGlasses:
      case DisplayIterator::ShowPour:
        if (idx == UnitIndex::U2) {
          myDisplay.clear(UnitIndex::U2);

          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(UnitIndex::U2) * 0,
                                  getFormattedBeerName(idx));
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(UnitIndex::U2) * 1,
                                  getFormattedPour(pour));
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getFontHeight(idx) * 2,
                                  getFormattedBeerWeight(beerWeight));

          myDisplay.setFont(UnitIndex::U2, FontSize::FONT_10);
          myDisplay.printPosition(UnitIndex::U2, -1,
                                  myDisplay.getDisplayHeight(UnitIndex::U2) -
                                      myDisplay.getFontHeight(UnitIndex::U2),
                                  getFormattedIP());
        }
        break;
    }
    myDisplay.show(UnitIndex::U2);
  }
}

void DisplayLayout::showHardwareStats(UnitIndex idx, bool isScaleConnected) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_10);

  if (isScaleConnected) {
    snprintf(&_buf[0], sizeof(_buf), "Last wgt: %.3f",
             myLevelDetection.getTotalWeight(idx));
    myDisplay.printLine(idx, 0, &_buf[0]);
    snprintf(&_buf[0], sizeof(_buf), "Stab wgt: %.3f",
             myLevelDetection.getTotalStableWeight(idx));
    myDisplay.printLine(idx, 1, &_buf[0]);
    snprintf(&_buf[0], sizeof(_buf), "Ave  wgt: %.3f",
             myLevelDetection.getStatsDetection(idx)->ave());
    myDisplay.printLine(idx, 2, &_buf[0]);
    snprintf(&_buf[0], sizeof(_buf), "Min/Max: %.3f/%.3f",
             myLevelDetection.getStatsDetection(idx)->min(),
             myLevelDetection.getStatsDetection(idx)->max());
    myDisplay.printLine(idx, 3, &_buf[0]);

    snprintf(&_buf[0], sizeof(_buf), "Raw  wgt: %.3f",
             myLevelDetection.getTotalRawWeight(idx));
    myDisplay.printLine(idx, 4, &_buf[0]);
  }

  myDisplay.show(idx);
}

void DisplayLayout::showCurrent(UnitIndex idx, bool isScaleConnected,
                                float beerWeight, float beerVolume,
                                float glasses, float pour, float temp,
                                bool stableLevel) {
  switch (myConfig.getDisplayLayoutType()) {
    default:
    case DisplayLayoutType::Default:
      showDefault(idx, isScaleConnected, beerWeight, glasses, pour, temp,
                  stableLevel);
      break;

    case DisplayLayoutType::Graph:
      showGraph(idx, isScaleConnected, beerVolume, pour);
      break;

    case DisplayLayoutType::GraphOne:
      showGraphOne(idx, isScaleConnected, beerVolume, pour, beerWeight);
      break;

    case DisplayLayoutType::HardwareStats:
      showHardwareStats(idx, isScaleConnected);
      break;
  }
}

// EOF
