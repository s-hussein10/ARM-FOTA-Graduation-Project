/******************************************************************************
 *  File        : RCC_private.h
 *  Layer       : MCAL
 *  Module      : RCC (Reset and Clock Control)
 *  Description : Register map, base address, and bit-field definitions.
 *                 This file is INTERNAL to the RCC driver only.
 *                 The application MUST NOT include this file directly.
 ******************************************************************************/

#ifndef MC_RCC_PRIVATE_H_
#define MC_RCC_PRIVATE_H_

#define BASE_ADDRESS 		(0x40023800)

typedef struct
{
				 // the offset
	u32 RCC_CR ; //0x00
	u32 RCC_PLLCFGR ; //0x04
	u32 RCC_CFGR ; //0x08
	u32 RCC_CIR ; //0x0c
	u32 RCC_AHB1RSTR ; //0x10
	u32 RCC_AHB2RSTR ; //0x14
	u32 RESERVED1 ; //0x18
	u32 RESERVED2 ; //0x1C
	u32 RCC_APB1RSTR ; //0x20
	u32 RCC_APB2RSTR ; //0x24
	u32 RESERVED3 ; //0x28
	u32 RESERVED4 ; //0x2C
	u32 RCC_AHB1ENR ; //0x30
	u32 RCC_AHB2ENR ; //0x34
	u32 RESERVED5 ; //0x38
	u32 RESERVED6 ; //0x3C
	u32 RCC_APB1ENR ; //0x40
	u32 RCC_APB2ENR ; //0x44
	u32 RESERVED7 ; //0x48
	u32 RESERVED8 ; //0x4C
	u32 RCC_AHB1LPENR ; //0x50
	u32 RCC_AHB2LPENR ; //0x54
	u32 RESERVED9 ; //0x58
	u32 RESERVED10 ; //0x5C
	u32 RCC_APB1LPENR ; //0x60
	u32 RCC_APB2LPENR ; //0x64
	u32 RESERVED11 ; //0x68
	u32 RESERVED12 ; //0x6C
	u32 RCC_BDCR ; //0x70
	u32 RCC_CSR ; //0x74
	u32 RESERVED13 ; //0x78
	u32 RESERVED14 ; //0x7C
	u32 RCC_SSCGR ; //0x80
	u32 RCC_PLLI2SCFGR ; //0x84
	u32 RESERVED15 ; //0x88
	u32 RCC_DCKCFGR ; //0x8C

}RCC_type;

#define RCC ((volatile RCC_type*)(BASE_ADDRESS))

/* RCC_CR bits */
#define RCC_CR_HSION        0
#define RCC_CR_HSIRDY        1
#define RCC_CR_HSEON        16
#define RCC_CR_HSERDY        17
#define RCC_CR_HSEBYP        18
#define RCC_CR_CSSON        19
#define RCC_CR_PLLON        24
#define RCC_CR_PLLRDY        25

/* RCC_CFGR bits */
#define RCC_CFGR_SW0        0
#define RCC_CFGR_SW1        1
#define RCC_CFGR_SWS0        2
#define RCC_CFGR_SWS1        3
#define RCC_CFGR_HPRE0        4
#define RCC_CFGR_PPRE1_0    10
#define RCC_CFGR_PPRE2_0    13

/* RCC_AHB1ENR bits (GPIO clocks) */
#define RCC_AHB1ENR_GPIOAEN    0
#define RCC_AHB1ENR_GPIOBEN    1
#define RCC_AHB1ENR_GPIOCEN    2
#define RCC_AHB1ENR_GPIODEN    3
#define RCC_AHB1ENR_GPIOEEN    4
#define RCC_AHB1ENR_GPIOHEN    7
#define RCC_AHB1ENR_CRCEN    12
#define RCC_AHB1ENR_DMA1EN    21
#define RCC_AHB1ENR_DMA2EN    22

/* RCC_APB1ENR bits */
#define RCC_APB1ENR_TIM2EN    0
#define RCC_APB1ENR_TIM3EN    1
#define RCC_APB1ENR_TIM4EN    2
#define RCC_APB1ENR_TIM5EN    3
#define RCC_APB1ENR_WWDGEN    11
#define RCC_APB1ENR_SPI2EN    14
#define RCC_APB1ENR_SPI3EN    15
#define RCC_APB1ENR_USART2EN    17
#define RCC_APB1ENR_I2C1EN    21
#define RCC_APB1ENR_I2C2EN    22
#define RCC_APB1ENR_I2C3EN    23
#define RCC_APB1ENR_PWREN    28

/* RCC_APB2ENR bits */
#define RCC_APB2ENR_TIM1EN    0
#define RCC_APB2ENR_USART1EN    4
#define RCC_APB2ENR_USART6EN    5
#define RCC_APB2ENR_ADC1EN    8
#define RCC_APB2ENR_SDIOEN    11
#define RCC_APB2ENR_SPI1EN    12
#define RCC_APB2ENR_SPI4EN    13
#define RCC_APB2ENR_SYSCFGEN    14
#define RCC_APB2ENR_TIM9EN    16
#define RCC_APB2ENR_TIM10EN    17
#define RCC_APB2ENR_TIM11EN    18

/* RCC_PLLCFGR bits */
#define RCC_PLLCFGR_PLLM0      0
#define RCC_PLLCFGR_PLLN0      6
#define RCC_PLLCFGR_PLLP0      16
#define RCC_PLLCFGR_PLLSRC     22
#define RCC_PLLCFGR_PLLQ0      24


// define system clock
#define HSI		 	0
#define HSE		 	1
#define PLL_HSI	 	2
#define PLL_HSE	 	3

#define MECHANICAL	0
#define ELECTRICAL 	1




#endif /* MC_RCC_PRIVATE_H_ */
