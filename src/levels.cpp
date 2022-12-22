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

LevelDetection::LevelDetection() {
  _kalmanLevel[0] = new KalmanLevelDetection(UnitIndex::U1);
  _kalmanLevel[1] = new KalmanLevelDetection(UnitIndex::U2);
  _statsLevel[0] = new StatsLevelDetection(UnitIndex::U1);
  _statsLevel[1] = new StatsLevelDetection(UnitIndex::U2);
}

void LevelDetection::update(UnitIndex idx, float raw) {
  if (isnan(raw)) {
    Log.notice(F("LEVL: No valid value read [%d]." CR), idx);
    return;
  }

  _stability[idx].add(raw);

  PERF_BEGIN("level-filter-raw");
  _rawLevel[idx].add(raw);
  PERF_END("level-filter-raw");

  PERF_BEGIN("level-filter-kalman");
  float kalman =
      getKalmanDetection(idx)->processValue(raw, _rawLevel[idx].average());
  PERF_END("level-filter-kalman");

  PERF_BEGIN("level-filter-stats");
  float stats =
      getStatsDetection(idx)->processValue(raw, _rawLevel[idx].average());

  if (getStatsDetection(idx)->newPourValue())
    pushPourUpdate(idx, getBeerStableVolume(idx), getPourVolume(idx));

  if (getStatsDetection(idx)->newStableValue())
    pushKegUpdate(idx, getBeerStableVolume(idx), getPourVolume(idx),
                  getNoStableGlasses(idx));

  PERF_END("level-filter-stats");
  // Log.notice(F("LEVL: raw=%F, kalman=%F, stats=%F [%d]." CR), raw, kalman,
  //           stats, idx);
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

  Log.notice(F("SCAL: Logging level change %s"), &s[0]);

  File f = LittleFS.open(LEVELS_FILENAME, "a");

  if (f && f.size() > LEVELS_FILEMAXSIZE) {
    f.close();
    LittleFS.remove(LEVELS_FILENAME2);
    LittleFS.rename(LEVELS_FILENAME, LEVELS_FILENAME2);
    f = LittleFS.open(LEVELS_FILENAME, "a");
  }

  if (f) {
    f.write(&s[0], strlen(&s[0]));
    f.close();
  }
}

bool LevelDetection::hasStableWeight(UnitIndex idx, LevelDetectionType type) {
  switch (type) {
    case LevelDetectionType::RAW:
      return true;
    case LevelDetectionType::AVERAGE:
      return getRawDetection(idx)->hasAverageValue();
    case LevelDetectionType::KALMAN:
      return getKalmanDetection(idx)->hasValue();
    case LevelDetectionType::STATS:
      return getStatsDetection(idx)->hasStableValue();
  }
  return false;
}

bool LevelDetection::hasPourWeight(UnitIndex idx, LevelDetectionType type) {
  switch (type) {
    case LevelDetectionType::RAW:
    case LevelDetectionType::AVERAGE:
      return false;
    case LevelDetectionType::KALMAN:
      return false;
    case LevelDetectionType::STATS:
      return getStatsDetection(idx)->hasStableValue();
  }
  return false;
}

float LevelDetection::getBeerWeight(UnitIndex idx, LevelDetectionType type) {
  float w = getTotalWeight(idx, type);

  if (!isnan(w)) {
    return w - myConfig.getKegWeight(idx);
  }

  return NAN;
}

float LevelDetection::getBeerStableWeight(UnitIndex idx,
                                          LevelDetectionType type) {
  float w = getTotalStableWeight(idx, type);

  if (!isnan(w)) {
    return w - myConfig.getKegWeight(idx);
  }

  return NAN;
}

float LevelDetection::getPourWeight(UnitIndex idx, LevelDetectionType type) {
  switch (type) {
    case LevelDetectionType::RAW:
    case LevelDetectionType::AVERAGE:
      return false;
    case LevelDetectionType::KALMAN:
      return false;
    case LevelDetectionType::STATS:
      return getStatsDetection(idx)->getPourValue();
  }
  return NAN;
}

float LevelDetection::getTotalRawWeight(UnitIndex idx) {
  return getRawDetection(idx)->getRawValue();
}

float LevelDetection::getTotalWeight(UnitIndex idx, LevelDetectionType type) {
  switch (type) {
    case LevelDetectionType::RAW:
      return getRawDetection(idx)->getRawValue();
    case LevelDetectionType::AVERAGE:
      return getRawDetection(idx)->getAverageValue();
    case LevelDetectionType::KALMAN:
      return getKalmanDetection(idx)->getValue();
    case LevelDetectionType::STATS:
      return getStatsDetection(idx)->getValue();
  }
  return NAN;
}

float LevelDetection::getTotalStableWeight(UnitIndex idx,
                                           LevelDetectionType type) {
  switch (type) {
    case LevelDetectionType::RAW:
      return getRawDetection(idx)->getRawValue();
    case LevelDetectionType::AVERAGE:
      return getRawDetection(idx)->getAverageValue();
    case LevelDetectionType::KALMAN:
      return getKalmanDetection(idx)->getValue();
    case LevelDetectionType::STATS:
      return getStatsDetection(idx)->getStableValue();
  }
  return NAN;
}

// EOF
