// ============================================================
// wifi.cpp — WiFi provisioning via captive portal (WiFiManager)
// ============================================================
#include "board_config.h"
#ifdef FEATURE_WIFI_PROVISIONING

#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <Preferences.h>
#include "logger.h"

// Provisioning AP security policy:
// - WIFI_AP_PASSWORD (min 8 chars): password-protected AP — recommended for production.
// - WIFI_AP_OPEN=1: explicit opt-in to an open (password-less) AP — not for production.
// Defining WIFI_AP_PASSWORD alongside WIFI_AP_OPEN=1 is a build error.
// Release builds (NDEBUG) with neither flag also error, to prevent accidentally
// shipping an open portal.
#if defined(WIFI_AP_PASSWORD) && WIFI_AP_OPEN
  #error "Define either WIFI_AP_PASSWORD or WIFI_AP_OPEN=1, not both."
#elif defined(WIFI_AP_PASSWORD)
  // ESP32 SoftAP requires a minimum password length of 8 characters.
  static_assert(sizeof(WIFI_AP_PASSWORD) - 1 >= 8,
                "WIFI_AP_PASSWORD must be at least 8 characters (ESP32 SoftAP minimum).");
#elif WIFI_AP_OPEN
  // Explicit opt-in: open AP accepted (WIFI_AP_OPEN != 0), warn at compile time.
  #pragma message("WIFI_AP_OPEN=1 — provisioning AP has no password. " \
                  "Set -D WIFI_AP_PASSWORD='\"yourpassword\"' to secure the captive portal.")
#elif defined(NDEBUG)
  // Release build with no AP policy set: fail loudly rather than silently ship an open portal.
  #error "Release build: set WIFI_AP_PASSWORD in build_flags to secure the provisioning AP, " \
         "or add WIFI_AP_OPEN=1 to explicitly allow an open AP."
#else
  // Debug build, no flag set: open AP allowed with a warning.
  #pragma message("WIFI_AP_PASSWORD not set — provisioning AP will be open. " \
                  "Set -D WIFI_AP_PASSWORD='\"yourpassword\"' to secure the captive portal.")
#endif

static WiFiManager wm;

static constexpr int kPortalTimeoutSec = 180;

// Builds the provisioning AP name: "ESP32-<version>-<mac>"
static String computeApName() {
  uint8_t macBytes[6] = {};
  esp_err_t macErr = esp_read_mac(macBytes, ESP_MAC_WIFI_STA);
  if (macErr != ESP_OK) {
    LOGF_STATUS("WiFi: esp_read_mac failed (0x%x) — MAC suffix will be 000000000000", (unsigned)macErr);
  }
  char macSuffix[13];
  snprintf(macSuffix, sizeof(macSuffix), "%02x%02x%02x%02x%02x%02x",
           (unsigned)macBytes[0], (unsigned)macBytes[1], (unsigned)macBytes[2],
           (unsigned)macBytes[3], (unsigned)macBytes[4], (unsigned)macBytes[5]);
  // WiFi SSIDs are limited to 32 bytes. Cap the version portion so the full
  // 12-char MAC suffix (which guarantees uniqueness) is always preserved.
  // Layout: "ESP32-" (6) + version (≤13) + "-" (1) + mac (12) = ≤32
  constexpr int kMaxVersionLen = 32 - 6 - 1 - 12;  // 13
  String versionStr = String(FIRMWARE_VERSION);
  if (versionStr.length() > kMaxVersionLen) versionStr = versionStr.substring(0, kMaxVersionLen);
  return String("ESP32-") + versionStr + "-" + macSuffix;
}

// Returns the AP name that will be used for provisioning (callable before setupWifi).
String wifiGetApName() {
  return computeApName();
}

// Returns true if WiFi has ever successfully connected on this device.
// Backed by Preferences — immune to stale NVS entries from other sketches.
bool wifiHasSavedCredentials() {
  Preferences prefs;
  if (!prefs.begin("wifi-state", true)) {
    return false;  // namespace not yet created — never configured
  }
  bool configured = prefs.getBool("configured", false);
  prefs.end();
  return configured;
}

static void markWifiConfigured() {
  Preferences prefs;
  prefs.begin("wifi-state", false);
  prefs.putBool("configured", true);
  prefs.end();
}

static void (*sApClientConnectedCb)(void) = nullptr;

// Register a callback invoked when a client connects to the provisioning AP.
void wifiSetApClientCallback(void (*cb)(void)) {
  sApClientConnectedCb = cb;
}

// Starts WiFi — connects to saved credentials, or starts captive portal if none are saved.
// Returns true if connected, false if not connected (portal timed out or skipped).
bool setupWifi() {
  // Uncomment to reset saved credentials during development:
  // wm.resetSettings();

  // Use 192.168.99.x — avoids conflict with common home-router subnets (192.168.4.x)
  // WiFi.softAPConfig() must be called before wm.autoConnect() so the AP interface
  // is configured before the WiFi stack starts (wm.setAPStaticIPConfig() alone was
  // found to prevent the AP from starting on this hardware).
  WiFi.softAPConfig(
      IPAddress(192, 168, 99, 1),
      IPAddress(192, 168, 99, 1),
      IPAddress(255, 255, 255, 0)
  );

  wm.setConfigPortalTimeout(kPortalTimeoutSec);
  wm.setConnectTimeout(30);
  wm.setWiFiAPChannel(6);  // channel 6 — most universally scanned by phones

  // Disable captive-portal DNS redirect.
  // WiFiManager's DNS server redirects every domain lookup to 192.168.99.1.
  // On Windows, NCSI probes (msftconnecttest.com etc.) get redirected and hit the
  // WebServer mid-transfer. The WebServer is single-client: it drops the in-progress
  // portal response to serve the NCSI request, stalling the HTML body.
  // Without DNS redirect, NCSI probes time out at DNS level and never reach the server.
  // Users navigate manually to http://192.168.99.1 — the instruction screen shows this.
  wm.setCaptivePortalEnable(false);

  // Apply sleep-disable and TX power after the AP starts (esp_wifi_set_ps /
  // esp_wifi_set_max_tx_power require the WiFi stack to be running).
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
  }, ARDUINO_EVENT_WIFI_AP_START);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    LOGF_STATUS("WiFi: client connected to AP, MAC=%02x:%02x:%02x:%02x:%02x:%02x",
      info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
      info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
      info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
    if (sApClientConnectedCb) sApClientConnectedCb();
  }, ARDUINO_EVENT_WIFI_AP_STACONNECTED);

  String apName = computeApName();

  LOG_STATUS("-- WiFi Setup ------------------------------------------");
  LOGF_STATUS("Connect to WiFi AP : %s", apName.c_str());
  LOG_STATUS("Then open          : http://192.168.99.1");
  LOGF_STATUS("Portal closes in   : %d min", kPortalTimeoutSec / 60);
#ifndef WIFI_AP_PASSWORD
  LOG_STATUS("WARNING: provisioning AP has no password — not for production.");
#endif
  LOG_STATUS("--------------------------------------------------------");

#ifdef WIFI_AP_PASSWORD
  bool connected = wm.autoConnect(apName.c_str(), WIFI_AP_PASSWORD);
#else
  bool connected = wm.autoConnect(apName.c_str());
#endif

  if (!connected) {
    LOG_STATUS("WiFi: failed to connect.");
    return false;
  }

  LOGF_STATUS("WiFi: connected, IP=%s", WiFi.localIP().toString().c_str());
  markWifiConfigured();
  return true;
}

#endif // FEATURE_WIFI_PROVISIONING
