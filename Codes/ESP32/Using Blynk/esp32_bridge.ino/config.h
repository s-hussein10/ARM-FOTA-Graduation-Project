#pragma once

#define BLYNK_TEMPLATE_ID "TMPL2-gR991VP"
#define BLYNK_TEMPLATE_NAME "ESP32STM32FOTA"
#define BLYNK_AUTH_TOKEN "-rW-RKuBlfr0cT3_zu-aSL2X8GuHfN5P"
#define BLYNK_PRINT Serial          // debug UART0 

// MY WiFi credentials.
// Set password to "" for open networks.
constexpr const char* WIFI_SSID = "Seifwifi";
constexpr const char* WIFI_PASS = "12345678";

 // UART0 Pins (Serial Monitor & Flashing)
constexpr uint8_t  STM_RX_PIN = 16;
constexpr uint8_t  STM_TX_PIN = 17;

constexpr uint32_t STM_BAUD   = 115200;

constexpr uint32_t STATUS_SEND_MS = 1000;   
constexpr uint32_t STM_TIMEOUT_MS = 3000;
