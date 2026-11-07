#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "MC_GPIO_interface.h"
#include "MC_SPI_interface.h"
#include "MC_STK_interface.h"

#include "HA_TFT_interface.h"


/* =========================================================
 * TFT Pins
 * ========================================================= */

#define TFT_RST_PORT    PORTA
#define TFT_RST_PIN     PIN0

#define TFT_DC_PORT     PORTA
#define TFT_DC_PIN      PIN2

#define TFT_CS_PORT     PORTA
#define TFT_CS_PIN      PIN4


/* =========================================================
 * TFT Commands
 * ========================================================= */

#define TFT_CMD_SLEEP_OUT     0x11
#define TFT_CMD_COLOR_MODE    0x3A
#define TFT_CMD_CASET         0x2A
#define TFT_CMD_RASET         0x2B
#define TFT_CMD_RAMWR         0x2C
#define TFT_CMD_DISPLAY_ON    0x29


/* =========================================================
 * TFT Configuration
 * ========================================================= */

#define TFT_COLOR_RGB565      0x05

#define TFT_WIDTH             128
#define TFT_HEIGHT            160

#define TFT_TOTAL_PIXELS      (TFT_WIDTH * TFT_HEIGHT)


/* =========================================================
 * Private Functions
 * ========================================================= */

static void TFT_voidReset(void)
{
    /* Reset HIGH */
    MGPIO_voidSetPinValue(TFT_RST_PORT,TFT_RST_PIN,PIN_HIGH);

    STK_voidDelay_ms(10);

    /* Reset LOW */
    MGPIO_voidSetPinValue(TFT_RST_PORT,TFT_RST_PIN,PIN_LOW);

    STK_voidDelay_ms(10);

    /* Reset HIGH */
    MGPIO_voidSetPinValue(TFT_RST_PORT,TFT_RST_PIN,PIN_HIGH);

    STK_voidDelay_ms(120);
}


/* --------------------------------------------------------- */

static void TFT_voidWriteCommand(u8 Copy_u8Command)
{
    /* Select TFT */
    MGPIO_voidSetPinValue(TFT_CS_PORT,TFT_CS_PIN,PIN_LOW);

    /* Command mode */
    MGPIO_voidSetPinValue(TFT_DC_PORT,TFT_DC_PIN,PIN_LOW);

    /* Send command */
    (void)MC_SPI_u8TransceiveByte(SPI_1,Copy_u8Command);

    /* Deselect TFT */
    MGPIO_voidSetPinValue(TFT_CS_PORT,TFT_CS_PIN,PIN_HIGH);
}


/* --------------------------------------------------------- */

static void TFT_voidWriteData(u8 Copy_u8Data)
{
    /* Select TFT */
    MGPIO_voidSetPinValue(TFT_CS_PORT,TFT_CS_PIN,PIN_LOW);

    /* Data mode */
    MGPIO_voidSetPinValue(TFT_DC_PORT,TFT_DC_PIN,PIN_HIGH);

    /* Send data */
    (void)MC_SPI_u8TransceiveByte(SPI_1,Copy_u8Data);

    /* Deselect TFT */
    MGPIO_voidSetPinValue(TFT_CS_PORT,TFT_CS_PIN,PIN_HIGH);}
/* --------------------------------------------------------- */

static void TFT_voidSetAddressWindow(u8 Copy_u8X0,u8 Copy_u8Y0,u8 Copy_u8X1,u8 Copy_u8Y1)

{
    /* Column Address Set */
    TFT_voidWriteCommand(TFT_CMD_CASET);

    TFT_voidWriteData(0x00);
    TFT_voidWriteData(Copy_u8X0);
    TFT_voidWriteData(0x00);
    TFT_voidWriteData(Copy_u8X1);


    /* Row Address Set */
    TFT_voidWriteCommand(TFT_CMD_RASET);

    TFT_voidWriteData(0x00);
    TFT_voidWriteData(Copy_u8Y0);
    TFT_voidWriteData(0x00);
    TFT_voidWriteData(Copy_u8Y1);
}


/* =========================================================
 * Public Functions
 * ========================================================= */

void TFT_voidInit(void)
{
    /* Hardware reset */
    TFT_voidReset();
    /* Exit Sleep Mode */
    TFT_voidWriteCommand(TFT_CMD_SLEEP_OUT);
    STK_voidDelay_ms(120);
    /* Set Color Format */
    TFT_voidWriteCommand(TFT_CMD_COLOR_MODE);
    TFT_voidWriteData(TFT_COLOR_RGB565);
    /* Turn Display ON */
    TFT_voidWriteCommand(TFT_CMD_DISPLAY_ON);
    STK_voidDelay_ms(20);
}


/* --------------------------------------------------------- */

void TFT_voidFillScreen(u16 Copy_u16Color)
{
    u32 Local_u32Index;
    u8 Local_u8HighByte;
    u8 Local_u8LowByte;
    /* Extract RGB565 bytes */
    Local_u8HighByte = (u8)(Copy_u16Color >> 8);
    Local_u8LowByte =  (u8)(Copy_u16Color);
    /* Set complete screen window */
    TFT_voidSetAddressWindow(0,0,TFT_WIDTH - 1,TFT_HEIGHT - 1);
    /* Start Memory Write */
    TFT_voidWriteCommand(TFT_CMD_RAMWR);
    /* Send all pixels */
    for (Local_u32Index = 0;Local_u32Index < TFT_TOTAL_PIXELS;Local_u32Index++)
    {
        TFT_voidWriteData(Local_u8HighByte);
        TFT_voidWriteData(Local_u8LowByte);
    }
}

/* --------------------------------------------------------- */

void TFT_voidDisplay(const u16 *Copy_pu16Image)
{
    u32 Local_u32Index;
    u8 Local_u8HighByte;
    u8 Local_u8LowByte;
    /* Set complete screen window */
    TFT_voidSetAddressWindow(0,0,TFT_WIDTH - 1,TFT_HEIGHT - 1);
    /* Start Memory Write */
    TFT_voidWriteCommand(TFT_CMD_RAMWR);
    /* Send image */
    for (Local_u32Index = 0;Local_u32Index < TFT_TOTAL_PIXELS;Local_u32Index++)
    {
        Local_u8HighByte =(u8)(Copy_pu16Image[Local_u32Index] >> 8);
        Local_u8LowByte =(u8)(Copy_pu16Image[Local_u32Index]);
        TFT_voidWriteData(Local_u8HighByte);
        TFT_voidWriteData(Local_u8LowByte);
    }
}
