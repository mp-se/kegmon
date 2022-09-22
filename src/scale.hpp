/*
MIT License

Copyright (c) 2021-22 Magnus

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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <HX711.h>

#include <kegconfig.hpp>
#include <levels.hpp>
#include <main.hpp>

#define LEVELS_FILENAME "levels.log"
#define LEVELS_FILENAME2 "levels2.log"
#define LEVELS_FILEMAXSIZE 2000

// Note! Internally we assume that everything are in Metric formats, weights=Kg
// and all volumes=Liters.

class WeightVolumeConverter {
 private:
  float _fg;
  UnitIndex _idx;

 public:
  explicit WeightVolumeConverter(UnitIndex idx) {
    _idx = idx;
    _fg = myConfig.getBeerFG(idx);
    if (_fg < 1) _fg = 1;
  }

  float weightToVolume(float kg) {
    float liter = isnan(kg) || kg == 0 ? 0 : kg / _fg;
    return liter;
  }

  float weightToGlasses(float kg) {
    float glassVol = myConfig.getGlassVolume(_idx);
    float glassWeight = glassVol * _fg;
    float glass = kg / glassWeight;
    return glass < 0 ? 0 : glass;
  }
};

class Scale {
 private:
  // Setup and scale handling
  HX711* _scale[2] = {0, 0};
  Stability _stability[2];
  KalmanLevelDetection* _kalmanLevel[2] = {0, 0};
  StatsLevelDetection* _statsLevel[2] = {0, 0};

  Scale(const Scale&) = delete;
  void operator=(const Scale&) = delete;

  void setScaleFactor(UnitIndex idx);
  void logLevels(float kegVolume1, float kegVolume2, float pourVolume1,
                 float pourVolume2);

 public:
  Scale();

  Stability& getStability(UnitIndex idx) { return _stability[idx]; }
  KalmanLevelDetection* getKalmanDetection(UnitIndex idx) {
    return _kalmanLevel[idx];
  }
  StatsLevelDetection* getStatsDetection(UnitIndex idx) {
    return _statsLevel[idx];
  }

  // Setup and scale handling
  void setup(bool force = false);
  void tare(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  bool isConnected(UnitIndex idx) { return _scale[idx] != 0 ? true : false; }
  int32_t readRaw(UnitIndex idx);
  float read(UnitIndex idx, bool updateStats);

  void pushKegUpdate(UnitIndex idx);
  void pushPourUpdate(UnitIndex idx);

  // Shortcuts to subclasses....
  bool hasWeight(UnitIndex idx) { return getKalmanDetection(idx)->hasValue(); }
  bool hasStableWeight(UnitIndex idx) {
    return getStatsDetection(idx)->hasStableValue();
  }
  bool hasPourWeight(UnitIndex idx) {
    return getStatsDetection(idx)->hasPourValue();
  }

  // Returns weights in kg
  float getTotalWeight(UnitIndex idx) {
    return getKalmanDetection(idx)->getValue();
  }
  float getTotalRawWeight(UnitIndex idx) {
    return getKalmanDetection(idx)->getRawValue();
  }
  float getTotalStableWeight(UnitIndex idx) {
    return getStatsDetection(idx)->getStableValue();
  }
  float getBeerWeight(UnitIndex idx) {
    return getKalmanDetection(idx)->hasValue()
               ? getKalmanDetection(idx)->getValue() -
                     myConfig.getKegWeight(idx)
               : NAN;
  }
  float getBeerStableWeight(UnitIndex idx) {
    return getStatsDetection(idx)->hasStableValue()
               ? getStatsDetection(idx)->getStableValue() -
                     myConfig.getKegWeight(idx)
               : NAN;
  }
  float getPourWeight(UnitIndex idx) {
    return getStatsDetection(idx)->getPourValue();
  }

  // Returns weights in liters
  float getBeerVolume(UnitIndex idx) {
    WeightVolumeConverter conv(idx);
    return conv.weightToVolume(getBeerWeight(idx));
  }
  float getBeerStableVolume(UnitIndex idx) {
    WeightVolumeConverter conv(idx);
    return conv.weightToVolume(getBeerStableWeight(idx));
  }
  float getPourVolume(UnitIndex idx) {
    WeightVolumeConverter conv(idx);
    return conv.weightToVolume(getPourWeight(idx));
  }

  float getNoGlasses(UnitIndex idx) {
    WeightVolumeConverter conv(idx);
    return conv.weightToGlasses(getBeerWeight(idx));
  }
  float getNoStableGlasses(UnitIndex idx) {
    WeightVolumeConverter conv(idx);
    return conv.weightToGlasses(getBeerStableWeight(idx));
  }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
