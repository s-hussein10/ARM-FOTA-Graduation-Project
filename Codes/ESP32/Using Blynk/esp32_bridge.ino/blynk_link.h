#pragma once
#include "types.h"

// Connect to WiFi and initialize Blynk client
void BlynkLink_voidInit(void);
// Run Blynk background tasks (call inside loop)
void BlynkLink_voidPoll(void);
// Push device telemetry to Blynk virtual pins V0..V3
void BlynkLink_voidSendStatus(const DeviceStatus* CopyPointerStatus, uint8_t Copy_u8Online);

