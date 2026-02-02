#pragma once
#include <Arduino.h>

void initLogger();

void logSensors(unsigned long t_ms,
                float pressure_hPa, float relAlt_m,
                float temp_C, float humidity_RH, float gas_Ohms,
                float ax, float ay, float az,
                float gx, float gy, float gz);

void logLoraData(const String &line);
void closeLogger();

int getFlightCount();
int getCurrentFlightNumber();
