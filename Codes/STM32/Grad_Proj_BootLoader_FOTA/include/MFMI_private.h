/*
 * MFMI_private.h
 *
 *  Created on: 30 Oct 2026
 *      Author: Sief
 */

#ifndef MFMI_PRIVATE_H_
#define MFMI_PRIVATE_H_

#define MFMI_BASE_ADDRESS 0x40023C00UL

typedef struct
{
    u32 ACR;
    u32 KEYR;
    u32 OPTEKEYR;
    u32 SR;
    u32 CR;
    u32 OPTCR;
}MFMI_t;

#define FMI ((volatile MFMI_t*)(MFMI_BASE_ADDRESS))

#define KEY1  0x45670123
#define KEY2  0xCDEF89AB

#define BSY   16

#define LOCK  31

#define STRT  16
#define MER   2
#define SER   1
#define PG    0

#define SNB   3

#define PSIZE 8

#define MUSK_SNB   15
#define MUSK_PSIZE 3




#endif /* MFMI_PRIVATE_H_ */
