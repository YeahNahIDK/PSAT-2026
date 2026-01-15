#include <Arduino.h>
#include "config.h"

static HardwareSerial LinkServo(1);

static void taskServoUart(void* ) {
  const int angles[] = {0, 90, 180, 90};
  int idx = 0;

  TickType_t last = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(1000); // 1 Hz

  for (;;) {
    int a = angles[idx];
    idx = (idx + 1) % 4;

    LinkServo.print("S,");
    LinkServo.print(a);
    LinkServo.print("\n");

    Serial.print("[SERVO_UART] Sent angle: ");
    Serial.println(a);

    vTaskDelayUntil(&last, period);
  }
}

void testServoUartStart() {
  // UART1 on chosen pins
  LinkServo.begin(BAUD_LINK, SERIAL_8N1, PIN_ARDUINO_RX, PIN_ARDUINO_TX);
  xTaskCreatePinnedToCore(taskServoUart, "servo_uart", 4096, nullptr, 2, nullptr, 0);
}
