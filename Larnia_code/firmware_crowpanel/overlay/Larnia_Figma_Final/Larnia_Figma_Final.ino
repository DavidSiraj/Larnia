/**
 * Larnia_Figma_Final
 * CrowPanel Advanced 7" ESP32-P4 1024x600
 *
 * FINAL LOGIC:
 * - No demo profiles, no demo teachers, no demo materials.
 * - User creates profiles/teachers/materials.
 * - Avatars are stored on SD card under /avatars/avatar_01 ... avatar_08.
 * - Wi-Fi WebSocket connects to backend; OpenAI key stays in backend.
 * - UI follows the Figma visual system: white pages, #125B90, #82CEDC, Poppins-ready typography.
 */

#include "board_config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_err.h>
#include <esp_ldo_regulator.h>

#include "esp_panel_drivers_conf.h"
#include "esp_panel_board_custom_conf.h"
#include "ESP_Panel_Library.h"
#include <lvgl.h>
#include "lvgl_v8_port.h"

#include "LarniaStorage.h"
#include "LarniaUi.h"
#include "LarniaNetwork.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static Board* board = nullptr;
static LarniaStorage storage;
static LarniaUi ui;
static LarniaNetwork network;

void larniaSendChatFromUi(const String& teacherId, const String& question)
{
    network.sendChatQuestion(teacherId, question);
}

void larniaReconnectNetworkFromUi()
{
    network.reconnect();
}

static String serialLine;

static void setupP4PowerRails()
{
    esp_ldo_channel_handle_t ldo3 = NULL;
    esp_ldo_channel_config_t ldo3_cfg = {.chan_id = 3, .voltage_mv = 2500};
    esp_ldo_acquire_channel(&ldo3_cfg, &ldo3);

    esp_ldo_channel_handle_t ldo4 = NULL;
    esp_ldo_channel_config_t ldo4_cfg = {.chan_id = 4, .voltage_mv = 3300};
    esp_ldo_acquire_channel(&ldo4_cfg, &ldo4);
}

static void setupPanelAndLvgl()
{
    board = new Board();
    assert(board->init());
#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#endif
    assert(board->begin());
    assert(lvgl_port_init(board->getLCD(), board->getTouch()));
}

static void handleSerialJson(const char* payload, size_t length)
{
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, payload, length)) return;
    String type = doc["type"] | "";
    if (lvgl_port_lock(-1)) {
        ui.handleEvent(type, doc.as<JsonVariant>());
        lvgl_port_unlock();
    }
}

static void processSerial()
{
    while (Serial.available()) {
        char c = char(Serial.read());
        if (c == '\r') continue;
        if (c == '\n') {
            serialLine.trim();
            if (serialLine.length()) handleSerialJson(serialLine.c_str(), serialLine.length());
            serialLine = "";
        } else if (serialLine.length() < 3500) serialLine += c;
        else serialLine = "";
    }
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println("[Larnia] Final firmware booting...");

    setupP4PowerRails();
    setupPanelAndLvgl();

    storage.begin();
    storage.loadAll();
    // Final build is empty by default. Do not seed demo profiles/teachers/materials.

    if (lvgl_port_lock(-1)) {
        ui.begin(&storage);
        lvgl_port_unlock();
    }

    network.begin(&ui, &storage);
    Serial.println("[Larnia] Ready. USB Serial is only debug; final communication is Wi-Fi WebSocket.");
}

void loop()
{
    network.loop();
    processSerial();
    if (lvgl_port_lock(10)) {
        ui.update();
        lvgl_port_unlock();
    }
    delay(8);
}
