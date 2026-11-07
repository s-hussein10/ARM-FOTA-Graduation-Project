/*
 * HA_SSD_interface.h
 *
 *  Created on: 9 Aug 2026
 *      Author: Sief
 */

#ifndef HA_SSD_INTERFACE_H_
#define HA_SSD_INTERFACE_H_

#include "STD_TYPES.h"
#include "MC_GPIO_interface.h"

#define SSD_NUM_SEGMENTS    8   /* a,b,c,d,e,f,g,dp */

typedef struct
{
	PORTS Port;
	PINS  SegmentPins[SSD_NUM_SEGMENTS];   /*  a,b,c,d,e,f,g,dp */
	u8    State;   /* SSD_COMMON_ANODE / SSD_COMMON_CATHODE */
}SSD_t;

#define SSD_COMMON_ANODE     1
#define SSD_COMMON_CATHODE   0

#define ZERO	0b00111111
#define ONE		0b00000110
#define TWO  	0b01011011
#define THERE 	0b01001111
#define FOUR	0b01100110
#define FIVE	0b01101101
#define	SIX		0b01111101
#define SEVEN	0b00000111
#define EIGHT	0b01111111
#define NINE	0b01101111

void SSD_voidInit    (SSD_t* Copy_pstructSsd);
void SSD_voidEnable  (SSD_t* Copy_pstructSsd, u8 Copy_u8Number);
void SSD_voidDisable (SSD_t* Copy_pstructSsd);



#endif /* HA_SSD_INTERFACE_H_ */
