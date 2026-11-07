#pragma once
#include "types.h"

// Initialize UART2 for STM32 communication
void STMLink_voidInit(void);
// Read incoming UART bytes and parse full lines (non-blocking)
void STMLink_voidPoll(void);

// Copy latest status if new data arrived; returns 1 if updated, 0 otherwise
uint8_t STMLink_u8TakeStatus(DeviceStatus* Copy_pstStatus);
// Return 1 if STM32 responded within timeout, 0 if offline
uint8_t STMLink_u8Connect(void);

// Send command to STM32 over UART2 as JSON
void STMLink_voidSend(const Command* Copy_pstCmd);