#include "config.h"
#include "types.h"
#include "stm_link.h"
#include "blynk_link.h"

// Global state tracking
static DeviceStatus G_State;
static uint32_t G_LastSendMs = 0;

void setup()
{
    Serial.begin(115200); // Initialize UART0 for USB debugging
    STMLink_voidInit();   // Initialize UART2 for STM32 link
    BlynkLink_voidInit(); // Connect to WiFi and Blynk server
}

void loop()
{
    STMLink_voidPoll();   // Read and parse incoming STM32 frames
    BlynkLink_voidPoll(); // Process Blynk cloud events

    // Update G_State whenever a new frame arrives from STM32
    STMLink_u8TakeStatus(&G_State);

    // Periodically push status to Blynk every STATUS_SEND_MS (1000 ms)
    if (millis() - G_LastSendMs >= STATUS_SEND_MS)
    {
        G_LastSendMs = millis();
        uint8_t IsOnline = STMLink_u8Connect();
        BlynkLink_voidSendStatus(&G_State, IsOnline);
    }
}