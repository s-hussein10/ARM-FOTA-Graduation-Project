/*
 * MC_DMA_private.h
 *
 *  Created on: 3 Sep 2026
 *      Author: Sief
 */

#ifndef MC_DMA_PRIVATE_H_
#define MC_DMA_PRIVATE_H_



/*----------------------------------------------------------------------------
 * Per-Stream register block
 * Offsets (relative to the stream's own base, i.e. relative to SxCR):
 *   SxCR    : 0x00  ( = 0x10 + 0x18*x from controller base )
 *   SxNDTR  : 0x04
 *   SxPAR   : 0x08
 *   SxM0AR  : 0x0C
 *   SxM1AR  : 0x10
 *   SxFCR   : 0x14
 *
 * NOTE: RM0368 section 9.5.10 text states the SxFCR offset formula as
 *       "0x24 + 0x24 * stream_number" -- this is a TYPO in the manual text.
 *       Table 37 (the authoritative register map) proves the real stride
 *       between ALL six registers of consecutive streams is 0x18, same as
 *       every other register in the stream. Confirmed against explicit
 *       offsets for Stream0..Stream3 in Table 37:
 *         S0CR=0x10 S0FCR=0x24 | S1CR=0x28 S1FCR=0x3C | S2CR=0x40 S2FCR=0x54
 *       => SxFCR = 0x24 + 0x18*x, NOT 0x24 + 0x24*x.
 *--------------------------------------------------------------------------*/
typedef struct
{
	u32 DMA_CR;      /* Stream x configuration register   */
	u32 DMA_NDTR;    /* Stream x number of data register   */
	u32 DMA_PAR;     /* Stream x peripheral address register */
	u32 DMA_M0AR;    /* Stream x memory 0 address register */
	u32 DMA_M1AR;    /* Stream x memory 1 address register */
	u32 DMA_FCR;     /* Stream x FIFO control register     */
} DMA_Stream_t;

/*----------------------------------------------------------------------------
 * Full DMA controller register map (DMA1 and DMA2 share this layout)
 *   0x00  LISR   - Low  Interrupt Status Register  (Streams 0..3, read-only)
 *   0x04  HISR   - High Interrupt Status Register  (Streams 4..7, read-only)
 *   0x08  LIFCR  - Low  Interrupt Flag Clear Reg   (Streams 0..3, rc_w1)
 *   0x0C  HIFCR  - High Interrupt Flag Clear Reg   (Streams 4..7, rc_w1)
 *   0x10  STREAM[0..7] - each block is 0x18 bytes wide (6 x u32)
 *--------------------------------------------------------------------------*/
typedef struct
{
	u32          DMA_LISR;
	u32          DMA_HISR;
	u32          DMA_LIFCR;   /* rc_w1: write 1 to clear -- same pattern as EXTI_PR */
	u32          DMA_HIFCR;   /* rc_w1: write 1 to clear */
	DMA_Stream_t DMA_STREAM[8];
} DMA_RegDef_t;

/*----------------------------------------------------------------------------
 * Base addresses (RM0368 memory map, Section 2.3 Table 1)
 *--------------------------------------------------------------------------*/
#define DMA1_BASE_ADDRESS      0x40026000U
#define DMA2_BASE_ADDRESS      0x40026400U

#define DMA1                   ((DMA_RegDef_t *)DMA1_BASE_ADDRESS)
#define DMA2                   ((DMA_RegDef_t *)DMA2_BASE_ADDRESS)

/*----------------------------------------------------------------------------
 * DMA_SxCR bit positions
 *--------------------------------------------------------------------------*/
#define DMA_SxCR_EN_POS         0U
#define DMA_SxCR_DMEIE_POS      1U
#define DMA_SxCR_TEIE_POS       2U
#define DMA_SxCR_HTIE_POS       3U
#define DMA_SxCR_TCIE_POS       4U
#define DMA_SxCR_PFCTRL_POS     5U

#define DMA_SxCR_DIR_POS        6U
#define DMA_SxCR_DIR_MASK       0x3U

#define DMA_SxCR_CIRC_POS       8U
#define DMA_SxCR_PINC_POS       9U
#define DMA_SxCR_MINC_POS       10U

#define DMA_SxCR_PSIZE_POS      11U
#define DMA_SxCR_PSIZE_MASK     0x3U

#define DMA_SxCR_MSIZE_POS      13U
#define DMA_SxCR_MSIZE_MASK     0x3U

#define DMA_SxCR_PINCOS_POS     15U

#define DMA_SxCR_PL_POS         16U
#define DMA_SxCR_PL_MASK        0x3U

#define DMA_SxCR_DBM_POS        18U
#define DMA_SxCR_CT_POS         19U

#define DMA_SxCR_PBURST_POS     21U
#define DMA_SxCR_PBURST_MASK    0x3U

#define DMA_SxCR_MBURST_POS     23U
#define DMA_SxCR_MBURST_MASK    0x3U

#define DMA_SxCR_CHSEL_POS      25U
#define DMA_SxCR_CHSEL_MASK     0x7U

/*----------------------------------------------------------------------------
 * DMA_SxFCR bit positions
 *--------------------------------------------------------------------------*/
#define DMA_SxFCR_FTH_POS       0U
#define DMA_SxFCR_FTH_MASK      0x3U

#define DMA_SxFCR_DMDIS_POS     2U

#define DMA_SxFCR_FS_POS        3U
#define DMA_SxFCR_FS_MASK       0x7U

#define DMA_SxFCR_FEIE_POS      7U

/*----------------------------------------------------------------------------
 * DMA_LISR / DMA_HISR / DMA_LIFCR / DMA_HIFCR bit-group base positions.
 * Each status/flag group repeats every 6 bits across 4 streams within a
 * single 32-bit register, at bit bases: 0, 6, 16, 22.
 * Within each 6-bit group the layout (from LSB) is:
 *   FEIFx(0) - Reserved(1) - DMEIFx(2) - TEIFx(3) - HTIFx(4) - TCIFx(5)
 *--------------------------------------------------------------------------*/
#define DMA_ISR_GROUP_WIDTH     6U

/* Bit offsets *within* a stream's 6-bit group */
#define DMA_ISR_FEIF_OFFSET     0U
#define DMA_ISR_DMEIF_OFFSET    2U
#define DMA_ISR_TEIF_OFFSET     3U
#define DMA_ISR_HTIF_OFFSET     4U
#define DMA_ISR_TCIF_OFFSET     5U

/*----------------------------------------------------------------------------
 * The 6-bit group "base" position is NOT a simple linear function of the
 * local stream index (0,1,2,3 within LISR/HISR, or 4,5,6,7 within HISR).
 * Confirmed bases per RM0368 9.5.1/9.5.2:
 *   local index 0 -> base bit 0
 *   local index 1 -> base bit 6
 *   local index 2 -> base bit 16
 *   local index 3 -> base bit 22
 * This lookup table is indexed by (StreamNumber % 4) and is used by
 * DMA_program.c when computing which bit to set/check/clear in
 * LISR/HISR/LIFCR/HIFCR for a given stream.
 *--------------------------------------------------------------------------*/
static const u8 DMA_au8ISRGroupBase[4] = {0U, 6U, 16U, 22U};



#endif /* MC_DMA_PRIVATE_H_ */
