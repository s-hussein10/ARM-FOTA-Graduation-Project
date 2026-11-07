/******************************************************************************
 *  File        : RCC_interface.h
 *  Layer       : MCAL
 *  Module      : RCC (Reset and Clock Control)
 *  Description : Public API to be included by the application / other
 *                 drivers that need to enable a clock source or read
 *                 the current system clock status.
 ******************************************************************************/
#ifndef MC_RCC_INTERFACE_H_
#define MC_RCC_INTERFACE_H_

void RCC_voidInit(void);
void RCC_voidEnblePeripheral(u8 copy_u8Bus, u8 copy_u8peripheral);
void RCC_voidDisablePeripheral(u8 copy_u8Bus, u8 copy_u8peripheral);

typedef enum
{
	AHB1 = 0,
	AHB2 = 1,
	APB1 = 2,
	APB2 = 3
}Peripheral_Bus;

// AHB1 peripheral
typedef enum
{
	AHB1_GPIOA    = 0,
	AHB1_GPIOB    = 1,
	AHB1_GPIOC    = 2,
	AHB1_GPIOD    = 3,
	AHB1_GPIOE    = 4,
	AHB1_GPIOHRST = 7,
	AHB1_CRCRST   = 12,
	AHB1_DMA1RST  = 21,
	AHB1_DMA2RST  = 22
}AHB1_Peripheral;

// AHB2 peripheral
typedef enum
{
	AHB2_OTGFSRST = 7,
}AHB2_Peripheral;

// APB1 peripheral
typedef enum
{
	APB1_TIM2RST    = 0,
	APB1_TIM3RST    = 1,
	APB1_TIM4RST    = 2,
	APB1_TIM5RST    = 3,
	APB1_WWDGRST    = 11,
	APB1_SPI2RST    = 14,
	APB1_SPI3RST    = 15,
	APB1_USART2RST  = 17,
	APB1_I2C1RST    = 21,
	APB1_I2C2RST    = 22,
	APB1_I2C3RST    = 23,
	APB1_PWRRST     = 28
}APB1_Peripheral;

// APB2 peripheral
typedef enum
{
	APB2_TIM1RST    = 0,
	APB2_USART1RST  = 4,
	APB2_USART6RST  = 5,
	APB2_ADC1RST    = 8,
	APB2_SDIORST    = 11,
	APB2_SPI1RST    = 12,
	APB2_SPI4RST    = 13,
	APB2_SYSCFGRST  = 14,
	APB2_TIM9RST    = 16,
	APB2_TIM10RST   = 17,
	APB2_TIM11RST   = 18
}APB2_Peripheral;

#endif /* MC_RCC_INTERFACE_H_ */
