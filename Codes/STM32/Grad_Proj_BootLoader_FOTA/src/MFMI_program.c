/*
 * MFMI_program.c
 *
 *  Created on: 30 Oct 2026
 *      Author: Sief
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MFMI_private.h"
#include "MFMI_interface.h"

void MFMI_voidMassErase(void)
{
    //Wait on Busy Flag
    while(GET_BIT(FMI->SR,BSY)==1);

    //Unlock CR Register
    if(GET_BIT(FMI->CR,LOCK)==1)
    {
        FMI->KEYR = KEY1;
        FMI->KEYR = KEY2;
    }

    //select Mass Erase
    FMI->CR |=(1<<MER);
    //start operation
    FMI->CR |=(1<<STRT);

    //Wait on Busy Flag
    while(GET_BIT(FMI->SR,BSY)==1);

    //Turn off mass erase
    FMI->CR &= ~(1<<MER);
}

void MFMI_voidSectorErase(Sector Copy_uddtSectorNumber)
{
    //Wait on Busy Flag
    while(GET_BIT(FMI->SR,BSY)==1);

    //Unlock CR Register
    if(GET_BIT(FMI->CR,LOCK)==1)
    {
        FMI->KEYR = KEY1;
        FMI->KEYR = KEY2;
    }

    //select Sector to erase
    FMI->CR &=~(MUSK_SNB<<SNB);
    FMI->CR |=(Copy_uddtSectorNumber<<SNB);
    //select Sector Erase
    FMI->CR |= (1<<SER);
    //start operation
    FMI->CR |=(1<<STRT);

    //Wait on Busy Flag
    while(GET_BIT(FMI->SR,BSY)==1);

    //Turn off mass erase
    FMI->CR &= ~(1<<SER);
}

void MFMI_voidSectorsErase(Sector Copy_uddtSectorStart,Sector Copy_uddtSectorEnd)
{
    u8 i=0;
    for(i=Copy_uddtSectorStart;i<=Copy_uddtSectorEnd;i++)
    {
        MFMI_voidSectorErase(i);
    }
}

void MFMI_voidProgramFlash(u32 Copy_u32Address,u16* Copy_pu16Ptr,u8 Copy_u8DataLength)
{
    u8 i=0;
    //Wait on Busy Flag
    while(GET_BIT(FMI->SR,BSY)==1);

    //Unlock CR Register
    if(GET_BIT(FMI->CR,LOCK)==1)
    {
        FMI->KEYR = KEY1;
        FMI->KEYR = KEY2;
    }

    //select width (16 bit) to be programed
    FMI->CR &=~(MUSK_PSIZE<<PSIZE);
    FMI->CR |=(1<<PSIZE);
    for(i=0;i<Copy_u8DataLength;i++)
    {
        //select Program mode
        FMI->CR |=(1<<PG);

        *((volatile u16*)Copy_u32Address)=Copy_pu16Ptr[i];
        Copy_u32Address+=2;

        //Wait on Busy Flag
        while(GET_BIT(FMI->SR,BSY)==1);

        //disable Program mode
        FMI->CR &= ~(1<<PG);
    }
}
