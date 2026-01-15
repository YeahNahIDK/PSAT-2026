#pragma once
#include <Arduino.h>
#include <FS.h>

// One row in the CSV log.
struct LogRow {
  uint32_t flightId = 0;
  uint32_t tMs = 0;

  float tempC = NAN;
  float hum = NAN;
  float pressPa = NAN;
  float altRelM = NAN;

  float ax=0, ay=0, az=0;
  float gx=0, gy=0, gz=0;
  float roll=0, pitch=0;
  float aMagG=0;
};

// Commands sent to the SD task (so ONLY the SD task touches SD)
enum class SDCmdType : uint8_t {
  OPEN_FLIGHT,
  CLOSE_FLIGHT
};

struct SDCmd {
  SDCmdType type;
  uint32_t flightId; // used for OPEN_FLIGHT
};

class SDLogger {
public:
  // Mount SD card
  bool begin();

  // Persistent flight counter stored in /FLIGHT_COUNTER.TXT
  // This returns the "next flight id" to use.
  bool loadNextFlightId(uint32_t &outNextId);

  // Save next flight id back to file
  bool saveNextFlightId(uint32_t nextId);

  // Open a new CSV for flightId
  bool startFlightFile(uint32_t flightId);

  // Close the current flight file
  void stop();

  // Append a row (fast prints)
  bool writeRow(const LogRow& r);

  // Force flush (slower but safer)
  void flush();

  bool isOpen() const { return _open; }
  String currentFilename() const { return _filename; }

private:
  bool _open = false;
  String _filename;
  File _file;
  uint32_t _rowsSinceFlush = 0;

  String makeFilename(uint32_t flightId);
};
