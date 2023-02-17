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
#ifndef SRC_KEGCONFIG_HPP_
#define SRC_KEGCONFIG_HPP_

#include <baseconfig.hpp>
#include <main.hpp>

#define PARAM_BREWFATHER_USERKEY "brewfather-userkey"
#define PARAM_BREWFATHER_APIKEY "brewfather-apikey"
#define PARAM_BREWSPY_TOKEN1 "brewspy-token1"
#define PARAM_BREWSPY_TOKEN2 "brewspy-token2"
#define PARAM_DISPLAY_LAYOUT "display-layout"
#define PARAM_WEIGHT_UNIT "weight-unit"
#define PARAM_VOLUME_UNIT "volume-unit"
#define PARAM_KEG_WEIGHT1 "keg-weight1"
#define PARAM_KEG_WEIGHT2 "keg-weight2"
#define PARAM_KEG_VOLUME1 "keg-volume1"
#define PARAM_KEG_VOLUME2 "keg-volume2"
#define PARAM_GLASS_VOLUME1 "glass-volume1"
#define PARAM_GLASS_VOLUME2 "glass-volume2"
#define PARAM_BEER_NAME1 "beer-name1"
#define PARAM_BEER_NAME2 "beer-name2"
#define PARAM_BEER_ABV1 "beer-abv1"
#define PARAM_BEER_ABV2 "beer-abv2"
#define PARAM_BEER_IBU1 "beer-ibu1"
#define PARAM_BEER_IBU2 "beer-ibu2"
#define PARAM_BEER_EBC1 "beer-ebc1"
#define PARAM_BEER_EBC2 "beer-ebc2"
#define PARAM_BEER_FG1 "beer-fg1"
#define PARAM_BEER_FG2 "beer-fg2"
#define PARAM_SCALE_FACTOR1 "scale-factor1"
#define PARAM_SCALE_FACTOR2 "scale-factor2"
#define PARAM_SCALE_OFFSET1 "scale-offset1"
#define PARAM_SCALE_OFFSET2 "scale-offset2"
#define PARAM_SCALE_TEMP_FORMULA1 "scale-temp-formula1"
#define PARAM_SCALE_TEMP_FORMULA2 "scale-temp-formula2"
#define PARAM_SCALE_DEVIATION_INCREASE "scale-deviation-increase"
#define PARAM_SCALE_DEVIATION_DECREASE "scale-deviation-decrease"
#define PARAM_SCALE_DEVIATION_KALMAN "scale-deviation-kalman"
#define PARAM_SCALE_READ_COUNT "scale-read-count"
#define PARAM_SCALE_READ_COUNT_CALIBRATION "scale-read-count-calibration"
#define PARAM_SCALE_STABLE_COUNT "scale-stable-count"
#define PARAM_LEVEL_DETECTION "level-detection"
#define PARAM_KALMAN_NOISE "kalman-noise"
#define PARAM_KALMAN_MEASUREMENT "kalman-measurement"
#define PARAM_KALMAN_ESTIMATION "kalman-estimation"
#define PARAM_KALMAN_ACTIVE "kalman-active"

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
  float _fg = 1;
};

#define WEIGHT_KG "kg"
#define WEIGHT_LBS "lbs"

#define VOLUME_CL "cl"
#define VOLUME_US "us-oz"
#define VOLUME_UK "uk-oz"

enum DisplayLayout { Default = 0, HardwareStats = 9 };

float convertIncomingWeight(float w);
float convertIncomingVolume(float v);
float convertOutgoingWeight(float w);
float convertOutgoingVolume(float v);
float convertOutgoingTemperature(float t);

class KegConfig : public BaseConfig {
 private:
  String _weightUnit = WEIGHT_KG;
  String _volumeUnit = VOLUME_CL;

  String _brewfatherUserKey = "";
  String _brewfatherApiKey = "";

  String _brewspyToken[2] = {"", ""};

  DisplayLayout _displayLayout = DisplayLayout::Default;

  float _scaleFactor[2] = {0, 0};
  int32_t _scaleOffset[2] = {0, 0};
  float _kegWeight[2] = {4, 4};          // Weight in kg
  float _kegVolume[2] = {19, 19};        // Weight in liters
  float _glassVolume[2] = {0.40, 0.40};  // Volume in liters
  BeerInfo _beer[2];

  float _scaleDeviationIncreaseValue = 0.4;  // kg
  float _scaleDeviationDecreaseValue = 0.1;  // kg
  float _scaleKalmanDeviation = 0.05;
  uint32_t _scaleStableCount = 8;
  int _scaleReadCount = 3;
  int _scaleReadCountCalibration = 30;
  String _scaleTempCompensationFormula[2] = {"", ""};

  LevelDetectionType _levelDetection = LevelDetectionType::STATS;

  /*
  bool _kalmanActive = true;
  float _kalmanMeasurement = 0.3;
  float _kalmanEstimation = 2;
  float _kalmanNoise = 0.01;
  */

 public:
  KegConfig(String baseMDNS, String fileName);

  void createJson(DynamicJsonDocument& doc, bool skipSecrets = true);
  void parseJson(DynamicJsonDocument& doc);

  const char* getBrewfatherUserKey() { return _brewfatherUserKey.c_str(); }
  void setBrewfatherUserKey(String s) {
    _brewfatherUserKey = s;
    _saveNeeded = true;
  }
  const char* getBrewfatherApiKey() { return _brewfatherApiKey.c_str(); }
  void setBrewfatherApiKey(String s) {
    _brewfatherApiKey = s;
    _saveNeeded = true;
  }

  const char* getBrewspyToken(UnitIndex idx) {
    return _brewspyToken[idx].c_str();
  }
  void setBrewspyToken(UnitIndex idx, String s) {
    _brewspyToken[idx] = s;
    _saveNeeded = true;
  }

  const char* getBeerName(UnitIndex idx) { return _beer[idx]._name.c_str(); }
  void setBeerName(UnitIndex idx, String s) {
    _beer[idx]._name = s;
    _saveNeeded = true;
  }
  float getBeerABV(UnitIndex idx) { return _beer[idx]._abv; }
  void setBeerABV(UnitIndex idx, float f) {
    _beer[idx]._abv = f;
    _saveNeeded = true;
  }
  float getBeerFG(UnitIndex idx) { return _beer[idx]._fg; }
  void setBeerFG(UnitIndex idx, float f) {
    _beer[idx]._fg = f;
    _saveNeeded = true;
  }
  int getBeerEBC(UnitIndex idx) { return _beer[idx]._ebc; }
  void setBeerEBC(UnitIndex idx, int i) {
    _beer[idx]._ebc = i;
    _saveNeeded = true;
  }
  int getBeerIBU(UnitIndex idx) { return _beer[idx]._ibu; }
  void setBeerIBU(UnitIndex idx, int i) {
    _beer[idx]._ibu = i;
    _saveNeeded = true;
  }

  float getKegWeight(UnitIndex idx) { return _kegWeight[idx]; }
  void setKegWeight(UnitIndex idx, float f) {
    _kegWeight[idx] = f;
    _saveNeeded = true;
  }

  float getKegVolume(UnitIndex idx) { return _kegVolume[idx]; }
  void setKegVolume(UnitIndex idx, float f) {
    _kegVolume[idx] = f;
    _saveNeeded = true;
  }

  float getGlassVolume(UnitIndex idx) { return _glassVolume[idx]; }
  void setGlassVolume(UnitIndex idx, float f) {
    _glassVolume[idx] = f;
    _saveNeeded = true;
  }

  const char* getWeightUnit() { return _weightUnit.c_str(); }
  bool isWeightUnitKG() { return _weightUnit.equals(WEIGHT_KG); }
  bool isWeightUnitLBS() { return _weightUnit.equals(WEIGHT_LBS); }
  void setWeightUnit(String s) {
    if (!s.compareTo(WEIGHT_KG) || !s.compareTo(WEIGHT_LBS)) {
      _weightUnit = s;
      _saveNeeded = true;
    }
  }

  const char* getVolumeUnit() { return _volumeUnit.c_str(); }
  bool isVolumeUnitCL() { return _volumeUnit.equals(VOLUME_CL); }
  bool isVolumeUnitUSOZ() { return _volumeUnit.equals(VOLUME_US); }
  bool isVolumeUnitUKOZ() { return _volumeUnit.equals(VOLUME_UK); }
  void setVolumeUnit(String s) {
    if (!s.compareTo(VOLUME_CL) || !s.compareTo(VOLUME_UK) || !s.compareTo(VOLUME_US)) {
      _volumeUnit = s;
      _saveNeeded = true;
    }
  }

  int32_t getScaleOffset(UnitIndex idx) { return _scaleOffset[idx]; }
  void setScaleOffset(UnitIndex idx, int32_t l) {
    _scaleOffset[idx] = l;
    _saveNeeded = true;
  }

  float getScaleFactor(UnitIndex idx) { return _scaleFactor[idx]; }
  void setScaleFactor(UnitIndex idx, float f) {
    _scaleFactor[idx] = f;
    _saveNeeded = true;
  }

  DisplayLayout getDisplayLayout() { return _displayLayout; }
  int getDisplayLayoutAsInt() { return _displayLayout; }
  void setDisplayLayout(DisplayLayout d) {
    _displayLayout = d;
    _saveNeeded = true;
  }
  void setDisplayLayout(int d) {
    _displayLayout = (DisplayLayout)d;
    _saveNeeded = true;
  }

  // This is the maximum allowed deviation
  float getScaleDeviationDecreaseValue() {
    return _scaleDeviationDecreaseValue;
  }  // 0.1 kg
  void setScaleDeviationDecreaseValue(float f) {
    _scaleDeviationDecreaseValue = f;
    _saveNeeded = true;
  }

  float getScaleDeviationIncreaseValue() {
    return _scaleDeviationIncreaseValue;
  }  // 0.1 kg
  void setScaleDeviationIncreaseValue(float f) {
    _scaleDeviationIncreaseValue = f;
    _saveNeeded = true;
  }

  // This is the maximum allowed deviation between kalman and raw value for
  // level checking to work.
  float getScaleKalmanDeviationValue() { return _scaleKalmanDeviation; }
  void setScaleKalmanDeviationValue(float f) {
    _scaleKalmanDeviation = f;
    _saveNeeded = true;
  }

  // This is the number of values in the statistics for the average value to be
  // classifed as stable. Loop interval is 2s
  uint32_t getScaleStableCount() { return _scaleStableCount; }
  void setScaleStableCount(uint32_t i) {
    _scaleStableCount = i;
    _saveNeeded = true;
  }

  int getScaleReadCount() { return _scaleReadCount; }
  void setScaleReadCount(uint32_t i) {
    _scaleReadCount = i;
    _saveNeeded = true;
  }

  int getScaleReadCountCalibration() { return _scaleReadCountCalibration; }
  void setScaleReadCountCalibration(uint32_t i) {
    _scaleReadCountCalibration = i;
    _saveNeeded = true;
  }

  LevelDetectionType getLevelDetection() { return _levelDetection; }
  int getLevelDetectionAsInt() { return _levelDetection; }
  /*void setLevelDetection(LevelDetectionType l) {
    _levelDetection = l;
    _saveNeeded = true;
  }
  void setLevelDetection(int l) {
    _levelDetection = (LevelDetectionType)l;
    _saveNeeded = true;
  }*/

  /*
  float getKalmanEstimation() { return _kalmanEstimation; }
  void setKalmanEstimation(float f) {
    _kalmanEstimation = f;
    _saveNeeded = true;
  }
  float getKalmanMeasurement() { return _kalmanMeasurement; }
  void setKalmanMeasurement(float f) {
    _kalmanMeasurement = f;
    _saveNeeded = true;
  }
  float getKalmanNoise() { return _kalmanNoise; }
  void setKalmanNoise(float f) {
    _kalmanNoise = f;
    _saveNeeded = true;
  }
  bool isKalmanActive() { return _kalmanActive; }
  void setKalmanActive(bool b) {
    _kalmanActive = b;
    _saveNeeded = true;
  }
  */

  const char* getScaleTempCompensationFormula(UnitIndex idx) {
    return _scaleTempCompensationFormula[idx].c_str();
  }
  void setScaleTempCompensationFormula(UnitIndex idx, String s) {
    _scaleTempCompensationFormula[idx] = s;
    _saveNeeded = true;
  }

  // These settings are used for debugging and checking stability of the scales.
  // Only influx is used for now
  const char* getTargetHttpPost() { return ""; }
  void setTargetHttpPost(String target) {}
  const char* getHeader1HttpPost() { return ""; }
  void setHeader1HttpPost(String header) {}
  const char* getHeader2HttpPost() { return ""; }
  void setHeader2HttpPost(String header) {}

  const char* getTargetHttpGet() { return ""; }
  void setTargetHttpGet(String target) {}
  const char* getHeader1HttpGet() { return ""; }
  void setHeader1HttpGet(String header) {}
  const char* getHeader2HttpGet() { return ""; }
  void setHeader2HttpGet(String header) {}

  const char* getTargetInfluxDB2() { return ""; }
  void setTargetInfluxDB2(String target) {}
  const char* getOrgInfluxDB2() { return ""; }
  void setOrgInfluxDB2(String org) {}
  const char* getBucketInfluxDB2() { return ""; }
  void setBucketInfluxDB2(String bucket) {}
  const char* getTokenInfluxDB2() { return ""; }
  void setTokenInfluxDB2(String token) {}

  // These are helper function to assist with formatting of values
  int getWeightPrecision() { return 2; }  // 2 decimans for kg
  int getVolumePrecision() { return 0; }  // no decimals for cl
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
