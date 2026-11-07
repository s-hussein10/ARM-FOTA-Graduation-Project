#include <WiFi.h>
#include <WebServer.h>
#include "OUT.h"

/* =========================================================
 * 1) Wi-Fi Credentials
 * ========================================================= */
const char* WIFI_SSID = "Eng.Seif";
const char* WIFI_PASS = "seif258258";

/* =========================================================
 * 2) UART2 Configuration (Connected to STM32 USART1)
 *    ESP32 RX2 (GPIO 16) <--- STM32 TX1 (PA9)
 *    ESP32 TX2 (GPIO 17) ---> STM32 RX1 (PA10)
 *    ESP32 GND           <--> STM32 GND
 * ========================================================= */
#define STM32_UART_BAUD   9600
#define ESP_RX2_PIN       16
#define ESP_TX2_PIN       17

/* Automatically calculate number of HEX lines in arr[] */
const int HEX_LINES_COUNT = sizeof(arr) / sizeof(arr[0]);

/* FOTA Non-Blocking State Machine */
enum FotaState {
    FOTA_IDLE,
    FOTA_ERASING,
    FOTA_SENDING,
    FOTA_DONE,
    FOTA_ERROR
};

WebServer server(80);
FotaState fotaState      = FOTA_IDLE;
int       fotaLineIndex  = 0;
int       fotaProgress   = 0;
String    fotaMessage    = "IDLE";
String    lastStmStatus  = "WAITING FOR STM32...";

/* =========================================================
 * Helper: Read newline-terminated response from STM32
 * ========================================================= */
String readFromSTM32(uint32_t timeoutMs)
{
    String response = "";
    uint32_t startTime = millis();

    while ((millis() - startTime) < timeoutMs)
    {
        while (Serial2.available() > 0)
        {
            char c = (char)Serial2.read();
            if (c == '\n')
            {
                response.trim();
                if (response.length() > 0)
                {
                    lastStmStatus = response;
                }
                return response;
            }
            else if (c != '\r')
            {
                response += c;
            }
        }
        server.handleClient();
        delay(1);
    }
    return response;
}

/* =========================================================
 * Endpoint 1: GET /status
 * Returns current STM32 status / active LED Mode
 * ========================================================= */
void handleStatus()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (fotaState == FOTA_IDLE || fotaState == FOTA_DONE)
    {
        while (Serial2.available() > 0)
        {
            readFromSTM32(25);
        }
    }
    server.send(200, "text/plain", lastStmStatus);
}

/* =========================================================
 * Endpoint 2: GET /cmd?val=<X>
 * Sends Mode ('0'..'5'), Enter Bootloader ('U'), or Jump ('J')
 * ========================================================= */
void handleCommand()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (fotaState == FOTA_ERASING || fotaState == FOTA_SENDING)
    {
        server.send(409, "text/plain", "BUSY: FOTA IN PROGRESS");
        return;
    }

    if (!server.hasArg("val"))
    {
        server.send(400, "text/plain", "ERROR: Missing 'val' parameter");
        return;
    }

    String val = server.arg("val");
    val.trim();

    while (Serial2.available() > 0)
    {
        Serial2.read();
    }

    Serial2.print(val);
    Serial.println("[ESP32 -> STM32] CMD: " + val);

    String reply = readFromSTM32(1500);
    if (reply.length() == 0)
    {
        reply = lastStmStatus;
    }

    server.send(200, "text/plain", reply);
}

/* =========================================================
 * Endpoint 3: GET /start_fota
 * Triggers flashing of arr[] from ESP32 Flash to STM32 Bootloader
 * ========================================================= */
void handleStartFota()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (fotaState == FOTA_ERASING || fotaState == FOTA_SENDING)
    {
        server.send(200, "text/plain", "FOTA ALREADY RUNNING");
        return;
    }

    fotaLineIndex = 0;
    fotaProgress  = 0;
    fotaMessage   = "ERASING STM32 APPLICATION FLASH...";
    fotaState     = FOTA_ERASING;

    Serial.println("[FOTA] Update triggered from Node-RED Dashboard!");
    server.send(200, "text/plain", "FOTA STARTED (" + String(HEX_LINES_COUNT) + " lines)");
}

/* =========================================================
 * Endpoint 4: GET /fota_progress
 * Returns JSON progress for Node-RED Dashboard Progress Bar
 * ========================================================= */
void handleFotaProgress()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");

    String stateStr = "IDLE";
    if (fotaState == FOTA_ERASING) stateStr = "ERASING";
    else if (fotaState == FOTA_SENDING) stateStr = "SENDING";
    else if (fotaState == FOTA_DONE) stateStr = "DONE";
    else if (fotaState == FOTA_ERROR) stateStr = "ERROR";

    String json = "{\"state\":\"" + stateStr + "\","
                  "\"progress\":" + String(fotaProgress) + ","
                  "\"line\":" + String(fotaLineIndex) + ","
                  "\"total\":" + String(HEX_LINES_COUNT) + ","
                  "\"message\":\"" + fotaMessage + "\"}";

    server.send(200, "application/json", json);
}

/* =========================================================
 * Process one step of FOTA inside loop()
 * ========================================================= */
void processFotaStep()
{
    if (fotaState == FOTA_ERASING)
    {
        /* Flush UART RX buffer before sending Erase command 'E' */
        while (Serial2.available() > 0) { Serial2.read(); }

        Serial.println("[FOTA] Sending 'E' to erase STM32 Flash Sectors 2..5...");
        Serial2.print('E');

        String ack = readFromSTM32(7000);
        if (ack.indexOf("OK") >= 0)
        {
            Serial.println("[FOTA] Flash Erased! Streaming HEX array...");
            fotaState     = FOTA_SENDING;
            fotaLineIndex = 0;
            fotaProgress  = 0;
            fotaMessage   = "FLASHING: 0%";
        }
        else
        {
            fotaState   = FOTA_ERROR;
            fotaMessage = "ERASE FAILED! Make sure STM32 is in Bootloader. Reply: " + ack;
            Serial.println("[FOTA ERROR] " + fotaMessage);
        }
    }
    else if (fotaState == FOTA_SENDING)
    {
        if (fotaLineIndex < HEX_LINES_COUNT)
        {
            while (Serial2.available() > 0) { Serial2.read(); }

            /* Send one Intel HEX record from arr[] to STM32 Bootloader */
            Serial2.print(arr[fotaLineIndex]);
            Serial2.print('\n');

            String ack = readFromSTM32(3000);
            if (ack.indexOf("OK") >= 0)
            {
                fotaLineIndex++;
                fotaProgress = (fotaLineIndex * 100) / HEX_LINES_COUNT;
                fotaMessage  = "FLASHING: " + String(fotaProgress) + "% (" +
                               String(fotaLineIndex) + "/" + String(HEX_LINES_COUNT) + ")";

                if ((fotaLineIndex % 50 == 0) || (fotaLineIndex == HEX_LINES_COUNT))
                {
                    Serial.println("[FOTA] " + fotaMessage);
                }
            }
            else
            {
                fotaState   = FOTA_ERROR;
                fotaMessage = "ERROR at line " + String(fotaLineIndex + 1) + ": " + ack;
                Serial.println("[FOTA ERROR] " + fotaMessage);
            }
        }
        else
        {
            fotaState     = FOTA_DONE;
            fotaProgress  = 100;
            fotaMessage   = "FOTA 100% COMPLETE - MAIN APP STARTED!";
            lastStmStatus = "APP RUNNING - WELCOME";
            Serial.println("[FOTA] 100% Complete! STM32 jumped to Main Application.");
        }
    }
}

/* =========================================================
 * Setup
 * ========================================================= */
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32 STARTED ===");
    Serial.flush();

    Serial2.begin(STM32_UART_BAUD, SERIAL_8N1, ESP_RX2_PIN, ESP_TX2_PIN);
    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi Connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Loaded HEX Array Lines: " + String(HEX_LINES_COUNT));

    /* Register HTTP Routes for Node-RED Dashboard */
    server.on("/status",        HTTP_GET, handleStatus);
    server.on("/cmd",           HTTP_GET, handleCommand);
    server.on("/start_fota",    HTTP_GET, handleStartFota);
    server.on("/fota_progress", HTTP_GET, handleFotaProgress);

    server.begin();
    Serial.println("HTTP Server Ready on Port 80");
}

/* =========================================================
 * Main Loop
 * ========================================================= */
void loop()
{
    server.handleClient();

    if (fotaState == FOTA_ERASING || fotaState == FOTA_SENDING)
    {
        processFotaStep();
    }
    else if (Serial2.available() > 0)
    {
        String msg = readFromSTM32(40);
        if (msg.length() > 0)
        {
            Serial.println("[STM32 -> ESP32] " + msg);
        }
    }
}