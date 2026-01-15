#include "sd_logger.h"
#include "config.h"
#include <SPI.h>
#include <SD.h>

// We create a dedicated SPI instance so we can explicitly set pins.
static SPIClass spiSD(FSPI);

static const char* COUNTER_FILE = "/FLIGHT_COUNTER.TXT";

bool SDLogger::begin() {
  // Initialise SPI pins for SD card
  spiSD.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  // Mount SD
  if (!SD.begin(PIN_SD_CS, spiSD)) return false;

  return true;
}

bool SDLogger::loadNextFlightId(uint32_t &outNextId) {
  // If the counter file doesn't exist yet, start from 1.
  if (!SD.exists(COUNTER_FILE)) {
    outNextId = 1;
    return saveNextFlightId(outNextId);
  }

  File f = SD.open(COUNTER_FILE, FILE_READ);
  if (!f) return false;

  String s = f.readStringUntil('\n');
  f.close();

  s.trim();
  uint32_t val = (uint32_t) s.toInt();
  if (val == 0) val = 1;

  outNextId = val;
  return true;
}

bool SDLogger::saveNextFlightId(uint32_t nextId) {
  File f = SD.open(COUNTER_FILE, FILE_WRITE);
  if (!f) return false;

  // Overwrite file content by truncating:
  // Arduino SD doesn't always support truncate nicely, so we remove then write.
  f.close();
  SD.remove(COUNTER_FILE);

  f = SD.open(COUNTER_FILE, FILE_WRITE);
  if (!f) return false;

  f.println(nextId);
  f.flush();
  f.close();
  return true;
}

String SDLogger::makeFilename(uint32_t flightId) {
  char buf[32];
  snprintf(buf, sizeof(buf), "/FLIGHT_%03lu.CSV", (unsigned long)flightId);
  return String(buf);
}

bool SDLogger::startFlightFile(uint32_t flightId) {
  stop(); // close any previous file

  _filename = makeFilename(flightId);
  _file = SD.open(_filename, FILE_WRITE);
  if (!_file) { _open = false; return false; }

  _open = true;
  _rowsSinceFlush = 0;

  // CSV header (names match columns exactly)
  _file.println(
    "flight_id,t_ms,temp_C,hum_pct,press_Pa,alt_rel_m,"
    "ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,roll_deg,pitch_deg,amag_g"
  );
  _file.flush();

  return true;
}

void SDLogger::stop() {
  if (_open) {
    _file.flush();
    _file.close();
  }
  _open = false;
  _filename = "";
  _rowsSinceFlush = 0;
}

bool SDLogger::writeRow(const LogRow& r) {
  if (!_open) return false;

  // We do "print" instead of building huge Strings (less heap fragmentation).
  _file.print(r.flightId); _file.print(",");
  _file.print(r.tMs); _file.print(",");

  _file.print(r.tempC, 2); _file.print(",");
  _file.print(r.hum, 2); _file.print(",");
  _file.print(r.pressPa, 1); _file.print(",");
  _file.print(r.altRelM, 2); _file.print(",");

  _file.print(r.ax, 4); _file.print(",");
  _file.print(r.ay, 4); _file.print(",");
  _file.print(r.az, 4); _file.print(",");

  _file.print(r.gx, 3); _file.print(",");
  _file.print(r.gy, 3); _file.print(",");
  _file.print(r.gz, 3); _file.print(",");

  _file.print(r.roll, 2); _file.print(",");
  _file.print(r.pitch, 2); _file.print(",");

  _file.println(r.aMagG, 4);

  // Periodic flush improves survivability if power is lost.
  _rowsSinceFlush++;
  if (_rowsSinceFlush >= FLUSH_EVERY_ROWS) {
    _file.flush();
    _rowsSinceFlush = 0;
  }

  return true;
}

void SDLogger::flush() {
  if (_open) _file.flush();
}
