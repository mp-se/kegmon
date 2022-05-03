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
#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <main.hpp>

#define CFG_APPNAME "KegScale"          // Name of firmware
#define CFG_FILENAME "/kegscale.json"   // Name of config file

class Config {
 private:
  bool _saveNeeded = false;

  String _id = "";
  String _mDNS = "";
  char _tempFormat = 'C';

  String _wifiSSID[2] = {"", ""};
  String _wifiPASS[2] = {"", ""};

  float _scaleFactor = 0.0;
  long _scaleOffset = 0L;

  float _kegWeight = 0;
  float _pintWeight = 0;

  int _weightPrecision = 2;
  
  void formatFileSystem();

 public:
  Config();
  const char* getID() { return _id.c_str(); }
  int getWifiConnectionTimeout() { return 30; }
  int getWifiPortalTimeout() { return 120; }

  float getKegWeight() { return _kegWeight; }
  void setKegWeight(float f) {
    _kegWeight = f;
    _saveNeeded = true;
  }

  float getPintWeight() { return _pintWeight; }
  void setPintWeight(float f) {
    _pintWeight = f;
    _saveNeeded = true;
  }

  int getWeightPrecision() { return _weightPrecision; }
  void setWeightPrecision(int i) { 
    _weightPrecision = i;
    _saveNeeded = true;
  }

  long getScaleOffset() { return _scaleOffset; }
  void setScaleOffset(long l) {
    _scaleOffset = l;
    _saveNeeded = true;
  }

  float getScaleFactor() { return _scaleFactor; }
  void setScaleFactor(float f) {
    _scaleFactor = f;
    _saveNeeded = true;
  }

  const char* getMDNS() { return _mDNS.c_str(); }
  void setMDNS(String s) {
    _mDNS = s;
    _saveNeeded = true;
  }

  const char* getWifiSSID(int idx) { return _wifiSSID[idx].c_str(); }
  void setWifiSSID(String s, int idx) {
    _wifiSSID[idx] = s;
    _saveNeeded = true;
  }
  const char* getWifiPass(int idx) { return _wifiPASS[idx].c_str(); }
  void setWifiPass(String s, int idx) {
    _wifiPASS[idx] = s;
    _saveNeeded = true;
  }

  char getTempFormat() { return _tempFormat; }
  void setTempFormat(char c) {
    if (c == 'C' || c == 'F') {
      _tempFormat = c;
      _saveNeeded = true;
    }
  }
  bool isTempC() { return _tempFormat == 'C'; }
  bool isTempF() { return _tempFormat == 'F'; }

  void createJson(DynamicJsonDocument& doc);
  bool saveFile();
  bool loadFile();
  void checkFileSystem();
  bool isSaveNeeded() { return _saveNeeded; }
  void setSaveNeeded() { _saveNeeded = true; }
};

extern Config myConfig;

#endif  // SRC_CONFIG_HPP_

// EOF
