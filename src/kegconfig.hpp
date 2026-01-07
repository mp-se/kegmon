/*
MIT License

Copyright (c) 2021-2025 Magnus

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

constexpr auto PARAM_BREWLOGGER_URL = "brewlogger_url";
constexpr auto PARAM_BREWFATHER_USERKEY = "brewfather_userkey";
constexpr auto PARAM_BREWFATHER_APIKEY = "brewfather_apikey";
constexpr auto PARAM_BREWSPY_TOKEN1 = "brewspy_token1";
constexpr auto PARAM_BREWSPY_TOKEN2 = "brewspy_token2";
constexpr auto PARAM_BREWSPY_TOKEN3 = "brewspy_token3";
constexpr auto PARAM_BREWSPY_TOKEN4 = "brewspy_token4";
constexpr auto PARAM_BREWPI_URL = "brewpi_url";
constexpr auto PARAM_CHAMBERCTRL_URL = "chamberctrl_url";
constexpr auto PARAM_BARHELPER_APIKEY = "barhelper_apikey";
constexpr auto PARAM_BARHELPER_MONITOR1 = "barhelper_monitor1";
constexpr auto PARAM_BARHELPER_MONITOR2 = "barhelper_monitor2";
constexpr auto PARAM_BARHELPER_MONITOR3 = "barhelper_monitor3";
constexpr auto PARAM_BARHELPER_MONITOR4 = "barhelper_monitor4";
constexpr auto PARAM_DISPLAY_LAYOUT = "display_layout";
constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";
constexpr auto PARAM_WEIGHT_UNIT = "weight_unit";
constexpr auto PARAM_VOLUME_UNIT = "volume_unit";
constexpr auto PARAM_KEG_WEIGHT1 = "keg_weight1";
constexpr auto PARAM_KEG_WEIGHT2 = "keg_weight2";
constexpr auto PARAM_KEG_WEIGHT3 = "keg_weight3";
constexpr auto PARAM_KEG_WEIGHT4 = "keg_weight4";
constexpr auto PARAM_KEG_VOLUME1 = "keg_volume1";
constexpr auto PARAM_KEG_VOLUME2 = "keg_volume2";
constexpr auto PARAM_KEG_VOLUME3 = "keg_volume3";
constexpr auto PARAM_KEG_VOLUME4 = "keg_volume4";
constexpr auto PARAM_GLASS_VOLUME1 = "glass_volume1";
constexpr auto PARAM_GLASS_VOLUME2 = "glass_volume2";
constexpr auto PARAM_GLASS_VOLUME3 = "glass_volume3";
constexpr auto PARAM_GLASS_VOLUME4 = "glass_volume4";
constexpr auto PARAM_BEER_NAME1 = "beer_name1";
constexpr auto PARAM_BEER_NAME2 = "beer_name2";
constexpr auto PARAM_BEER_NAME3 = "beer_name3";
constexpr auto PARAM_BEER_NAME4 = "beer_name4";
constexpr auto PARAM_BEER_ID1 = "beer_id1";
constexpr auto PARAM_BEER_ID2 = "beer_id2";
constexpr auto PARAM_BEER_ID3 = "beer_id3";
constexpr auto PARAM_BEER_ID4 = "beer_id4";
constexpr auto PARAM_BEER_ABV1 = "beer_abv1";
constexpr auto PARAM_BEER_ABV2 = "beer_abv2";
constexpr auto PARAM_BEER_ABV3 = "beer_abv3";
constexpr auto PARAM_BEER_ABV4 = "beer_abv4";
constexpr auto PARAM_BEER_IBU1 = "beer_ibu1";
constexpr auto PARAM_BEER_IBU2 = "beer_ibu2";
constexpr auto PARAM_BEER_IBU3 = "beer_ibu3";
constexpr auto PARAM_BEER_IBU4 = "beer_ibu4";
constexpr auto PARAM_BEER_EBC1 = "beer_ebc1";
constexpr auto PARAM_BEER_EBC2 = "beer_ebc2";
constexpr auto PARAM_BEER_EBC3 = "beer_ebc3";
constexpr auto PARAM_BEER_EBC4 = "beer_ebc4";
constexpr auto PARAM_BEER_FG1 = "beer_fg1";
constexpr auto PARAM_BEER_FG2 = "beer_fg2";
constexpr auto PARAM_BEER_FG3 = "beer_fg3";
constexpr auto PARAM_BEER_FG4 = "beer_fg4";
constexpr auto PARAM_SCALE_FACTOR1 = "scale_factor1";
constexpr auto PARAM_SCALE_FACTOR2 = "scale_factor2";
constexpr auto PARAM_SCALE_FACTOR3 = "scale_factor3";
constexpr auto PARAM_SCALE_FACTOR4 = "scale_factor4";
constexpr auto PARAM_SCALE_OFFSET1 = "scale_offset1";
constexpr auto PARAM_SCALE_OFFSET2 = "scale_offset2";
constexpr auto PARAM_SCALE_OFFSET3 = "scale_offset3";
constexpr auto PARAM_SCALE_OFFSET4 = "scale_offset4";
constexpr auto PARAM_SCALE_TEMP_FORMULA1 = "scale_temp_formula1";
constexpr auto PARAM_SCALE_TEMP_FORMULA2 = "scale_temp_formula2";
constexpr auto PARAM_SCALE_TEMP_FORMULA3 = "scale_temp_formula3";
constexpr auto PARAM_SCALE_TEMP_FORMULA4 = "scale_temp_formula4";
constexpr auto PARAM_SCALE_DEVIATION_INCREASE = "scale_deviation_increase";
constexpr auto PARAM_SCALE_DEVIATION_DECREASE = "scale_deviation_decrease";
constexpr auto PARAM_SCALE_DEVIATION_KALMAN = "scale_deviation_kalman";
constexpr auto PARAM_SCALE_READ_COUNT = "scale_read_count";
constexpr auto PARAM_SCALE_READ_COUNT_CALIBRATION =
    "scale_read_count_calibration";
constexpr auto PARAM_SCALE_STABLE_COUNT = "scale_stable_count";
constexpr auto PARAM_LEVEL_DETECTION = "level_detection";
constexpr auto PARAM_KALMAN_NOISE = "kalman_noise";
constexpr auto PARAM_KALMAN_MEASUREMENT = "kalman_measurement";
constexpr auto PARAM_KALMAN_ESTIMATION = "kalman_estimation";
constexpr auto PARAM_KALMAN_ACTIVE = "kalman_active";

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
  float _fg = 1;
  String _id = "";
};

constexpr auto WEIGHT_KG = "kg";
constexpr auto WEIGHT_LBS = "lbs";

constexpr auto VOLUME_CL = "cl";
constexpr auto VOLUME_US = "us-oz";
constexpr auto VOLUME_UK = "uk-oz";

enum DisplayLayoutType {
  Default = 0,
  Graph = 1,
  GraphOne = 2,
  HardwareStats = 9
};
enum TempSensorType {
  SensorDS18B20 = 0
};

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

  String _brewspyToken[4] = {"", "", "", ""};

  String _barhelperApiKey = "";
  String _barhelperMonitor[4] = {"Kegmon TAP 1", "Kegmon TAP 2", "Kegmon TAP 3",
                                 "Kegmon TAP 4"};

  String _brewpiUrl = "";
  String _chamberCtrlUrl = "";

  String _brewLoggerUrl = "";

  DisplayLayoutType _displayLayout = DisplayLayoutType::Default;
  TempSensorType _tempSensor = TempSensorType::SensorDS18B20;

  float _scaleFactor[4] = {0, 0, 0, 0};
  int32_t _scaleOffset[4] = {0, 0, 0, 0};
  float _kegWeight[4] = {4, 4, 4, 4};                // Weight in kg
  float _kegVolume[4] = {19, 19, 19, 19};            // Weight in liters
  float _glassVolume[4] = {0.40, 0.40, 0.40, 0.40};  // Volume in liters
  BeerInfo _beer[4];

  float _scaleDeviationIncreaseValue = 0.4;  // kg
  float _scaleDeviationDecreaseValue = 0.1;  // kg
  float _scaleKalmanDeviation = 0.05;
  uint32_t _scaleStableCount = 8;
  int _scaleReadCount = 7; // Default in HX711 library, should be odd number 
  int _scaleReadCountCalibration = 29;
  String _scaleTempCompensationFormula[2] = {"", ""};

  LevelDetectionType _levelDetection = LevelDetectionType::STATS;

 public:
  KegConfig(String baseMDNS, String fileName);

  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);
  void migrateSettings();

  const char* getBrewfatherUserKey() const {
    return _brewfatherUserKey.c_str();
  }
  void setBrewfatherUserKey(String s) {
    _brewfatherUserKey = s;
    _saveNeeded = true;
  }
  const char* getBrewfatherApiKey() const { return _brewfatherApiKey.c_str(); }
  void setBrewfatherApiKey(String s) {
    _brewfatherApiKey = s;
    _saveNeeded = true;
  }

  const char* getBarhelperApiKey() const { return _barhelperApiKey.c_str(); }
  void setBarhelperApiKey(String s) {
    _barhelperApiKey = s;
    _saveNeeded = true;
  }
  const char* getBarhelperMonitor(UnitIndex idx) const {
    return _barhelperMonitor[idx].c_str();
  }
  void setBarhelperMonitor(UnitIndex idx, String s) {
    _barhelperMonitor[idx] = s;
    _saveNeeded = true;
  }

  const char* getBrewpiUrl() const { return _brewpiUrl.c_str(); }
  void setBrewpiUrl(String s) {
    _brewpiUrl = s;
    _saveNeeded = true;
  }

  const char* getChamberCtrlUrl() const { return _chamberCtrlUrl.c_str(); }
  void setChamberCtrlUrl(String s) {
    _chamberCtrlUrl = s;
    _saveNeeded = true;
  }

  const char* getBrewLoggerUrl() const { return _brewLoggerUrl.c_str(); }
  void setBrewLoggerUrl(String s) {
    _brewLoggerUrl = s;
    _saveNeeded = true;
  }

  const char* getBrewspyToken(UnitIndex idx) const {
    return _brewspyToken[idx].c_str();
  }
  void setBrewspyToken(UnitIndex idx, String s) {
    _brewspyToken[idx] = s;
    _saveNeeded = true;
  }

  const char* getBeerName(UnitIndex idx) const {
    return _beer[idx]._name.c_str();
  }
  void setBeerName(UnitIndex idx, String s) {
    _beer[idx]._name = s;
    _saveNeeded = true;
  }
  const char* getBeerId(UnitIndex idx) const { return _beer[idx]._id.c_str(); }
  void setBeerId(UnitIndex idx, String s) {
    _beer[idx]._id = s;
    _saveNeeded = true;
  }
  float getBeerABV(UnitIndex idx) const { return _beer[idx]._abv; }
  void setBeerABV(UnitIndex idx, float f) {
    _beer[idx]._abv = f;
    _saveNeeded = true;
  }
  float getBeerFG(UnitIndex idx) const { return _beer[idx]._fg; }
  void setBeerFG(UnitIndex idx, float f) {
    _beer[idx]._fg = f;
    _saveNeeded = true;
  }
  int getBeerEBC(UnitIndex idx) const { return _beer[idx]._ebc; }
  void setBeerEBC(UnitIndex idx, int i) {
    _beer[idx]._ebc = i;
    _saveNeeded = true;
  }
  int getBeerIBU(UnitIndex idx) const { return _beer[idx]._ibu; }
  void setBeerIBU(UnitIndex idx, int i) {
    _beer[idx]._ibu = i;
    _saveNeeded = true;
  }

  float getKegWeight(UnitIndex idx) const { return _kegWeight[idx]; }
  void setKegWeight(UnitIndex idx, float f) {
    _kegWeight[idx] = f;
    _saveNeeded = true;
  }

  float getKegVolume(UnitIndex idx) const { return _kegVolume[idx]; }
  void setKegVolume(UnitIndex idx, float f) {
    _kegVolume[idx] = f;
    _saveNeeded = true;
  }

  float getGlassVolume(UnitIndex idx) const { return _glassVolume[idx]; }
  void setGlassVolume(UnitIndex idx, float f) {
    _glassVolume[idx] = f;
    _saveNeeded = true;
  }

  const char* getWeightUnit() const { return _weightUnit.c_str(); }
  bool isWeightUnitKG() const { return _weightUnit.equals(WEIGHT_KG); }
  bool isWeightUnitLBS() const { return _weightUnit.equals(WEIGHT_LBS); }
  void setWeightUnit(String s) {
    if (!s.compareTo(WEIGHT_KG) || !s.compareTo(WEIGHT_LBS)) {
      _weightUnit = s;
      _saveNeeded = true;
    }
  }

  const char* getVolumeUnit() const { return _volumeUnit.c_str(); }
  bool isVolumeUnitCL() const { return _volumeUnit.equals(VOLUME_CL); }
  bool isVolumeUnitUSOZ() const { return _volumeUnit.equals(VOLUME_US); }
  bool isVolumeUnitUKOZ() const { return _volumeUnit.equals(VOLUME_UK); }
  void setVolumeUnit(String s) {
    if (!s.compareTo(VOLUME_CL) || !s.compareTo(VOLUME_UK) ||
        !s.compareTo(VOLUME_US)) {
      _volumeUnit = s;
      _saveNeeded = true;
    }
  }

  int32_t getScaleOffset(UnitIndex idx) const { return _scaleOffset[idx]; }
  void setScaleOffset(UnitIndex idx, int32_t l) {
    _scaleOffset[idx] = l;
    _saveNeeded = true;
  }

  float getScaleFactor(UnitIndex idx) const { return _scaleFactor[idx]; }
  void setScaleFactor(UnitIndex idx, float f) {
    _scaleFactor[idx] = f;
    _saveNeeded = true;
  }

  DisplayLayoutType getDisplayLayoutType() const { return _displayLayout; }
  int getDisplayLayoutTypeAsInt() const { return _displayLayout; }
  void setDisplayLayoutType(DisplayLayoutType d) {
    _displayLayout = d;
    _saveNeeded = true;
  }
  void setDisplayLayoutType(int d) {
    _displayLayout = (DisplayLayoutType)d;
    _saveNeeded = true;
  }

  TempSensorType getTempSensorType() const { return _tempSensor; }
  int getTempSensorTypeAsInt() const { return _tempSensor; }
  void setTempSensorType(TempSensorType t) {
    _tempSensor = t;
    _saveNeeded = true;
  }
  void setTempSensorType(int t) {
    _tempSensor = (TempSensorType)t;
    _saveNeeded = true;
  }

  // This is the maximum allowed deviation
  float getScaleDeviationDecreaseValue() const {
    return _scaleDeviationDecreaseValue;
  }  // 0.1 kg
  void setScaleDeviationDecreaseValue(float f) {
    _scaleDeviationDecreaseValue = f;
    _saveNeeded = true;
  }

  float getScaleDeviationIncreaseValue() const {
    return _scaleDeviationIncreaseValue;
  }  // 0.1 kg
  void setScaleDeviationIncreaseValue(float f) {
    _scaleDeviationIncreaseValue = f;
    _saveNeeded = true;
  }

  // This is the maximum allowed deviation between kalman and raw value for
  // level checking to work.
  float getScaleKalmanDeviationValue() const { return _scaleKalmanDeviation; }
  void setScaleKalmanDeviationValue(float f) {
    _scaleKalmanDeviation = f;
    _saveNeeded = true;
  }

  // This is the number of values in the statistics for the average value to be
  // classifed as stable. Loop interval is 2s
  uint32_t getScaleStableCount() const { return _scaleStableCount; }
  /*void setScaleStableCount(uint32_t i) {
    _scaleStableCount = i;
    _saveNeeded = true;
  }*/

  int getScaleReadCount() const { return _scaleReadCount; }
  /*void setScaleReadCount(uint32_t i) {
    _scaleReadCount = i;
    _saveNeeded = true;
  }*/

  int getScaleReadCountCalibration() const { return _scaleReadCountCalibration; }
  /*void setScaleReadCountCalibration(uint32_t i) {
    _scaleReadCountCalibration = i;
    _saveNeeded = true;
  }*/

  LevelDetectionType getLevelDetection() const { return _levelDetection; }
  int getLevelDetectionAsInt() const { return _levelDetection; }
  /*void setLevelDetection(LevelDetectionType l) {
    _levelDetection = l;
    _saveNeeded = true;
  }
  void setLevelDetection(int l) {
    _levelDetection = (LevelDetectionType)l;
    _saveNeeded = true;
  }*/

  /*
  float getKalmanEstimation() const { return _kalmanEstimation; }
  void setKalmanEstimation(float f) {
    _kalmanEstimation = f;
    _saveNeeded = true;
  }
  float getKalmanMeasurement() const { return _kalmanMeasurement; }
  void setKalmanMeasurement(float f) {
    _kalmanMeasurement = f;
    _saveNeeded = true;
  }
  float getKalmanNoise() const { return _kalmanNoise; }
  void setKalmanNoise(float f) {
    _kalmanNoise = f;
    _saveNeeded = true;
  }
  bool isKalmanActive() const { return _kalmanActive; }
  void setKalmanActive(bool b) {
    _kalmanActive = b;
    _saveNeeded = true;
  }
  */

  const char* getScaleTempCompensationFormula(UnitIndex idx) const {
    return _scaleTempCompensationFormula[idx].c_str();
  }
  void setScaleTempCompensationFormula(UnitIndex idx, String s) {
    _scaleTempCompensationFormula[idx] = s;
    _saveNeeded = true;
  }

  // These are helper function to assist with formatting of values
  int getWeightPrecision() const { return 2; }  // 2 decimans for kg
  int getVolumePrecision() const { return 0; }  // no decimals for cl
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
