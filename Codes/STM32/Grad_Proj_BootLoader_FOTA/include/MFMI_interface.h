/*
 * MFMI_interface.h
 *
 *  Created on: 30 Oct 2026
 *      Author: Sief
 */

#ifndef MFMI_INTERFACE_H_
#define MFMI_INTERFACE_H_

typedef enum
{
    sector0=0,
    sector1,
    sector2,
    sector3,
    sector4,
    sector5
}Sector;

void MFMI_voidMassErase(void);
void MFMI_voidSectorErase(Sector Copy_uddtSectorNumber);
void MFMI_voidSectorsErase(Sector Copy_uddtSectorStart, Sector Copy_uddtSectorEnd);
void MFMI_voidProgramFlash(u32 Copy_u32Address, u16* Copy_pu16Ptr, u8 Copy_u8DataLength);



#endif /* MFMI_INTERFACE_H_ */
