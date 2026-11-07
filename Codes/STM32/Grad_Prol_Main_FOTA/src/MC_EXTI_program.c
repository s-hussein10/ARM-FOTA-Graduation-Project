/*
 * MC_EXTI_program.c
 *
 *  Created on: 13 Aug 2026
 *      Author: Sief
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MC_EXTI_private.h"
#include "MC_EXTI_config.h"
#include "MC_EXTI_interface.h"


/*
 * =====================================================================
 * Function: EXTI_voidInit
 * =====================================================================
 * Purpose:
 *   Fully configure one EXTI line from scratch:
 *     1) Route the line to the correct GPIO port  (SYSCFG_EXTICRx)
 *     2) Select the trigger edge                  (EXTI_RTSR / EXTI_FTSR)
 *     3) Enable the line as an interrupt source    (EXTI_IMR)
 *
 * IMPORTANT ASSUMPTION (documented on purpose):
 *   This function assumes the RCC clock for SYSCFG (and for the target
 *   GPIO port) is ALREADY enabled by the user in main() BEFORE calling
 *   this function. This driver does NOT enable any RCC clock itself.
 *   If you forget to enable the SYSCFG clock, writes to SYSCFG_EXTICR
 *   will be silently ignored (register won't actually update) and the
 *   line will never trigger, even though the code compiles fine.
 * =====================================================================
 */
void EXTI_voidInit(
    EXTI_Line Copy_u8Line_Number,
    u8 Copy_u8Port,
    EXTI_Trigger Copy_u8Trigger_Type)
{
    u8 Local_u8RegisterIndex;
    u8 Local_u8BitPosition;

    volatile u32 *Local_pu32EXTICR;

    if (Copy_u8Line_Number > 15U)
    {
        return;
    }

    Local_u8RegisterIndex = Copy_u8Line_Number / 4U;
    Local_u8BitPosition   = (Copy_u8Line_Number % 4U) * 4U;

    Local_pu32EXTICR = &(SYSCFG_EXTI->SYSCFG_EXTICR1);

    Local_pu32EXTICR[Local_u8RegisterIndex] &=
        ~((u32)0x0FU << Local_u8BitPosition);

    Local_pu32EXTICR[Local_u8RegisterIndex] |=
        ((u32)Copy_u8Port << Local_u8BitPosition);


    switch (Copy_u8Trigger_Type)
    {
        case EXTI_TRIGGER_RISING:

            CLR_BIT(EXTI->EXTI_FTSR, Copy_u8Line_Number);
            SET_BIT(EXTI->EXTI_RTSR, Copy_u8Line_Number);

            break;


        case EXTI_TRIGGER_FALLING:

            CLR_BIT(EXTI->EXTI_RTSR, Copy_u8Line_Number);
            SET_BIT(EXTI->EXTI_FTSR, Copy_u8Line_Number);

            break;


        case EXTI_TRIGGER_BOTH:

            SET_BIT(EXTI->EXTI_RTSR, Copy_u8Line_Number);
            SET_BIT(EXTI->EXTI_FTSR, Copy_u8Line_Number);

            break;


        default:
            return;
    }


    SET_BIT(EXTI->EXTI_IMR, Copy_u8Line_Number);
}

/*
 * =====================================================================
 * Function: EXTI_voidEnableLine
 * =====================================================================
 * Purpose:
 *   Re-enable a line that was previously configured with EXTI_voidInit
 *   but later disabled with EXTI_voidDisableLine.
 *
 *   This function does NOT touch the Port routing or the Trigger edge -
 *   it ONLY sets the interrupt mask bit (IMR). Those other settings are
 *   assumed to already be correct from a previous EXTI_voidInit call.
 * =====================================================================
 */
void EXTI_voidEnableLine(EXTI_Line Copy_u8Line_Number)
{
    SET_BIT(EXTI->EXTI_IMR, Copy_u8Line_Number);
}


/*
 * =====================================================================
 * Function: EXTI_voidDisableLine
 * =====================================================================
 * Purpose:
 *   Temporarily stop a line from generating interrupts, WITHOUT losing
 *   its Port/Trigger configuration. Useful if you want to "pause" an
 *   external interrupt source and resume it later with EnableLine.
 * =====================================================================
 */
void EXTI_voidDisableLine(EXTI_Line Copy_u8Line_Number)
{
    CLR_BIT(EXTI->EXTI_IMR, Copy_u8Line_Number);
}


/*
 * =====================================================================
 * Function: EXTI_voidClearPendingFlag
 * =====================================================================
 * Purpose:
 *   Clear the pending flag for a line inside EXTI_PR.
 *
 * CRITICAL HARDWARE BEHAVIOR (rc_w1 - "read/clear, write 1 to clear"):
 *   Unlike a normal register bit, you do NOT clear a PR bit by writing
 *   0 to it. Writing 0 does nothing. You must WRITE 1 to the bit to
 *   clear it. This is why we use SET_BIT here even though the intent
 *   is "clearing" the flag - that's just how this specific register
 *   behaves on real hardware (documented in the reference manual).
 *
 * WHERE TO CALL THIS:
 *   This function must be called INSIDE the line's ISR (interrupt
 *   handler), near the end, AFTER you've handled the event. It should
 *   NOT be called from main() and NOT from EXTI_voidInit(). If you
 *   forget to call it, the processor may think the interrupt is still
 *   pending and re-enter the ISR unexpectedly.
 * =====================================================================
 */
void EXTI_voidClearPendingFlag(EXTI_Line Copy_Line)
{
    EXTI->EXTI_PR = (1U << Copy_Line);   /* Writing 1 clears the flag (rc_w1) */
}


static void (*Global_apfCallback[16])(void) = {NULL};

void EXTI_voidSetCallback(EXTI_Line Copy_Line, void (*Copy_pfCallback)(void))
 {
	Global_apfCallback[Copy_Line] = Copy_pfCallback;
 }


//************************************************************//

void EXTI0_IRQHandler(void)
{
    EXTI_voidClearPendingFlag(EXTI_Line0);

    if (Global_apfCallback[0] != NULL)
    {
        Global_apfCallback[0]();
    }
}

void EXTI1_IRQHandler(void)
{
    EXTI_voidClearPendingFlag(EXTI_Line1);

    if (Global_apfCallback[1] != NULL)
    {
        Global_apfCallback[1]();
    }
}

void EXTI2_IRQHandler(void)
{
    EXTI_voidClearPendingFlag(EXTI_Line2);

    if (Global_apfCallback[2] != NULL)
    {
        Global_apfCallback[2]();
    }
}


void EXTI3_IRQHandler(void)
{
    EXTI_voidClearPendingFlag(EXTI_Line3);

    if (Global_apfCallback[3] != NULL)
    {
        Global_apfCallback[3]();
    }
}

void EXTI4_IRQHandler(void)
{
    EXTI_voidClearPendingFlag(EXTI_Line4);

    if (Global_apfCallback[4] != NULL)
    {
        Global_apfCallback[4]();
    }
}

void EXTI9_5_IRQHandler(void)
{
    u8 Local_u8Line;

    for (Local_u8Line = 5; Local_u8Line <= 9; Local_u8Line++)
    {
        if (GET_BIT(EXTI->EXTI_PR, Local_u8Line))
        {
            EXTI_voidClearPendingFlag(Local_u8Line);
            if (Global_apfCallback[Local_u8Line] != NULL)
                Global_apfCallback[Local_u8Line]();
        }
    }
}

void EXTI15_10_IRQHandler(void)
{
	u8 Local_u8Line;
    for (Local_u8Line = 10; Local_u8Line <= 15; Local_u8Line++)
    {
        if (GET_BIT(EXTI->EXTI_PR, Local_u8Line))
        {
            EXTI_voidClearPendingFlag(Local_u8Line);
            if (Global_apfCallback[Local_u8Line] != NULL)
                Global_apfCallback[Local_u8Line]();
        }
    }
}



















