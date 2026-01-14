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
#include <algorithm>
#include <changedetection.hpp>

bool ChangeDetectionEventQueue::push(const ChangeDetectionEvent& event) {
  portENTER_CRITICAL(&_lock);
  size_t nextTail = nextIndex(_tail);

  if (nextTail == _head) {
    // Queue full - drop oldest event (overflow)
    _head = nextIndex(_head);
  }

  _events[_tail] = event;
  _tail = nextTail;
  portEXIT_CRITICAL(&_lock);
  return true;
}

bool ChangeDetectionEventQueue::pop(ChangeDetectionEvent& event) {
  portENTER_CRITICAL(&_lock);

  if (_head == _tail) {
    // Queue empty
    portEXIT_CRITICAL(&_lock);
    return false;
  }

  event = _events[_head];
  _head = nextIndex(_head);
  portEXIT_CRITICAL(&_lock);
  return true;
}

bool ChangeDetectionEventQueue::isEmpty() const { return _head == _tail; }

size_t ChangeDetectionEventQueue::count() const {
  portENTER_CRITICAL(reinterpret_cast<portMUX_TYPE*>(&_lock));
  size_t result =
      (_tail >= _head) ? (_tail - _head) : (MAX_EVENTS - _head + _tail);
  portEXIT_CRITICAL(reinterpret_cast<portMUX_TYPE*>(&_lock));
  return result;
}

bool ChangeDetection::isWeightValid(float weight, UnitIndex idx) const {
  float minValidWeight = 0.0f;
  float maxValidWeight = myConfig.getBeerVolumeToWeight(idx);
  return weight >= minValidWeight && weight <= maxValidWeight;
}

bool ChangeDetection::isWeightAbsent(float weight) const {
  return weight < _weightAbsentThreshold;
}

bool ChangeDetection::isWeightPresent(float weight, UnitIndex idx) const {
  return weight >= myConfig.getKegWeight(idx);
}

bool ChangeDetection::isWithinStabilityWindow(float weight,
                                              float stableWeight) const {
  // Asymmetric thresholds: allow small decreases (pours), but reject increases
  // (drift)
  if (weight < stableWeight) {
    // Decrease: allow decreases up to _levelDecreaseThreshold
    return (stableWeight - weight) <= _levelDecreaseThreshold;
  } else {
    // Increase: only allow increases up to _levelIncreaseThreshold
    return (weight - stableWeight) <= _levelIncreaseThreshold;
  }
}

void ChangeDetection::transitionState(UnitIndex idx,
                                      ChangeDetectionState newState,
                                      uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  if (scale.state != newState) {
    // Record state change statistics
    uint64_t durationInCurrentState = timestampMs - scale.stateEntryTimeMs;
    _stats.recordStateChange(
        static_cast<int>(idx), static_cast<int>(scale.state),
        static_cast<int>(newState), durationInCurrentState);

    scale.previousState = scale.state;
    scale.state = newState;
    scale.stateEntryTimeMs = timestampMs;
  }
}

void ChangeDetection::fireEvent(UnitIndex idx,
                                ChangeDetectionEventType eventType,
                                uint64_t timestampMs) {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  ChangeDetectionEvent event;
  event.type = eventType;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  WeightVolumeConverter converter(idx);

  switch (eventType) {
    case ChangeDetectionEventType::STABLE_DETECTED: {
      event.stable.stableWeightKg = scale.stableWeight;
      event.stable.stableVolumeL = converter.weightToVolume(scale.stableWeight);
      event.stable.durationMs = timestampMs - scale.stateEntryTimeMs;
      break;
    }

    case ChangeDetectionEventType::POUR_STARTED: {
      event.pour.prePourWeightKg = scale.stableWeight;
      event.pour.averageSlopeKgSec = getAverageSlope(idx);
      break;
    }

    case ChangeDetectionEventType::POUR_COMPLETED: {
      event.pour.prePourWeightKg = scale.prePourWeight;
      event.pour.postPourWeightKg = scale.stableWeight;
      event.pour.pourWeightKg = scale.prePourWeight - scale.stableWeight;
      event.pour.pourVolumeL =
          converter.weightToVolume(event.pour.pourWeightKg);
      event.pour.durationMs = timestampMs - scale.stateEntryTimeMs;
      event.pour.averageSlopeKgSec = getAverageSlope(idx);

      // Record pour statistics
      _stats.recordPour(static_cast<int>(idx), event.pour.pourVolumeL,
                        event.pour.durationMs);
      break;
    }

    case ChangeDetectionEventType::KEG_REMOVED: {
      event.weight.previousWeightKg = scale.previousWeight;
      event.weight.currentWeightKg = scale.stableWeight;
      break;
    }

    case ChangeDetectionEventType::KEG_REPLACED: {
      event.weight.previousWeightKg = scale.previousWeight;
      event.weight.currentWeightKg = scale.stableWeight;

      // Record keg replacement statistics
      _stats.recordKegReplacement(static_cast<int>(idx),
                                  event.weight.currentWeightKg);
      break;
    }

    case ChangeDetectionEventType::INVALID_WEIGHT: {
      // Note: invalid weight data is populated by fireInvalidWeightEvent()
      break;
    }

    default:
      break;
  }

  _eventQueue.push(event);
}

float ChangeDetection::calculateSlope(UnitIndex idx, float currentValue,
                                      uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  if (scale.slopeReadings == 0) {
    // First reading in this window
    scale.previousFilterValue = currentValue;
    scale.lastSlopeUpdateMs = timestampMs;
    scale.slopeReadings = 1;
    return 0.0f;
  }

  uint64_t timeDeltaMs = timestampMs - scale.lastSlopeUpdateMs;
  if (timeDeltaMs > 0) {
    float weightDelta = currentValue - scale.previousFilterValue;
    float slope = weightDelta / (timeDeltaMs / 1000.0f);  // kg/sec

    scale.accumulatedSlope += slope;
    scale.slopeReadings++;
    scale.previousFilterValue = currentValue;
    scale.lastSlopeUpdateMs = timestampMs;

    return slope;
  }

  return 0.0f;
}

float ChangeDetection::getAverageSlope(UnitIndex idx) const {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  if (scale.slopeReadings > 0) {
    return scale.accumulatedSlope / scale.slopeReadings;
  }
  return 0.0f;
}

void ChangeDetection::resetSlope(UnitIndex idx) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  scale.accumulatedSlope = 0.0f;
  scale.slopeReadings = 0;
}

ChangeDetection::ChangeDetection()
    : _stabilityFilter(FilterType::FILTER_MEDIAN),
      _pourDetectionFilter(FilterType::FILTER_KALMAN),
      _levelStabilizationDurationMs(0),
      _pourDurationMs(0),
      _kegAbsenceDurationMs(0),
      _kegReplacementDurationMs(0),
      _weightAbsentThreshold(0.0f),
      _levelIncreaseThreshold(0.0f),
      _levelDecreaseThreshold(0.0f),
      _pourSlopeThreshold(0.0f) {
  loadConfiguration();
}

void ChangeDetection::loadConfiguration() {
  _stabilityFilter =
      static_cast<FilterType>(myConfig.getStabilityDetectionFilterIndex());
  _pourDetectionFilter =
      static_cast<FilterType>(myConfig.getPourDetectionFilterIndex());
  _levelStabilizationDurationMs =
      myConfig.getLevelStabilizationDurationSeconds() * 1000U;
  _pourDurationMs = myConfig.getPourDurationSeconds() * 1000U;
  _kegAbsenceDurationMs = myConfig.getKegAbsenceDurationSeconds() * 1000U;
  _kegReplacementDurationMs =
      myConfig.getKegReplacementDurationSeconds() * 1000U;
  _weightAbsentThreshold = myConfig.getWeightAbsentThreshold();
  _levelIncreaseThreshold = myConfig.getLevelIncreaseThreshold();
  _levelDecreaseThreshold = myConfig.getLevelDecreaseThreshold();
  _pourSlopeThreshold = myConfig.getPourSlopeThreshold();
}

void ChangeDetection::fireStartupEvent(uint64_t timestampMs) {
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::SYSTEM_STARTUP;
  event.unitIndex = UnitIndex(0);
  event.timestampMs = timestampMs;
  _eventQueue.push(event);
}

void ChangeDetection::fireInvalidWeightEvent(UnitIndex idx, float currentWeight,
                                             uint64_t timestampMs) {
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::INVALID_WEIGHT;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  event.invalid.weightKg = currentWeight;
  event.invalid.minValidWeightKg = myConfig.getMinValidWeight(idx);
  event.invalid.maxValidWeightKg = myConfig.getMaxValidWeight(idx);

  _eventQueue.push(event);
}

// Splits oversized pours into multiple events based on configured glass volume
void ChangeDetection::fireMultiplePourEvents(
    UnitIndex idx, const ChangeDetectionEvent& originalEvent,
    uint64_t timestampMs) {
  const float glassVolumeL = myConfig.getGlassVolume(idx);
  const float pourVolumeL = originalEvent.pour.pourVolumeL;

  // If pour is within glass size, just fire the event as-is
  if (pourVolumeL <= glassVolumeL) {
    _eventQueue.push(originalEvent);
    _stats.recordPour(static_cast<int>(idx), originalEvent.pour.pourVolumeL,
                      originalEvent.pour.durationMs);
    return;
  }

  // Calculate number of pours needed to account for total volume
  const int numPours = static_cast<int>(ceil(pourVolumeL / glassVolumeL));
  const float volumePerPour = pourVolumeL / numPours;
  const float weightPerPour = originalEvent.pour.pourWeightKg / numPours;
  const uint64_t timePerPour = originalEvent.pour.durationMs / numPours;

  // Fire individual pour events for each "glass" worth of volume
  for (int i = 0; i < numPours; i++) {
    ChangeDetectionEvent event = originalEvent;
    event.pour.pourVolumeL = volumePerPour;
    event.pour.pourWeightKg = weightPerPour;
    event.pour.postPourWeightKg =
        originalEvent.pour.prePourWeightKg - (weightPerPour * (i + 1));
    event.pour.durationMs = timePerPour;
    // Timestamp is offset for each pour
    event.timestampMs = timestampMs + (i * timePerPour);

    _eventQueue.push(event);
    // Record statistics for each individual pour
    _stats.recordPour(static_cast<int>(idx), volumePerPour, timePerPour);
  }
}

// Constructs POUR_COMPLETED event and handles multi-pour splitting
void ChangeDetection::firePourCompletedWithSplitting(UnitIndex idx,
                                                     uint64_t timestampMs) {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::POUR_COMPLETED;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  WeightVolumeConverter converter(idx);

  event.pour.prePourWeightKg = scale.prePourWeight;
  event.pour.postPourWeightKg = scale.stableWeight;
  event.pour.pourWeightKg = scale.prePourWeight - scale.stableWeight;
  event.pour.pourVolumeL = converter.weightToVolume(event.pour.pourWeightKg);
  event.pour.durationMs = timestampMs - scale.stateEntryTimeMs;
  event.pour.averageSlopeKgSec = getAverageSlope(idx);

  // Use the multi-pour splitting logic
  fireMultiplePourEvents(idx, event, timestampMs);
}

// Passive detection: when stabilizing at a new level, check if weight dropped
// This catches pours that may have been missed by active slope detection
void ChangeDetection::checkPassiveDetection(UnitIndex idx,
                                            uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  // Only check if we have a baseline to compare against
  if (scale.baselineStableWeight <= 0.0f) {
    // First time entering stable state, set the baseline
    scale.baselineStableWeight = scale.stableWeight;
    return;
  }

  // Check if weight has dropped significantly since baseline
  float weightDifference = scale.baselineStableWeight - scale.stableWeight;

  // Use minimum threshold to avoid false positives from noise/drift
  // (typically 0.1 kg = 100g minimum to be considered a real pour)
  if (weightDifference > _levelDecreaseThreshold) {
    // Weight has definitely dropped by more than minimum threshold - a pour
    // occurred! Fire POUR_COMPLETED event with the passive-detected volume
    // fireMultiplePourEvents will split it based on configured glass size

    ChangeDetectionEvent event;
    event.type = ChangeDetectionEventType::POUR_COMPLETED;
    event.unitIndex = idx;
    event.timestampMs = timestampMs;

    WeightVolumeConverter converter(idx);

    event.pour.prePourWeightKg = scale.baselineStableWeight;
    event.pour.postPourWeightKg = scale.stableWeight;
    event.pour.pourWeightKg = weightDifference;
    event.pour.pourVolumeL = converter.weightToVolume(weightDifference);
    event.pour.durationMs = 0;  // Passive detection doesn't track timing
    event.pour.averageSlopeKgSec =
        0.0f;  // Passive detection doesn't calculate slope

    // Use multi-pour splitting - if volume > glass size, splits into multiple
    // events
    fireMultiplePourEvents(idx, event, timestampMs);

    // Update baseline to new level
    scale.baselineStableWeight = scale.stableWeight;
  }
}

void ChangeDetection::update(UnitIndex idx, const ScaleReadingResult& result,
                             uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  float currentWeight = result.getFilterValue(_stabilityFilter);
  float pourDetectionWeight = result.getFilterValue(_pourDetectionFilter);

  // Update state machine
  uint64_t timeInState = timestampMs - scale.stateEntryTimeMs;

  switch (scale.state) {
    case ChangeDetectionState::Idle:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
      } else {
        transitionState(idx, ChangeDetectionState::Stabilizing, timestampMs);
      }
      break;

    case ChangeDetectionState::Stabilizing:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
      } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight) ||
                 scale.stableWeight == 0.0f) {
        scale.stableWeight = currentWeight;
        if (timeInState >= _levelStabilizationDurationMs) {
          transitionState(idx, ChangeDetectionState::Stable, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::STABLE_DETECTED,
                    timestampMs);
          resetSlope(idx);
        }
      } else {
        scale.stableWeight = currentWeight;
        scale.stateEntryTimeMs = timestampMs;  // Reset timer
      }
      break;

    case ChangeDetectionState::Stable:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else if (currentWeight >=
                 scale.stableWeight + _levelIncreaseThreshold) {
        // Large increase: possible keg replacement
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::ReplacingKeg, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REPLACED, timestampMs);
      } else {
        // Monitor slope in current data for pour detection
        calculateSlope(idx, pourDetectionWeight, timestampMs);

        if (getAverageSlope(idx) < _pourSlopeThreshold) {
          // Weight dropping fast enough = active pour detected
          scale.prePourWeight = scale.stableWeight;
          resetSlope(idx);
          scale.pourVolume = 0.0f;
          transitionState(idx, ChangeDetectionState::Pouring, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::POUR_STARTED, timestampMs);
        } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight)) {
          // Small drift within tolerance: update baseline to track gradual
          // changes
          scale.stableWeight = currentWeight;
        }
      }
      break;

    case ChangeDetectionState::Pouring:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else {
        // Calculate slope for pour detection
        calculateSlope(idx, pourDetectionWeight, timestampMs);

        // Update current pour volume (for real-time display)
        const float currentWeightDrop = scale.prePourWeight - currentWeight;
        WeightVolumeConverter converter(idx);
        scale.pourVolume = converter.weightToVolume(currentWeightDrop);

        if (getAverageSlope(idx) > _pourSlopeThreshold) {
          // Still pouring
          if (timeInState >= _pourDurationMs) {
            scale.stableWeight = currentWeight;
            transitionState(idx, ChangeDetectionState::Restabilizing,
                            timestampMs);
            firePourCompletedWithSplitting(idx, timestampMs);
          }
        } else if (timeInState >= _pourDurationMs) {
          // Pour complete
          scale.stableWeight = currentWeight;
          transitionState(idx, ChangeDetectionState::Restabilizing,
                          timestampMs);
          firePourCompletedWithSplitting(idx, timestampMs);
        }
      }
      break;

    case ChangeDetectionState::Restabilizing:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight)) {
        if (timeInState >= _levelStabilizationDurationMs) {
          transitionState(idx, ChangeDetectionState::Stable, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::STABLE_DETECTED,
                    timestampMs);

          // Check passive detection: did weight drop significantly?
          // This catches pours missed by active slope detection
          checkPassiveDetection(idx, timestampMs);
          resetSlope(idx);
        }
      } else {
        scale.stableWeight = currentWeight;
        scale.stateEntryTimeMs = timestampMs;  // Reset timer
      }
      break;

    case ChangeDetectionState::KegAbsent:
      if (isWeightPresent(currentWeight, idx)) {
        // Keg reappeared
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::ReplacingKeg, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REPLACED, timestampMs);
      }
      break;

    case ChangeDetectionState::ReplacingKeg:
      if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight)) {
        if (timeInState >= _kegReplacementDurationMs) {
          transitionState(idx, ChangeDetectionState::Stable, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::STABLE_DETECTED,
                    timestampMs);
          resetSlope(idx);
        }
      } else {
        scale.stableWeight = currentWeight;
        scale.stateEntryTimeMs = timestampMs;  // Reset timer
      }
      break;

    case ChangeDetectionState::InvalidWeight:
      if (isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::Stabilizing, timestampMs);
      }
      break;

    default:
      break;
  }
}

ChangeDetectionState ChangeDetection::getState(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].state;
}

float ChangeDetection::getStableWeight(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].stableWeight;
}

float ChangeDetection::getPouringVolume(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].pourVolume;
}

float ChangeDetection::getStableVolume(UnitIndex idx) const {
  // Convert stable weight to beer volume using FG
  WeightVolumeConverter converter(idx);
  return converter.weightToVolume(getStableWeight(idx));
}

float ChangeDetection::getLastPourVolume(UnitIndex idx) const {
  // Return the last/max pour volume from statistics
  return getStatistics(idx).maxPourVolume;
}

const char* ChangeDetection::getStateString(UnitIndex idx) const {
  switch (getState(idx)) {
    case ChangeDetectionState::Idle:
      return "Idle";
    case ChangeDetectionState::Stabilizing:
      return "Stabilizing";
    case ChangeDetectionState::Stable:
      return "Stable";
    case ChangeDetectionState::Pouring:
      return "Pouring";
    case ChangeDetectionState::Restabilizing:
      return "Restabilizing";
    case ChangeDetectionState::KegAbsent:
      return "KegAbsent";
    case ChangeDetectionState::ReplacingKeg:
      return "ReplacingKeg";
    case ChangeDetectionState::InvalidWeight:
      return "InvalidWeight";
    default:
      return "Unknown";
  }
}

float ChangeDetection::getConfidence(UnitIndex idx) const {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  uint64_t timeInState = (millis() - scale.stateEntryTimeMs);
  float confidence = 0.0f;

  switch (scale.state) {
    case ChangeDetectionState::Stabilizing:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_levelStabilizationDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::Pouring:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_pourDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::KegAbsent:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_kegAbsenceDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::ReplacingKeg:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_kegReplacementDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::Stable:
      confidence = 100.0f;
      break;
    default:
      confidence = 0.0f;
      break;
  }

  return std::min(confidence, 100.0f);
}

bool ChangeDetection::getNextEvent(ChangeDetectionEvent& event) {
  return _eventQueue.pop(event);
}

bool ChangeDetection::hasQueuedEvents() const { return !_eventQueue.isEmpty(); }

size_t ChangeDetection::getPendingEventCount() const {
  return _eventQueue.count();
}

// EOF
