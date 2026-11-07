/*
 * HA_LedMatrix_config.h
 *
 *  Created on: 8 Aug 2026
 *      Author: Sief
 */

#ifndef HA_LEDMATRIX_CONFIG_H_
#define HA_LEDMATRIX_CONFIG_H_


#include "MC_GPIO_interface.h"
#include  "STD_TYPES.h"

#define ROWS_PORT  		 PORTA
#define COLUMN_PORT   	 PORTB

static const PINS Rows_Pins[8]    = {PIN0, PIN1, PIN2, PIN3, PIN4, PIN5, PIN6, PIN7};
static const PINS Columns_Pins[8] = {PIN0, PIN1, PIN2, PIN3, PIN4, PIN5, PIN6, PIN7};


#endif /* HA_LEDMATRIX_CONFIG_H_ */
