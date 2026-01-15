#pragma once
#include <Arduino.h>
#include "storage/sd_logger.h"

// Queue of log rows (produced by "log task", consumed by SD task)
extern QueueHandle_t gLogQueue;

// Queue of SD commands (open/close flight files)
extern QueueHandle_t gCmdQueue;

// Task entry functions
void taskIMU(void* pv);
void taskBME(void* pv);
void taskFlight(void* pv);
void taskLog(void* pv);
void taskSD(void* pv);
