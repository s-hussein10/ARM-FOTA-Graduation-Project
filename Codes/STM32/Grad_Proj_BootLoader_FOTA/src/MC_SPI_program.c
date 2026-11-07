#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "MC_SPI_interface.h"
#include "MC_SPI_private.h"
#include "MC_SPI_config.h"

/*
 * MC_SPI_voidInit
 * -----------------
 * Configures one SPI peripheral (SPI1/SPI2/SPI3) according to the fields
 * in Copy_psConfig. The peripheral is disabled at the start and re-enabled
 * only at the very end, once every field has been written - this avoids
 * the peripheral operating with a half-finished configuration.
 */
void MC_SPI_voidInit(const SPI_Config_t* Copy_psConfig)
{
	volatile SPI_type* Local_psSPI = NULL;

	/* Resolve which SPI instance (register base) we are configuring.
	 * If an invalid peripheral is passed, return immediately to avoid
	 * writing through a NULL/garbage pointer. */
	switch (Copy_psConfig->SPI_Peripheral)
	{
	case SPI_1 : Local_psSPI = SPI1 ; break;
	case SPI_2 : Local_psSPI = SPI2 ; break;
	case SPI_3 : Local_psSPI = SPI3 ; break;
	default: return ;
	}

	/* Clock must be enabled by caller before this call (driver does not
	 * manage RCC - same convention as every other MCAL driver here). */

	/* Disable the peripheral first, so none of the configuration bits
	 * below are applied while SPI is actively running. */
	CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SPE);

	/* Master or Slave (MSTR bit). Master generates its own SCK; Slave
	 * receives SCK from an external Master. */
	switch (Copy_psConfig->DeviceMode)
	{
	case SPI_MODE_MASTER : SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_MSTR);
	break;
	case SPI_MODE_SLAVE  : CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_MSTR);
	break;
	default: return ;
	}

	/* Clock polarity (CPOL): idle level of SCK when no data is being
	 * transferred. LOW = idle at 0, HIGH = idle at 1. */
	switch (Copy_psConfig->ClockPolarity)
	{
	case SPI_CPOL_HIGH : SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_CPOL);
	break;
	case SPI_CPOL_LOW  : CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_CPOL);
	break;
	default: return ;
	}

	/* Clock phase (CPHA): which SCK edge data is sampled on.
	 * FIRST_EDGE = sample on the 1st clock transition,
	 * SECOND_EDGE = sample on the 2nd clock transition.
	 * CPOL+CPHA together define SPI Mode 0/1/2/3. */
	switch (Copy_psConfig->ClockPhase)
	{
	case SPI_CPHA_FIRST_EDGE  : CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_CPHA);
	break;
	case SPI_CPHA_SECOND_EDGE : SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_CPHA);
	break;
	default: return ;
	}

	/* Baud rate prescaler (BR[2:0]): divides the peripheral clock (APBx)
	 * down to the actual SCK frequency. Only meaningful in Master mode -
	 * in Slave mode SCK comes from the external Master, so these bits
	 * are simply ignored by the hardware. Each case sets/clears the
	 * 3 BR bits directly since this is a small fixed set of encodings
	 * (no need for a mask+shift, unlike bigger fields like STOP in USART). */
	switch (Copy_psConfig->BaudRatePrescaler)
	{
	case SPI_BAUD_PRESCALER_2 :
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_4 :
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_8 :
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_16 :
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_32 :
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_64 :
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_128 :
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	case SPI_BAUD_PRESCALER_256 :
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR0);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR1);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_BR2);
		break;
	default:
		return ;
	}

	/* Data frame format (DFF): 8-bit or 16-bit words per DR access. */
	switch (Copy_psConfig->DataFrame)
	{
	case SPI_DFF_8BIT  : CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_DFF);
	break;
	case SPI_DFF_16BIT : SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_DFF);
	break;
	default: return ;
	}

	/* Bit order on the wire: MSB-first (most common) or LSB-first. */
	switch (Copy_psConfig->FirstBit)
	{
	case SPI_FIRSTBIT_MSB : CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_LSBFIRST);
	break;
	case SPI_FIRSTBIT_LSB : SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_LSBFIRST);
	break;
	default: return ;
	}

	/* NSS (Slave Select) management - deliberately asymmetric by design:
	 *
	 * - Master -> Software NSS (SSM=1, SSI=1):
	 *     SSM=1 lets firmware fully control the internal NSS signal via SSI,
	 *     instead of relying on a physical NSS pin. SSI=1 forces the internal
	 *     NSS high, which tells the Master hardware "I am the only master on
	 *     the bus" and prevents a spurious Mode Fault (MODF) error. No GPIO
	 *     pin handling is required for NSS on the Master side.
	 *
	 * - Slave -> Hardware NSS (SSM=0):
	 *     SSM=0 means NSS is driven by the actual physical pin (PA4, AF5),
	 *     controlled externally by the Master. The Slave peripheral is only
	 *     "selected" and active while that physical pin is held low by the
	 *     Master. SSI is a don't-care in this mode (only relevant when
	 *     SSM=1), cleared here purely for register cleanliness.
	 *     NOTE: configuring PA4 as ALTERNATE_FUNC/AF5 is the caller's
	 *     (main.c's) responsibility, not this driver's - same convention
	 *     as RCC clock enabling above. */
	switch (Copy_psConfig->DeviceMode)
	{
	case SPI_MODE_MASTER :
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SSM);
		SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SSI);
		break;
	case SPI_MODE_SLAVE  :
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SSM);
		CLR_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SSI);
		break;
	default: return ;
	}

	/* Full-duplex 2-line mode is the hardware default (BIDIMODE=0),
	 * so no extra register write is needed here. */

	/* Enable the peripheral last, only after every other field above
	 * has already been written - avoids operating with a half-set config. */
	SET_BIT(Local_psSPI->SPI_CR1, SPI_CR1_SPE);
}

/*
 * MC_SPI_u8TransceiveByte
 * --------------------------
 * SPI is inherently full-duplex: every byte written out also shifts a
 * byte in at the same time. So "send" and "receive" are really the same
 * hardware operation - this single blocking function handles both.
 */
u8 MC_SPI_u8TransceiveByte(SPI_Peripheral_t Copy_enSPI, u8 Copy_u8DataToSend)
{
	volatile SPI_type* Local_psSPI = NULL;
	u8 Local_u8ReceivedData = 0;

	/* Resolve instance pointer, same pattern as Init. */
	switch (Copy_enSPI)
	{
	case SPI_1 : Local_psSPI = SPI1 ; break;
	case SPI_2 : Local_psSPI = SPI2 ; break;
	case SPI_3 : Local_psSPI = SPI3 ; break;
	default: return 0 ;
	}

	/* Wait until the Transmit buffer is empty (TXE=1), meaning DR is
	 * free and ready to accept the next byte to send. */
	while (GET_BIT(Local_psSPI->SPI_SR, SPI_SR_TXE) == 0)
	{
		/* busy-wait - timeout protection deferred */
	}

	/* Writing to DR loads the byte into the shift register and starts
	 * the actual SCK-driven transfer. */
	Local_psSPI->SPI_DR = Copy_u8DataToSend;

	/* Wait until a full byte has been shifted in (RXNE=1), meaning the
	 * received byte is ready to be read from DR. */
	while (GET_BIT(Local_psSPI->SPI_SR, SPI_SR_RXNE) == 0)
	{
		/* busy-wait - timeout protection deferred */
	}

	/* Reading DR returns the byte that was clocked in during this
	 * same transfer, and also clears RXNE. */
	Local_u8ReceivedData = Local_psSPI->SPI_DR;

	/* Wait until the peripheral is no longer busy (BSY=0) before
	 * returning, so the caller can safely assume the bus is idle
	 * (important before disabling SPI or toggling NSS manually). */
	while (GET_BIT(Local_psSPI->SPI_SR, SPI_SR_BSY) == 1)
	{
		/* busy-wait - timeout protection deferred */
	}

	return Local_u8ReceivedData;
}

/*
 * MC_SPI_voidTransmitBuffer
 * ----------------------------
 * Sends a buffer of bytes out over SPI. Each byte also produces a
 * received byte (full-duplex), but here we simply discard it since
 * the caller only cares about transmission.
 */
void MC_SPI_voidTransmitBuffer(SPI_Peripheral_t Copy_enSPI, const u8* Copy_pu8TxBuffer, u16 Copy_u16Length)
{
	u16 Local_u16Counter;

	for (Local_u16Counter = 0; Local_u16Counter < Copy_u16Length; Local_u16Counter++)
	{
		MC_SPI_u8TransceiveByte(Copy_enSPI, Copy_pu8TxBuffer[Local_u16Counter]);
	}
}

/*
 * MC_SPI_voidReceiveBuffer
 * ---------------------------
 * Receives a buffer of bytes over SPI. Since SPI can only shift data in
 * while it's also shifting data out, we must transmit *something* for
 * every byte we want to receive - 0xFF is sent as a neutral/dummy filler
 * byte that the receiving side is expected to ignore.
 */
void MC_SPI_voidReceiveBuffer(SPI_Peripheral_t Copy_enSPI, u8* Copy_pu8RxBuffer, u16 Copy_u16Length)
{
	u16 Local_u16Counter;

	for (Local_u16Counter = 0; Local_u16Counter < Copy_u16Length; Local_u16Counter++)
	{
		Copy_pu8RxBuffer[Local_u16Counter] = MC_SPI_u8TransceiveByte(Copy_enSPI, 0xFF);
	}
}