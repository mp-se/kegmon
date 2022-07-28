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
#define PARAM_WEIGHT_UNIT "weight-unit"
#define PARAM_VOLUME_UNIT "volume-unit"
#define PARAM_KEG_WEIGHT1 "keg-weight1"
#define PARAM_KEG_WEIGHT2 "keg-weight2"
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

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
  float _fg = 1;
};

#define WEIGHT_KG "kg"
#define WEIGHT_LBS "lbs"

#define VOLUME_CL "kg"
#define VOLUME_US "us-oz"
#define VOLUME_UK "uk-oz"

class KegConfig : public BaseConfig {
 private:
  String _weightUnit = WEIGHT_KG;
  String _volumeUnit = VOLUME_CL;

  String _brewfatherUserKey = "";
  String _brewfatherApiKey = "";

  float _scaleFactor[2] = {0, 0};
  int32_t _scaleOffset[2] = {0, 0};
  float _kegWeight[2] = {0, 0}; // Weight in kg
  float _glassVolume[2] = {0.40, 0.40}; // Volume in liters
  BeerInfo _beer[2];

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

  const char* getBeerName(int idx) { return _beer[idx]._name.c_str(); }
  void setBeerName(int idx, String s) {
    _beer[idx]._name = s;
    _saveNeeded = true;
  }
  float getBeerABV(int idx) { return _beer[idx]._abv; }
  void setBeerABV(int idx, float f) {
    _beer[idx]._abv = f;
    _saveNeeded = true;
  }
  float getBeerFG(int idx) { return _beer[idx]._fg; }
  void setBeerFG(int idx, float f) {
    _beer[idx]._fg = f;
    _saveNeeded = true;
  }
  int getBeerEBC(int idx) { return _beer[idx]._ebc; }
  void setBeerEBC(int idx, int i) {
    _beer[idx]._ebc = i;
    _saveNeeded = true;
  }
  int getBeerIBU(int idx) { return _beer[idx]._ibu; }
  void setBeerIBU(int idx, int i) {
    _beer[idx]._ibu = i;
    _saveNeeded = true;
  }

  float getKegWeight(int idx) { return _kegWeight[idx]; }
  void setKegWeight(int idx, float f) {
    _kegWeight[idx] = f;
    _saveNeeded = true;
  }

  float getGlassVolume(int idx) { return _glassVolume[idx]; }
  void setGlassVolume(int idx, float f) {
    _glassVolume[idx] = f;
    _saveNeeded = true;
  }

  const char* getWeightUnit() { return _weightUnit.c_str(); }
  void setWeightUnit(String s) {
    _weightUnit = s;
    _saveNeeded = true;
  }

  const char* getVolumeUnit() { return _volumeUnit.c_str(); }
  void setVolumeUnit(String s) {
    _volumeUnit = s;
    _saveNeeded = true;
  }

  int32_t getScaleOffset(int idx) { return _scaleOffset[idx]; }
  void setScaleOffset(int idx, int32_t l) {
    _scaleOffset[idx] = l;
    _saveNeeded = true;
  }

  float getScaleFactor(int idx) { return _scaleFactor[idx]; }
  void setScaleFactor(int idx, float f) {
    _scaleFactor[idx] = f;
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

  const char* getTargetInfluxDB2() { return PUSH_INFLUX_TARGET; }
  void setTargetInfluxDB2(String target) {}
  const char* getOrgInfluxDB2() { return PUSH_INFLUX_ORG; }
  void setOrgInfluxDB2(String org) {}
  const char* getBucketInfluxDB2() { return PUSH_INFLUX_BUCKET; }
  void setBucketInfluxDB2(String bucket) {}
  const char* getTokenInfluxDB2() { return PUSH_INFLUX_TOKEN; }
  void setTokenInfluxDB2(String token) {}

  const char* getTargetMqtt() { return ""; }
  void setTargetMqtt(String target) {}
  int getPortMqtt() { return 0; }
  void setPortMqtt(int port) {}
  const char* getUserMqtt() { return ""; }
  void setUserMqtt(String user) {}
  const char* getPassMqtt() { return ""; }
  void setPassMqtt(String pass) {}

  // These are helper function to assist with formatting of values
  int getWeightPrecision() {
    return 3;
  }
};

class KegAdvancedConfig {
 private:
 public:
  // This is the maximum allowed deviation from the current average.
  float getDeviationValue() { return 0.1; }  // 0.1 kg

  // This is the number of values in the statistics for the average value to be
  // classifed as stable.
  uint32_t getStableCount() { return 5; }  // 5*2 s = 10 s
};

extern KegConfig myConfig;
extern KegAdvancedConfig myAdvancedConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
