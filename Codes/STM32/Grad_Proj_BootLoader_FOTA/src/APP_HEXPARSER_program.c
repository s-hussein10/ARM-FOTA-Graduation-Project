/*
 * APP_HEXPARSER_program.c
 */
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "APP_HEXPARSER_interface.h"
#include "MFMI_interface.h"

/* Application starts at Sector 2 (0x08008000) */
#define APP_FLASH_BASE_ADDR   (0x08008000UL)

u32 Global_u32HighAddress = 0x08000000UL;

static u8 APP_HEXPARSER_u8AsciToHex(u8 Copy_u8Variable)
{
    u8 Local_u8Result = 0U;

    if ((Copy_u8Variable >= '0') && (Copy_u8Variable <= '9'))
    {
        Local_u8Result = Copy_u8Variable - '0';
    }
    else if ((Copy_u8Variable >= 'A') && (Copy_u8Variable <= 'F'))
    {
        Local_u8Result = (Copy_u8Variable - 'A') + 10U;
    }
    else if ((Copy_u8Variable >= 'a') && (Copy_u8Variable <= 'f'))
    {
        Local_u8Result = (Copy_u8Variable - 'a') + 10U;
    }

    return Local_u8Result;
}

static void APP_HEXPARSER_voidParseUpperAddress(const char *Copy_pcRecord)
{
    u32 Digit0 = APP_HEXPARSER_u8AsciToHex((u8)Copy_pcRecord[9]);
    u32 Digit1 = APP_HEXPARSER_u8AsciToHex((u8)Copy_pcRecord[10]);
    u32 Digit2 = APP_HEXPARSER_u8AsciToHex((u8)Copy_pcRecord[11]);
    u32 Digit3 = APP_HEXPARSER_u8AsciToHex((u8)Copy_pcRecord[12]);

    Global_u32HighAddress = (Digit0 << 28) | (Digit1 << 24) | (Digit2 << 20) | (Digit3 << 16);
}

void APP_HEXPARSER_voidParserData(const char *Copy_u8Record)
{
    u8 CC_High  = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[1]);
    u8 CC_Low   = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[2]);
    u8 CC_Value = (u8)((CC_High << 4) | CC_Low);

    u32 Address0 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[3]);
    u32 Address1 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[4]);
    u32 Address2 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[5]);
    u32 Address3 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[6]);

    u32 Address = (Global_u32HighAddress & 0xFFFF0000UL) |
                  (Address0 << 12) | (Address1 << 8) | (Address2 << 4) | Address3;

    u8  Digit0, Digit1, Digit2, Digit3;
    u16 Data[16] = {0};
    u8  HalfWordCount = CC_Value / 2U;
    u8  i;

    for (i = 0U; i < HalfWordCount; i++)
    {
        Digit0 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[9U  + (i * 4U)]);
        Digit1 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[10U + (i * 4U)]);
        Digit2 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[11U + (i * 4U)]);
        Digit3 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[12U + (i * 4U)]);

        /* Little-endian 16-bit half-word packing */
        Data[i] = (u16)((Digit2 << 12) | (Digit3 << 8) | (Digit0 << 4) | Digit1);
    }

    /* Handle trailing odd byte if CC_Value is odd */
    if ((CC_Value % 2U) != 0U)
    {
        Digit0 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[9U  + (HalfWordCount * 4U)]);
        Digit1 = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[10U + (HalfWordCount * 4U)]);
        Data[HalfWordCount] = (u16)(0xFF00U | (Digit0 << 4) | Digit1);
        HalfWordCount++;
    }

    /* Protect Bootloader Flash (Sectors 0 & 1: 0x08000000 - 0x08007FFF) */
    if ((Address >= APP_FLASH_BASE_ADDR) && (HalfWordCount > 0U))
    {
        MFMI_voidProgramFlash(Address, Data, HalfWordCount);
    }
}

void APP_HEXPARSER_voidParserRecord(const char *Copy_u8Record)
{
    u8 Local_u8RecordType = APP_HEXPARSER_u8AsciToHex((u8)Copy_u8Record[8]);

    switch (Local_u8RecordType)
    {
        case 0U: /* Data Record */
            APP_HEXPARSER_voidParserData(Copy_u8Record);
            break;

        case 1U: /* End Of File Record (:00000001FF) */
            break;

        case 4U: /* Extended Linear Address Record (:02000004xxxx..) */
            APP_HEXPARSER_voidParseUpperAddress(Copy_u8Record);
            break;

        default:
            break;
    }
}
