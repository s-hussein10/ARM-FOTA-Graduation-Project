/*
 * HA_LedMatrix_interface.h
 *
 *  Created on: 7 Aug 2026
 *      Author: Sief
 */

#ifndef HA_LEDMATRIX_INTERFACE_H_
#define HA_LEDMATRIX_INTERFACE_H_

#include  "STD_TYPES.h"

void LedMatrix_voidInit(void);
void LedMatrix_voidDisplayFrame(const u8 Copy_u8Frame[8]);
void LedMatrix_voidClearLedMatrix(void);
void LedMatrix_voidSetNewDelay(u32 Copy_u32NewDelay);
u32 LedMatrix_u32GetTimeDelay(void);


#endif /* HA_LEDMATRIX_INTERFACE_H_ */
