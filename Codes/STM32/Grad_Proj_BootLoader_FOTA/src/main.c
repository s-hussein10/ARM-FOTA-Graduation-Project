#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MC_RCC_interface.h"
#include "MC_GPIO_interface.h"
#include "MC_STK_interface.h"
#include "MC_SPI_interface.h"
#include "MC_SPI_config.h"
#include "MC_USART_interface.h"
#include "MC_USART_private.h"

#include "HA_TFT_interface.h"
#include "MFMI_interface.h"
#include "APP_HEXPARSER_interface.h"

/* =========================================================
 * Bootloader Configuration & Flash Layout
 *   Bootloader : Sectors 0..1 (0x08000000 -> 0x08007FFF, 32 KB)
 *   Application: Sectors 2..5 (0x08008000 -> 0x0803FFFF, 224 KB)
 * ========================================================= */
#define APP_BASE_ADDRESS      (0x08008000UL)
#define SCB_VTOR_REG          (*((volatile u32*)0xE000ED08UL))
#define STK_CTRL_REG          (*((volatile u32*)0xE000E010UL))

/* Bootloader Status LEDs (on PORTB so no rewiring is needed) */
#define BL_LED_PORT           PORTB
#define BL_WAIT_LED_PIN       PIN0   /* Steady ON while waiting in Bootloader */
#define BL_FLASH_LED_PIN      PIN1   /* Toggles on every flashed HEX record   */

/* TFT Dimensions */
#define TFT_SCREEN_W          (128U)
#define TFT_SCREEN_H          (160U)

extern u32 Global_u32HighAddress;

/* =========================================================
 * Static Buffers & Prototypes
 * ========================================================= */
static u16  Global_au16ScreenBuf[TFT_SCREEN_W * TFT_SCREEN_H];
static char Global_acHexBuffer[64];

static void Bootloader_voidJumpToApp(void);
static u8   Bootloader_u8IsAppValid(void);

static u8   TFT_u8GetGlyphCol(char Copy_cCh, u8 Copy_u8Col);
static void TFT_voidDrawCharBuf(u8 Copy_u8X, u8 Copy_u8Y, char Copy_cCh, u16 Copy_u16Color, u8 Copy_u8Scale);
static void TFT_voidDrawStrBuf(u8 Copy_u8X, u8 Copy_u8Y, const char *Copy_pcStr, u16 Copy_u16Color, u8 Copy_u8Scale);
static void TFT_voidShowScreen(u16 Copy_u16Bg, u16 Copy_u16Fg, const char *Copy_pcLine1, const char *Copy_pcLine2);

/* =========================================================
 * Bootloader Main
 * ========================================================= */
int main(void)
{
    u8  Local_u8RxChar;
    u8  Local_u8Idx         = 0U;
    u8  Local_u8FlashToggle = 0U;
    u8  Local_u8Busy        = 0U;
    u32 Local_u32BeaconCnt  = 0U;

    /* 1) Initialize Clocks */
    RCC_voidInit();
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOA);
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOB);
    RCC_voidEnblePeripheral(APB2, APB2_SPI1RST);
    RCC_voidEnblePeripheral(APB2, APB2_USART1RST);

    STK_voidInit();

    /* 2) Initialize Bootloader 2 LEDs (PB0 = Waiting, PB1 = Flashing) */
    MGPIO_voidSetPinMode(BL_LED_PORT, BL_WAIT_LED_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(BL_LED_PORT, BL_WAIT_LED_PIN, PUSH_PULL, MED_SPEED);
    MGPIO_voidSetPinValue(BL_LED_PORT, BL_WAIT_LED_PIN, PIN_HIGH); /* Waiting LED ON */

    MGPIO_voidSetPinMode(BL_LED_PORT, BL_FLASH_LED_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(BL_LED_PORT, BL_FLASH_LED_PIN, PUSH_PULL, MED_SPEED);
    MGPIO_voidSetPinValue(BL_LED_PORT, BL_FLASH_LED_PIN, PIN_LOW);  /* Flashing LED OFF */

    /* Ensure remaining LEDs (PB2..PB7) and Buzzer (PA8) are OFF */
    for (u8 pin = PIN2; pin <= PIN7; pin++)
    {
        MGPIO_voidSetPinMode(PORTB, (PINS)pin, OUTPUT);
        MGPIO_voidSetOutputConfig(PORTB, (PINS)pin, PUSH_PULL, MED_SPEED);
        MGPIO_voidSetPinValue(PORTB, (PINS)pin, PIN_LOW);
    }
    MGPIO_voidSetPinMode(PORTA, PIN8, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN8, PUSH_PULL, MED_SPEED);
    MGPIO_voidSetPinValue(PORTA, PIN8, PIN_LOW);

    /* 3) Initialize TFT Control Pins (PA0=RST, PA2=DC, PA4=CS) & SPI1 (PA5, PA7) */
    MGPIO_voidSetPinMode(PORTA, PIN0, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN0, PUSH_PULL, HIGH_SPEED);

    MGPIO_voidSetPinMode(PORTA, PIN2, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN2, PUSH_PULL, HIGH_SPEED);

    MGPIO_voidSetPinMode(PORTA, PIN4, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN4, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetPinValue(PORTA, PIN4, PIN_HIGH);

    MGPIO_voidSetPinMode(PORTA, PIN5, ALTERNATE_FUNC);
    MGPIO_voidSetOutputConfig(PORTA, PIN5, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN5, AF5);

    MGPIO_voidSetPinMode(PORTA, PIN7, ALTERNATE_FUNC);
    MGPIO_voidSetOutputConfig(PORTA, PIN7, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN7, AF5);

    MC_SPI_voidInit(&SPI1_MasterConfig);
    TFT_voidInit();

    /* 4) Initialize USART1 (PA9=TX, PA10=RX, AF7, 9600 bps) for ESP32 */
    MGPIO_voidSetPinMode(PORTA, PIN9, ALTERNATE_FUNC);
    MGPIO_voidSetOutputConfig(PORTA, PIN9, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN9, AF7);

    MGPIO_voidSetPinMode(PORTA, PIN10, ALTERNATE_FUNC);
    MGPIO_voidSetInputConfig(PORTA, PIN10, PULL_UP);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN10, AF7);

    USART_ConfigType Local_stUartCfg = {
        .INDEX       = USART1_INDEX,
        .Parity      = NO_PARITY,
        .Word_Length = NO_8_BIT,
        .Stop_Bits   = NO_1_BIT,
        .BaudRate    = 9600UL,
        .Mode        = TX_RX
    };
    USART_voidInit(&Local_stUartCfg);

    /* 5) Display Waiting Message on TFT & Notify ESP32 / Node-RED Dashboard */
    TFT_voidShowScreen(0x001F, 0xFFFF, "BOOTLOADER", "WAIT HEX");
    USART_voidSendStr(USART1_INDEX, (u8*)"BOOTLOADER: WAITING FOR HEX\n");

    /* =====================================================
     * Bootloader Loop
     * ===================================================== */
    while (1)
    {
        /* Periodically remind ESP32 of Bootloader status while idle (before Erase starts) */
        if (Local_u8Busy == 0U)
        {
            Local_u32BeaconCnt++;
            if (Local_u32BeaconCnt >= 4000000UL)
            {
                Local_u32BeaconCnt = 0U;
                if (GET_BIT(USART1->USART_SR, RXNE) == 0U)
                {
                    USART_voidSendStr(USART1_INDEX, (u8*)"BOOTLOADER: WAITING FOR HEX\n");
                }
            }
        }

        /* Check if ESP32 sent a byte over USART1 */
        if (GET_BIT(USART1->USART_SR, RXNE) == 1U)
        {
            Local_u8RxChar = USART_u8ReceiveChar(USART1_INDEX);

            /* Command 'E': Erase Application Flash Sectors (Sectors 2 -> 5) */
            if ((Local_u8RxChar == 'E') || (Local_u8RxChar == 'e'))
            {
                Local_u8Busy = 1U;
                MGPIO_voidSetPinValue(BL_LED_PORT, BL_FLASH_LED_PIN, PIN_HIGH);
                TFT_voidShowScreen(0xF800, 0xFFFF, "BOOTLOADER", "ERASING");

                /* Erase Sectors 2, 3, 4, and 5 (0x08008000 -> 0x0803FFFF) */
                MFMI_voidSectorsErase((Sector)2, (Sector)5);
                Global_u32HighAddress = 0x08000000UL;

                /* Update TFT to FLASHING before sending OK so CPU is ready for HEX lines */
                TFT_voidShowScreen(0xFFE0, 0x0000, "BOOTLOADER", "FLASHING");
                USART_voidSendStr(USART1_INDEX, (u8*)"OK\n");
            }
            /* Command ':': Start of an Intel HEX Record Line */
            else if (Local_u8RxChar == ':')
            {
                Local_u8Busy = 1U;
                Local_u8Idx  = 0U;
                Global_acHexBuffer[Local_u8Idx++] = ':';

                /* Read full HEX record until '\n' */
                while (1)
                {
                    u8 Local_u8Ch = USART_u8ReceiveChar(USART1_INDEX);
                    if (Local_u8Ch == '\n')
                    {
                        break;
                    }
                    else if ((Local_u8Ch != '\r') && (Local_u8Idx < 62U))
                    {
                        Global_acHexBuffer[Local_u8Idx++] = (char)Local_u8Ch;
                    }
                }
                Global_acHexBuffer[Local_u8Idx] = '\0';

                /* Toggle LED 2 (PB1) to indicate active flashing */
                Local_u8FlashToggle ^= 1U;
                MGPIO_voidSetPinValue(BL_LED_PORT, BL_FLASH_LED_PIN,
                                      (Local_u8FlashToggle == 1U) ? PIN_HIGH : PIN_LOW);

                /* Parse and program record to Flash */
                APP_HEXPARSER_voidParserRecord(Global_acHexBuffer);

                /* Check if this record is End-Of-File (:00000001FF -> Record[8] == '1') */
                if ((Global_acHexBuffer[7] == '0') && (Global_acHexBuffer[8] == '1'))
                {
                    /* Acknowledge final line so Node-RED reaches 100% */
                    USART_voidSendStr(USART1_INDEX, (u8*)"OK\n");

                    TFT_voidShowScreen(0x07E0, 0x0000, "FLASH OK", "START APP");
                    STK_voidDelay_ms(300);

                    MGPIO_voidSetPinValue(BL_LED_PORT, BL_WAIT_LED_PIN, PIN_LOW);
                    MGPIO_voidSetPinValue(BL_LED_PORT, BL_FLASH_LED_PIN, PIN_LOW);

                    Bootloader_voidJumpToApp();
                }
                else
                {
                    /* Acknowledge line so ESP32 / Node-RED sends the next line */
                    USART_voidSendStr(USART1_INDEX, (u8*)"OK\n");
                }
            }
            /* Command 'J': Jump directly to existing Application */
            else if ((Local_u8RxChar == 'J') || (Local_u8RxChar == 'j'))
            {
                if (Bootloader_u8IsAppValid() == 1U)
                {
                    USART_voidSendStr(USART1_INDEX, (u8*)"JUMPING TO APP\n");
                    TFT_voidShowScreen(0x07E0, 0x0000, "BOOTLOADER", "START APP");
                    STK_voidDelay_ms(150);

                    MGPIO_voidSetPinValue(BL_LED_PORT, BL_WAIT_LED_PIN, PIN_LOW);
                    MGPIO_voidSetPinValue(BL_LED_PORT, BL_FLASH_LED_PIN, PIN_LOW);

                    Bootloader_voidJumpToApp();
                }
                else
                {
                    TFT_voidShowScreen(0xF800, 0xFFFF, "BOOTLOADER", "NO APP");
                    USART_voidSendStr(USART1_INDEX, (u8*)"BOOTLOADER: NO APP FLASHED\n");
                }
            }
        }
    }
}

/* =========================================================
 * Check if a valid Application stack pointer exists at 0x08008000
 * ========================================================= */
static u8 Bootloader_u8IsAppValid(void)
{
    u32 Local_u32AppMsp = *((volatile u32*)APP_BASE_ADDRESS);
    /* STM32F401 has 64 KB SRAM: 0x20000000 -> 0x20010000 */
    if ((Local_u32AppMsp >= 0x20000000UL) && (Local_u32AppMsp <= 0x20010000UL))
    {
        return 1U;
    }
    return 0U;
}

/* =========================================================
 * Clean Cortex-M4 Jump to Main Application at 0x08008000
 * ========================================================= */
static void Bootloader_voidJumpToApp(void)
{
    if (Bootloader_u8IsAppValid() == 0U)
    {
        return;
    }

    u32 Local_u32AppMsp          = *((volatile u32*)APP_BASE_ADDRESS);
    u32 Local_u32ResetHandlerVal = *((volatile u32*)(APP_BASE_ADDRESS + 4UL));
    void (*Local_pfAppReset)(void) = (void (*)(void))Local_u32ResetHandlerVal;

    /* Disable interrupts & stop SysTick during vector handover */
    __asm volatile ("cpsid i");
    STK_CTRL_REG = 0UL;

    /* Relocate Vector Table to Application Base Address */
    SCB_VTOR_REG = APP_BASE_ADDRESS;

    /* Set Main Stack Pointer (MSP) and jump to Application Reset_Handler */
    __asm volatile ("MSR msp, %0" : : "r" (Local_u32AppMsp) : );
    Local_pfAppReset();
}

/* =========================================================
 * TFT Screen Text Rendering Functions (5x7 Scaled Font)
 * ========================================================= */
static u8 TFT_u8GetGlyphCol(char Copy_cCh, u8 Copy_u8Col)
{
    u8 Local_au8Cols[5] = {0, 0, 0, 0, 0};

    switch (Copy_cCh)
    {
        case 'A': Local_au8Cols[0]=0x7E; Local_au8Cols[1]=0x11; Local_au8Cols[2]=0x11; Local_au8Cols[3]=0x11; Local_au8Cols[4]=0x7E; break;
        case 'B': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x36; break;
        case 'C': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x22; break;
        case 'D': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x3E; break;
        case 'E': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x41; break;
        case 'F': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x09; Local_au8Cols[3]=0x09; Local_au8Cols[4]=0x01; break;
        case 'G': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x3A; break;
        case 'H': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x08; Local_au8Cols[2]=0x08; Local_au8Cols[3]=0x08; Local_au8Cols[4]=0x7F; break;
        case 'I': Local_au8Cols[0]=0x41; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x7F; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x41; break;
        case 'K': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x08; Local_au8Cols[2]=0x14; Local_au8Cols[3]=0x22; Local_au8Cols[4]=0x41; break;
        case 'L': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x40; Local_au8Cols[2]=0x40; Local_au8Cols[3]=0x40; Local_au8Cols[4]=0x40; break;
        case 'N': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x06; Local_au8Cols[2]=0x08; Local_au8Cols[3]=0x30; Local_au8Cols[4]=0x7F; break;
        case 'O': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x3E; break;
        case 'P': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x09; Local_au8Cols[3]=0x09; Local_au8Cols[4]=0x06; break;
        case 'R': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x19; Local_au8Cols[3]=0x29; Local_au8Cols[4]=0x46; break;
        case 'S': Local_au8Cols[0]=0x26; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x32; break;
        case 'T': Local_au8Cols[0]=0x01; Local_au8Cols[1]=0x01; Local_au8Cols[2]=0x7F; Local_au8Cols[3]=0x01; Local_au8Cols[4]=0x01; break;
        case 'W': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x20; Local_au8Cols[2]=0x18; Local_au8Cols[3]=0x20; Local_au8Cols[4]=0x7F; break;
        case 'X': Local_au8Cols[0]=0x63; Local_au8Cols[1]=0x14; Local_au8Cols[2]=0x08; Local_au8Cols[3]=0x14; Local_au8Cols[4]=0x63; break;
        default:  break;
    }

    return Local_au8Cols[Copy_u8Col];
}

static void TFT_voidDrawCharBuf(u8 Copy_u8X, u8 Copy_u8Y, char Copy_cCh, u16 Copy_u16Color, u8 Copy_u8Scale)
{
    u8 Local_u8Col, Local_u8Row, Local_u8Dx, Local_u8Dy, Local_u8Bits;

    for (Local_u8Col = 0U; Local_u8Col < 5U; Local_u8Col++)
    {
        Local_u8Bits = TFT_u8GetGlyphCol(Copy_cCh, Local_u8Col);
        for (Local_u8Row = 0U; Local_u8Row < 7U; Local_u8Row++)
        {
            if (GET_BIT(Local_u8Bits, Local_u8Row) == 1U)
            {
                for (Local_u8Dy = 0U; Local_u8Dy < Copy_u8Scale; Local_u8Dy++)
                {
                    for (Local_u8Dx = 0U; Local_u8Dx < Copy_u8Scale; Local_u8Dx++)
                    {
                        u16 Local_u16Px = Copy_u8X + (Local_u8Col * Copy_u8Scale) + Local_u8Dx;
                        u16 Local_u16Py = Copy_u8Y + (Local_u8Row * Copy_u8Scale) + Local_u8Dy;
                        if ((Local_u16Px < TFT_SCREEN_W) && (Local_u16Py < TFT_SCREEN_H))
                        {
                            Global_au16ScreenBuf[(Local_u16Py * TFT_SCREEN_W) + Local_u16Px] = Copy_u16Color;
                        }
                    }
                }
            }
        }
    }
}

static void TFT_voidDrawStrBuf(u8 Copy_u8X, u8 Copy_u8Y, const char *Copy_pcStr, u16 Copy_u16Color, u8 Copy_u8Scale)
{
    u8 Local_u8CurX = Copy_u8X;
    while (*Copy_pcStr != '\0')
    {
        TFT_voidDrawCharBuf(Local_u8CurX, Copy_u8Y, *Copy_pcStr, Copy_u16Color, Copy_u8Scale);
        Local_u8CurX += (6U * Copy_u8Scale);
        Copy_pcStr++;
    }
}

static void TFT_voidShowScreen(u16 Copy_u16Bg, u16 Copy_u16Fg, const char *Copy_pcLine1, const char *Copy_pcLine2)
{
    for (u32 idx = 0U; idx < (TFT_SCREEN_W * TFT_SCREEN_H); idx++)
    {
        Global_au16ScreenBuf[idx] = Copy_u16Bg;
    }

    TFT_voidDrawStrBuf(6U, 50U, Copy_pcLine1, Copy_u16Fg, 2U);
    TFT_voidDrawStrBuf(10U, 85U, Copy_pcLine2, Copy_u16Fg, 2U);

    TFT_voidDisplay(Global_au16ScreenBuf);
}
