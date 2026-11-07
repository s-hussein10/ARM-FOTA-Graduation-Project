/*
 * SSD_program.c
 *
 *  Created on: 29 Aug 2025
 *      Author: Sief
 */
#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MC_RCC_interface.h"
#include "MC_GPIO_interface.h"

#include "HA_SSD_interface.h"

static const u16 num_arr[10] = {ZERO, ONE, TWO, THERE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

static void SSD_voidWritePattern(SSD_t* Copy_pstructSsd, u16 Copy_u16Pattern)
{
    u8 Local_u8Idx;
    u8 Local_uddtBitVal;

    for (Local_u8Idx = 0u; Local_u8Idx < SSD_NUM_SEGMENTS; Local_u8Idx++)
    {
        /* Use PIN_HIGH and PIN_LOW instead of HIGH and LOW */
        Local_uddtBitVal = (GET_BIT(Copy_u16Pattern, Local_u8Idx)) ? PIN_HIGH : PIN_LOW;

        if (Copy_pstructSsd->State == SSD_COMMON_ANODE)
        {
            Local_uddtBitVal = (Local_uddtBitVal == PIN_HIGH) ? PIN_LOW : PIN_HIGH;
        }

        /* Use MGPIO instead of GPIO */
        MGPIO_voidSetAtomicPinValue(Copy_pstructSsd->Port, Copy_pstructSsd->SegmentPins[Local_u8Idx], Local_uddtBitVal);
    }
}

void SSD_voidInit(SSD_t* Copy_pstructSsd)
{
    u8 Local_u8Idx;

    RCC_voidEnblePeripheral(AHB1, (u8)Copy_pstructSsd->Port);

    for (Local_u8Idx = 0u; Local_u8Idx < SSD_NUM_SEGMENTS; Local_u8Idx++)
    {
        /* Use MGPIO instead of GPIO */
        MGPIO_voidSetPinMode(Copy_pstructSsd->Port, Copy_pstructSsd->SegmentPins[Local_u8Idx], OUTPUT);
        MGPIO_voidSetOutputConfig(Copy_pstructSsd->Port, Copy_pstructSsd->SegmentPins[Local_u8Idx], PUSH_PULL, LOW_SPEED);
    }

    SSD_voidDisable(Copy_pstructSsd);
}

void SSD_voidEnable(SSD_t* Copy_pstructSsd, u8 Copy_u8Number)
{
    if (Copy_u8Number <= 9u)
    {
        SSD_voidWritePattern(Copy_pstructSsd, num_arr[Copy_u8Number]);
    }
}

void SSD_voidDisable(SSD_t* Copy_pstructSsd)
{
    SSD_voidWritePattern(Copy_pstructSsd, 0x00u);
}
