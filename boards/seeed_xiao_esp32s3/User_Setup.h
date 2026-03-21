// ============================================================
// User_Setup.h — TFT_eSPI configuration for Seeed XIAO ESP32-S3
// with Seeed Studio Round Display (SKU 104030087, GC9A01 240×240)
//
// This file lives in boards/seeed_xiao_esp32s3/ which is already
// on the -I include path, so it shadows the library's default
// User_Setup.h automatically — no extra build flags needed.
// ============================================================

#define USER_SETUP_LOADED

// ── Display driver ────────────────────────────────────────
#define GC9A01_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ── SPI control pins (XIAO ESP32-S3 Dn → GPIO) ───────────
// D1=GPIO2  D3=GPIO4  D6=GPIO7
#define TFT_CS    D1   // GPIO 2 — chip select
#define TFT_DC    D3   // GPIO 4 — data/command
#define TFT_RST  -1    // tied to 3V3 on the Round Display PCB

// Backlight — driven HIGH to turn on
#define TFT_BL    D6   // GPIO 7
#define TFT_BACKLIGHT_ON HIGH

// ── SPI bus (hardware SPI2 on XIAO ESP32-S3) ─────────────
// SCK=D8/GPIO9  MOSI=D10/GPIO11  MISO=D9/GPIO10 (unused by display)
// TFT_eSPI auto-selects hardware SPI when MOSI/SCK are not overridden.

// ── Fonts ─────────────────────────────────────────────────
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4

// ── SPI speed ─────────────────────────────────────────────
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
