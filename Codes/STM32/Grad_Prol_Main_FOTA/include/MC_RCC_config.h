/******************************************************************************
 *  File        : RCC_config.h
 *  Layer       : MCAL
 *  Module      : RCC (Reset and Clock Control)
 *  Description : Pre-build configuration. Edit the values below according
 *                 to your board/crystal before compiling the driver.
 ******************************************************************************/

#ifndef MC_RCC_CONFIG_H_
#define MC_RCC_CONFIG_H_

/*
 * OPTIONS
 * 1- HSI
 * 2- HSE
 * 3- PLL_HSI
 * 4- PLL_HSE
 *
 * */

#define SYSTEM_CLOCK 	HSI

/*
 * OPTIONS
 * 1- MECHANICAL
 * 2- ELECTRICAL
 * */

#define HSE_DEFINE_TYPE		MECHANICAL

#endif /* MC_RCC_CONFIG_H_ */
