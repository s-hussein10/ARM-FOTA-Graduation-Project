#include  "BIT_MATH.h"
#include  "STD_TYPES.h"
#include  "MC_GPIO_config.h"
#include  "MC_GPIO_interface.h"
#include  "MC_GPIO_private.h"

#define GPIO_MODE_MASK    3   // N=2 bits per pin => (1<<N)-1 = (2^N)-1 = 3

#define WRITE_FIELD_SET_BIT(REG, MASK, POSITION, VALUE)  \
    do { \
        (REG) &= ~((MASK) << (POSITION)); \
        (REG) |= ((VALUE) << (POSITION)); \
    } while(0)

/*----------------------------------------------------------*/
// Select Pin Mode
/*
 * BUG FIX (root cause of "PC13 not lighting, PA15 lights instead" issue):
 *
 * The ORIGINAL code had this function defined TWICE - once as the outer
 * function, and once again INSIDE its own body:
 *
 *     void MGPIO_voidSetPinMode(...)
 *     {
 *         void MGPIO_voidSetPinMode(...)   // <-- nested duplicate, never called!
 *         {
 *             ... all the real switch-case logic was trapped in here ...
 *         }
 *     }
 *
 * GCC with the GNU extensions (-std=gnu11, used in this project's build
 * flags) allows nested function DEFINITIONS as a compiler extension.
 * This means the code compiled with ZERO errors and ZERO warnings, but
 * the outer function's body only ever DEFINED the inner function - it
 * never CALLED it. So every time MGPIO_voidSetPinMode() was invoked from
 * main.c, the outer function ran, did nothing, and returned immediately.
 * The MODER register was never actually written.
 *
 * Practical effect: any pin you tried to configure as OUTPUT (e.g. PC13
 * for the LED) silently stayed in its RESET state (INPUT), so writing to
 * ODR/BSRR afterward had no real electrical effect on that pin.
 *
 * FIX: keep only ONE definition of the function, with the switch-case
 * logic directly inside it (no nested duplicate).
 */
void MGPIO_voidSetPinMode(PORTS Copy_uddtPort, PINS Copy_uddtPin, MODES Copy_uddtMode)
{
    switch(Copy_uddtPort)
    {
        case PORTA:
            if (Copy_uddtPin <= PIN12)   // protect SWD pins PA13(SWDIO)/PA14(SWCLK)/PA15(JTDI) for debugging
            {
                WRITE_FIELD_SET_BIT(GPIOA->GPIOx_MODER, 3, (2 * Copy_uddtPin), Copy_uddtMode);
            }
            break;
        case PORTB:
            // pins in PORTB  16 pin
            WRITE_FIELD_SET_BIT(GPIOB->GPIOx_MODER, 3, (2 * Copy_uddtPin), Copy_uddtMode);
            break;
        case PORTC:
            // Pins in PORTC 16 pin
            WRITE_FIELD_SET_BIT(GPIOC->GPIOx_MODER, 3, (2 * Copy_uddtPin), Copy_uddtMode);
            break;
        default:
            break;   // invalid port, do nothing
    }
}

//Output Functions
/*
 * BUG FIX: same nested-duplicate-function issue as MGPIO_voidSetPinMode
 * above. The OTYPER/OSPEEDR configuration logic was trapped inside an
 * inner function that was never called. Fixed the same way: single
 * definition, logic directly inside it.
 */
void MGPIO_voidSetOutputConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin,TYPES Copy_uddtType, SPEED Copy_uddtSpeed)
{
	switch(Copy_uddtPort)
	{
		case PORTA:
			if (Copy_uddtPin <= PIN12)   // Protect SWD pins: PA13(SWDIO), PA14(SWCLK), PA15(JTDI)
			{
				/* OTYPER: 1 bit per pin -> only 2 options (Push-Pull=0 / Open-Drain=1)
				 * => use SET_BIT / CLR_BIT (no need for field write) */
				if (Copy_uddtType == PUSH_PULL)
				{
					CLR_BIT(GPIOA->GPIOx_OTYPER, Copy_uddtPin);   // clear bit -> Push-Pull
				}
				else if (Copy_uddtType == OPEN_DRAIN)
				{
					SET_BIT(GPIOA->GPIOx_OTYPER, Copy_uddtPin);   // set bit -> Open-Drain
				}
				/* OSPEEDR: 2 bits per pin -> 4 options (LOW/MED/HIGH/VERY_HIGH = 0,1,2,3)
				 * => 1 bit not enough to represent 4 states, so use WRITE_FIELD_SET_BIT
				 * MASK = 3 (binary 11) because (1<<2)-1 = 3 -> covers 2 bits
				 * POSITION = 2 * pin_number -> each pin occupies 2 bits in the register */
				WRITE_FIELD_SET_BIT(GPIOA->GPIOx_OSPEEDR, 3, (2 * Copy_uddtPin), Copy_uddtSpeed);
			}
			break;
		case PORTB:
			/* No reserved/debug pins in PORTB -> all 16 pins allowed, same logic as PORTA */
			if (Copy_uddtType == PUSH_PULL)
			{
				CLR_BIT(GPIOB->GPIOx_OTYPER, Copy_uddtPin);
			}
			else if (Copy_uddtType == OPEN_DRAIN)
			{
				SET_BIT(GPIOB->GPIOx_OTYPER, Copy_uddtPin);
			}
			WRITE_FIELD_SET_BIT(GPIOB->GPIOx_OSPEEDR, 3, (2 * Copy_uddtPin), Copy_uddtSpeed);
			break;
		case PORTC:
			/* No reserved/debug pins in PORTC -> all 16 pins allowed, same logic as PORTA */
			if (Copy_uddtType == PUSH_PULL)
			{
				CLR_BIT(GPIOC->GPIOx_OTYPER, Copy_uddtPin);
			}
			else if (Copy_uddtType == OPEN_DRAIN)
			{
				SET_BIT(GPIOC->GPIOx_OTYPER, Copy_uddtPin);
			}
			WRITE_FIELD_SET_BIT(GPIOC->GPIOx_OSPEEDR, 3, (2 * Copy_uddtPin), Copy_uddtSpeed);
			break;
		default:
			break;   // Invalid port -> do nothing
	}
}

/*
 * This function was NOT affected by the nested-function bug - only one
 * definition existed here, so it always worked correctly. Left unchanged.
 */
void MGPIO_voidSetPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin,PIN_VALUE Copy_uddtPinValue)
{
	switch(Copy_uddtPort)
		{
			case PORTA:
				if (Copy_uddtPinValue == PIN_LOW)
				{
					CLR_BIT(GPIOA->GPIOx_ODR , Copy_uddtPin);
				}
				else if (Copy_uddtPinValue == PIN_HIGH)
				{
					SET_BIT(GPIOA->GPIOx_ODR , Copy_uddtPin);
				}
				break;
			case PORTB:
				if (Copy_uddtPinValue == PIN_LOW)
				{
					CLR_BIT(GPIOB->GPIOx_ODR , Copy_uddtPin);
				}
				else if (Copy_uddtPinValue == PIN_HIGH)
				{
					SET_BIT(GPIOB->GPIOx_ODR , Copy_uddtPin);
				}
				break;
			case PORTC:
				if (Copy_uddtPinValue == PIN_LOW)
				{
					CLR_BIT(GPIOC->GPIOx_ODR , Copy_uddtPin);
				}
				else if (Copy_uddtPinValue == PIN_HIGH)
				{
					SET_BIT(GPIOC->GPIOx_ODR , Copy_uddtPin);
				}
				break;
		}
}

/*
 * This function was also NOT affected by the nested-function bug - only
 * one definition existed here. It correctly writes BSRR atomically.
 * Left unchanged.
 */
void MGPIO_voidSetAtomicPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin, PIN_VALUE Copy_uddtPinValue)
{
    switch(Copy_uddtPort)
    {
        case PORTA:
            if (Copy_uddtPin <= PIN12)
            {
                if (Copy_uddtPinValue == PIN_HIGH)
                {
                    GPIOA->GPIOx_BSRR = (1UL << Copy_uddtPin);
                }
                else
                {
                    GPIOA->GPIOx_BSRR = (1UL << (Copy_uddtPin + 16U));
                }
            }
            break;

        case PORTB:
            if (Copy_uddtPinValue == PIN_HIGH)
            {
                GPIOB->GPIOx_BSRR = (1UL << Copy_uddtPin);
            }
            else
            {
                GPIOB->GPIOx_BSRR = (1UL << (Copy_uddtPin + 16U));
            }
            break;

        case PORTC:
            if (Copy_uddtPinValue == PIN_HIGH)
            {
                GPIOC->GPIOx_BSRR = (1UL << Copy_uddtPin);
            }
            else
            {
                GPIOC->GPIOx_BSRR = (1UL << (Copy_uddtPin + 16U));
            }
            break;

        default:
            break;
    }
}
//Input Functions
/*
 * Not affected by the nested-function bug either - only one definition
 * existed here. Left unchanged.
 */
void MGPIO_voidSetInputConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin, INPUT_MODE Copy_uddtInputMode)
{
	switch(Copy_uddtPort)
	{
		case PORTA:
			if (Copy_uddtPin <= PIN12)   // Protect SWD pins PA13(SWDIO)/PA14(SWCLK)/PA15(JTDI)
			{
				/* PUPDR: 2 bits per pin -> 3 possible options (Float=0/Pull-Up=1/Pull-Down=2)
				 * MASK = 3 (binary 11) because (1<<2)-1 = 3 -> covers 2 bits
				 * POSITION = pin_number * 2 -> each pin occupies 2 bits in the register */
				WRITE_FIELD_SET_BIT(GPIOA->GPIOx_PUPDR, 3, (Copy_uddtPin * 2), Copy_uddtInputMode);
			}
			break;
		case PORTB:
			// No reserved/debug pins in PORTB -> all 16 pins allowed, same logic as PORTA
			WRITE_FIELD_SET_BIT(GPIOB->GPIOx_PUPDR, 3, (Copy_uddtPin * 2), Copy_uddtInputMode);
			break;
		case PORTC:
			// No reserved/debug pins in PORTC -> all 16 pins allowed, same logic as PORTA
			WRITE_FIELD_SET_BIT(GPIOC->GPIOx_PUPDR, 3, (Copy_uddtPin * 2), Copy_uddtInputMode);
			break;
		default:
			break;   // Invalid port, do nothing
	}
}

/*
 * Not affected by the nested-function bug either - only one definition
 * existed here. Left unchanged.
 */
u8 MGPIO_u8GetPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin)
{
	u8 Local_u8PinValue = 0;   // Default/safe value returned if port is invalid
	switch(Copy_uddtPort)
	{
		case PORTA:
			/* IDR: 1 bit per pin, read-only register that reflects the actual
			 * electrical state of the pin (HIGH/LOW). No write happens here,
			 * so no need for SWD pin protection (reading PA13 is harmless). */
			Local_u8PinValue = GET_BIT(GPIOA->GPIOx_IDR, Copy_uddtPin);
			break;
		case PORTB:
			Local_u8PinValue = GET_BIT(GPIOB->GPIOx_IDR, Copy_uddtPin);
			break;
		case PORTC:
			Local_u8PinValue = GET_BIT(GPIOC->GPIOx_IDR, Copy_uddtPin);
			break;
		default:
			break;   // Invalid port, Local_u8PinValue stays 0
	}
	return Local_u8PinValue;
}

// Alternate Function
/*
 * Not affected by the nested-function bug either - only one definition
 * existed here. This one was always working correctly (which is also
 * why removing the AF calls earlier didn't fix the LED issue - AF was
 * never the actual problem).
 */
void MGPIO_voidSetAlternateFunctionConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin, ALT_FUNC Copy_uddtAltFunc)
{
	switch(Copy_uddtPort)
	{
		case PORTA:
			if (Copy_uddtPin <= PIN12)   // Protect SWD pins PA13/14/15
			{
				if (Copy_uddtPin < 8)
				{
					/* AFRL: controls pins 0-7, 4 bits per pin
					 * -> 16 possible AF values (AF0 to AF15)
					 * MASK = 15 (binary 1111) because (1<<4)-1 = 15 -> covers 4 bits
					 * POSITION = pin_number * 4 */
					WRITE_FIELD_SET_BIT(GPIOA->GPIOx_AFRL, 15, (Copy_uddtPin * 4), Copy_uddtAltFunc);
				}
				else   // pins 8 to 12 here (13,14,15 already blocked above)
				{
					/* AFRH: controls pins 8-15, same 4-bit-per-pin idea
					 * but position must be re-based by subtracting 8,
					 * because bit 0 of AFRH corresponds to PIN8, not PIN0 */
					WRITE_FIELD_SET_BIT(GPIOA->GPIOx_AFRH, 15, ((Copy_uddtPin - 8) * 4), Copy_uddtAltFunc);
				}
			}
			break;
		case PORTB:
			// No reserved/debug pins in PORTB -> all 16 pins allowed
			if (Copy_uddtPin < 8)
			{
				WRITE_FIELD_SET_BIT(GPIOB->GPIOx_AFRL, 15, (Copy_uddtPin * 4), Copy_uddtAltFunc);
			}
			else if (Copy_uddtPin <= PIN15)
			{
				WRITE_FIELD_SET_BIT(GPIOB->GPIOx_AFRH, 15, ((Copy_uddtPin - 8) * 4), Copy_uddtAltFunc);
			}
			break;
		case PORTC:
			// No reserved/debug pins in PORTC -> all 16 pins allowed
			if (Copy_uddtPin < 8)
			{
				WRITE_FIELD_SET_BIT(GPIOC->GPIOx_AFRL, 15, (Copy_uddtPin * 4), Copy_uddtAltFunc);
			}
			else if (Copy_uddtPin <= PIN15)
			{
				WRITE_FIELD_SET_BIT(GPIOC->GPIOx_AFRH, 15, ((Copy_uddtPin - 8) * 4), Copy_uddtAltFunc);
			}
			break;
		default:
			break;   // Invalid port, do nothing
	}
}
