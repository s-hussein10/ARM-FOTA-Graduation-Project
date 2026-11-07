#include "config.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "blynk_link.h"
#include "cmd_router.h"

// Receive button command codes from Blynk V10
BLYNK_WRITE(V10)   
{
    CMD_voidHandelCmd(param.asInt());
}

// Receive brightness slider value (0..100) from Blynk V11
BLYNK_WRITE(V11)  
{   
    CMD_voidHandelBrightnes(param.asInt());
}

void BlynkLink_voidInit(void)
{
    // Connect to WiFi and authenticate with Blynk cloud
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void BlynkLink_voidPoll(void)
{
    // Handle Blynk events and maintain connection
    Blynk.run();
}

void BlynkLink_voidSendStatus(const DeviceStatus* CopyPointerStatus, uint8_t Copy_u8Online)
{
    // Do nothing if disconnected from Blynk server
    if (!Blynk.connected()) 
    {
        return;
    }

    // Show OFFLINE on V0 if STM32 timed out
    if (Copy_u8Online == 0)
    {
        Blynk.virtualWrite(V0, "OFFLINE"); 
        return;
    }
    else if (Copy_u8Online == 1 && CopyPointerStatus != nullptr)
    {
        // Push telemetry values to Dashboard gauges/displays
        Blynk.virtualWrite(V0, stateName(CopyPointerStatus->state)); 
        Blynk.virtualWrite(V1, CopyPointerStatus->brightness);
        Blynk.virtualWrite(V2, CopyPointerStatus->progress);
        Blynk.virtualWrite(V3, CopyPointerStatus->err);
        return;
    }
}