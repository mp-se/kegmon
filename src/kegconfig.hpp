/*
MIT License

Copyright (c) 2021-2024 Magnus

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

constexpr auto PARAM_BREWFATHER_USERKEY = "brewfather_userkey";
constexpr auto PARAM_BREWFATHER_APIKEY = "brewfather_apikey";
constexpr auto PARAM_BREWSPY_TOKEN1 = "brewspy_token1";
constexpr auto PARAM_BREWSPY_TOKEN2 = "brewspy_token2";
constexpr auto PARAM_BREWPI_URL = "brewpi_url";
constexpr auto PARAM_DISPLAY_LAYOUT = "display_layout";
constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";
constexpr auto PARAM_DISPLAY_DRIVER = "display_driver";
constexpr auto PARAM_SCALE_SENSOR = "scale_sensor";
constexpr auto PARAM_WEIGHT_UNIT = "weight_unit";
constexpr auto PARAM_VOLUME_UNIT = "volume_unit";
constexpr auto PARAM_KEG_WEIGHT1 = "keg_weight1";
constexpr auto PARAM_KEG_WEIGHT2 = "keg_weight2";
constexpr auto PARAM_KEG_VOLUME1 = "keg_volume1";
constexpr auto PARAM_KEG_VOLUME2 = "keg_volume2";
constexpr auto PARAM_GLASS_VOLUME1 = "glass_volume1";
constexpr auto PARAM_GLASS_VOLUME2 = "glass_volume2";
constexpr auto PARAM_BEER_NAME1 = "beer_name1";
constexpr auto PARAM_BEER_NAME2 = "beer_name2";
constexpr auto PARAM_BEER_ABV1 = "beer_abv1";
constexpr auto PARAM_BEER_ABV2 = "beer_abv2";
constexpr auto PARAM_BEER_IBU1 = "beer_ibu1";
constexpr auto PARAM_BEER_IBU2 = "beer_ibu2";
constexpr auto PARAM_BEER_EBC1 = "beer_ebc1";
constexpr auto PARAM_BEER_EBC2 = "beer_ebc2";
constexpr auto PARAM_BEER_FG1 = "beer_fg1";
constexpr auto PARAM_BEER_FG2 = "beer_fg2";
constexpr auto PARAM_SCALE_FACTOR1 = "scale_factor1";
constexpr auto PARAM_SCALE_FACTOR2 = "scale_factor2";
constexpr auto PARAM_SCALE_OFFSET1 = "scale_offset1";
constexpr auto PARAM_SCALE_OFFSET2 = "scale_offset2";
constexpr auto PARAM_SCALE_TEMP_FORMULA1 = "scale_temp_formula1";
constexpr auto PARAM_SCALE_TEMP_FORMULA2 = "scale_temp_formula2";
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
};

struct HardwareInfo {
#if defined(ESP8266)
  int _displayData = D2;
  int _displayClock = D1;
  int _scale1Data = D3;
  int _scale1Clock = D4;
  int _scale2Data = D5;
  int _scale2Clock = D8;
  int _tempData = D7;
  int _tempPower = D6;
#elif defined(ESP32S2)
  int _displayData = SDA;
  int _displayClock = SCL;
  int _scale1Data = A17;
  int _scale1Clock = A15;
  int _scale2Data = A6;
  int _scale2Clock = A11;
  int _tempData = A10;
  int _tempPower = A8;
#elif defined(ESP32S3)
  int _displayData = SDA;
  int _displayClock = SCL;
  int _scale1Data = A17;
  int _scale1Clock = A15;
  int _scale2Data = A11;
  int _scale2Clock = A9;
  int _tempData = A10;
  int _tempPower = A12;
#else
#error "Not a supported target"
#endif
};

constexpr auto PARAM_PIN_DISPLAY_DATA = "pin_display_data";
constexpr auto PARAM_PIN_DISPLAY_CLOCK = "pin_display_clock";
constexpr auto PARAM_PIN_SCALE1_DATA = "pin_scale1_data";
constexpr auto PARAM_PIN_SCALE1_CLOCK = "pin_scale1_clock";
constexpr auto PARAM_PIN_SCALE2_DATA = "pin_scale2_data";
constexpr auto PARAM_PIN_SCALE2_CLOCK = "pin_scale2_clock";
constexpr auto PARAM_PIN_TEMP_DATA = "pin_temp_data";
constexpr auto PARAM_PIN_TEMP_POWER = "pin_temp_power";

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
  SensorDHT22 = 0,
  SensorDS18B20 = 1,
  SensorBME280 = 2,
  SensorBrewPI = 3
};
enum ScaleSensorType { ScaleHX711 = 0, ScaleNAU7802 = 1 };
enum DisplayDriverType { OLED_1306 = 0, LCD = 1 };

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

  String _brewpiUrl = "";

  DisplayLayoutType _displayLayout = DisplayLayoutType::Default;
  TempSensorType _tempSensor = TempSensorType::SensorDS18B20;
  ScaleSensorType _scaleSensor = ScaleSensorType::ScaleHX711;
  DisplayDriverType _displayDriver = DisplayDriverType::OLED_1306;

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
  HardwareInfo _pins;

  /*
  bool _kalmanActive = true;
  float _kalmanMeasurement = 0.3;
  float _kalmanEstimation = 2;
  float _kalmanNoise = 0.01;
  */

 public:
  KegConfig(String baseMDNS, String fileName);

  void createJson(JsonObject& doc);
  void parseJson(JsonObject& doc);
  void migrateSettings();

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

  const char* getBrewpiUrl() { return _brewpiUrl.c_str(); }
  void setBrewpiUrl(String s) {
    _brewpiUrl = s;
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
    if (!s.compareTo(VOLUME_CL) || !s.compareTo(VOLUME_UK) ||
        !s.compareTo(VOLUME_US)) {
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

  DisplayLayoutType getDisplayLayoutType() { return _displayLayout; }
  int getDisplayLayoutTypeAsInt() { return _displayLayout; }
  void setDisplayLayoutType(DisplayLayoutType d) {
    _displayLayout = d;
    _saveNeeded = true;
  }
  void setDisplayLayoutType(int d) {
    _displayLayout = (DisplayLayoutType)d;
    _saveNeeded = true;
  }

  TempSensorType getTempSensorType() { return _tempSensor; }
  int getTempSensorTypeAsInt() { return _tempSensor; }
  void setTempSensorType(TempSensorType t) {
    _tempSensor = t;
    _saveNeeded = true;
  }
  void setTempSensorType(int t) {
    _tempSensor = (TempSensorType)t;
    _saveNeeded = true;
  }

  ScaleSensorType getScaleSensorType() { return _scaleSensor; }
  int getScaleSensorTypeAsInt() { return _scaleSensor; }
  void setScaleSensorType(ScaleSensorType t) {
    _scaleSensor = t;
    _saveNeeded = true;
  }
  void setScaleSensorType(int t) {
    _scaleSensor = (ScaleSensorType)t;
    _saveNeeded = true;
  }

  DisplayDriverType getDisplayDriverType() { return _displayDriver; }
  int getDisplayDriverTypeAsInt() { return _displayDriver; }
  void setDisplayDriverType(DisplayDriverType t) {
    _displayDriver = t;
    _saveNeeded = true;
  }
  void setDisplayDriverType(int t) {
    _displayDriver = (DisplayDriverType)t;
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

  // Hardware related methods
  int getPinDisplayData() { return _pins._displayData; }
  void setPinDisplayData(int pin) {
    _pins._displayData = pin;
    _saveNeeded = true;
  }
  int getPinDisplayClock() { return _pins._displayClock; }
  void setPinDisplayClock(int pin) {
    _pins._displayClock = pin;
    _saveNeeded = true;
  }

  int getPinScale1Data() { return _pins._scale1Data; }
  void setPinScale1Data(int pin) {
    _pins._scale1Data = pin;
    _saveNeeded = true;
  }
  int getPinScale1Clock() { return _pins._scale1Clock; }
  void setPinScale1Clock(int pin) {
    _pins._scale1Clock = pin;
    _saveNeeded = true;
  }

  int getPinScale2Data() { return _pins._scale2Data; }
  void setPinScale2Data(int pin) {
    _pins._scale2Data = pin;
    _saveNeeded = true;
  }
  int getPinScale2Clock() { return _pins._scale2Clock; }
  void setPinScale2Clock(int pin) {
    _pins._scale2Clock = pin;
    _saveNeeded = true;
  }

  int getPinTempData() { return _pins._tempData; }
  void setPinTempData(int pin) {
    _pins._tempData = pin;
    _saveNeeded = true;
  }
  int getPinTempPower() { return _pins._tempPower; }
  void setPinTempPower(int pin) {
    _pins._tempPower = pin;
    _saveNeeded = true;
  }

  // These are helper function to assist with formatting of values
  int getWeightPrecision() { return 2; }  // 2 decimans for kg
  int getVolumePrecision() { return 0; }  // no decimals for cl
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
