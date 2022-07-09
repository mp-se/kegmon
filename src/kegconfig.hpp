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

#define PARAM_WEIGHT_PRECISION "weight-precision"
#define PARAM_BREWFATHER_USERKEY "brewfather-userkey"
#define PARAM_BREWFATHER_APIKEY "brewfather-apikey"
#define PARAM_KEG_WEIGHT1 "keg-weight1"
#define PARAM_KEG_WEIGHT2 "keg-weight2"
#define PARAM_PINT_WEIGHT1 "pint-weight1"
#define PARAM_PINT_WEIGHT2 "pint-weight2"
#define PARAM_SCALE_FACTOR1 "scale-factor1"
#define PARAM_SCALE_FACTOR2 "scale-factor2"
#define PARAM_SCALE_WEIGHT1 "scale-weight1"
#define PARAM_SCALE_WEIGHT2 "scale-weight2"
#define PARAM_SCALE_RAW1 "scale-raw1"
#define PARAM_SCALE_RAW2 "scale-raw2"
#define PARAM_SCALE_OFFSET1 "scale-offset1"
#define PARAM_SCALE_OFFSET2 "scale-offset2"
#define PARAM_PINTS1 "pints1"
#define PARAM_PINTS2 "pints2"
#define PARAM_BEER_NAME1 "beer-name1"
#define PARAM_BEER_NAME2 "beer-name2"
#define PARAM_BEER_ABV1 "beer-abv1"
#define PARAM_BEER_ABV2 "beer-abv2"
#define PARAM_BEER_IBU1 "beer-ibu1"
#define PARAM_BEER_IBU2 "beer-ibu2"
#define PARAM_BEER_EBC1 "beer-ebc1"
#define PARAM_BEER_EBC2 "beer-ebc2"

struct BeerInfo {
  String _name = "";
  float _abv = 0.0;
  int _ebc = 0;
  int _ibu = 0;
};

class KegConfig : public BaseConfig {
 private:
  int _weightPrecision = 2;
  String _brewfatherUserKey = "";
  String _brewfatherApiKey = "";

  float _scaleFactor[2] = {0, 0};
  int32_t _scaleOffset[2] = {0, 0};
  float _kegWeight[2] = {0, 0};
  float _pintWeight[2] = {0, 0};
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

  float getPintWeight(int idx) { return _pintWeight[idx]; }
  void setPintWeight(int idx, float f) {
    _pintWeight[idx] = f;
    _saveNeeded = true;
  }

  int getWeightPrecision() { return _weightPrecision; }
  void setWeightPrecision(int i) {
    _weightPrecision = i;
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
};

extern KegConfig myConfig;

#endif  // SRC_KEGCONFIG_HPP_

// EOF
