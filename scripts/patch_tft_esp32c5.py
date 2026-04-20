"""
patch_tft_esp32c5.py — PlatformIO pre-build extra_script

ESP-IDF 5.x changed several GPIO peripheral registers from plain `uint32_t`
fields to typed C++ structs (e.g. gpio_out_w1tc_reg_t).  TFT_eSPI's generic
ESP32 processor file (TFT_eSPI_ESP32.h) still uses bare integer assignment:

    GPIO.out_w1tc = (1 << TFT_DC)   // fails on C5: struct ≠ int

The fix is to access the underlying 32-bit value via the `.val` member:

    GPIO.out_w1tc.val = (1 << TFT_DC)

This script runs as a PlatformIO `pre:` extra_script so it patches the file
inside .pio/libdeps/ before compilation begins.  It is idempotent — already-
patched files are left unchanged.
"""

Import("env")  # noqa: F821  (PlatformIO injects this)

import os

_REPLACEMENTS = [
    ("GPIO.out_w1tc = (", "GPIO.out_w1tc.val = ("),
    ("GPIO.out_w1ts = (", "GPIO.out_w1ts.val = ("),
]


def _patch_file(path: str) -> None:
    with open(path, encoding="utf-8") as fh:
        original = fh.read()

    patched = original
    for old, new in _REPLACEMENTS:
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
            _patch_file(os.path.join(root, "TFT_eSPI_ESP32.h"))


_find_and_patch(env)  # noqa: F821
