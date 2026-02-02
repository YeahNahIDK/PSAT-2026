#include <Arduino.h>
#include "imu.h"
#include "altimeter.h"
#include "logger.h"
 #include "uart_link.h"   

#define LORA_UART_RX 16
#define LORA_UART_TX 17

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== UART TEST: SENDER ===");

  // Start UART link
  uartInit(LORA_UART_RX, LORA_UART_TX, 115200);
  Serial.println("UART initialized on RX=16, TX=17 @115200");
}

void loop() {
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 1000) {  // send every 1 s
    lastSend = millis();

    float alt = random(1000, 2000) / 10.0;
    float vel = random(-50, 50) / 10.0;
    float acc = random(0, 20) / 10.0;

    // send a fake telemetry line
    uartSendTelemetry(alt, vel, acc);

    Serial.printf("[SENT] ALT:%.2f VEL:%.2f ACC:%.2f\n", alt, vel, acc);
  }
}

/* // ---- Timing ----
unsigned long lastIMUSample  = 0;
const unsigned long imuInterval  = 1000 / 104;  // ~104 Hz

unsigned long lastAltSample  = 0;
const unsigned long altInterval  = 40;          // ~25 Hz

unsigned long lastLog       = 0;
const unsigned long logInterval   = 100;        // ~10 Hz

unsigned long lastPrint     = 0;
const unsigned long printInterval = 1000;       // ~1 Hz

unsigned long lastTelemetry = 0;
const unsigned long telemetryInterval = 100;    // ~10 Hz downlink

// ---- Data ----
ImuSample imuSample;
float baroPressure = NAN, baroAlt = NAN;
bool  baroCalibrated = false;

// For derived velocity
float lastAlt = NAN;
unsigned long lastAltTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

 uartInit(LORA_UART_RX, LORA_UART_TX);  

  // Init IMU
  if (!initIMU()) {
    Serial.println("[IMU] init failed!");
    while (1) { delay(10); }
  }
  Serial.println("[IMU] init OK.");

  // Init Altimeter
  initAltimeter();

  // Init SD logger
  initLogger();

  lastAltTime = millis();
}

void loop() {
  unsigned long now = millis();

  // ---- IMU task (~104 Hz) ----
  if (now - lastIMUSample >= imuInterval) {
    lastIMUSample = now;
    serviceIMU();
    readIMU(imuSample);
  }

  // ---- Altimeter task (~25 Hz) ----
  if (now - lastAltSample >= altInterval) {
    lastAltSample = now;

    float p0;
    bool done;
    if (!baroCalibrated) {
      if (updateAltimeterCalibration(p0, done) && done) {
        setSeaLevelPressure(p0);
        baroCalibrated = true;
        Serial.printf("[ALT] Calibrated p0: %.2f hPa\n", p0);
      }
    }
    if (baroCalibrated) {
      readAltimeter(baroPressure, baroAlt);
    }
  }

  // ---- Logging task (~10 Hz) ----
  if (now - lastLog >= logInterval) {
    lastLog = now;
    logSensors(now,
               baroPressure, baroAlt,
               imuSample.ax, imuSample.ay, imuSample.az,
               imuSample.gx, imuSample.gy, imuSample.gz);
  }

  // ---- Telemetry task (~10 Hz) ----
  if (now - lastTelemetry >= telemetryInterval) {
    lastTelemetry = now;

    float velocity = 0.0f;
    if (!isnan(baroAlt)) {
      float dt = (now - lastAltTime) / 1000.0f;
      if (dt > 0.0005f && !isnan(lastAlt)) {
        velocity = (baroAlt - lastAlt) / dt;
      }
      lastAlt = baroAlt;
      lastAltTime = now;
    }

    float accMag = sqrtf(imuSample.ax * imuSample.ax +
                         imuSample.ay * imuSample.ay +
                         imuSample.az * imuSample.az);

   uartSendTelemetry(baroAlt, velocity, accMag);  
  }

  // ---- Command handling (non-blocking) ----
    String cmd;
    if (uartReadCommand(cmd)) {
     Serial.println(String("[CMD] ") + cmd);
   }

  // ---- Debug print (~1 Hz) ----
  if (now - lastPrint >= printInterval) {
    lastPrint = now;
    Serial.printf("[%.2f s] Flight %d | ALT: %.2f hPa, %.2f m | "
                  "IMU: A[%.2f %.2f %.2f] G[%.2f %.2f %.2f]\n",
                  imuSample.t,
                  getCurrentFlightNumber(),
                  baroPressure, baroAlt,
                  imuSample.ax, imuSample.ay, imuSample.az,
                  imuSample.gx, imuSample.gy, imuSample.gz);
  }
}

*/ 