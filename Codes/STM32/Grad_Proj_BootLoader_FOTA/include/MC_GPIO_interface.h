/*
 * MC_GPIO_interface.h
 *
 *  Created on: 28 Jul 2026
 *      Author: Seif
 */
#ifndef MC_GPIO_INTERFACE_H_
#define MC_GPIO_INTERFACE_H_

#include "STD_TYPES.h"

/* GPIO Ports available on this MCU */
typedef enum
{
	PORTA = 0,
	PORTB,
	PORTC
}PORTS;

/* GPIO Pin numbers (0 to 15) for each port */
typedef enum
{
	PIN0 = 0,
	PIN1,
	PIN2,
	PIN3,
	PIN4,
	PIN5,
	PIN6,
	PIN7,
	PIN8,
	PIN9,
	PIN10,
	PIN11,
	PIN12,
	PIN13,
	PIN14,
	PIN15
}PINS;

/* Pin operating mode: digital in/out, AF, or analog */
typedef enum
{
	INPUT = 0,
	OUTPUT,
	ALTERNATE_FUNC,
	ANALOG
}MODES;

/* Output driver type (only relevant when pin is OUTPUT or AF) */
typedef enum
{
	PUSH_PULL = 0,
	OPEN_DRAIN
}TYPES;

/* Output speed / slew rate setting */
typedef enum
{
	LOW_SPEED = 0,
	MED_SPEED,
	HIGH_SPEED,
	VERY_HIGH_SPEED
}SPEED;

/* Logic level of a pin: low or high */
typedef enum
{
	PIN_LOW = 0,
	PIN_HIGH
}PIN_VALUE;

/* Internal pull resistor configuration (used mainly for INPUT mode) */
typedef enum
{
	FLOAT = 0,
	PULL_UP,
	PULL_DOWN
}INPUT_MODE;

/* Alternate function number (AF0 - AF15), mapped via AFRL/AFRH registers */
typedef enum
{
	AF0 = 0,
	AF1,
	AF2,
	AF3,
	AF4,
	AF5,
	AF6,
	AF7,
	AF8,
	AF9,
	AF10,
	AF11,
	AF12,
	AF13,
	AF14,
	AF15
}ALT_FUNC;

/*----------------------------------------------------------
 * Sets the mode of a specific pin (INPUT / OUTPUT / AF / ANALOG)
 *----------------------------------------------------------*/
void MGPIO_voidSetPinMode(PORTS Copy_uddtPort, PINS Copy_uddtPin, MODES Copy_uddtMode);

/*----------------------------------------------------------
 * Output Functions
 *----------------------------------------------------------*/

/* Configures output type (push-pull/open-drain) and speed for a pin
 * NOTE: Should be called AFTER setting the pin mode to OUTPUT (or AF) */
void MGPIO_voidSetOutputConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin, TYPES Copy_uddtType, SPEED Copy_uddtSpeed);

/* Sets pin value using ODR register (read-modify-write, NOT interrupt-safe) */
void MGPIO_voidSetPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin, PIN_VALUE Copy_uddtPinValue);

/* Sets pin value using BSRR register (single atomic write, interrupt/ISR-safe) */
void MGPIO_voidSetAtomicPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin, PIN_VALUE Copy_uddtPinValue);

/*----------------------------------------------------------
 * Input Functions
 *----------------------------------------------------------*/

/* Configures pull resistor state for a pin set as INPUT */
void MGPIO_voidSetInputConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin, INPUT_MODE Copy_uddtInputMode);

/* Reads and returns the current logic level of a pin (0 = LOW, 1 = HIGH) */
u8 MGPIO_u8GetPinValue(PORTS Copy_uddtPort, PINS Copy_uddtPin);

/*----------------------------------------------------------
 * Alternate Function
 *----------------------------------------------------------*/

/* Selects which alternate function (AF0-AF15) is mapped to a pin
 * NOTE: Pin mode must be set to ALTERNATE_FUNC first */
void MGPIO_voidSetAlternateFunctionConfig(PORTS Copy_uddtPort, PINS Copy_uddtPin, ALT_FUNC Copy_uddtAltFunc);

#endif /* MC_GPIO_INTERFACE_H_ */
