#include "types.h"
#include "stm_link.h"
#include "config.h"

// Internal module state
static DeviceStatus S_Status;
static uint8_t S_NewStatus = 0;
static char S_Buff[128];
static uint8_t S_BuffIndex = 0;

void STMLink_voidInit(void)
{
    // Start UART2 on configured RX/TX pins
    Serial2.begin(STM_BAUD, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);
    S_BuffIndex = 0;
    S_Status.valid = false;
}

// Helper function to parse JSON line: {"st":1,"b":50,"m":0,"p":0,"e":0}
static void parseLine(const char* Copy_pcLine)
{
    int st, b, m, p, e;

    // Extract 5 integers matching the fixed JSON format
    if (sscanf(Copy_pcLine, "{\"st\":%d,\"b\":%d,\"m\":%d,\"p\":%d,\"e\":%d}", &st, &b, &m, &p, &e) == 5)
    {
        // Clamp values to valid ranges and store in S_Status
        S_Status.state        = (uint8_t)constrain(st, 0, 4);
        S_Status.brightness   = (uint8_t)constrain(b, 0, 100);
        S_Status.menu         = (uint8_t)constrain(m, 0, 255);
        S_Status.progress     = (uint8_t)constrain(p, 0, 100);
        S_Status.err          = (uint8_t)constrain(e, 0, 255);
        
        S_Status.lastUpdateMs = millis(); // Record arrival timestamp
        S_Status.valid        = true;     // Mark status as valid
        S_NewStatus           = 1;        // Notify caller of new data
    }
}

void STMLink_voidPoll(void)
{
    // Read all available bytes from UART2 without blocking
    while (Serial2.available() > 0)
    {
        char c = (char)Serial2.read();
        if (c == '\r')
        {
            continue; // Ignore carriage return
        }
        if (c == '\n')
        {
            S_Buff[S_BuffIndex] = '\0'; // Null-terminate string
            S_BuffIndex = 0;            // Reset buffer index for next line
            parseLine(S_Buff);          // Parse complete line
        }
        else
        {
            // Store character if space remains, otherwise reset on overflow
            if (S_BuffIndex < sizeof(S_Buff) - 1)
            {
                S_Buff[S_BuffIndex++] = c;
            }
            else
            {
                S_BuffIndex = 0;
            }
        }
    }
}

uint8_t STMLink_u8TakeStatus(DeviceStatus* Copy_pstStatus)
{
    // Check if a new status frame was received
    if (S_NewStatus == 1)
    {
        *Copy_pstStatus = S_Status; // Copy struct to caller pointer
        S_NewStatus = 0;            // Clear flag
        return 1;
    }

    return 0;
}

uint8_t STMLink_u8Connect(void)
{
    // Online if at least one valid frame arrived within timeout window
    if (S_Status.valid == true && (millis() - S_Status.lastUpdateMs) < STM_TIMEOUT_MS)
    {
        return 1;
    }
    return 0;
}

void STMLink_voidSend(const Command* Copy_pstCmd)
{
    if (Copy_pstCmd == nullptr) 
    {
        return;
    }
    
    // Format and send JSON command with or without value
    if (Copy_pstCmd->hasValue == true)
    {
        Serial2.printf("{\"c\":\"%s\",\"v\":%d}\n", Copy_pstCmd->name, Copy_pstCmd->value);
    }
    else
    {
        Serial2.printf("{\"c\":\"%s\"}\n", Copy_pstCmd->name);
    }
}