#pragma once
#include <Arduino.h>

enum class FlightState {
  IDLE,      // not armed
  ARMED,     // armed, waiting for liftoff
  IN_FLIGHT, // flight detected
  LANDED     // landed detected (optional state)
};

struct FlightInfo {
  FlightState state = FlightState::IDLE;

  // flight number (we’ll assign from SD persistent counter)
  uint32_t flightId = 0;

  // baseline altitude at arming
  float baselineAltM = 0.0f;

  // max relative altitude observed during flight
  float maxAltRelM = 0.0f;

  uint32_t liftoffMs = 0;
  uint32_t landedMs  = 0;
};

class FlightDetector {
public:
  void begin();

  // Call when you want to arm the system. Baseline is the current absolute altitude.
  void arm(float currentAltM);

  // Update the detector with the latest values.
  // accMagG: acceleration magnitude (g)
  // altRelM: altitude relative to baseline (m)
  // nowMs: current time (ms since boot)
  void update(float accMagG, float altRelM, uint32_t nowMs);

  // Events
  bool justLiftoff() const { return _justLiftoff; }
  bool justLanded()  const { return _justLanded;  }
  void clearEvents();

  const FlightInfo& info() const { return _info; }

  // Set flight ID when we start a flight (coming from SD counter)
  void setActiveFlightId(uint32_t id) { _info.flightId = id; }

private:
  FlightInfo _info;
  bool _justLiftoff = false;
  bool _justLanded  = false;

  // timers used to enforce "hold time" thresholds
  uint32_t _aboveStartMs = 0;
  uint32_t _landStartMs  = 0;
};
