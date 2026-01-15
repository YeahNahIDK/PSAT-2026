#include "app_state.h"

// Global shared objects (defined once here)
SharedSensors gSensors;
SemaphoreHandle_t gSensorsMutex = nullptr;
