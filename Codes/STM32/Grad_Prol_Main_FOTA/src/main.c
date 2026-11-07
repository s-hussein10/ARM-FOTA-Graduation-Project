#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "HA_LedMatrix_interface.h"
#include "HA_SSD_interface.h"
#include "HA_STP_interface.h"
#include "HA_TFT_interface.h"

#include "MC_RCC_interface.h"
#include "MC_GPIO_interface.h"
#include "MC_EXTI_interface.h"
#include "MC_NVIC_interface.h"
#include "MC_STK_interface.h"
#include "MC_STK_config.h"
#include "MC_DMA_interface.h"
#include "MC_SPI_interface.h"
#include "MC_SPI_config.h"
#include "MC_USART_interface.h"


/* =========================================================
 * IR / NEC timing thresholds
 * ========================================================= */
#define IR_BIT0_MIN_TICKS     (  900UL * STK_TICKS_PER_US)
#define IR_BIT0_MAX_TICKS     ( 1400UL * STK_TICKS_PER_US)

#define IR_BIT1_MIN_TICKS     ( 1900UL * STK_TICKS_PER_US)
#define IR_BIT1_MAX_TICKS     ( 2600UL * STK_TICKS_PER_US)

#define EXTI_SYSCFG_PORTA     (0U)
#define NVIC_EXTI1_IRQ        (7U)

/* =========================================================
 * LEDs / Buzzer / Matrix STP / TFT Screen Definitions
 * ========================================================= */
#define LED_ANIM_SPEED        (30000UL)
#define BUZZER_BEEP_TIME      (15000UL)
#define BUZZER_PORT           PORTA
#define BUZZER_PIN            PIN8
#define MATRIX_STP_PORT       PORTB
#define MATRIX_SDI_PIN        PIN12
#define MATRIX_SHCP_PIN       PIN13
#define MATRIX_STCP_PIN       PIN15

#define TFT_SCREEN_W          (128U)
#define TFT_SCREEN_H          (160U)

/* =========================================================
 * Global Variables
 * ========================================================= */
volatile u32 Global_u32Arr[40] = {0};
volatile u8  Global_u8Counter  = 0;
volatile u8  Global_u8StartFlag = 0;
volatile u8  Global_u8CommandValue = 0;

volatile u8  Global_u8LedStep     = 0;
volatile u32 Global_u32LedCounter = 0;

volatile u32 Global_u32BuzzerCounter = 0;

/* Variable to hold the currently displayed number (0-9) */
volatile u8  Global_s8CurrentNumber = 1;

/* Screen Buffer for TFT Text Rendering (128 x 160 RGB565) */
static u16 Global_au16ScreenBuf[TFT_SCREEN_W * TFT_SCREEN_H];

/* 5 Shapes for LED Matrix (8 rows per shape) */
static const u8 Global_au8MatrixShapes[6][8] = {
    /* Mode 0: Off */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* Mode 1 (Chaser): Arrow -> */
    {0x08, 0x0C, 0xFE, 0xFF, 0xFF, 0xFE, 0x0C, 0x08},
    /* Mode 2 (Ping-Pong): Diamond */
    {0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18},
    /* Mode 3 (Sorting): Stairs / Bars */
    {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF},
    /* Mode 4 (Bubbles): Circles */
    {0x66, 0x99, 0x99, 0x66, 0x66, 0x99, 0x99, 0x66},
    /* Mode 5 (Blink): X Shape */
    {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}
};

/* =========================================================
 * Function Prototypes
 * ========================================================= */
static void IR_voidEXTI1Callback(void);
static void IR_voidSysTickTimeoutCallback(void);

static void LEDs_voidInit(void);
static void LEDs_voidTurnOffAll(void);
static void LED_voidMode1_Chaser(void);
static void LED_voidMode2_PingPong(void);
static void LED_voidMode3_Sorting(void);
static void LED_voidMode4_Bubbles(void);
static void LED_voidMode5_Blink(void);
static void LEDs_voidUpdateCurrentMode(void);

static void Buzzer_voidInit(void);
static void Buzzer_voidTriggerBeep(void);
static void Buzzer_voidUpdate(void);

static void MatrixSTP_voidInit(void);
static void MatrixSTP_voidSend16Bits(u8 Copy_u8Cols, u8 Copy_u8Rows);
static void MatrixSTP_voidRefreshRow(void);

static u8   TFT_u8GetGlyphCol(char Copy_cCh, u8 Copy_u8Col);
static void TFT_voidDrawCharBuf(u8 Copy_u8X, u8 Copy_u8Y, char Copy_cCh, u16 Copy_u16Color, u8 Copy_u8Scale);
static void TFT_voidDrawStrBuf(u8 Copy_u8X, u8 Copy_u8Y, const char *Copy_pcStr, u16 Copy_u16Color, u8 Copy_u8Scale);
static void TFT_voidShowModeScreen(u16 Copy_u16Bg, u16 Copy_u16Fg, const char *Copy_pcLine1, const char *Copy_pcLine2);


typedef enum
{
    CMD_POWER = 0, //case 9:   CMD_EQ;         EQ
    CMD_MODE     , //case 67:  CMD_PLAY_PAUSE; >||
    CMD_UP       , //case 7:   CMD_VOL_MINUS;  -
    CMD_DOWN     , //case 21:  CMD_VOL_PLUS;   +
    CMD_OK       , //case 70:  CMD_CH;         CH
    CMD_BACK     , //case 69:  CMD_CH_MINUS;   CH-

    CMD_NUMBER_0,
    CMD_NUMBER_1,
    CMD_NUMBER_2,
    CMD_NUMBER_3,
    CMD_NUMBER_4,
    CMD_NUMBER_5,
    CMD_NUMBER_6,
    CMD_NUMBER_7,
    CMD_NUMBER_8,
    CMD_NUMBER_9,
    CMD_NONE

} Commands_T_IR;

/* =========================================================
 * Main
 * ========================================================= */
int main(void)
{
    /* 1) Initialize RCC */
    RCC_voidInit();
    /* 2) Enable GPIO clock */
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOA);
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOB);
    RCC_voidEnblePeripheral(AHB1, AHB1_GPIOC);
    /* 3) Enable SYSCFG clock */
    RCC_voidEnblePeripheral(APB2, APB2_SYSCFGRST);
    /* 4) Enable COMMU clock */
    RCC_voidEnblePeripheral(APB2, APB2_SPI1RST);
    RCC_voidEnblePeripheral(AHB1, AHB1_DMA2RST);
    RCC_voidEnblePeripheral(APB2, APB2_USART1RST);

    // SysTick Initialization
    STK_voidInit();

    /* =====================================================
     * Communication protocols -> SPI , UART
     * ===================================================== */

    // TFT Control Pins: RST -> PA0, DC -> PA2, CS -> PA4

    /* PA0 -> TFT_RST */
    MGPIO_voidSetPinMode(PORTA, PIN0, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN0, PUSH_PULL, HIGH_SPEED);

    /* PA2 -> TFT_DC (A0) */
    MGPIO_voidSetPinMode(PORTA, PIN2, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN2, PUSH_PULL, HIGH_SPEED);

    /* PA4 -> TFT_CS (start HIGH = deselected) */
    MGPIO_voidSetPinMode(PORTA, PIN4, OUTPUT);
    MGPIO_voidSetOutputConfig(PORTA, PIN4, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetPinValue(PORTA, PIN4, PIN_HIGH);

    // SPI1 Pins: SCK -> PA5, MOSI -> PA7 (AF5)
    MGPIO_voidSetPinMode(PORTA, PIN5, ALTERNATE_FUNC);
    MGPIO_voidSetOutputConfig(PORTA, PIN5, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN5, AF5);

    MGPIO_voidSetPinMode(PORTA, PIN7, ALTERNATE_FUNC);
    MGPIO_voidSetOutputConfig(PORTA, PIN7, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetAlternateFunctionConfig(PORTA, PIN7, AF5);

    /* Initialize SPI1 then TFT */
    MC_SPI_voidInit(&SPI1_MasterConfig);
    TFT_voidInit();

    //USART_voidInit(USART1_INDEX);

    // IR Receiver -> PA1
    MGPIO_voidSetPinMode(PORTA, PIN1, INPUT);
    MGPIO_voidSetInputConfig(PORTA, PIN1, PULL_UP);

    // Setup IR AFTER TFT_voidInit() finishes its delays
    STK_voidSetCallBack(IR_voidSysTickTimeoutCallback);
    EXTI_voidInit(EXTI_Line1, EXTI_SYSCFG_PORTA, EXTI_TRIGGER_FALLING);
    EXTI_voidSetCallback(EXTI_Line1, IR_voidEXTI1Callback);
    NVIC_voidEnable(NVIC_EXTI1_IRQ);

    LEDs_voidInit();
    Buzzer_voidInit();
    MatrixSTP_voidInit();

    /* Show Welcome Screen at startup */
    TFT_voidShowModeScreen(0x0000, 0xFFFF, "WELCOME", "FOTA PROJ");

    /* =====================================================
     * Main Loop (Polling the Command Value)
     * ===================================================== */
    while (1)
    {
        if (Global_u8CommandValue != 0)
        {
            Global_u8LedStep = 0U;
            Buzzer_voidTriggerBeep();

            switch (Global_u8CommandValue)
            {
                /* Button 0 -> Turn OFF */
                case 22:
                    Global_s8CurrentNumber = 0;
                    TFT_voidShowModeScreen(0x0000, 0xFFFF, "SYSTEM", "LEDS OFF");
                    LEDs_voidTurnOffAll();
                    MatrixSTP_voidRefreshRow();
                    break;

                /* Button 1 -> Mode 1: Chaser */
                case 12:
                    Global_s8CurrentNumber = 1;
                    TFT_voidShowModeScreen(0xF800, 0xFFFF, "MODE 1", "CHASER");
                    LED_voidMode1_Chaser();
                    MatrixSTP_voidRefreshRow();
                    break;

                /* Button 2 -> Mode 2: Ping-Pong */
                case 24:
                    Global_s8CurrentNumber = 2;
                    TFT_voidShowModeScreen(0x07E0, 0x0000, "MODE 2", "PING PONG");
                    LED_voidMode2_PingPong();
                    MatrixSTP_voidRefreshRow();
                    break;

                /* Button 3 -> Mode 3: Sorting */
                case 94:
                    Global_s8CurrentNumber = 3;
                    TFT_voidShowModeScreen(0x001F, 0xFFFF, "MODE 3", "SORTING");
                    LED_voidMode3_Sorting();
                    MatrixSTP_voidRefreshRow();
                    break;

                /* Button 4 -> Mode 4: Bubbles */
                case 8:
                    Global_s8CurrentNumber = 4;
                    TFT_voidShowModeScreen(0xFFFF, 0x0000, "MODE 4", "BUBBLES");
                    LED_voidMode4_Bubbles();
                    MatrixSTP_voidRefreshRow();
                    break;

                /* Button 5 -> Mode 5: Blink */
                case 28:
                    Global_s8CurrentNumber = 5;
                    TFT_voidShowModeScreen(0xFFE0, 0x0000, "MODE 5", "BLINK");
                    LED_voidMode5_Blink();
                    MatrixSTP_voidRefreshRow();
                    break;

                case 90: Global_s8CurrentNumber = 6; break;
                case 66: Global_s8CurrentNumber = 7; break;
                case 82: Global_s8CurrentNumber = 8; break;
                case 74: Global_s8CurrentNumber = 9; break;

                /* Increment Button (VOL+) -> Code 21 */
                case 21:
                    if (Global_s8CurrentNumber < 5)
                    {
                        Global_s8CurrentNumber++;
                    }
                    break;

                /* Decrement Button (VOL-) -> Code 7 */
                case 7:
                    if (Global_s8CurrentNumber > 0)
                    {
                        Global_s8CurrentNumber--;
                    }
                    break;

                default:
                    /* Ignore unhandled buttons */
                    break;
            }

            Global_u8CommandValue = 0;
        }

        /* Non-blocking updates in main loop */
        LEDs_voidUpdateCurrentMode();
        Buzzer_voidUpdate();
        MatrixSTP_voidRefreshRow();
    }
}


//*********************************************************************************************************************************
/* =========================================================
 * EXTI1 Callback (Records Time between Falling Edges)
 * ========================================================= */
static void IR_voidEXTI1Callback(void)
{
    if (Global_u8StartFlag == 0U)
    {
        /* First Falling Edge: Enable Interrupt & Start timeout for 15,000 us */
        Global_u8StartFlag = 1U;
        STK_voidEnableInterrupt();
        STK_voidStartPeriodicInterrupt(15000UL * STK_TICKS_PER_US);
    }
    else
    {
        /* Store elapsed time, protect against Array out-of-bounds */
        if (Global_u8Counter < 40U)
        {
            Global_u32Arr[Global_u8Counter] = STK_u32GetElapsedTime();
            Global_u8Counter++;
        }

        /* Restart SysTick timeout for 4,000 us */
        STK_voidStartPeriodicInterrupt(4000UL * STK_TICKS_PER_US);
    }
}


/* =========================================================
 * SysTick Timeout Callback (Decodes the Array)
 * ========================================================= */
static void IR_voidSysTickTimeoutCallback(void)
{
    u8 Local_u8Iterator = 0;

    /* Stop SysTick Interrupt to make it act as a Single-Shot timer */
    STK_voidDisableInterrupt();

    /* Reset flags for the next IR frame */
    Global_u8StartFlag = 0U;
    Global_u8Counter   = 0U;

    /* Decode the Data (Command Bits are from index 17 to 24 in the array) */
    for (Local_u8Iterator = 0; Local_u8Iterator < 8; Local_u8Iterator++)
    {
        if ((Global_u32Arr[17 + Local_u8Iterator] >= IR_BIT0_MIN_TICKS) &&
            (Global_u32Arr[17 + Local_u8Iterator] <= IR_BIT0_MAX_TICKS))
        {
            CLR_BIT(Global_u8CommandValue, Local_u8Iterator);
        }
        else if ((Global_u32Arr[17 + Local_u8Iterator] >= IR_BIT1_MIN_TICKS) &&
                 (Global_u32Arr[17 + Local_u8Iterator] <= IR_BIT1_MAX_TICKS))
        {
            SET_BIT(Global_u8CommandValue, Local_u8Iterator);
        }
    }
}
//*********************************************************************************************************************************


/* =========================================================
 * 8 LEDs Functions (PB0 -> PB7 using GPIO directly)
 * ========================================================= */
static void LEDs_voidInit(void)
{
    u8 Local_u8Pin;
    for (Local_u8Pin = PIN0; Local_u8Pin <= PIN7; Local_u8Pin++)
    {
        MGPIO_voidSetPinMode(PORTB, (PINS)Local_u8Pin, OUTPUT);
        MGPIO_voidSetOutputConfig(PORTB, (PINS)Local_u8Pin, PUSH_PULL, MED_SPEED);
        MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8Pin, PIN_LOW);
    }
}

static void LEDs_voidTurnOffAll(void)
{
    u8 Local_u8Pin;
    for (Local_u8Pin = PIN0; Local_u8Pin <= PIN7; Local_u8Pin++)
    {
        MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8Pin, PIN_LOW);
    }
}

/* Mode 1: Chaser */
static void LED_voidMode1_Chaser(void)
{
    LEDs_voidTurnOffAll();
    MGPIO_voidSetPinValue(PORTB, (PINS)Global_u8LedStep, PIN_HIGH);
    Global_u8LedStep++;
    if (Global_u8LedStep > PIN7)
    {
        Global_u8LedStep = PIN0;
    }
}

/* Mode 2: Ping-Pong */
static void LED_voidMode2_PingPong(void)
{
    static u8 Local_u8Pin = PIN0;
    static u8 Local_u8Dir = 0U;
    if (Global_u8LedStep == 0U)
    {
        Local_u8Pin      = PIN0;
        Local_u8Dir      = 0U;
        Global_u8LedStep = 1U;
    }
    LEDs_voidTurnOffAll();
    MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8Pin, PIN_HIGH);
    if (Local_u8Dir == 0U)
    {
        if (Local_u8Pin < PIN7)
        {
            Local_u8Pin++;
        }
        else
        {
            Local_u8Dir = 1U;
            Local_u8Pin--;
        }
    }
    else
    {
        if (Local_u8Pin > PIN0)
        {
            Local_u8Pin--;
        }
        else
        {
            Local_u8Dir = 0U;
            Local_u8Pin++;
        }
    }
}

/* Mode 3: Sorting / Stacking */
static void LED_voidMode3_Sorting(void)
{
    static u8 Local_u8TargetPin = PIN7;
    static u8 Local_u8MovingPin = PIN0;
    u8 Local_u8Pin;
    if (Global_u8LedStep == 0U)
    {
        Local_u8TargetPin = PIN7;
        Local_u8MovingPin = PIN0;
        Global_u8LedStep  = 1U;
    }
    for (Local_u8Pin = PIN0; Local_u8Pin <= PIN7; Local_u8Pin++)
    {
        if (Local_u8Pin > Local_u8TargetPin)
        {
            MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8Pin, PIN_HIGH);
        }
        else
        {
            MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8Pin, PIN_LOW);
        }
    }
    MGPIO_voidSetPinValue(PORTB, (PINS)Local_u8MovingPin, PIN_HIGH);
    if (Local_u8MovingPin < Local_u8TargetPin)
    {
        Local_u8MovingPin++;
    }
    else
    {
        Local_u8MovingPin = PIN0;
        if (Local_u8TargetPin > PIN0)
        {
            Local_u8TargetPin--;
        }
        else
        {
            Global_u8LedStep = 0U;
        }
    }
}

/* Mode 4: Bubbles */
static void LED_voidMode4_Bubbles(void)
{
    LEDs_voidTurnOffAll();
    switch (Global_u8LedStep)
    {
        case 0:
            MGPIO_voidSetPinValue(PORTB, PIN3, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN4, PIN_HIGH);
            break;
        case 1:
            MGPIO_voidSetPinValue(PORTB, PIN2, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN5, PIN_HIGH);
            break;
        case 2:
            MGPIO_voidSetPinValue(PORTB, PIN1, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN6, PIN_HIGH);
            break;
        case 3:
            MGPIO_voidSetPinValue(PORTB, PIN0, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN7, PIN_HIGH);
            break;
        case 4:
            MGPIO_voidSetPinValue(PORTB, PIN1, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN6, PIN_HIGH);
            break;
        case 5:
            MGPIO_voidSetPinValue(PORTB, PIN2, PIN_HIGH);
            MGPIO_voidSetPinValue(PORTB, PIN5, PIN_HIGH);
            break;
        default:
            break;
    }
    Global_u8LedStep++;
    if (Global_u8LedStep >= 6U)
    {
        Global_u8LedStep = 0U;
    }
}

/* Mode 5: Blink / Alternate */
static void LED_voidMode5_Blink(void)
{
    if ((Global_u8LedStep % 2U) == 0U)
    {
        MGPIO_voidSetPinValue(PORTB, PIN0, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN1, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN2, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN3, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN4, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN5, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN6, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN7, PIN_LOW);
    }
    else
    {
        MGPIO_voidSetPinValue(PORTB, PIN0, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN1, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN2, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN3, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN4, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN5, PIN_HIGH);
        MGPIO_voidSetPinValue(PORTB, PIN6, PIN_LOW);
        MGPIO_voidSetPinValue(PORTB, PIN7, PIN_HIGH);
    }
    Global_u8LedStep ^= 1U;
}

static void LEDs_voidUpdateCurrentMode(void)
{
    Global_u32LedCounter++;
    if (Global_u32LedCounter >= LED_ANIM_SPEED)
    {
        Global_u32LedCounter = 0U;
        switch (Global_s8CurrentNumber)
        {
            case 1: LED_voidMode1_Chaser();   break;
            case 2: LED_voidMode2_PingPong(); break;
            case 3: LED_voidMode3_Sorting();  break;
            case 4: LED_voidMode4_Bubbles();  break;
            case 5: LED_voidMode5_Blink();    break;
            default:
                LEDs_voidTurnOffAll();
                break;
        }
    }
}


//*************************************************************************
/* =========================================================
 * Buzzer Functions (PA8)
 * ========================================================= */
static void Buzzer_voidInit(void)
{
    MGPIO_voidSetPinMode(BUZZER_PORT, BUZZER_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(BUZZER_PORT, BUZZER_PIN, PUSH_PULL, MED_SPEED);
    MGPIO_voidSetPinValue(BUZZER_PORT, BUZZER_PIN, PIN_LOW);
}

static void Buzzer_voidTriggerBeep(void)
{
    MGPIO_voidSetPinValue(BUZZER_PORT, BUZZER_PIN, PIN_HIGH);
    Global_u32BuzzerCounter = BUZZER_BEEP_TIME;
}

static void Buzzer_voidUpdate(void)
{
    if (Global_u32BuzzerCounter > 0U)
    {
        Global_u32BuzzerCounter--;
        if (Global_u32BuzzerCounter == 0U)
        {
            MGPIO_voidSetPinValue(BUZZER_PORT, BUZZER_PIN, PIN_LOW);
        }
    }
}


/* =========================================================
 * LED Matrix via STP Functions (PB12, PB13, PB15)
 * ========================================================= */
static void MatrixSTP_voidInit(void)
{
    MGPIO_voidSetPinMode(MATRIX_STP_PORT, MATRIX_SDI_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(MATRIX_STP_PORT, MATRIX_SDI_PIN, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetPinMode(MATRIX_STP_PORT, MATRIX_SHCP_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(MATRIX_STP_PORT, MATRIX_SHCP_PIN, PUSH_PULL, HIGH_SPEED);
    MGPIO_voidSetPinMode(MATRIX_STP_PORT, MATRIX_STCP_PIN, OUTPUT);
    MGPIO_voidSetOutputConfig(MATRIX_STP_PORT, MATRIX_STCP_PIN, PUSH_PULL, HIGH_SPEED);
}

static void MatrixSTP_voidSend16Bits(u8 Copy_u8Cols, u8 Copy_u8Rows)
{
    s8 Local_s8Bit;
    u16 Local_u16Frame = ((u16)Copy_u8Cols << 8) | Copy_u8Rows;
    for (Local_s8Bit = 15; Local_s8Bit >= 0; Local_s8Bit--)
    {
        if (GET_BIT(Local_u16Frame, Local_s8Bit) == 1U)
        {
            MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_SDI_PIN, PIN_HIGH);
        }
        else
        {
            MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_SDI_PIN, PIN_LOW);
        }
        MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_SHCP_PIN, PIN_LOW);
        MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_SHCP_PIN, PIN_HIGH);
    }
    MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_STCP_PIN, PIN_LOW);
    MGPIO_voidSetAtomicPinValue(MATRIX_STP_PORT, MATRIX_STCP_PIN, PIN_HIGH);
}

static void MatrixSTP_voidRefreshRow(void)
{
    static u8  Local_u8Row        = 0U;
    static u16 Local_u16RowHold   = 0U;
    u8 Local_u8Mode = Global_s8CurrentNumber;
    u8 Local_u8RowEnable;
    u8 Local_u8ColData;

    Local_u16RowHold++;
    if (Local_u16RowHold < 300U)
    {
        return;
    }
    Local_u16RowHold = 0U;

    if ((Local_u8Mode == 0U) || (Local_u8Mode > 5U))
    {
        MatrixSTP_voidSend16Bits(0x00U, 0x00U);
        return;
    }
    Local_u8RowEnable = (u8)(~(1U << Local_u8Row));
    Local_u8ColData   = Global_au8MatrixShapes[Local_u8Mode][Local_u8Row];

    MatrixSTP_voidSend16Bits(Local_u8ColData, Local_u8RowEnable);

    Local_u8Row++;
    if (Local_u8Row >= 8U)
    {
        Local_u8Row = 0U;
    }
}


/* =========================================================
 * TFT Screen Text Rendering Functions (5x7 Scaled Font)
 * ========================================================= */
static u8 TFT_u8GetGlyphCol(char Copy_cCh, u8 Copy_u8Col)
{
    u8 Local_au8Cols[5] = {0, 0, 0, 0, 0};

    switch (Copy_cCh)
    {
        case '0': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x3E; break;
        case '1': Local_au8Cols[0]=0x00; Local_au8Cols[1]=0x42; Local_au8Cols[2]=0x7F; Local_au8Cols[3]=0x40; Local_au8Cols[4]=0x00; break;
        case '2': Local_au8Cols[0]=0x62; Local_au8Cols[1]=0x51; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x46; break;
        case '3': Local_au8Cols[0]=0x22; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x36; break;
        case '4': Local_au8Cols[0]=0x1F; Local_au8Cols[1]=0x10; Local_au8Cols[2]=0x10; Local_au8Cols[3]=0x7F; Local_au8Cols[4]=0x10; break;
        case '5': Local_au8Cols[0]=0x27; Local_au8Cols[1]=0x45; Local_au8Cols[2]=0x45; Local_au8Cols[3]=0x45; Local_au8Cols[4]=0x39; break;

        case 'A': Local_au8Cols[0]=0x7E; Local_au8Cols[1]=0x11; Local_au8Cols[2]=0x11; Local_au8Cols[3]=0x11; Local_au8Cols[4]=0x7E; break;
        case 'B': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x36; break;
        case 'C': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x22; break;
        case 'D': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x3E; break;
        case 'E': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x41; break;
        case 'F': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x09; Local_au8Cols[3]=0x09; Local_au8Cols[4]=0x01; break;
        case 'G': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x3A; break;
        case 'H': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x08; Local_au8Cols[2]=0x08; Local_au8Cols[3]=0x08; Local_au8Cols[4]=0x7F; break;
        case 'I': Local_au8Cols[0]=0x41; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x7F; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x41; break;
        case 'J': Local_au8Cols[0]=0x20; Local_au8Cols[1]=0x40; Local_au8Cols[2]=0x40; Local_au8Cols[3]=0x3F; Local_au8Cols[4]=0x01; break;
        case 'K': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x08; Local_au8Cols[2]=0x14; Local_au8Cols[3]=0x22; Local_au8Cols[4]=0x41; break;
        case 'L': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x40; Local_au8Cols[2]=0x40; Local_au8Cols[3]=0x40; Local_au8Cols[4]=0x40; break;
        case 'M': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x02; Local_au8Cols[2]=0x0C; Local_au8Cols[3]=0x02; Local_au8Cols[4]=0x7F; break;
        case 'N': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x06; Local_au8Cols[2]=0x08; Local_au8Cols[3]=0x30; Local_au8Cols[4]=0x7F; break;
        case 'O': Local_au8Cols[0]=0x3E; Local_au8Cols[1]=0x41; Local_au8Cols[2]=0x41; Local_au8Cols[3]=0x41; Local_au8Cols[4]=0x3E; break;
        case 'P': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x09; Local_au8Cols[3]=0x09; Local_au8Cols[4]=0x06; break;
        case 'R': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x09; Local_au8Cols[2]=0x19; Local_au8Cols[3]=0x29; Local_au8Cols[4]=0x46; break;
        case 'S': Local_au8Cols[0]=0x26; Local_au8Cols[1]=0x49; Local_au8Cols[2]=0x49; Local_au8Cols[3]=0x49; Local_au8Cols[4]=0x32; break;
        case 'T': Local_au8Cols[0]=0x01; Local_au8Cols[1]=0x01; Local_au8Cols[2]=0x7F; Local_au8Cols[3]=0x01; Local_au8Cols[4]=0x01; break;
        case 'U': Local_au8Cols[0]=0x3F; Local_au8Cols[1]=0x40; Local_au8Cols[2]=0x40; Local_au8Cols[3]=0x40; Local_au8Cols[4]=0x3F; break;
        case 'W': Local_au8Cols[0]=0x7F; Local_au8Cols[1]=0x20; Local_au8Cols[2]=0x18; Local_au8Cols[3]=0x20; Local_au8Cols[4]=0x7F; break;
        case 'Y': Local_au8Cols[0]=0x07; Local_au8Cols[1]=0x08; Local_au8Cols[2]=0x70; Local_au8Cols[3]=0x08; Local_au8Cols[4]=0x07; break;
        default:  break;
    }

    return Local_au8Cols[Copy_u8Col];
}

static void TFT_voidDrawCharBuf(u8 Copy_u8X, u8 Copy_u8Y, char Copy_cCh, u16 Copy_u16Color, u8 Copy_u8Scale)
{
    u8 Local_u8Col, Local_u8Row, Local_u8Dx, Local_u8Dy;
    u8 Local_u8Bits;

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

static void TFT_voidShowModeScreen(u16 Copy_u16Bg, u16 Copy_u16Fg, const char *Copy_pcLine1, const char *Copy_pcLine2)
{
    u32 Local_u32Idx;
    for (Local_u32Idx = 0U; Local_u32Idx < (TFT_SCREEN_W * TFT_SCREEN_H); Local_u32Idx++)
    {
        Global_au16ScreenBuf[Local_u32Idx] = Copy_u16Bg;
    }

    TFT_voidDrawStrBuf(10U, 50U, Copy_pcLine1, Copy_u16Fg, 2U);
    TFT_voidDrawStrBuf(10U, 85U, Copy_pcLine2, Copy_u16Fg, 2U);

    TFT_voidDisplay(Global_au16ScreenBuf);
}
