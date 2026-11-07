/*
 * MC_NVIC_private.h
 *
 *  Created on: 7 Aug 2026
 *      Author: Sief
 */

#ifndef MC_NVIC_PRIVATE_H_
#define MC_NVIC_PRIVATE_H_

#define NVIC_BASE_ADDRESS				(0xE000E100)

typedef struct
{
							// the offset
	u32 NVIC_ISER[8] ;    		//0X100
	u32 RESERVED1[24];			//97 / 4 =24reg

	u32 NVIC_ICER[8] ;    		//0X180
	u32 RESERVED2[24];			//97 / 4 =24reg

	u32 NVIC_ISPR[8] ;    		//0x200
	u32 RESERVED3[24];			//97 / 4 =24reg

	u32 NVIC_ICPR[8] ;    		//0X280
	u32 RESERVED4[24];			//97 / 4 =24reg

	u32 NVIC_IABR[8] ;			//0X300
	u32 RESERVED5[56];			//225 / 4 =56reg

//	u32 NVIC_IPR [60];			//0X400 		define the group and subgroup
	u8  NVIC_IPR [240];			//0X400 		define the group and subgroup
	u32 RESERVED6[644];			//2576 / 4 =644reg

	u32 NVIC_STIR    ;			//0XF00

}NVIC_type;


#define NVIC			((volatile NVIC_type *)NVIC_BASE_ADDRESS)


#define SCB_AIRCR 		*(volatile u32 * )(0xE000ED0C)


#endif /* MC_NVIC_PRIVATE_H_ */
