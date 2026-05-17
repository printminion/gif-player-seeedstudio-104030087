"""
patch_tft_esp32c5.py — PlatformIO pre-build extra_script

TFT_eSPI's generic ESP32 processor files use several IDF3/IDF4 APIs that no
longer exist on ESP32-C5 (which ships with IDF 5.x).  Four classes of issue:

1. GPIO struct-register assignment (TFT_eSPI_ESP32.h)
   IDF5.x changed GPIO peripheral registers from plain `uint32_t` fields to
   typed C++ structs.  Bare integer assignment no longer compiles:
       GPIO.out_w1tc = (1 << TFT_DC)   // error: struct ≠ int
   Fix: access the underlying value through `.val`:
       GPIO.out_w1tc.val = (1 << TFT_DC)

2. VSPI constant missing in .h (TFT_eSPI_ESP32.h)
   Classic ESP32 had VSPI (=3) and HSPI (=2) SPI peripheral constants.
   ESP32-C5 only defines FSPI (=0); the `SPI_PORT` macro must be updated:
       #define SPI_PORT VSPI   →   #define SPI_PORT FSPI

3. VSPI / VSPI_HOST missing in .c (TFT_eSPI_ESP32.c)
   The .c file also uses VSPI directly for the Arduino SPIClass constructor
   and VSPI_HOST at the IDF spi_host_device_t layer:
       SPIClass(VSPI)      →  SPIClass(FSPI)
       VSPI_HOST           →  SPI2_HOST

4. SPI data-length register renamed (TFT_eSPI_ESP32.c)
   SPI_MOSI_DLEN_REG() was renamed to SPI_MS_DLEN_REG() in IDF5.x for newer
   chips (including C5).  The macro signature is identical; only the name
   changed.

This script runs as a PlatformIO `pre:` extra_script so it patches the files
inside .pio/libdeps/ before compilation begins.  It is idempotent — already-
patched files are left unchanged.
"""

Import("env")  # noqa: F821  (PlatformIO injects this)

import os

# Patches applied to TFT_eSPI_ESP32.h
_H_REPLACEMENTS = [
    # Fix 1: GPIO struct-register assignment
    ("GPIO.out_w1tc = (", "GPIO.out_w1tc.val = ("),
    ("GPIO.out_w1ts = (", "GPIO.out_w1ts.val = ("),
    # Fix 2: VSPI SPI_PORT macro → FSPI (the C5's SPI2 bus in Arduino)
    ("#define SPI_PORT VSPI", "#define SPI_PORT FSPI"),
]

# Patches applied to TFT_eSPI_ESP32.c
_C_REPLACEMENTS = [
    # Fix 3a: Arduino SPIClass constructor uses VSPI directly → FSPI
    ("SPIClass(VSPI)", "SPIClass(FSPI)"),
    # Fix 3b: IDF spi_host_device_t uses VSPI_HOST → SPI2_HOST
    ("VSPI_HOST", "SPI2_HOST"),
    # Fix 4: SPI data-length register renamed in IDF5.x
    ("SPI_MOSI_DLEN_REG(", "SPI_MS_DLEN_REG("),
]


def _patch_file(path: str, replacements: list) -> None:
    with open(path, encoding="utf-8") as fh:
        original = fh.read()

    patched = original
    for old, new in replacements:
        patched = patched.replace(old, new)

    if patched == original:
        print(f"[patch_tft_esp32c5] already up-to-date: {path}")
        return

    with open(path, "w", encoding="utf-8") as fh:
        fh.write(patched)
    print(f"[patch_tft_esp32c5] patched: {path}")


def _find_and_patch(env_obj) -> None:  # type: ignore[no-untyped-def]
    libdeps_dir = env_obj.subst("$PROJECT_LIBDEPS_DIR")
    env_name = env_obj.subst("$PIOENV")
    search_root = os.path.join(libdeps_dir, env_name)

    if not os.path.isdir(search_root):
        print(f"[patch_tft_esp32c5] libdeps dir not found: {search_root}")
        return

    for root, _dirs, files in os.walk(search_root):
        if "TFT_eSPI_ESP32.h" in files:
            _patch_file(os.path.join(root, "TFT_eSPI_ESP32.h"), _H_REPLACEMENTS)
        if "TFT_eSPI_ESP32.c" in files:
            _patch_file(os.path.join(root, "TFT_eSPI_ESP32.c"), _C_REPLACEMENTS)


_find_and_patch(env)  # noqa: F821
