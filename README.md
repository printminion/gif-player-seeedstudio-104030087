# GIF Player for Seeed Studio Round Display (104030087)

[![Web Installer](https://img.shields.io/badge/Web%20Installer-Flash%20Now-blue?style=for-the-badge&logo=googlechrome)](https://printminion.github.io/gif-player-seeedstudio-104030087/)

> 🚀 **[Flash the firmware directly from your browser — no software required!](https://printminion.github.io/gif-player-seeedstudio-104030087/)**

GIF animation player for the [Seeed Studio 1.28" Round Display](https://www.seeedstudio.com/Seeed-Studio-Round-Display-for-XIAO-p-5638.html) (240×240 pixels) paired with a Seeed XIAO ESP32 board. Touch the screen to cycle through GIF animations stored on the SD card. Based on [seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch](https://github.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch).

## Demo Videos

- ▶️ [Overview](https://www.youtube.com/watch?v=ahHhqOX7bnU&list=PLFmnthuksGmygz3sO5g7YNrulodxvZLMR&index=1)
- ▶️ [Disassembly](https://www.youtube.com/watch?v=CCpf--G0LIk&list=PLFmnthuksGmygz3sO5g7YNrulodxvZLMR&index=2)
- ▶️ [Assembly](https://www.youtube.com/watch?v=qn8rkIOUXXI&list=PLFmnthuksGmygz3sO5g7YNrulodxvZLMR&index=3)

## 3D Printed Enclosure

Print your own enclosure for the Seeed Studio Round Display and XIAO ESP32 board:

- 🖨️ [Collection of cases for Seeed Studio Round Display on Cults3D](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
- 🖨️ [All printminion designs on Cults3D](https://cults3d.com/@printminion)

[![Case variant 1](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/01_104030087_drxO1-bump_legs-v32.png)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
[![Case variant 2](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/02-104030087_drxO1-short_legs-v32.png)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
[![Case variant 3](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/03-104030087_drxO1-straight_long_legs-v32.png)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
[![Case variant 4](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/04-104030087_drxO1-scary_finger_legs-v32.png)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)

[![Photo 1](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/PXL_20230611_171226277.jpg)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
[![Photo 2](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/PXL_20230611_171325293.jpg)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
[![Photo 3](https://raw.githubusercontent.com/printminion/seeedstudio-xiao-TFT_eSPI_GifPlayer_With_Touch/main/assets/PXL_20230611_171341001.jpg)](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)

### Parts list

- [Seeed Studio Round Display for XIAO — 1.28" round touch screen, 240×240, 65k colors, RTC, TF card slot](https://www.seeedstudio.com/Seeed-Studio-Round-Display-for-XIAO-p-5638.html)
- [Seeed XIAO ESP32C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) or [Seeed XIAO ESP32S3](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html)
- [Screws](https://amzn.to/3CkXUDs)
- [USB-C Adapter Cable (CMUP-CF, 10CM)](https://de.aliexpress.com/item/1005005416767887.html)
- For battery assembly:
  - [Battery LP852040](https://amzn.to/3J7gliF)
  - [Switch SS12D00 3pin](https://amzn.to/43yqVYp)
  - [Connector MX1,25 male 2P](https://amzn.to/43zPvb9)
  - [Connector for battery PG 2.0 female 2P](https://amzn.to/3N3io8B)

## Supported Boards

| Board               | Environment            |
| ------------------- | ---------------------- |
| Seeed XIAO ESP32-C3 | `seeed_xiao_esp32c3`   |
| Seeed XIAO ESP32-S3 | `seeed_xiao_esp32s3`   |
| Seeed XIAO ESP32-C6 | `seeed_xiao_esp32c6`   |
| Generic ESP32       | `generic_esp32`        |

Each board has a matching `-debug` environment with verbose serial logging enabled.

## Features

- **GIF player** — plays GIF animations from a microSD card on the round display
- **Touch control** — tap the screen to advance to the next GIF
- **WiFi provisioning** — captive portal via [WiFiManager](https://github.com/tzapu/WiFiManager) on first boot
- **OTA updates** — browser-based firmware upload at `/update` via [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- **Auto-update** — checks GitHub Releases on boot and applies updates automatically
- **Web installer** — flash directly from your browser at the [installer page](https://printminion.github.io/gif-player-seeedstudio-104030087/)

## Building

Install [PlatformIO](https://platformio.org/) then:

```bash
# Build all environments
PLATFORMIO_BUILD_FLAGS="-D WIFI_AP_OPEN=1" pio run

# Build a specific release board
PLATFORMIO_BUILD_FLAGS="-D WIFI_AP_OPEN=1" pio run -e seeed_xiao_esp32c3

# Build debug variant (verbose serial logging)
pio run -e seeed_xiao_esp32c3-debug

# Upload to connected board
PLATFORMIO_BUILD_FLAGS="-D WIFI_AP_OPEN=1" pio run -e seeed_xiao_esp32c3 -t upload

# Monitor serial output
pio device monitor -b 115200
```

## CI/CD

- **Release build** — push a tag like `v1.0.0` → GitHub Actions builds all environments, creates a GitHub Release with `.bin` assets, and deploys the web installer to GitHub Pages.
- **Dev build** — push to the `dev` branch → builds all environments and deploys to the `dev/` channel on GitHub Pages.

## Connect & Support

- 🐦 **Follow on X/Twitter:** [@printminion](https://x.com/printminion)
- 🖨️ **3D Enclosures:** [Cults3D collection for Seeed Studio Round Display](https://cults3d.com/en/design-collections/printminion/seeed-studio-round-display-for-xiao-1-28-inch-round-touch-screen-240x240)
- 🖨️ **All 3D designs:** [printminion on Cults3D](https://cults3d.com/@printminion)
- ☕ **Buy Me a Coffee:** [buymeacoffee.com/printminion](https://buymeacoffee.com/printminion)

## License

MIT
