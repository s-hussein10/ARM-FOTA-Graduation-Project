/*
 * MC_GPIO_private.h
 *
 *  Created on: 28 Jul 2026
 *      Author: Sief
 */

#ifndef MC_GPIO_PRIVATE_H_
#define MC_GPIO_PRIVATE_H_

// THE Base Address
#define GPIOA_BASE_ADDRESS				(0x40020000)
#define GPIOB_BASE_ADDRESS				(0x40020400)
#define GPIOC_BASE_ADDRESS				(0x40020800)

typedef struct
{
							// the offset
	u32 GPIOx_MODER;		//0X00
	u32 GPIOx_OTYPER;		//0X04
	u32 GPIOx_OSPEEDR;		//0X08
	u32 GPIOx_PUPDR;		//0X0C
	u32 GPIOx_IDR;			//0X10
	u32 GPIOx_ODR;			//0X14
	u32 GPIOx_BSRR;			//0X18
	u32 GPIOx_LCKR;			//0X1C
	u32 GPIOx_AFRL;			//0X20
	u32 GPIOx_AFRH;			//0X24

}GPIO_type;

#define GPIOA			((volatile GPIO_type *)GPIOA_BASE_ADDRESS)
#define GPIOB			((volatile GPIO_type *)GPIOB_BASE_ADDRESS)
#define GPIOC			((volatile GPIO_type *)GPIOC_BASE_ADDRESS)


#endif /* MC_GPIO_PRIVATE_H_ */
