/*
 * MC_DMA_program.c
 *
 *  Created on: 4 Sep 2026
 *      Author: Sief
 */
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "MC_DMA_private.h"
#include "MC_DMA_interface.h"

/*----------------------------------------------------------------------------
 * Callback storage: one function pointer slot per Stream, per controller.
 * [0] = DMA1 (streams 0..7), [1] = DMA2 (streams 0..7).
 * Filled by MC_DMA_voidSetCallback, invoked from the relevant
 * DMAx_Streamy_IRQHandler (to be written in the interrupt vector file)
 * after checking TCIF and clearing it.
 *--------------------------------------------------------------------------*/
static void (*Global_apfDMACallback[2][8])(void) = {NULL};

/*----------------------------------------------------------------------------
 * MC_DMA_voidInit
 *--------------------------------------------------------------------------*/
void MC_DMA_voidInit(const DMA_Config_t* Copy_psConfig)
{
	DMA_RegDef_t* Local_psDMA = NULL;

	/* Step 1: resolve which controller (DMA1/DMA2) this config targets */
	switch (Copy_psConfig->INDEX)
	{
		case DMA1_INDEX:
			Local_psDMA = DMA1;
			break;

		case DMA2_INDEX:
			Local_psDMA = DMA2;
			break;

		default:
			return;
	}

	/* Step 2: make sure the Stream is disabled before touching SxCR/SxFCR.
	 * Most control bits are read-only hardware-protected while EN=1. */

	CLR_BIT(Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR, DMA_SxCR_EN_POS);
	while (GET_BIT(Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR, DMA_SxCR_EN_POS) == 1)
	{
		/* wait for hardware to actually clear EN */
	}


	/* Step 3: start from a clean register so every |= below is safe
	 * (no leftover bits from a previous configuration on this Stream) */
	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR  = 0;
	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_FCR = 0;

	/* Step 4: write all static SxCR fields, one OR per field */
	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->CHANNEL << DMA_SxCR_CHSEL_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->DIRECTION << DMA_SxCR_DIR_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->MEMORY_SIZE << DMA_SxCR_MSIZE_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->PERIPHERAL_SIZE << DMA_SxCR_PSIZE_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->MEMORY_INCREMENT << DMA_SxCR_MINC_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->PERIPHERAL_INCREMENT << DMA_SxCR_PINC_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->CIRCULAR_MODE << DMA_SxCR_CIRC_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->PRIORITY << DMA_SxCR_PL_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->MEMORY_BURST << DMA_SxCR_MBURST_POS);

	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_CR |=(Copy_psConfig->PERIPHERAL_BURST << DMA_SxCR_PBURST_POS);


	/* Step 5: write SxFCR (FIFO threshold). DMDIS left at 0 (Direct mode)
	 * unless FIFO threshold requires it -- caller controls via config if
	 * you later extend DMA_Config_t with an explicit DMDIS field. */
	Local_psDMA->DMA_STREAM[Copy_psConfig->STREAM].DMA_FCR |=(Copy_psConfig->FIFO_THRESHOLD << DMA_SxFCR_FTH_POS);
}


/*----------------------------------------------------------------------------
 * MC_DMA_voidStart
 *--------------------------------------------------------------------------*/
void MC_DMA_voidStart(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream,u32 Copy_u32PeripheralAddr, u32 Copy_u32MemoryAddr,u16 Copy_u16DataLength)
{
	DMA_RegDef_t* Local_psDMA = NULL;

	switch (Copy_enuIndex)
	{
		case DMA1_INDEX:
			Local_psDMA = DMA1;
			break;

		case DMA2_INDEX:
			Local_psDMA = DMA2;
			break;

		default:
			return;
	}

	/* Ensure Stream is stopped before loading new address/length --
	 * PAR/M0AR/NDTR are also read-only hardware-protected while EN=1 */
	CLR_BIT(Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_CR, DMA_SxCR_EN_POS);
	while (GET_BIT(Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_CR, DMA_SxCR_EN_POS) == 1)
	{
		/* wait for hardware to actually clear EN */
	}

	/* RM0368 order: configure PAR/M0AR/NDTR first, THEN set EN last */
	Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_PAR  = Copy_u32PeripheralAddr;
	Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_M0AR = Copy_u32MemoryAddr;
	Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_NDTR = Copy_u16DataLength;

	SET_BIT(Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_CR, DMA_SxCR_EN_POS);
}

/*----------------------------------------------------------------------------
 * MC_DMA_voidStop
 *--------------------------------------------------------------------------*/
void MC_DMA_voidStop(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream)
{
	DMA_RegDef_t* Local_psDMA = NULL;

	switch (Copy_enuIndex)
	{
		case DMA1_INDEX:
			Local_psDMA = DMA1;
			break;

		case DMA2_INDEX:
			Local_psDMA = DMA2;
			break;

		default:
			return;
	}

	CLR_BIT(Local_psDMA->DMA_STREAM[Copy_u8Stream].DMA_CR, DMA_SxCR_EN_POS);
}

/*----------------------------------------------------------------------------
 * MC_DMA_u8GetFlagStatus
 *--------------------------------------------------------------------------*/
u8 MC_DMA_u8GetFlagStatus(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, u8 Copy_u8FlagType)
{
	DMA_RegDef_t* Local_psDMA = NULL;
	u8            Local_u8GroupIndex;
	u8            Local_u8BitPosition;
	u8            Local_u8FlagValue;

	switch (Copy_enuIndex)
	{
		case DMA1_INDEX:
			Local_psDMA = DMA1;
			break;

		case DMA2_INDEX:
			Local_psDMA = DMA2;
			break;

		default:
			return 0;
	}

	/* Streams 0-3 live in the Low register, 4-7 in the High register.
	 * Within either register, the group base repeats every 4 streams. */
	Local_u8GroupIndex  = Copy_u8Stream % 4;
	Local_u8BitPosition = DMA_au8ISRGroupBase[Local_u8GroupIndex] + Copy_u8FlagType;

	if (Copy_u8Stream < 4)
	{
		Local_u8FlagValue = GET_BIT(Local_psDMA->DMA_LISR, Local_u8BitPosition);
	}
	else
	{
		Local_u8FlagValue = GET_BIT(Local_psDMA->DMA_HISR, Local_u8BitPosition);
	}

	return Local_u8FlagValue;
}

/*----------------------------------------------------------------------------
 * MC_DMA_voidClearFlag
 *--------------------------------------------------------------------------*/
void MC_DMA_voidClearFlag(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, u8 Copy_u8FlagType)
{
	DMA_RegDef_t* Local_psDMA = NULL;
	u8            Local_u8GroupIndex;
	u8            Local_u8BitPosition;

	switch (Copy_enuIndex)
	{
		case DMA1_INDEX:
			Local_psDMA = DMA1;
			break;

		case DMA2_INDEX:
			Local_psDMA = DMA2;
			break;

		default:
			return;
	}

	Local_u8GroupIndex  = Copy_u8Stream % 4;
	Local_u8BitPosition = DMA_au8ISRGroupBase[Local_u8GroupIndex] + Copy_u8FlagType;

	/* LIFCR/HIFCR are rc_w1: writing 1 clears the flag, writing 0 does
	 * nothing. Never use |= here with a stale read -- write the single
	 * bit directly, same pattern as EXTI_PR. */
	if (Copy_u8Stream < 4)
	{
		Local_psDMA->DMA_LIFCR = (1U << Local_u8BitPosition);
	}
	else
	{
		Local_psDMA->DMA_HIFCR = (1U << Local_u8BitPosition);
	}
}

/*----------------------------------------------------------------------------
 * MC_DMA_voidSetCallback
 *--------------------------------------------------------------------------*/
void MC_DMA_voidSetCallback(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, void (*Copy_pvCallbackFunc)(void))
{
	if ((Copy_enuIndex == DMA1_INDEX || Copy_enuIndex == DMA2_INDEX) && (Copy_u8Stream < 8))
	{
		Global_apfDMACallback[Copy_enuIndex][Copy_u8Stream] = Copy_pvCallbackFunc;
	}
}
