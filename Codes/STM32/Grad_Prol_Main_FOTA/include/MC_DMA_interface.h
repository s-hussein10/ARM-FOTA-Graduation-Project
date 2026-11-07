/*
 * MC_DMA_interface.h
 *
 *  Created on: 4 Sep 2026
 *      Author: Sief
 *
 *  Public interface for the DMA MCAL driver (STM32F401CC).
 *  Target usage in this project: USART1 P2M (RX) and M2P (TX) transfers.
 *  M2M (memory-to-memory) is intentionally out of scope for now.
 *
 *  NOTE: Clock enabling for DMA1/DMA2 (RCC AHB1ENR) is the CALLER's
 *        responsibility, same convention as every other MCAL driver here.
 */
#ifndef MC_DMA_INTERFACE_H_
#define MC_DMA_INTERFACE_H_


/*----------------------------------------------------------------------------
 * Which DMA controller (DMA1 or DMA2). Note: only DMA2 can service
 * memory-to-memory transfers and AHB/APB2 peripherals like USART1/USART6;
 * DMA1 is limited to APB1 peripherals (e.g. USART2/3, SPI2/3, I2C...).
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA1_INDEX = 0,
	DMA2_INDEX
}DMA_INDEX;

/*----------------------------------------------------------------------------
 * Channel select (CHSEL bits in SxCR) -- picks WHICH peripheral request
 * line is routed into a given Stream. This is NOT the Stream number itself;
 * a Stream can theoretically be wired to different peripherals depending
 * on which channel is selected. Refer to the "DMA1/DMA2 request mapping"
 * table in RM0368 to know which (Stream, Channel) pair a given peripheral
 * uses -- it is fixed per peripheral, not a free choice.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_CH0 = 0,
	DMA_CH1,
	DMA_CH2,
	DMA_CH3,
	DMA_CH4,
	DMA_CH5,
	DMA_CH6,
	DMA_CH7
}DMA_CHANNEL;

/*----------------------------------------------------------------------------
 * Memory burst transfer configuration (MBURST bits in SxCR).
 * Only meaningful when FIFO mode is enabled (DMDIS=1); ignored in
 * Direct mode. Single = one beat per burst (no bursting).
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_MBURST_Single = 0,
	DMA_MBURST_INCR4,
	DMA_MBURST_INCR8,
	DMA_MBURST_INCR16
}DMA_MEMORY_BURST;

/*----------------------------------------------------------------------------
 * Peripheral burst transfer configuration (PBURST bits in SxCR).
 * Same rules as memory burst above, applied to the peripheral side.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_PBURST_Single = 0,
	DMA_PBURST_INCR4,
	DMA_PBURST_INCR8,
	DMA_PBURST_INCR16
}DMA_PERIPHERAL_BURST;

/*----------------------------------------------------------------------------
 * Stream priority level (PL bits in SxCR). Used by the DMA arbiter when
 * multiple Streams request the bus at the same time. Higher priority wins;
 * ties are broken by lowest Stream number.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_PRIORITY_Low = 0,
	DMA_PRIORITY_Medium,
	DMA_PRIORITY_High,
	DMA_PRIORITY_VeryHigh
}DMA_PRIORITY;

/*----------------------------------------------------------------------------
 * Memory-side data width (MSIZE bits in SxCR). Must be chosen to match
 * the actual width of the buffer in RAM you are transferring to/from.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_MSIZE_Byte = 0,
	DMA_MSIZE_HalfWord,
	DMA_MSIZE_Word,
	DMA_MSIZE_Reserved
}DMA_MEMORY_SIZE;

/*----------------------------------------------------------------------------
 * Peripheral-side data width (PSIZE bits in SxCR). For USART's DR register
 * this is normally Byte (or HalfWord if 9-bit data frame is used).
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_PSIZE_Byte = 0,
	DMA_PSIZE_HalfWord,
	DMA_PSIZE_Word,
	DMA_PSIZE_Reserved
}DMA_PERIPHERAL_DATA;

/*----------------------------------------------------------------------------
 * Transfer direction (DIR bits in SxCR).
 *   Peripheral_to_memory -> used for USART RX (DR -> RAM buffer)
 *   Memory_to_peripheral -> used for USART TX (RAM buffer -> DR)
 *   Memory_to_memory     -> NOT supported on DMA1, only DMA2. Out of scope
 *                           for the current USART1 use case.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_DIR_PeripheralToMemory = 0,
	DMA_DIR_MemoryToPeripheral,
	DMA_DIR_MemoryToMemory,
	DMA_DIR_Reserved
}DMA_DIRECTION_DATA;

/*----------------------------------------------------------------------------
 * FIFO threshold level (FTH bits in SxFCR) -- only relevant when FIFO mode
 * is enabled (DMDIS=1). Selects how full the FIFO must be before a burst
 * is triggered. Not to be confused with FS (FIFO status), which is a
 * read-only hardware flag and therefore intentionally NOT a config option.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_FIFO_THRESHOLD_1_4 = 0,
	DMA_FIFO_THRESHOLD_1_2,
	DMA_FIFO_THRESHOLD_3_4,
	DMA_FIFO_THRESHOLD_Full
}DMA_FIFO_THRESHOLD;

/*----------------------------------------------------------------------------
 * Memory address increment mode (MINC bit in SxCR).
 * Enable  -> M0AR increments automatically after each transfer (normal
 *            case for a RAM buffer, e.g. receiving a string into an array).
 * Disable -> M0AR stays fixed (rare; e.g. repeatedly writing the same
 *            memory-mapped location).
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_MINC_Disable = 0,
	DMA_MINC_Enable
}DMA_MEMORY_INCREMENT;

/*----------------------------------------------------------------------------
 * Peripheral address increment mode (PINC bit in SxCR).
 * Almost always Disable for a peripheral like USART->DR, since the
 * peripheral's data register is a single fixed address.
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_PINC_Disable = 0,
	DMA_PINC_Enable
}DMA_PERIPHERAL_INCREMENT;

/*----------------------------------------------------------------------------
 * Circular mode (CIRC bit in SxCR).
 * Enable  -> after NDTR reaches 0, the Stream automatically reloads the
 *            original NDTR/address and restarts (useful for continuous
 *            streaming, e.g. ADC scan or continuous UART RX buffer).
 * Disable -> Stream stops automatically after NDTR reaches 0 (normal mode,
 *            used for one-shot transfers like a single USART string RX).
 *--------------------------------------------------------------------------*/
typedef enum
{
	DMA_CIRC_Disable = 0,
	DMA_CIRC_Enable
}DMA_CIRCULAR_MODE;

/*----------------------------------------------------------------------------
 * Static, per-Stream configuration -- set once via MDMA_voidInit and does
 * NOT change between transfers.
 *
 * Deliberately EXCLUDES peripheral/memory addresses and transfer length:
 * those three change on every transfer even when the Stream's fixed setup
 * (Channel, Direction, sizes, burst, priority...) stays constant. They are
 * passed separately to MDMA_voidStart so the same Stream configuration can
 * be reused across multiple transfers without re-running Init.
 *--------------------------------------------------------------------------*/
typedef struct
{
	DMA_INDEX                   INDEX;               /* DMA1 or DMA2 */
	u8                          STREAM;              /* Stream number, 0..7 */
	DMA_CHANNEL                 CHANNEL;             /* Request channel, 0..7 */
	DMA_DIRECTION_DATA          DIRECTION;
	DMA_MEMORY_SIZE             MEMORY_SIZE;
	DMA_PERIPHERAL_DATA         PERIPHERAL_SIZE;
	DMA_MEMORY_BURST            MEMORY_BURST;
	DMA_PERIPHERAL_BURST        PERIPHERAL_BURST;
	DMA_PRIORITY                PRIORITY;
	DMA_FIFO_THRESHOLD          FIFO_THRESHOLD;
	DMA_MEMORY_INCREMENT        MEMORY_INCREMENT;
	DMA_PERIPHERAL_INCREMENT    PERIPHERAL_INCREMENT;
	DMA_CIRCULAR_MODE           CIRCULAR_MODE;
}DMA_Config_t;

/*----------------------------------------------------------------------------
 * MC_DMA_voidInit
 * Applies the fixed, one-time configuration for a Stream (Channel,
 * Direction, sizes, burst, priority, increment modes, FIFO threshold...).
 * Does NOT enable the Stream (EN bit) and does NOT set addresses/length --
 * call MDMA_voidStart afterwards to actually begin a transfer.
 * Precondition: DMA1/DMA2 clock must already be enabled by the caller
 * (RCC->AHB1ENR), and the Stream must be disabled (EN=0) before calling
 * this, since most SxCR/SxFCR bits are read-only while EN=1.
 *--------------------------------------------------------------------------*/
void MC_DMA_voidInit(const DMA_Config_t* Copy_psConfig);

/*----------------------------------------------------------------------------
 * MC_DMA_voidStart
 * Sets the peripheral address (SxPAR), memory address (SxM0AR), and
 * transfer length (SxNDTR) for the given Stream, then sets EN=1 to begin
 * the transfer. Can be called repeatedly on an already-Init'd Stream with
 * different addresses/lengths (e.g. receiving into a different buffer
 * each time), as long as the Stream is disabled (EN=0) before each call --
 * enforced internally by first checking/clearing EN if needed.
 *--------------------------------------------------------------------------*/
void MC_DMA_voidStart(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream,u32 Copy_u32PeripheralAddr, u32 Copy_u32MemoryAddr,u16 Copy_u16DataLength);

/*----------------------------------------------------------------------------
 * MC_DMA_voidStop
 * Clears EN on the given Stream, halting any in-progress transfer
 * immediately. Does not clear interrupt flags -- call MDMA_voidClearFlag
 * separately if needed before reusing the Stream.
 *--------------------------------------------------------------------------*/
void MC_DMA_voidStop(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream);

/*----------------------------------------------------------------------------
 * MC_DMA_u8GetFlagStatus
 * Polls a single status flag (Transfer Complete, Half-Transfer, Transfer
 * Error, Direct-mode Error, FIFO Error) for the given Stream by reading
 * LISR (Streams 0-3) or HISR (Streams 4-7), using DMA_au8ISRGroupBase to
 * locate the correct 6-bit group. Returns 1 if the flag is set, else 0.
 * Copy_u8FlagType: use DMA_ISR_TCIF_OFFSET / HTIF / TEIF / DMEIF / FEIF
 * offsets defined in the private header.
 *--------------------------------------------------------------------------*/
u8 MC_DMA_u8GetFlagStatus(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, u8 Copy_u8FlagType);

/*----------------------------------------------------------------------------
 * MC_DMA_voidClearFlag
 * Clears a single status flag for the given Stream by writing 1 to the
 * corresponding bit in LIFCR (Streams 0-3) or HIFCR (Streams 4-7) --
 * these are rc_w1 registers, same pattern as EXTI_PR. Must be called
 * before reusing a Stream if that flag was previously set, otherwise
 * some flags can block a fresh transfer from being recognized correctly.
 *--------------------------------------------------------------------------*/
void MC_DMA_voidClearFlag(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, u8 Copy_u8FlagType);

/*----------------------------------------------------------------------------
 * MC_DMA_voidSetCallback
 * Registers a user function to be called from the DMA Stream's ISR when
 * a Transfer Complete interrupt fires (requires TCIE to have been enabled,
 * and the corresponding IRQ to be enabled in NVIC by the caller). Same
 * single-slot-per-Stream pattern as the EXTI driver's SetCallback.
 *--------------------------------------------------------------------------*/
void MC_DMA_voidSetCallback(DMA_INDEX Copy_enuIndex, u8 Copy_u8Stream, void (*Copy_pvCallbackFunc)(void));

#endif /* MC_DMA_INTERFACE_H_ */
