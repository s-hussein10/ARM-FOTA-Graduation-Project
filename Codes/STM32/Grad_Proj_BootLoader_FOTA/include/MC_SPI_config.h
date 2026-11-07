/*
 * MC_SPI_config.h
 *
 *  Created on: 4 Sep 2026
 *      Author: Sief
 */

#ifndef MC_SPI_CONFIG_H_
#define MC_SPI_CONFIG_H_

#include "MC_SPI_interface.h"

/* SPI1 - Master, Mode 0 (CPOL=0, CPHA=0), 8-bit, MSB first, prescaler /8 (placeholder) */
static const SPI_Config_t SPI1_MasterConfig =
{
    .SPI_Peripheral     = SPI_1,
    .DeviceMode         = SPI_MODE_MASTER,
    .ClockPolarity      = SPI_CPOL_LOW,
    .ClockPhase         = SPI_CPHA_FIRST_EDGE,
    .DataFrame          = SPI_DFF_8BIT,
    .FirstBit           = SPI_FIRSTBIT_MSB,
    .BaudRatePrescaler  = SPI_BAUD_PRESCALER_8
};

/* SPI1 - Slave, Mode 0 (CPOL=0, CPHA=0), 8-bit, MSB first */
/* Note: BaudRatePrescaler is ignored in Slave mode - clock comes from external Master */
static const SPI_Config_t SPI1_SlaveConfig =
{
    .SPI_Peripheral     = SPI_1,
    .DeviceMode         = SPI_MODE_SLAVE,
    .ClockPolarity      = SPI_CPOL_LOW,
    .ClockPhase         = SPI_CPHA_FIRST_EDGE,
    .DataFrame          = SPI_DFF_8BIT,
    .FirstBit           = SPI_FIRSTBIT_MSB,
    .BaudRatePrescaler  = SPI_BAUD_PRESCALER_8
};

#endif /* MC_SPI_CONFIG_H_ */
