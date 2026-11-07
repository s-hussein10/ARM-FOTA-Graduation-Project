#pragma once
#include "types.h"

// Route button command code from Blynk V10 to STM32 or ESP32
void CMD_voidHandelCmd(int Copy_s32Code);

// Route brightness value (0..100) from Blynk V11 to STM32
void CMD_voidHandelBrightnes(int Copy_s32Value);