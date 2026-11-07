/*
 * HA_STP_program.c
 *
 *  Created on: 19 Aug 2026
 *      Author: Sief
 */
#include  "BIT_MATH.h"
#include  "STD_TYPES.h"

#include  "HA_STP_config.h"
#include  "HA_STP_interface.h"

#include  "MC_GPIO_interface.h"
#include  "MC_STK_interface.h"


// Init SDI, SHIFT_CLOCK, LATCH_CLOCK pins as push-pull outputs
void STP_voidInit(void)
{
	// SDI: serial data input to the shift register
	MGPIO_voidSetPinMode(STP_PORT , STP_SDI , OUTPUT);
	MGPIO_voidSetOutputConfig(STP_PORT , STP_SDI , PUSH_PULL , LOW_SPEED);
	// LATCH_CLOCK: pushes shifted data to output pins
	MGPIO_voidSetPinMode(STP_PORT , STP_LATCH_CLOCK , OUTPUT);
	MGPIO_voidSetOutputConfig(STP_PORT , STP_LATCH_CLOCK , PUSH_PULL , LOW_SPEED);
	// SHIFT_CLOCK: shifts each bit into the register
	MGPIO_voidSetPinMode(STP_PORT , STP_SHIFT_CLOCK , OUTPUT);
	MGPIO_voidSetOutputConfig(STP_PORT , STP_SHIFT_CLOCK , PUSH_PULL , LOW_SPEED);

}

/*
 * Shifts one byte into the register, bit by bit (LSB first)
 * ex: 0b01101110
 * Note: does not update outputs by itself -> call STP_voidSetLatch() after
 */
void STP_voidSetShift(u8 Copy_u8InputData)
{
	u8 Local_u8Bit ;

	// Loop over all 8 bits without touching the original data
	for(u8 itrator = 0 ; itrator<8 ; itrator++)
	{
		// Extract bit (itrator) from the input byte
		Local_u8Bit = GET_BIT(Copy_u8InputData , itrator);

		// Put the bit on SDI before the clock edge
		MGPIO_voidSetPinValue(STP_PORT , STP_SDI , Local_u8Bit);

		// Rising edge (low->high) shifts the SDI bit into the register
		MGPIO_voidSetPinValue(STP_PORT , STP_SHIFT_CLOCK , PIN_LOW);
		STK_voidDelay_ms(20);
		MGPIO_voidSetPinValue(STP_PORT , STP_SHIFT_CLOCK , PIN_HIGH);
	}
}

/*
 * Latch pulse: moves all 8 shifted bits to the output pins at once
 * Call once after all needed SetShift calls (useful for daisy-chained registers)
 */
void STP_voidSetLatch(void)
{
	MGPIO_voidSetPinValue(STP_PORT , STP_LATCH_CLOCK , PIN_LOW);
	STK_voidDelay_ms(20);
	MGPIO_voidSetPinValue(STP_PORT , STP_LATCH_CLOCK , PIN_HIGH);

}
