#include "types.h"
#include "stm_link.h"
#include "cmd_router.h"

void CMD_voidHandelCmd(int Copy_s32Code)
{
    Command cmd;
    const char* CMD_NAME = nullptr;

    switch (Copy_s32Code)
    {
        case CMD_NONE:    return; // Ignore button release (0)
        case CMD_POWER:   CMD_NAME = "POWER"; break;
        case CMD_MODE:    CMD_NAME = "MODE";  break;
        case CMD_UP:      CMD_NAME = "UP";    break;
        case CMD_DOWN:    CMD_NAME = "DOWN";  break;
        case CMD_OK:      CMD_NAME = "OK";    break;
        case CMD_BACK:    CMD_NAME = "BACK";  break;
        case CMD_RESTART: 
            ESP.restart(); // Reboot ESP32
            return;

        default:
            // Handle numeric buttons NUM0..NUM9 (codes 10..19)
            if (Copy_s32Code >= CMD_NUM0 && Copy_s32Code <= CMD_NUM9)
            {
                int digit = Copy_s32Code - CMD_NUM0;
                snprintf(cmd.name, sizeof(cmd.name), "NUM%d", digit);
                cmd.hasValue = false;
                STMLink_voidSend(&cmd);
            }
            return;
    }

    // Send standard command if matched
    if (CMD_NAME != nullptr)
    {
        snprintf(cmd.name, sizeof(cmd.name), "%s", CMD_NAME);
        cmd.hasValue = false;
        STMLink_voidSend(&cmd);
    }
}

void CMD_voidHandelBrightnes(int Copy_s32Value)
{
    Command cmd;
    snprintf(cmd.name, sizeof(cmd.name), "BRIGHTNESS");
    cmd.value    = (int16_t)constrain(Copy_s32Value, 0, 100);
    cmd.hasValue = true;
    STMLink_voidSend(&cmd);
}