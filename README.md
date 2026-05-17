# GIF Player for Round Display for Seeed Studio XIAO 104030087 by @printminion

Animated GIF player for the [Seeed Studio XIAO ESP32-S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html?sensecap_affiliate=unUJe6L&referring_service=link)
with [1.28" Round Display (SKU 104030087)](https://www.seeedstudio.com/Seeed-Studio-Round-Display-for-XIAO-p-5638.html).
Copy GIF files to the SD card and they play automatically. Touch the screen to navigate.

## Flash via Web Installer

👉 **[Open Web Flasher](https://printminion.github.io/gif-player-seeedstudio-104030087)**
Flash firmware directly from your browser — no software required.

## Hardware

| Part                                 | SKU       |
|--------------------------------------|-----------|
| Seeed Studio XIAO ESP32-S3           | 113991114 |
| Seeed Studio Round Display for XIAO  | 104030087 |

## Getting Started

1. Flash firmware using the [Web Installer](https://printminion.github.io/gif-player-seeedstudio-104030087)
2. Insert a FAT32-formatted microSD card
3. Create a `/data` folder on the card
4. Copy `.gif` files into `/data`
5. Power on — GIFs play automatically

**Touch controls:**

- **Left arrow** — previous GIF
- **Right arrow** — next GIF
- **Mode button (bottom)** — toggle still / auto-play

## Firmware Variants

| Variant           | Description                            | PlatformIO environment     |
|-------------------|----------------------------------------|----------------------------|
| No WiFi (default) | GIF playback only — minimal build      | `seeed_xiao_esp32s3`       |
| WiFi              | GIF playback + WiFi provisioning + OTA | `seeed_xiao_esp32s3-wifi`  |

## Building Locally

Install [PlatformIO](https://platformio.org/) then:

```bash
# Build the default (no-wifi) variant
pio run -e seeed_xiao_esp32s3

# Build the WiFi variant
PLATFORMIO_BUILD_FLAGS="-D WIFI_AP_OPEN=1" pio run -e seeed_xiao_esp32s3-wifi

# Flash to connected device
pio run -e seeed_xiao_esp32s3 -t upload

# Monitor serial output
pio device monitor -b 115200
```

## Project Structure

```text
├── src/
│   ├── main.cpp          # GIF player logic, touch UI
│   ├── wifi.cpp          # WiFi provisioning (WiFi variant only)
│   ├── ota.cpp           # OTA updates (WiFi variant only)
│   └── version_check.cpp # Auto-update check (WiFi variant only)
├── boards/seeed_xiao_esp32s3/
│   ├── board_config.h    # Pin map and feature flags
│   └── User_Setup.h      # TFT_eSPI display config
├── docs/                 # Web installer (GitHub Pages)
├── project.json          # Board list, variants, installer config
└── platformio.ini        # Auto-generated from project.json
```

## CI/CD

- **Release** — push a `v*` tag → builds all variants, creates GitHub Release with `.bin` assets, deploys web installer to GitHub Pages
- **Dev** — push to `dev` branch → builds and deploys to the `dev/` channel

## Connect & Support

- 🐦 [@printminion](https://x.com/printminion)
- 🖨️ [3D Enclosures on Cults3D](https://cults3d.com/@printminion)
- ☕ [Buy Me a Coffee](https://buymeacoffee.com/printminion)

## License

MIT
