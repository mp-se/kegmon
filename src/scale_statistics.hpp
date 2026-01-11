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
#ifndef SRC_SCALE_STATISTICS_HPP_
#define SRC_SCALE_STATISTICS_HPP_

#include <cstdint>

// Per-scale health statistics
struct ScaleStatistics {
  // Reading counts
  uint32_t totalReadings = 0;
  uint32_t validReadings = 0;
  uint32_t invalidReadings = 0;

  // Timing
  uint64_t lastReadingTimeMs = 0;
  uint64_t firstReadingTimeMs = 0;

  // Raw value tracking (for calibration drift detection)
  float rawMin = 0.0f;
  float rawMax = 0.0f;
  float rawAverage = 0.0f;
  float rawSum = 0.0f;

  // Filter statistics
  float currentVariance = 0.0f;
  float stableStateVariance = 0.0f;
  uint32_t stableStateSamples = 0;

  // Calibration drift
  float calibrationDriftPerHour = 0.0f;  // kg/hour
  uint64_t lastDriftCheckMs = 0;
  float lastDriftCheckRaw = 0.0f;

  float getReadingQuality() const {
    if (totalReadings == 0) return 0.0f;
    return (static_cast<float>(validReadings) / totalReadings) * 100.0f;
  }

  float getReadingFrequency() const {
    if (firstReadingTimeMs == 0 || lastReadingTimeMs == 0) return 0.0f;
    uint64_t durationMs = lastReadingTimeMs - firstReadingTimeMs;
    if (durationMs == 0) return 0.0f;
    return (static_cast<float>(totalReadings) / durationMs) *
           1000.0f;  // samples/sec
  }

  float getRawAverage() const {
    if (totalReadings == 0) return 0.0f;
    return rawSum / totalReadings;
  }

  void recordReading(float rawValue, bool isValid, uint64_t timestampMs) {
    totalReadings++;
    if (isValid) {
      validReadings++;
      rawSum += rawValue;

      if (totalReadings == 1) {
        rawMin = rawValue;
        rawMax = rawValue;
        firstReadingTimeMs = timestampMs;
      } else {
        if (rawValue < rawMin) rawMin = rawValue;
        if (rawValue > rawMax) rawMax = rawValue;
      }
    } else {
      invalidReadings++;
    }
    lastReadingTimeMs = timestampMs;
  }

  void recordStableVariance(float variance) {
    if (stableStateSamples == 0) {
      stableStateVariance = variance;
    } else {
      // Running average of variance
      stableStateVariance =
          (stableStateVariance * stableStateSamples + variance) /
          (stableStateSamples + 1);
    }
    stableStateSamples++;
  }

  void reset() {
    totalReadings = 0;
    validReadings = 0;
    invalidReadings = 0;
    lastReadingTimeMs = 0;
    firstReadingTimeMs = 0;
    rawMin = 0.0f;
    rawMax = 0.0f;
    rawAverage = 0.0f;
    rawSum = 0.0f;
    currentVariance = 0.0f;
    stableStateVariance = 0.0f;
    stableStateSamples = 0;
    calibrationDriftPerHour = 0.0f;
    lastDriftCheckMs = 0;
    lastDriftCheckRaw = 0.0f;
  }
};

// Global statistics manager for all 4 scales
class ScaleStatisticsManager {
 private:
  ScaleStatistics _scales[4];

 public:
  ScaleStatisticsManager() = default;

  void recordReading(int scaleIndex, float rawValue, bool isValid,
                     uint64_t timestampMs) {
    if (scaleIndex < 4) {
      _scales[scaleIndex].recordReading(rawValue, isValid, timestampMs);
    }
  }

  void recordStableVariance(int scaleIndex, float variance) {
    if (scaleIndex < 4) {
      _scales[scaleIndex].recordStableVariance(variance);
    }
  }

  const ScaleStatistics& getStatistics(int scaleIndex) const {
    return _scales[scaleIndex];
  }

  void resetStatistics(int scaleIndex) {
    if (scaleIndex < 4) {
      _scales[scaleIndex].reset();
    }
  }

  void resetAllStatistics() {
    for (int i = 0; i < 4; i++) {
      _scales[i].reset();
    }
  }
};

#endif  // SRC_SCALE_STATISTICS_HPP_

// EOF
