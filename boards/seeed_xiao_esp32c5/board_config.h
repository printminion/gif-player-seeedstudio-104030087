#pragma once

// ============================================================
// board_config.h — Seeed XIAO ESP32-C5
// ============================================================

// Onboard LED (active LOW on XIAO C5)
#define LED_PIN           27
#define LED_ACTIVE_LOW    true

// UART
#define SERIAL_BAUD       115200

// Feature flags
#define FEATURE_WIFI_PROVISIONING
#define FEATURE_OTA
#define FEATURE_VERSION_CHECK

// ============================================================
// Peripheral pin map — fill in for your hardware
//
// Seeed XIAO ESP32-C5 GPIO aliases:
//   D0=1  D1=0  D2=25 D3=7  D4=23 D5=24
//   D6=11 D7=12 D8=8  D9=9  D10=10
//
// SPI bus (hardware SPI)
// #define PIN_SPI_SCK     8   // D8
// #define PIN_SPI_MOSI   10   // D10
// #define PIN_SPI_MISO    9   // D9
//
// SPI device chip-selects (one per device)
// #define PIN_DISPLAY_CS  0   // D1
// #define PIN_SD_CS      25   // D2
//
// Display control lines
// #define PIN_DISPLAY_DC  7   // D3
// #define PIN_DISPLAY_RST -1  // -1 if tied to 3V3 (no reset needed)
// #define PIN_DISPLAY_BL 11   // D6 — backlight PWM
//
// I2C bus (sensors, touch controllers, etc.)
// #define PIN_I2C_SDA    23   // D4
// #define PIN_I2C_SCL    24   // D5
// #define PIN_I2C_INT    12   // D7 — interrupt line (optional)
// ============================================================
