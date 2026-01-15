#include "tasks.h"
#include <Wire.h>

#include "config.h"
#include "app_state.h"

#include "sensors/bme680_sensor.h"
#include "sensors/mpu6050_sensor.h"
#include "flight/flight_detector.h"
#include "storage/sd_logger.h"

// Queues (created in main.cpp)
QueueHandle_t gLogQueue = nullptr;
QueueHandle_t gCmdQueue = nullptr;

// These live in this compilation unit and are used by tasks
static MPU6050Sensor imu;
static BME680Sensor bme;
static FlightDetector flight;
static SDLogger sd;

// Flight counter state (persistent on SD)
static uint32_t nextFlightId = 1;

// Helper: take an altitude baseline without delay()
// We just do a handful of reads spaced by task scheduling.
// Called from flight task; it will naturally run over multiple loops.
static bool baselineValid = false;
static float baselineAltM = 0.0f;

void taskIMU(void* pv) {
  // Initialise IMU in the task so we control when it happens
  if (!imu.begin()) {
    // If init fails, we stop this task forever (better than spamming)
    vTaskSuspend(nullptr);
  }

  // Periodic scheduling without delay():
  // vTaskDelayUntil gives stable timing.
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000 / IMU_HZ);

  for (;;) {
    IMUData local;
    imu.update(local);

    // Write to shared snapshot
    xSemaphoreTake(gSensorsMutex, portMAX_DELAY);
    gSensors.imu = local;
    xSemaphoreGive(gSensorsMutex);

    vTaskDelayUntil(&lastWake, period);
  }
}

void taskBME(void* pv) {
  if (!bme.begin()) {
    vTaskSuspend(nullptr);
  }

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000 / BME_HZ);

  for (;;) {
    BMEData local;
    bme.read(local);

    xSemaphoreTake(gSensorsMutex, portMAX_DELAY);
    gSensors.bme = local;
    xSemaphoreGive(gSensorsMutex);

    vTaskDelayUntil(&lastWake, period);
  }
}

void taskFlight(void* pv) {
  flight.begin();

  // We arm once we have a baseline altitude.
  // Baseline altitude is just "current altitude at rest on pad".
  // Without RTC/GPS, we still have reliable ms timestamps.

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(20); // 50 Hz-ish logic loop

  // Simple baseline accumulation (average ~25 samples)
  float sumAlt = 0.0f;
  uint32_t countAlt = 0;

  for (;;) {
    IMUData imuLocal;
    BMEData bmeLocal;

    // Take a snapshot of latest sensors
    xSemaphoreTake(gSensorsMutex, portMAX_DELAY);
    imuLocal = gSensors.imu;
    bmeLocal = gSensors.bme;
    xSemaphoreGive(gSensorsMutex);

    // Build baseline once at start
    if (!baselineValid && isfinite(bmeLocal.altitudeM)) {
      sumAlt += bmeLocal.altitudeM;
      countAlt++;

      if (countAlt >= 25) {
        baselineAltM = sumAlt / (float)countAlt;
        bme.setAltBaseline(baselineAltM);
        baselineValid = true;

        // Arm flight detector now that we have a baseline
        flight.arm(baselineAltM);
      }

      vTaskDelayUntil(&lastWake, period);
      continue;
    }

    if (!baselineValid) {
      // Still waiting for good altitude readings
      vTaskDelayUntil(&lastWake, period);
      continue;
    }

    float altRel = bme.altRelative(bmeLocal.altitudeM);
    flight.update(imuLocal.aMagG, altRel, millis());

    // Liftoff event: tell SD task to open a new file
    if (flight.justLiftoff()) {
      // Assign flight ID from persistent counter
      uint32_t thisFlight = nextFlightId;
      flight.setActiveFlightId(thisFlight);

      // Command SD task to open the file
      SDCmd cmd;
      cmd.type = SDCmdType::OPEN_FLIGHT;
      cmd.flightId = thisFlight;
      xQueueSend(gCmdQueue, &cmd, 0);

      // Increment counter for next time and persist via SD task
      nextFlightId++;

      flight.clearEvents();
    }

    // Landed event: tell SD task to close file
    if (flight.justLanded()) {
      SDCmd cmd;
      cmd.type = SDCmdType::CLOSE_FLIGHT;
      cmd.flightId = 0;
      xQueueSend(gCmdQueue, &cmd, 0);

      // OPTIONAL: re-arm baseline after landing
      // Here we keep the same baseline unless you want to re-average.
      flight.clearEvents();
      flight.arm(baselineAltM);
    }

    vTaskDelayUntil(&lastWake, period);
  }
}

void taskLog(void* pv) {
  // This task packages the latest sensor snapshot into LogRow objects
  // and pushes them onto the log queue at LOG_HZ.

  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000 / LOG_HZ);

  for (;;) {
    IMUData imuLocal;
    BMEData bmeLocal;

    xSemaphoreTake(gSensorsMutex, portMAX_DELAY);
    imuLocal = gSensors.imu;
    bmeLocal = gSensors.bme;
    xSemaphoreGive(gSensorsMutex);

    // Only log once baseline exists (altRel meaningful)
    if (baselineValid) {
      LogRow row;
      row.flightId = flight.info().flightId;   // 0 until liftoff assigns it
      row.tMs = millis();

      row.tempC = bmeLocal.tempC;
      row.hum = bmeLocal.humidity;
      row.pressPa = bmeLocal.pressurePa;
      row.altRelM = bme.altRelative(bmeLocal.altitudeM);

      row.ax = imuLocal.ax; row.ay = imuLocal.ay; row.az = imuLocal.az;
      row.gx = imuLocal.gx; row.gy = imuLocal.gy; row.gz = imuLocal.gz;
      row.roll = imuLocal.roll; row.pitch = imuLocal.pitch;
      row.aMagG = imuLocal.aMagG;

      // Push row to SD task (non-blocking).
      // If the queue is full, we drop the sample rather than stall the system.
      xQueueSend(gLogQueue, &row, 0);
    }

    vTaskDelayUntil(&lastWake, period);
  }
}

void taskSD(void* pv) {
  // This task is the ONLY place we touch SD card.
  // It:
  // - mounts SD
  // - loads flight counter
  // - opens/closes flight files based on commands
  // - writes CSV rows

  if (!sd.begin()) {
    // If SD init fails, we suspend this task (logging won't work)
    vTaskSuspend(nullptr);
  }

  // Load persistent "nextFlightId"
  uint32_t loaded = 1;
  if (sd.loadNextFlightId(loaded)) {
    nextFlightId = loaded;
  } else {
    nextFlightId = 1;
    sd.saveNextFlightId(nextFlightId);
  }

  LogRow row;
  SDCmd cmd;

  for (;;) {
    // Handle commands quickly if present
    while (xQueueReceive(gCmdQueue, &cmd, 0) == pdPASS) {
      if (cmd.type == SDCmdType::OPEN_FLIGHT) {
        // Persist the incremented counter immediately
        // (we already incremented nextFlightId in flight task)
        sd.saveNextFlightId(nextFlightId);

        // Open the flight file
        sd.startFlightFile(cmd.flightId);
      } else if (cmd.type == SDCmdType::CLOSE_FLIGHT) {
        sd.flush();
        sd.stop();
      }
    }

    // Write any queued rows (block briefly waiting for data)
    if (xQueueReceive(gLogQueue, &row, pdMS_TO_TICKS(50)) == pdPASS) {
      // Only write rows when a flight file is open.
      // Before liftoff, flightId==0 and sd.isOpen()==false, so nothing is logged.
      if (sd.isOpen()) {
        sd.writeRow(row);
      }
    }

    // Loop repeats; no delay() used.
  }
}
