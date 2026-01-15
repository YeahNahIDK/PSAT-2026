#include "flight_detector.h"
#include "config.h"
#include <math.h>

void FlightDetector::begin() {
  _info = FlightInfo{};
  _justLiftoff = false;
  _justLanded = false;
  _aboveStartMs = 0;
  _landStartMs = 0;
}

void FlightDetector::arm(float currentAltM) {
  _info.state = FlightState::ARMED;
  _info.baselineAltM = currentAltM;
  _info.maxAltRelM = 0.0f;
  _info.liftoffMs = 0;
  _info.landedMs = 0;

  _aboveStartMs = 0;
  _landStartMs = 0;
  clearEvents();
}

void FlightDetector::update(float accMagG, float altRelM, uint32_t nowMs) {
  _justLiftoff = false;
  _justLanded  = false;

  // Track max altitude while armed or flying
  if (_info.state == FlightState::ARMED || _info.state == FlightState::IN_FLIGHT) {
    if (altRelM > _info.maxAltRelM) _info.maxAltRelM = altRelM;
  }

  // ---------- Detect liftoff ----------
  if (_info.state == FlightState::ARMED) {
    // Condition A: acceleration spike
    bool accelHigh = (accMagG >= LIFTOFF_G_THRESHOLD);

    if (accelHigh) {
      if (_aboveStartMs == 0) _aboveStartMs = nowMs;

      bool heldLongEnough = (nowMs - _aboveStartMs) >= LIFTOFF_HOLD_MS;
      bool altRaisedEnough = (_info.maxAltRelM >= MIN_ALT_RISE_M);

      // We require BOTH:
      // - acceleration above threshold for some time
      // - altitude has begun increasing enough to avoid false triggers
      if (heldLongEnough && altRaisedEnough) {
        _info.state = FlightState::IN_FLIGHT;
        _info.liftoffMs = nowMs;
        _justLiftoff = true;

        // reset landing timer
        _landStartMs = 0;
      }
    } else {
      // If accel drops below threshold, reset the hold timer
      _aboveStartMs = 0;
    }
  }

  // ---------- Detect landing ----------
  if (_info.state == FlightState::IN_FLIGHT) {
    // near 1g and altitude close to baseline for a while
    bool near1g = (accMagG <= LAND_G_THRESHOLD);
    bool nearGround = (fabsf(altRelM) <= LAND_ALT_WINDOW_M);

    if (near1g && nearGround) {
      if (_landStartMs == 0) _landStartMs = nowMs;

      if ((nowMs - _landStartMs) >= LAND_HOLD_MS) {
        _info.state = FlightState::LANDED;
        _info.landedMs = nowMs;
        _justLanded = true;
      }
    } else {
      _landStartMs = 0;
    }
  }
}

void FlightDetector::clearEvents() {
  _justLiftoff = false;
  _justLanded = false;
}
