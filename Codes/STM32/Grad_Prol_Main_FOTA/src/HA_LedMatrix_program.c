/*
 * HA_LedMatrix_program.c
 *
 *  Created on: 7 Aug 2026
 *      Author: Sief
 */

#include "BIT_MATH.h"
#include "STD_TYPES.h"
#include "MC_GPIO_interface.h"
#include "MC_RCC_interface.h"
#include "MC_STK_interface.h"
#include "HA_LedMatrix_interface.h"
#include "HA_LedMatrix_config.h"

static u32 DelayTime = 200;

/*----------------------------------------------------------
 * Initializes GPIO pins used for rows and columns.
 * Must be called once before using any other matrix function.
 *----------------------------------------------------------*/
void LedMatrix_voidInit(void)
{
    u8 Local_u8Index;

    /* Enable clock for GPIOA (rows) and GPIOB (columns) on AHB1 bus.
     * Without this, the GPIO registers won't respond at all. */
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOA);
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOB);

    /* Configure all 8 row pins as OUTPUT, Push-Pull, High Speed.
     * Push-Pull is required because rows need to actively drive HIGH (to
     * supply current) and LOW (to cut it off) - not just sink current. */
    for(Local_u8Index = 0; Local_u8Index < 8; Local_u8Index++)
    {
        MGPIO_voidSetPinMode(ROWS_PORT, Rows_Pins[Local_u8Index], OUTPUT);
        MGPIO_voidSetOutputConfig(ROWS_PORT, Rows_Pins[Local_u8Index], PUSH_PULL, HIGH_SPEED);
    }

    /* Configure all 8 column pins as OUTPUT, Push-Pull, High Speed.
     * Same reasoning as rows - columns need to actively drive HIGH/LOW. */
    for(Local_u8Index = 0; Local_u8Index < 8; Local_u8Index++)
    {
        MGPIO_voidSetPinMode(COLUMN_PORT, Columns_Pins[Local_u8Index], OUTPUT);
        MGPIO_voidSetOutputConfig(COLUMN_PORT, Columns_Pins[Local_u8Index], PUSH_PULL, HIGH_SPEED);
    }
}

/*----------------------------------------------------------
 * Turns OFF all LEDs on the matrix.
 * Rows are pulled LOW (no power to any row) AND columns are pulled
 * HIGH (path to ground is blocked) as a double safety measure -
 * either one alone is enough to keep LEDs off, but doing both protects
 * against partial/unexpected states.
 *----------------------------------------------------------*/
void LedMatrix_voidClearLedMatrix(void)
{
    u8 Local_u8Index;

    /* Cut power to every row */
    for(Local_u8Index = 0; Local_u8Index < 8; Local_u8Index++)
    {
        MGPIO_voidSetAtomicPinValue(ROWS_PORT, Rows_Pins[Local_u8Index], PIN_LOW);
    }

    /* Block every column's path to ground */
    for(Local_u8Index = 0; Local_u8Index < 8; Local_u8Index++)
    {
        MGPIO_voidSetAtomicPinValue(COLUMN_PORT, Columns_Pins[Local_u8Index], PIN_HIGH);
    }
}

/*----------------------------------------------------------
 * Displays one full frame (8x8 pattern) using row-by-row scanning
 * (multiplexing). This function scans ALL 8 rows ONCE - it must be
 * called repeatedly (e.g. in main's while loop) to keep the image
 * visible, since only one row is ever lit at any single instant.
 *
 * Copy_u8Frame[row] = one byte per row, where each bit represents
 * one LED in that row (bit=1 -> LED ON, bit=0 -> LED OFF).
 *----------------------------------------------------------*/
void LedMatrix_voidDisplayFrame(const u8 Copy_u8Frame[8])
{
    u8 Local_u8RowIndex;    /* which row we are currently scanning/lighting */
    u8 Local_u8ClearIndex;  /* used only to turn off all rows before lighting the new one */
    u8 Local_u8ColIndex;    /* which column we are currently setting */

    for(Local_u8RowIndex = 0; Local_u8RowIndex < 8; Local_u8RowIndex++)
    {
        /* Step 1: Turn OFF all rows first.
         * This prevents "ghosting" - i.e. more than one row being
         * energized at the same time while we change the column pattern. */
        for(Local_u8ClearIndex = 0; Local_u8ClearIndex < 8; Local_u8ClearIndex++)
        {
            MGPIO_voidSetAtomicPinValue(ROWS_PORT, Rows_Pins[Local_u8ClearIndex], PIN_LOW);
        }

        /* Step 2: Set up the column pattern for THIS row, based on the
         * corresponding byte in the frame. Each bit is checked one by one. */
        for(Local_u8ColIndex = 0; Local_u8ColIndex < 8; Local_u8ColIndex++)
        {
            if(GET_BIT(Copy_u8Frame[Local_u8RowIndex], Local_u8ColIndex) == 1)
            {
                /* LED should be ON -> column must be LOW to complete
                 * the current path to ground (common cathode columns) */
                MGPIO_voidSetAtomicPinValue(COLUMN_PORT, Columns_Pins[Local_u8ColIndex], PIN_LOW);
            }
            else
            {
                /* LED should be OFF -> column must be HIGH to block
                 * the current path, even if it was LOW in a previous row */
                MGPIO_voidSetAtomicPinValue(COLUMN_PORT, Columns_Pins[Local_u8ColIndex], PIN_HIGH);
            }
        }

        /* Step 3: Energize the current row now that columns are ready.
         * Only THIS row is powered - all others are still LOW from Step 1. */
        MGPIO_voidSetAtomicPinValue(ROWS_PORT, Rows_Pins[Local_u8RowIndex], PIN_HIGH);

        /* Step 4: Keep the row ON for a short time so the human eye can
         * perceive it (Persistence of Vision). Using microseconds gives
         * precise control over the refresh rate - with 200us per row,
         * a full 8-row cycle takes 1.6ms -> ~625Hz refresh rate (no flicker). */
        STK_voidDelay_us(DelayTime);

        /* Step 5: Turn the row back OFF before moving to the next one.
         * This avoids overlapping two rows being lit simultaneously. */
        MGPIO_voidSetAtomicPinValue(ROWS_PORT, Rows_Pins[Local_u8RowIndex], PIN_LOW);
    }
}

void LedMatrix_voidSetNewDelay(u32 Copy_u32NewDelay)
{
	DelayTime = Copy_u32NewDelay ;
}

u32 LedMatrix_u32GetTimeDelay(void)
{
	return DelayTime ;
}



















