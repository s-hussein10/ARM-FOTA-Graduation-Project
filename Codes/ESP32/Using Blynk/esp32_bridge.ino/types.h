#pragma once
#include <Arduino.h>

// Latest status reported by the STM32
struct DeviceStatus {
  uint8_t  state;          // 0=OFF 1=DEMO 2=SETUP 3=UPDATING 4=ERROR
  uint8_t  brightness;     // 0..100
  uint8_t  menu;
  uint8_t  progress;       // 0..100
  uint8_t  err;
  uint32_t lastUpdateMs;   // millis() when the last line arrived
  bool     valid;          // false until the first line arrives
};

// A command going to the STM32 (char array, not String, so it is safe to copy/queue)
struct Command {
  char    name[16];        // "POWER", "UP", "BRIGHTNESS", ...
  int16_t value;
  bool    hasValue;
};

// Codes that Blynk sends on V10
enum CmdCode : int {
  CMD_NONE    = 0,
  CMD_POWER   = 1,
  CMD_MODE    = 2,
  CMD_UP      = 3,
  CMD_DOWN    = 4,
  CMD_OK      = 5,
  CMD_BACK    = 6,
  CMD_NUM0    = 10,        // NUM0..NUM9 = 10..19
  CMD_NUM9    = 19,
  CMD_RESTART = 20         // handled by the ESP32 itself
};

// State number -> text for Blynk V0
inline const char* stateName(uint8_t s) {
  static const char* const names[] = {"OFF", "DEMO", "SETUP", "UPDATING", "ERROR"};
  return (s < 5) ? names[s] : "ERROR";
}