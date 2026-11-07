/*
 * MC_EXTI_interface.h
 *
 *  Created on: 13 Aug 2026
 *      Author: Sief
 */

#ifndef MC_EXTI_INTERFACE_H_
#define MC_EXTI_INTERFACE_H_

#include "STD_TYPES.h"

typedef enum
{
    EXTI_Line0 = 0,
	EXTI_Line1,
	EXTI_Line2,
	EXTI_Line3,
	EXTI_Line4,
	EXTI_Line5,
	EXTI_Line6,
	EXTI_Line7,
	EXTI_Line8,
	EXTI_Line9,
	EXTI_Line10,
	EXTI_Line11,
	EXTI_Line12,
	EXTI_Line13,
	EXTI_Line14,
	EXTI_Line15
} EXTI_Line;


typedef enum
{
    EXTI_TRIGGER_RISING = 0,
    EXTI_TRIGGER_FALLING,
    EXTI_TRIGGER_BOTH
} EXTI_Trigger;


void EXTI_voidInit(EXTI_Line Copy_u8Line_Number , u8 Copy_u8Port , EXTI_Trigger Copy_u8Trigger_Type);
void EXTI_voidEnableLine(EXTI_Line Copy_u8Line_Number );
void EXTI_voidDisableLine(EXTI_Line Copy_u8Line_Number);
void EXTI_voidClearPendingFlag(EXTI_Line Copy_Line);

void EXTI_voidSetCallback(EXTI_Line Copy_Line, void (*Copy_pfCallback)(void));

#endif /* MC_EXTI_INTERFACE_H_ */
