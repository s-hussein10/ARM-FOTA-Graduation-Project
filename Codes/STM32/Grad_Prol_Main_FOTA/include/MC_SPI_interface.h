/*
 * MC_SPI_interface.h
 *
 *  Created on: 4 Sep 2026
 *      Author: Sief
 */

#ifndef MC_SPI_INTERFACE_H_
#define MC_SPI_INTERFACE_H_

#include "STD_TYPES.h"

typedef enum
{
    SPI_1 = 0U,
    SPI_2,
    SPI_3
}SPI_Peripheral_t;

typedef enum
{
    SPI_MODE_SLAVE  = 0U,
    SPI_MODE_MASTER = 1U
}SPI_DeviceMode_t;

typedef enum
{
    SPI_CPOL_LOW  = 0U,
    SPI_CPOL_HIGH = 1U
}SPI_ClockPolarity_t;

typedef enum
{
    SPI_CPHA_FIRST_EDGE  = 0U,
    SPI_CPHA_SECOND_EDGE = 1U
}SPI_ClockPhase_t;

typedef enum
{
    SPI_DFF_8BIT  = 0U,
    SPI_DFF_16BIT = 1U
}SPI_DataFrame_t;

typedef enum
{
    SPI_FIRSTBIT_MSB = 0U,
    SPI_FIRSTBIT_LSB = 1U
}SPI_FirstBit_t;

typedef enum
{
    SPI_BAUD_PRESCALER_2   = 0U,
    SPI_BAUD_PRESCALER_4   = 1U,
    SPI_BAUD_PRESCALER_8   = 2U,
    SPI_BAUD_PRESCALER_16  = 3U,
    SPI_BAUD_PRESCALER_32  = 4U,
    SPI_BAUD_PRESCALER_64  = 5U,
    SPI_BAUD_PRESCALER_128 = 6U,
    SPI_BAUD_PRESCALER_256 = 7U
}SPI_BaudRatePrescaler_t;

typedef struct
{
    SPI_Peripheral_t         SPI_Peripheral;
    SPI_DeviceMode_t         DeviceMode;
    SPI_ClockPolarity_t      ClockPolarity;
    SPI_ClockPhase_t         ClockPhase;
    SPI_DataFrame_t          DataFrame;
    SPI_FirstBit_t           FirstBit;
    SPI_BaudRatePrescaler_t  BaudRatePrescaler;
}SPI_Config_t;

void MC_SPI_voidInit(const SPI_Config_t* Copy_psConfig);
u8   MC_SPI_u8TransceiveByte(SPI_Peripheral_t Copy_enSPI, u8 Copy_u8DataToSend);
void MC_SPI_voidTransmitBuffer(SPI_Peripheral_t Copy_enSPI, const u8* Copy_pu8TxBuffer, u16 Copy_u16Length);
void MC_SPI_voidReceiveBuffer(SPI_Peripheral_t Copy_enSPI, u8* Copy_pu8RxBuffer, u16 Copy_u16Length);

#endif /* MC_SPI_INTERFACE_H_ */
