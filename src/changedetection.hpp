/*
MIT License

Copyright (c) 2026 Magnus

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
#ifndef SRC_CHANGEDETECTION_HPP_
#define SRC_CHANGEDETECTION_HPP_

#include <changedetection_statistics.hpp>
#include <cmath>
#include <filters/filter_base.hpp>
#include <kegconfig.hpp>
#include <scale.hpp>
#include <weightvolume.hpp>

enum class ChangeDetectionState {
  Idle,           // Initializing, no stable level established
  Stabilizing,    // Settling to a stable level
  Stable,         // Stable level confirmed
  Pouring,        // Pour detected, waiting for stabilization
  Restabilizing,  // After pour, settling to new level
  KegAbsent,      // Weight dropped below threshold (keg removed)
  ReplacingKeg,   // Keg reappeared, settling to new level
  InvalidWeight   // Sensor reading out of valid range
};

// Event types fired by change detection state machine
enum class ChangeDetectionEventType {
  SYSTEM_STARTUP,   // System initialized/session started
  STABLE_DETECTED,  // Entered Stable state (baseline locked)
  POUR_STARTED,     // Entered Pouring state
  POUR_COMPLETED,   // Exiting Pouring state (pour finished)
  KEG_REMOVED,      // Entered KegAbsent state
  KEG_REPLACED,     // Entered ReplacingKeg state
  INVALID_WEIGHT    // Sensor reading out of valid range
};

// Self-contained event with all data needed for processing
struct ChangeDetectionEvent {
  ChangeDetectionEventType type;
  UnitIndex unitIndex = UnitIndex(0);  // U1
  uint64_t timestampMs = 0;

  // Data for STABLE_DETECTED
  struct {
    float stableWeightKg = 0.0f;
    float stableVolumeL = 0.0f;
    uint64_t durationMs = 0;  // Time to stabilize
  } stable;

  // Data for POUR_STARTED, POUR_COMPLETED
  struct {
    float prePourWeightKg = 0.0f;
    float postPourWeightKg = 0.0f;
    float pourWeightKg = 0.0f;
    float pourVolumeL = 0.0f;
    uint64_t durationMs = 0;
    float averageSlopeKgSec = 0.0f;
  } pour;

  // Data for KEG_REMOVED, KEG_REPLACED
  struct {
    float previousWeightKg = 0.0f;
    float currentWeightKg = 0.0f;
  } weight;

  // Data for INVALID_WEIGHT
  struct {
    float weightKg = 0.0f;            // Weight that triggered invalid state
    float minValidWeightKg = 0.0f;    // Minimum valid weight
    float maxValidWeightKg = 0.0f;    // Maximum valid weight
  } invalid;
};

// Thread-safe circular queue for change detection events
class ChangeDetectionEventQueue {
 private:
  static constexpr size_t MAX_EVENTS = 64;
  ChangeDetectionEvent _events[MAX_EVENTS];
  volatile size_t _head = 0;
  volatile size_t _tail = 0;
  mutable portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;  // ESP32 spinlock

  size_t nextIndex(size_t current) const { return (current + 1) % MAX_EVENTS; }

 public:
  ChangeDetectionEventQueue() = default;

  // Push event to queue (called from detection thread)
  bool push(const ChangeDetectionEvent& event);

  // Pop event from queue (called from main loop)
  bool pop(ChangeDetectionEvent& event);

  bool isEmpty() const;

  size_t count() const;
};

// Per-scale state tracking
struct ChangeDetectionInstance {
  ChangeDetectionState state = ChangeDetectionState::Idle;
  ChangeDetectionState previousState = ChangeDetectionState::Idle;

  // Timing and state tracking
  uint64_t stateEntryTimeMs = 0;
  float stableWeight = 0.0f;
  float previousFilterValue = 0.0f;
  float accumulatedSlope = 0.0f;
  int slopeReadings = 0;
  float pourVolume = 0.0f;
  uint64_t lastSlopeUpdateMs = 0;
  float lastSlopeValue = 0.0f;

  // Pour tracking
  float prePourWeight = 0.0f;   // Weight before pour started
  float previousWeight = 0.0f;  // For weight change detection
  float baselineStableWeight =
      0.0f;  // Stable weight baseline for passive detection
};

// Multi-scale change detection with consensus voting
// Manages pour detection and keg absence detection for all 4 scales
class ChangeDetection {
 private:
  ChangeDetectionInstance _scales[MAX_SCALES];
  ChangeDetectionEventQueue _eventQueue;

  // Configuration (shared across all scales)
  FilterType _stabilityFilter;
  FilterType _pourDetectionFilter;
  uint64_t _levelStabilizationDurationMs;
  uint64_t _pourDurationMs;
  uint64_t _kegAbsenceDurationMs;
  uint64_t _kegReplacementDurationMs;
  float _weightAbsentThreshold;
  float _levelIncreaseThreshold;
  float _levelDecreaseThreshold;
  float _pourSlopeThreshold;

  // Statistics manager
  ChangeDetectionStatisticsManager _stats;

  // Helper methods
  bool isWeightValid(float weight, UnitIndex idx) const;
  bool isWeightAbsent(float weight) const;
  bool isWeightPresent(float weight, UnitIndex idx) const;
  bool isWithinStabilityWindow(float weight, float stableWeight) const;

  void transitionState(UnitIndex idx, ChangeDetectionState newState,
                       uint64_t timestampMs);

  void fireEvent(UnitIndex idx, ChangeDetectionEventType eventType,
                 uint64_t timestampMs);

  float calculateSlope(UnitIndex idx, float currentValue, uint64_t timestampMs);

  float getAverageSlope(UnitIndex idx) const;
  void resetSlope(UnitIndex idx);

  // Helper to split oversized pours into multiple events based on glass volume
  void fireMultiplePourEvents(UnitIndex idx,
                              const ChangeDetectionEvent& originalEvent,
                              uint64_t timestampMs);

  // Fire POUR_COMPLETED event with automatic splitting if needed
  void firePourCompletedWithSplitting(UnitIndex idx, uint64_t timestampMs);

  // Passive detection: fire event if stable level dropped (pour was detected)
  void checkPassiveDetection(UnitIndex idx, uint64_t timestampMs);

 public:
  ChangeDetection();

  void loadConfiguration();

  // Fire a startup event to mark the beginning of a monitoring session
  void fireStartupEvent(uint64_t timestampMs);

  // Fire an invalid weight event when sensor reading is out of valid range
  void fireInvalidWeightEvent(UnitIndex idx, float currentWeight,
                              uint64_t timestampMs);

  // Update the state machine for a single scale
  void update(UnitIndex idx, const ScaleReadingResult& result,
              uint64_t timestampMs);

  ChangeDetectionState getState(UnitIndex idx) const;
  float getStableWeight(UnitIndex idx) const;
  float getPourVolume(UnitIndex idx) const;
  const char* getStateString(UnitIndex idx) const;
  float getConfidence(UnitIndex idx) const;

  // Event queue access (called from main loop)
  bool getNextEvent(ChangeDetectionEvent& event);
  bool hasQueuedEvents() const;
  size_t getPendingEventCount() const;

  // Statistics access
  const ChangeDetectionStatistics& getStatistics(UnitIndex idx) const {
    return _stats.getStatistics(static_cast<int>(idx));
  }

  void resetStatistics(UnitIndex idx) {
    _stats.resetStatistics(static_cast<int>(idx));
  }

  void resetAllStatistics() { _stats.resetAllStatistics(); }
};

extern ChangeDetection myChangeDetection;

#endif  // SRC_CHANGEDETECTION_HPP_

// EOF
