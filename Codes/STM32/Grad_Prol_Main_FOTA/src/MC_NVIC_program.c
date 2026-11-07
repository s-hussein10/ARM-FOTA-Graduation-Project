/*
 * MC_NVIC_program.c
 *
 *  Created on: 7 Aug 2026
 *      Author: Sief
 *
 *  NOTE: Annotated copy. ISER/ICER/ISPR/ICPR/IABR indexing bug fixed (Position/32 for the
 *        register, Position%32 for the bit). SetGroupPeriority renamed -> SetGroupPriority.
 */
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "MC_NVIC_interface.h"
#include "MC_NVIC_private.h"
#include "MC_NVIC_config.h"

/* Global variable that remembers which PRIGROUP split (Group/SubGroup) the user
 * configured via NVIC_voidSetGroupConfig(). SetGroupPriority() reads this later
 * to know how to split the priority byte. */
GROUP_CONFIG Global_UddtGROUP_CONFIG;

/*
 * NVIC_voidEnable
 * ----------------
 * Enables (unmasks) the interrupt with the given IRQ number (Vector Position)
 * in the NVIC, by setting its bit in the NVIC_ISER (Interrupt Set-Enable Register) array.
 *
 * ISER is made of 8 x 32-bit registers -> 256 possible IRQ lines total.
 * To reach IRQ number N:
 *   - which 32-bit register?  N / 32   (REVIEW: currently using N directly, not N/32)
 *   - which bit inside it?    N % 32   (this part is correct)
 */
void NVIC_voidEnable (u8 Copy_u8VectorPosition)
{
	/* FIXED: register index = VectorPosition/32 (which of the 8 ISER registers),
	 * bit index = VectorPosition%32 (which bit inside that register). */
	SET_BIT(NVIC->NVIC_ISER[Copy_u8VectorPosition/32] ,Copy_u8VectorPosition%32 );
}

/*
 * NVIC_voidPendingFlagEnable
 * ---------------------------
 * Forces the interrupt into the "pending" state via NVIC_ISPR
 * (Interrupt Set-Pending Register), without needing the real hardware event.
 * Useful for software-triggered testing of an ISR.
 */
void NVIC_voidPendingFlagEnable (u8 Copy_u8VectorPosition)
{
	/* FIXED: same register/bit split as Enable() above. */
	SET_BIT(NVIC->NVIC_ISPR[Copy_u8VectorPosition/32] ,Copy_u8VectorPosition%32 );
}

/*
 * NVIC_voidPendingFlagDisable
 * -----------------------------
 * Clears a pending interrupt request via NVIC_ICPR (Interrupt Clear-Pending Register),
 * so it will NOT fire even though it was previously pending.
 */
void NVIC_voidPendingFlagDisable(u8 Copy_u8VectorPosition)
{
	/* FIXED: same register/bit split as Enable() above. */
	SET_BIT(NVIC->NVIC_ICPR[Copy_u8VectorPosition/32] ,Copy_u8VectorPosition%32 );
}

/*
 * NVIC_voidSetGroupPriority
 * ----------------------------
 * Sets the Group priority and SubGroup priority for a given IRQ, inside the
 * NVIC_IPR (Interrupt Priority Register) array.
 *
 * Unlike ISER/ICER/ISPR/ICPR (1 bit per interrupt, 32 interrupts packed per register),
 * NVIC_IPR is one BYTE per interrupt, so indexing directly with Copy_u8VectorPosition
 * here is correct (no /32 needed).
 *
 * The split between Group bits and SubGroup bits depends on which PRIGROUP
 * configuration was chosen earlier via NVIC_voidSetGroupConfig() (stored in
 * Global_UddtGROUP_CONFIG). Only the upper nibble of the priority byte is used
 * on Cortex-M4 (4 implemented priority bits), which is why we always clear/set
 * bits within the upper nibble.
 */
void NVIC_voidSetGroupPriority(u8 Copy_u8VectorPosition , u8 Copy_u8Group, u8 Copy_u8SubGroup)
{
	switch (Global_UddtGROUP_CONFIG)
	{
		case GROUP16_SUB0:
			/* 4 Group bits, 0 SubGroup bits -> Group occupies bits[7:4] */
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] &= 0xF ;
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] |= ((Copy_u8Group & 0xF) << 4) ;
			break;

		case GROUP8_SUB2:
			/* 3 Group bits (bits[7:5]), 1 SubGroup bit (bit[4]) */
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] &= 0xF ;
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] |= ((Copy_u8Group & 0x7) << 5) | ((Copy_u8SubGroup & 0x1) << 4);
			break;

		case GROUP4_SUB4:
			/* 2 Group bits (bits[7:6]), 2 SubGroup bits (bits[5:4]) */
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] &= 0xF ;
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] |= ((Copy_u8Group & 0x3) << 6) | ((Copy_u8SubGroup & 0x3) << 4);
			break;

		case GROUP2_SUB8:
			/* 1 Group bit (bit[7]), 3 SubGroup bits (bits[6:4]) */
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] &= 0xF ;
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] |= ((Copy_u8Group & 0x1) << 7) | ((Copy_u8SubGroup & 0x7) << 4);
			break;

		case GROUP0_SUB16:
			/* 0 Group bits, 4 SubGroup bits -> SubGroup occupies bits[7:4] */
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] &= 0xF ;
			NVIC -> NVIC_IPR[Copy_u8VectorPosition] |= ((Copy_u8SubGroup & 0xF) << 4);
			break;

		default:
			/* Global_UddtGROUP_CONFIG was never set via NVIC_voidSetGroupConfig()
			 * before calling this function. Do nothing to avoid corrupting IPR
			 * with an undefined split. */
			break;
	}
}
/*
 * NVIC_u8ReadActiveFlag
 * -----------------------
 * Reads whether the given interrupt is currently ACTIVE (being serviced right now)
 * via NVIC_IABR (Interrupt Active Bit Register). Useful when dealing with nested
 * interrupts to check execution state.
 */
u8   NVIC_u8ReadActiveFlag (u8 Copy_u8VectorPosition)
{
	/* FIXED: same register/bit split as Enable() above. */
	return GET_BIT(NVIC->NVIC_IABR[Copy_u8VectorPosition/32] ,Copy_u8VectorPosition%32 );
}

/*
 * NVIC_voidSetGroupConfig
 * --------------------------
 * Configures the PRIGROUP field inside SCB->AIRCR, which decides how the 4
 * implemented priority bits are split between "Group priority" (pre-emption)
 * and "SubGroup priority" (only matters when Group priorities are equal).
 *
 * Steps:
 *   1) Read current AIRCR into a local copy (do read-modify-write off-chip register safely).
 *   2) Remember the chosen split globally, so SetGroupPriority() knows how to
 *      interpret Group/SubGroup values later.
 *   3) Clear upper 16 bits of the local copy (they held VECTKEYSTAT on read, irrelevant now).
 *   4) Write the VECTKEY = 0x05FA into bits[31:16]. This key MUST be written on every
 *      write to AIRCR, otherwise the hardware silently ignores the write (write-protection).
 *   5) Clear the existing PRIGROUP field (bits[10:8]) without touching other bits.
 *   6) Insert the new PRIGROUP value into bits[10:8].
 *   7) Commit everything to the real AIRCR register in one single write.
 */
void NVIC_voidSetGroupConfig(GROUP_CONFIG Copy_u8GroupConfig)
{
	u32 Local_AIRCR = SCB_AIRCR  ;
	Global_UddtGROUP_CONFIG = Copy_u8GroupConfig;
	Local_AIRCR &= 0x0000FFFF    ; //discard VECTKEYSTAT (upper 16 bits read back)
	Local_AIRCR |= (0x05FA <<16) ; //VECTKEY: required to unlock write access to AIRCR
	Local_AIRCR &=  ~(7<<8)		 ; //clear old PRIGROUP field (bits 10:8)
	Local_AIRCR |=  (Copy_u8GroupConfig<<8 ); //set new PRIGROUP field
	SCB_AIRCR = Local_AIRCR;
}
