/*
 * MC_STK_interface.h
 *
 *  Created on: 3 Aug 2026
 *      Author: Sief
 */

#ifndef MC_STK_INTERFACE_H_
#define MC_STK_INTERFACE_H_


void STK_voidInit(void);
void STK_voidEnableInterrupt(void);
void STK_voidDisableInterrupt(void);

void STK_voidDelay_ms(u32 copy_u32DelayTime);
void STK_voidDelay_us(u32 copy_u32DelayTime);

u32 STK_u32GetElapsedTime(void);
u32 STK_u32GetRemaningTime(void);
void STK_voidResetCounter(void);

void STK_voidSetCallBack(void (*Copy_pvCallback)(void));
void SysTick_Handler(void);

void STK_voidStartPeriodicInterrupt(u32 Copy_u32Ticks);

#endif /* MC_STK_INTERFACE_H_ */
