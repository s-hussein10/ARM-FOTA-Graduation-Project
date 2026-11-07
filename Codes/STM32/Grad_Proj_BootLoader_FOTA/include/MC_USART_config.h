/*
 * MC_USART_config.h
 *
 *  Created on: 31 Aug 2026
 *      Author: Sief
 */

#ifndef MC_USART_CONFIG_H_
#define MC_USART_CONFIG_H_


/* APB2 peripheral clock (USART1, USART6)  MUST match RCC SYSTEM_CLOCK setting.
 * Current assumption: SYSTEM_CLOCK = HSI (16MHz), no PLL, no prescaler on APB2.
 * If you change SYSTEM_CLOCK later, update this value accordingly. */
#define USART_APB2_CLOCK_HZ    16000000UL



#endif /* MC_USART_CONFIG_H_ */
