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
#include <kegconfig.hpp>
#include <log.hpp>
#include <main.hpp>

KegConfig::KegConfig(String baseMDNS, String fileName)
    : BaseConfig(baseMDNS, fileName) {}

void KegConfig::createJson(DynamicJsonDocument& doc, bool skipSecrets) {
  // Call base class functions
  createJsonBase(doc, skipSecrets);
  createJsonWifi(doc, skipSecrets);
  createJsonOta(doc, skipSecrets);

  // Handle project specific config
  doc[PARAM_WEIGHT_PRECISION] = getWeightPrecision();

  doc[PARAM_BREWFATHER_APIKEY] = getBrewfatherApiKey();
  doc[PARAM_BREWFATHER_USERKEY] = getBrewfatherUserKey();

  doc[PARAM_SCALE_FACTOR1] = getScaleFactor(0);
  doc[PARAM_SCALE_OFFSET1] = getScaleOffset(0);
  doc[PARAM_KEG_WEIGHT1] = getKegWeight(0);
  doc[PARAM_PINT_WEIGHT1] = getPintWeight(0);
  doc[PARAM_BEER_NAME1] = getBeerName(0);
  doc[PARAM_BEER_ABV1] = getBeerABV(0);
  doc[PARAM_BEER_EBC1] = getBeerEBC(0);
  doc[PARAM_BEER_IBU1] = getBeerIBU(0);

  doc[PARAM_SCALE_FACTOR2] = getScaleFactor(1);
  doc[PARAM_SCALE_OFFSET2] = getScaleOffset(1);
  doc[PARAM_KEG_WEIGHT2] = getKegWeight(1);
  doc[PARAM_PINT_WEIGHT2] = getPintWeight(1);
  doc[PARAM_BEER_NAME2] = getBeerName(1);
  doc[PARAM_BEER_ABV2] = getBeerABV(1);
  doc[PARAM_BEER_EBC2] = getBeerEBC(1);
  doc[PARAM_BEER_IBU2] = getBeerIBU(1);
}

void KegConfig::parseJson(DynamicJsonDocument& doc) {
  // Call base class functions
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonOta(doc);

  // Handle project specific config
  if (!doc[PARAM_WEIGHT_PRECISION].isNull())
    setWeightPrecision(doc[PARAM_WEIGHT_PRECISION].as<int>());

  if (!doc[PARAM_BREWFATHER_APIKEY].isNull())
    setBrewfatherApiKey(doc[PARAM_BREWFATHER_APIKEY]);
  if (!doc[PARAM_BREWFATHER_USERKEY].isNull())
    setBrewfatherUserKey(doc[PARAM_BREWFATHER_USERKEY]);

  if (!doc[PARAM_SCALE_FACTOR1].isNull())
    setScaleFactor(0, doc[PARAM_SCALE_FACTOR1].as<float>());
  if (!doc[PARAM_SCALE_OFFSET1].isNull())
    setScaleOffset(0, doc[PARAM_SCALE_OFFSET1].as<float>());
  if (!doc[PARAM_KEG_WEIGHT1].isNull())
    setKegWeight(0, doc[PARAM_KEG_WEIGHT1].as<float>());
  if (!doc[PARAM_PINT_WEIGHT1].isNull())
    setPintWeight(0, doc[PARAM_PINT_WEIGHT1].as<float>());
  if (!doc[PARAM_BEER_NAME1].isNull()) setBeerName(0, doc[PARAM_BEER_NAME1]);
  if (!doc[PARAM_BEER_EBC1].isNull())
    setBeerEBC(0, doc[PARAM_BEER_EBC1].as<int>());
  if (!doc[PARAM_BEER_ABV1].isNull())
    setBeerABV(0, doc[PARAM_BEER_ABV1].as<float>());
  if (!doc[PARAM_BEER_IBU1].isNull())
    setBeerIBU(0, doc[PARAM_BEER_IBU1].as<int>());

  if (!doc[PARAM_SCALE_FACTOR2].isNull())
    setScaleFactor(1, doc[PARAM_SCALE_FACTOR2].as<float>());
  if (!doc[PARAM_SCALE_OFFSET2].isNull())
    setScaleOffset(1, doc[PARAM_SCALE_OFFSET2].as<float>());
  if (!doc[PARAM_KEG_WEIGHT2].isNull())
    setKegWeight(1, doc[PARAM_KEG_WEIGHT2].as<float>());
  if (!doc[PARAM_PINT_WEIGHT2].isNull())
    setPintWeight(1, doc[PARAM_PINT_WEIGHT2].as<float>());
  if (!doc[PARAM_BEER_NAME2].isNull()) setBeerName(1, doc[PARAM_BEER_NAME2]);
  if (!doc[PARAM_BEER_EBC2].isNull())
    setBeerEBC(1, doc[PARAM_BEER_EBC2].as<int>());
  if (!doc[PARAM_BEER_ABV2].isNull())
    setBeerABV(1, doc[PARAM_BEER_ABV2].as<float>());
  if (!doc[PARAM_BEER_IBU2].isNull())
    setBeerIBU(1, doc[PARAM_BEER_IBU2].as<int>());
}

// EOF
