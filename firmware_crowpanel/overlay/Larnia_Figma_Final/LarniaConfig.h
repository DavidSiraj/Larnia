#pragma once

// Larnia final firmware configuration for CrowPanel Advanced 7" ESP32-P4 1024x600.
// Fill these before upload, or send runtime settings from backend later.

#define LARNIA_WIFI_SSID        "Larnia"
#define LARNIA_WIFI_PASSWORD    "larnia1234"
#define LARNIA_BACKEND_HOST     "192.168.137.1"
#define LARNIA_BACKEND_PORT     3000
#define LARNIA_WS_PATH          "/crowpanel"

#define LARNIA_SCREEN_WIDTH     1024
#define LARNIA_SCREEN_HEIGHT    600
#define LARNIA_AUTO_SLEEP_MS    120000UL

#define LARNIA_SD_AVATAR_ROOT   "/avatars"
#define LARNIA_SD_UI_ROOT       "/ui"
