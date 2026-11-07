/*
 * MC_NVIC_interface.h
 *
 *  Created on: 7 Aug 2026
 *      Author: Sief
 */

#ifndef MC_NVIC_INTERFACE_H_
#define MC_NVIC_INTERFACE_H_


typedef enum
{
	GROUP16_SUB0 = 3 ,
	GROUP8_SUB2  = 4 ,
	GROUP4_SUB4  = 5 ,
	GROUP2_SUB8  = 6 ,
	GROUP0_SUB16 = 7
}GROUP_CONFIG;


void NVIC_voidEnable (u8 Copy_u8VectorPosition);
void NVIC_voidDisable(u8 Copy_u8VectorPosition);

void NVIC_voidPendingFlagEnable (u8 Copy_u8VectorPosition);
void NVIC_voidPendingFlagDisable(u8 Copy_u8VectorPosition);


void NVIC_voidSetGroupPriority(u8 Copy_u8VectorPosition , u8 Copy_u8Group,u8 Copy_u8SubGroup);

u8   NVIC_u8ReadActiveFlag (u8 Copy_u8VectorPosition);

void NVIC_voidSetGroupConfig(GROUP_CONFIG Copy_u8GroupConfig);


#endif /* MC_NVIC_INTERFACE_H_ */
