/*
 * MC_EXTI_private.h
 *
 *  Created on: 13 Aug 2026
 *      Author: Sief
 */

#include "STD_TYPES.h"

#ifndef MC_EXTI_PRIVATE_H_
#define MC_EXTI_PRIVATE_H_


#define EXTI_BASE_ADDRESS				(0x40013C00UL)

typedef struct
{
								// the offset
	u32 EXTI_IMR ;    			//0X00
	u32 EXTI_EMR ;    			//0X04
	u32 EXTI_RTSR ;    			//0x08
	u32 EXTI_FTSR ;    			//0X0C
	u32 EXTI_SWIER ;			//0X10
	u32  EXTI_PR ;				//0X14

}EXTI_type;


#define EXTI			((volatile EXTI_type *)EXTI_BASE_ADDRESS)


#define SYSCFG_EXTI_BASE_ADDRESS				(0x40013800UL)

typedef struct
{
								// the offset
    u32 RESERVED0;        		// 0x00 - MEMRMP
    u32 RESERVED1;        		// 0x04 - PMC
	u32 SYSCFG_EXTICR1;         // 0x08
    u32 SYSCFG_EXTICR2;         // 0x0C
    u32 SYSCFG_EXTICR3;         // 0x10
    u32 SYSCFG_EXTICR4;         // 0x14
}SYSCFG_EXTI_type;


#define SYSCFG_EXTI			((volatile SYSCFG_EXTI_type *)SYSCFG_EXTI_BASE_ADDRESS)




#endif /* MC_EXTI_PRIVATE_H_ */
