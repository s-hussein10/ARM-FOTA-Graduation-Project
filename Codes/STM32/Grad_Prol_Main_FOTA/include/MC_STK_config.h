/*
 * MC_STK_config.h
 *
 *  Created on: 3 Aug 2026
 *      Author: Sief
 */

#ifndef MC_STK_CONFIG_H_
#define MC_STK_CONFIG_H_

// _config.h

/* SysTick tick calculations — MUST match RCC SYSTEM_CLOCK and STK_CLOCK_BUS_SOURCE settings.
 * Current assumption: SYSTEM_CLOCK = HSI (16MHz), STK_CLOCK_BUS_SOURCE = AHB_DIV_BY_8
 * => SysTick frequency = 16MHz / 8 = 2MHz
 * If you ever change SYSTEM_CLOCK or STK_CLOCK_BUS_SOURCE, update these values.
 */
#define STK_TICKS_PER_MS    2000U
#define STK_TICKS_PER_US    2U


/*
 * the options
 * 1- AHB
 * 2- AHB_DIV_BY_8
 *
 */

#define STK_CLOCK_BUS_SOURCE		AHB_DIV_BY_8

#endif /* MC_STK_CONFIG_H_ */
