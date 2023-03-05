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
#include <kegpush.hpp>
#include <levels.hpp>
#include <perf.hpp>
#include <scale.hpp>
#include <temp.hpp>

// Used for introduce noise on the signal to see if it accurate enough
// #define ENABLE_ADDING_NOISE

LevelDetection::LevelDetection() {
  _rawLevel[0] = new RawLevelDetection(UnitIndex::U1, 0.001, 0.001, 0.001);
  _rawLevel[1] = new RawLevelDetection(UnitIndex::U2, 0.001, 0.001, 0.001);
  _statsLevel[0] = new StatsLevelDetection(UnitIndex::U1);
  _statsLevel[1] = new StatsLevelDetection(UnitIndex::U2);
#if defined(ENABLE_ADDING_NOISE)
  randomSeed(12345L);
#endif
}

void LevelDetection::update(UnitIndex idx, float raw, float temp) {
  if (isnan(raw)) {
    Log.notice(F("LVL : No valid value read [%d]." CR), idx);
    return;
  }

#if defined(ENABLE_ADDING_NOISE)
  float max = raw;
  float err = random(-max, max);
  Log.notice(F("LVL : raw=%F, err=%F [%d]." CR), raw, err / 10, idx);
  // raw += err/5; // 20%
  // raw += err/10; // 10%
  raw += err / 20;  // 5%
#endif

  _stability[idx].add(raw);

  PERF_BEGIN("level-filter-raw");
  _rawLevel[idx]->add(raw, temp);
  float average = _rawLevel[idx]->getAverageValue();
  float tempCorr = _rawLevel[idx]->getTempCorrValue();
  float slope = _rawLevel[idx]->getSlopeValue();
  PERF_END("level-filter-raw");

  PERF_BEGIN("level-filter-stats");
  float stats = getStatsDetection(idx)->processValue(
      raw, getRawDetection(idx)->getKalmanValue());

  if (getStatsDetection(idx)->newPourValue())
    pushPourUpdate(idx, getBeerStableVolume(idx), getPourVolume(idx));

  if (getStatsDetection(idx)->newStableValue())
    pushKegUpdate(idx, getBeerStableVolume(idx), getPourVolume(idx),
                  getNoStableGlasses(idx));
  PERF_END("level-filter-stats");

  Log.verbose(F("LVL : raw=%F, ave=%F, temp=%F, stat=%F, slope=%F [%d]." CR),
              raw, average, tempCorr, stats, slope, idx);
}

void LevelDetection::pushKegUpdate(UnitIndex idx, float stableVol,
                                   float pourVol, float glasses) {
  myPush.pushKegInformation(idx, stableVol, pourVol, glasses);
  // Log.notice(F("LEVL: New level found: vol=%F, pour=%F [%d]." CR), stableVol,
  // pourVol, idx);

  switch (idx) {
    case UnitIndex::U1:
      logLevels(stableVol, NAN, NAN, NAN);
      break;

    case UnitIndex::U2:
      logLevels(NAN, stableVol, NAN, NAN);
      break;
  }
}

void LevelDetection::pushPourUpdate(UnitIndex idx, float stableVol,
                                    float pourVol) {
  myPush.pushPourInformation(idx, pourVol);
  // Log.notice(F("LEVL: New pour found: vol=%F, pour=%F [%d]." CR), stableVol,
  // pourVol, idx);

  switch (idx) {
    case UnitIndex::U1:
      logLevels(stableVol, NAN, pourVol, NAN);
      break;

    case UnitIndex::U2:
      logLevels(NAN, stableVol, NAN, pourVol);
      break;
  }
}

void LevelDetection::logLevels(float kegVolume1, float kegVolume2,
                               float pourVolume1, float pourVolume2) {
  struct tm timeinfo;
  time_t now = time(nullptr);
  char s[100];
  gmtime_r(&now, &timeinfo);
  snprintf(&s[0], sizeof(s), "%04d-%02d-%02d %02d:%02d:%02d;%f;%f;%f;%f\n",
           1900 + timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
           kegVolume1 < 0 ? 0 : kegVolume1, kegVolume2 < 0 ? 0 : kegVolume2,
           pourVolume1 < 0 ? 0 : pourVolume1,
           pourVolume2 < 0 ? 0 : pourVolume2);

  Log.notice(F("LVL : Logging level change %s"), &s[0]);

  File f = LittleFS.open(LEVELS_FILENAME, "a");

  if (f && f.size() > LEVELS_FILEMAXSIZE) {
    f.close();
    LittleFS.remove(LEVELS_FILENAME2);
    LittleFS.rename(LEVELS_FILENAME, LEVELS_FILENAME2);
    f = LittleFS.open(LEVELS_FILENAME, "a");
  }

  if (f) {
#if defined(ESP8266)
    f.write(&s[0], strlen(&s[0]));
#else
    f.write((unsigned char*)&s[0], strlen(&s[0]));
#endif
    f.close();
  }
}

bool LevelDetection::hasStableWeight(UnitIndex idx, LevelDetectionType type) {
  bool f = false;

  switch (type) {
    case LevelDetectionType::RAW:
      f = true;
      break;
    case LevelDetectionType::STATS:
      f = getStatsDetection(idx)->hasStableValue();
      break;
  }

  // Log.notice(F("LVL : StableWeight %s [%d]" CR), f ? "true" : "false", idx);
  return f;
}

bool LevelDetection::hasPourWeight(UnitIndex idx, LevelDetectionType type) {
  bool f = false;

  switch (type) {
    case LevelDetectionType::RAW:
      f = false;
      break;
    case LevelDetectionType::STATS:
      f = getStatsDetection(idx)->hasStableValue();
      break;
  }

  // Log.notice(F("LVL : PourWeight %s [%d]" CR), f ? "true" : "false", idx);
  return f;
}

float LevelDetection::getBeerWeight(UnitIndex idx, LevelDetectionType type) {
  float w = getTotalWeight(idx, type);
  // Log.notice(F("LVL : BeerWeight %F [%d]" CR), w, idx);

  if (!isnan(w)) {
    return w - myConfig.getKegWeight(idx);
  }

  return NAN;
}

float LevelDetection::getBeerStableWeight(UnitIndex idx,
                                          LevelDetectionType type) {
  float w = getTotalStableWeight(idx, type);
  // Log.notice(F("LVL : TotalStableWeight %F [%d]" CR), w, idx);

  if (!isnan(w)) {
    return w - myConfig.getKegWeight(idx);
  }

  return NAN;
}

float LevelDetection::getPourWeight(UnitIndex idx, LevelDetectionType type) {
  float w = NAN;

  switch (type) {
    case LevelDetectionType::RAW:
      break;
    case LevelDetectionType::STATS:
      w = getStatsDetection(idx)->getPourValue();
      break;
  }

  // Log.notice(F("LVL : PourWeight %F [%d]" CR), w, idx);
  return w;
}

float LevelDetection::getTotalRawWeight(UnitIndex idx) {
  return getRawDetection(idx)->getRawValue();
}

float LevelDetection::getTotalWeight(UnitIndex idx, LevelDetectionType type) {
  float w = NAN;

  switch (type) {
    case LevelDetectionType::RAW:
      w = getRawDetection(idx)->getRawValue();
      break;
    case LevelDetectionType::STATS:
      w = getStatsDetection(idx)->getValue();
      break;
  }

  // Log.notice(F("LVL : TotalWeight %F [%d]" CR), w, idx);
  return w;
}

float LevelDetection::getTotalStableWeight(UnitIndex idx,
                                           LevelDetectionType type) {
  float w = NAN;

  switch (type) {
    case LevelDetectionType::RAW:
      w = getRawDetection(idx)->getRawValue();
      break;
    case LevelDetectionType::STATS:
      w = getStatsDetection(idx)->getStableValue();
      break;
  }

  // Log.notice(F("LVL : TotalStableWeight %F [%d]" CR), w, idx);
  return w;
}

// EOF
