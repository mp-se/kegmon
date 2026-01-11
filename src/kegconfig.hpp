/*
MIT License

Copyright (c) 2021-2026 Magnus

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
// UNUSED: Currently not persisted in config
// constexpr auto PARAM_TEMP_SENSOR = "temp_sensor";
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

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
  float _fg = 1;
  String _id = "";
};

struct KegInfo {
  float scaleFactor = 0.0f;
  int32_t scaleOffset = 0;
  float kegWeight = 4.0f;    // Weight in kg
  float kegVolume = 19.0f;   // Volume in liters
  float glassVolume = 0.40f; // Volume in liters
};

constexpr auto WEIGHT_KG = "kg";
constexpr auto WEIGHT_LBS = "lbs";

constexpr auto VOLUME_CL = "cl";
constexpr auto VOLUME_US = "us-oz";
constexpr auto VOLUME_UK = "uk-oz";

enum DisplayLayoutType {
  Default = 0
};
enum TempSensorType { SensorDS18B20 = 0 };

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

  KegInfo _kegs[4];
  BeerInfo _beer[4];

  int _scaleReadCount = 7;  // Default in HX711 library, should be odd number
  int _scaleReadCountCalibration = 29;

  // Change detection parameters 
  int _stabilityDetectionFilterIndex = 12;  // FILTER_KALMAN
  int _pourDetectionFilterIndex = 12;       // FILTER_KALMAN
  uint32_t _levelStabilizationDurationSeconds =
      3;                                     // Reduced for rapid pour detection
  uint32_t _pourDurationSeconds = 2;         // Reduced for smaller pours
  uint32_t _kegAbsenceDurationSeconds = 30;  // Reduced from 60
  uint32_t _kegReplacementDurationSeconds = 30;  // Reduced from 60
  float _weightAbsentThreshold = 0.1f;           // kg
  float _levelIncreaseThreshold = 0.4f;  // kg (keg replacement, ignore drift)
  float _levelDecreaseThreshold = 0.1f;  // kg (pour detection, 100-150g pours)
  float _pourSlopeThreshold =
      -0.002f;  // kg/sec minimum downward slope (for slow pours)

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

  float getKegWeight(UnitIndex idx) const { return _kegs[idx].kegWeight; }
  void setKegWeight(UnitIndex idx, float f) {
    _kegs[idx].kegWeight = f;
    _saveNeeded = true;
  }

  float getKegVolume(UnitIndex idx) const { return _kegs[idx].kegVolume; }
  void setKegVolume(UnitIndex idx, float f) {
    _kegs[idx].kegVolume = f;
    _saveNeeded = true;
  }

  float getGlassVolume(UnitIndex idx) const { return _kegs[idx].glassVolume; }
  void setGlassVolume(UnitIndex idx, float f) {
    _kegs[idx].glassVolume = f;
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

  int32_t getScaleOffset(UnitIndex idx) const { return _kegs[idx].scaleOffset; }
  void setScaleOffset(UnitIndex idx, int32_t l) {
    _kegs[idx].scaleOffset = l;
    _saveNeeded = true;
  }

  float getScaleFactor(UnitIndex idx) const { return _kegs[idx].scaleFactor; }
  void setScaleFactor(UnitIndex idx, float f) {
    _kegs[idx].scaleFactor = f;
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
  // UNUSED: Not persisted in config - use runtime defaults only
  // int getTempSensorTypeAsInt() const { return _tempSensor; }
  // void setTempSensorType(TempSensorType t) {
  //   _tempSensor = t;
  //   _saveNeeded = true;
  // }
  // void setTempSensorType(int t) {
  //   _tempSensor = (TempSensorType)t;
  //   _saveNeeded = true;
  // }

  int getScaleReadCount() const { return _scaleReadCount; }
  int getScaleReadCountCalibration() const {
    return _scaleReadCountCalibration;
  }

  int getStabilityDetectionFilterIndex() const {
    return _stabilityDetectionFilterIndex;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setStabilityDetectionFilterIndex(int i) {
  //   _stabilityDetectionFilterIndex = i;
  //   _saveNeeded = true;
  // }

  int getPourDetectionFilterIndex() const { return _pourDetectionFilterIndex; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourDetectionFilterIndex(int i) {
  //   _pourDetectionFilterIndex = i;
  //   _saveNeeded = true;
  // }

  float getLevelStabilizationDurationSeconds() const {
    return _levelStabilizationDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelStabilizationDurationSeconds(uint32_t i) {
  //   _levelStabilizationDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getPourDurationSeconds() const { return _pourDurationSeconds; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourDurationSeconds(uint32_t i) {
  //   _pourDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getKegAbsenceDurationSeconds() const {
    return _kegAbsenceDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setKegAbsenceDurationSeconds(uint32_t i) {
  //   _kegAbsenceDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  uint32_t getKegReplacementDurationSeconds() const {
    return _kegReplacementDurationSeconds;
  }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setKegReplacementDurationSeconds(uint32_t i) {
  //   _kegReplacementDurationSeconds = i;
  //   _saveNeeded = true;
  // }

  float getWeightAbsentThreshold() const { return _weightAbsentThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setWeightAbsentThreshold(float f) {
  //   _weightAbsentThreshold = f;
  //   _saveNeeded = true;
  // }

  float getLevelIncreaseThreshold() const { return _levelIncreaseThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelIncreaseThreshold(float f) {
  //   _levelIncreaseThreshold = f;
  //   _saveNeeded = true;
  // }

  float getLevelDecreaseThreshold() const { return _levelDecreaseThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setLevelDecreaseThreshold(float f) {
  //   _levelDecreaseThreshold = f;
  //   _saveNeeded = true;
  // }

  float getPourSlopeThreshold() const { return _pourSlopeThreshold; }
  // UNUSED: Not persisted in config - use runtime defaults only
  // void setPourSlopeThreshold(float f) {
  //   _pourSlopeThreshold = f;
  //   _saveNeeded = true;
  // }

  float getBeerVolumeToWeight(UnitIndex idx) const {
    // Assume beer density of ~1.0 kg/L (slightly less than water)
    // Returns max weight = keg weight + beer weight
    return _kegs[idx].kegWeight + _kegs[idx].kegVolume;
  }

  // These are helper function to assist with formatting of values
  int getWeightPrecision() const { return 2; }  // 2 decimans for kg
  int getVolumePrecision() const { return 0; }  // no decimals for cl
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
