/*
 * MC_STK_program.c
 *
 *  Created on: 3 Aug 2026
 *      Author: Sief
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MC_STK_config.h"
#include "MC_STK_interface.h"
#include "MC_STK_private.h"

static void (*STK_Callback)(void) = NULL;

void STK_voidInit(void)
{
#if STK_CLOCK_BUS_SOURCE ==  AHB
	SET_BIT(SYSTICK->STK_CTRL , CLK_SOURCE);
#elif STK_CLOCK_BUS_SOURCE ==  AHB_DIV_BY_8
	CLR_BIT(SYSTICK->STK_CTRL , CLK_SOURCE);
#endif
}

void STK_voidEnableInterrupt(void)
{
	SET_BIT(SYSTICK->STK_CTRL , TICK_INT);
}

void STK_voidDisableInterrupt(void)
{
	CLR_BIT(SYSTICK->STK_CTRL , TICK_INT);
}

// for delay
void STK_voidDelay_ms(u32 copy_u32DelayTime)
{
	u32 No_Tickes = STK_TICKS_PER_MS * copy_u32DelayTime;
	SYSTICK->STK_LOAD = No_Tickes;
	SYSTICK->STK_VAL = 0 ;
	SET_BIT(SYSTICK->STK_CTRL , ENABLE);

	while (! GET_BIT(SYSTICK->STK_CTRL , COUN_FLAG))
	{
		asm("NOP");
	}
}

void STK_voidDelay_us(u32 copy_u32DelayTime)
{
	u32 No_Tickes = STK_TICKS_PER_US * copy_u32DelayTime;
	SYSTICK->STK_LOAD = No_Tickes;
	SYSTICK->STK_VAL = 0 ;
	SET_BIT(SYSTICK->STK_CTRL , ENABLE);

	while (! GET_BIT(SYSTICK->STK_CTRL , COUN_FLAG))
	{
		asm("NOP");
	}

}


void STK_voidStartPeriodicInterrupt(u32 Copy_u32Ticks)
{
    Copy_u32Ticks &= 0x00FFFFFFUL;

    SYSTICK->STK_LOAD = Copy_u32Ticks;
    SYSTICK->STK_VAL  = 0;

    Global_u32IntervalTicks = Copy_u32Ticks;

    SET_BIT(SYSTICK->STK_CTRL, ENABLE);
}


u32 STK_u32GetElapsedTime(void)
{
    u32 Local_u32CurrentValue;
    u32 Local_u32Elapsed;

    Local_u32CurrentValue = SYSTICK->STK_VAL;

    if (Local_u32CurrentValue <= Global_u32IntervalTicks)
    {
        Local_u32Elapsed =
            Global_u32IntervalTicks - Local_u32CurrentValue;
    }
    else
    {
        Local_u32Elapsed = 0;
    }

    return Local_u32Elapsed;
}

u32 STK_u32GetRemaningTime(void)
{
    return (SYSTICK->STK_VAL);
}

void STK_voidResetCounter(void)
{
	SYSTICK->STK_VAL=0;
}

void STK_voidSetCallBack(void (*Copy_pvCallback)(void))
{
    STK_Callback = Copy_pvCallback;
}

void SysTick_Handler(void)
{
    if (STK_Callback != NULL)
    {
        STK_Callback();
    }
}
