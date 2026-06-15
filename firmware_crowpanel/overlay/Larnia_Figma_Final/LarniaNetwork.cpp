#include "LarniaNetwork.h"
#include "LarniaConfig.h"
#include <WiFi.h>

LarniaNetwork* LarniaNetwork::instance = nullptr;

static String cfgSsid(LarniaStorage* storage)
{
    if (storage && storage->settings().wifiSsid.length()) return storage->settings().wifiSsid;
    return String(LARNIA_WIFI_SSID);
}

static String cfgPassword(LarniaStorage* storage)
{
    if (storage && storage->settings().wifiPassword.length()) return storage->settings().wifiPassword;
    return String(LARNIA_WIFI_PASSWORD);
}

static String cfgHost(LarniaStorage* storage)
{
    if (storage && storage->settings().backendHost.length()) return storage->settings().backendHost;
    return String(LARNIA_BACKEND_HOST);
}

static uint16_t cfgPort(LarniaStorage* storage)
{
    if (storage && storage->settings().backendPort > 0) return storage->settings().backendPort;
    return LARNIA_BACKEND_PORT;
}

void LarniaNetwork::begin(LarniaUi* ui, LarniaStorage* storage)
{
    _ui = ui;
    _storage = storage;
    instance = this;
    reconnect();
}

void LarniaNetwork::reconnect()
{
    _connected = false;
    _ws.disconnect();

    if (connectWifi()) {
        connectBackend();
    }
}

bool LarniaNetwork::connectWifi()
{
    String ssid = cfgSsid(_storage);
    String pass = cfgPassword(_storage);

    if (ssid.length() == 0) {
        Serial.println("[WiFi] SSID is empty. Set it in Nastavenia -> Generálne.");
        if (_ui) _ui->setOnline(false);
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(300);

    Serial.print("[WiFi] Connecting to: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[WiFi] Connected. IP: ");
        Serial.println(WiFi.localIP());
        if (_ui) _ui->setOnline(true);
        return true;
    }

    Serial.print("[WiFi] Not connected. Status: ");
    Serial.println((int)WiFi.status());
    if (_ui) _ui->setOnline(false);
    return false;
}

void LarniaNetwork::connectBackend()
{
    String host = cfgHost(_storage);
    uint16_t port = cfgPort(_storage);

    if (host.length() == 0) {
        Serial.println("[Backend] Host is empty. Set Backend IP in Nastavenia -> Generálne.");
        return;
    }

    Serial.print("[Backend] WebSocket connecting to ws://");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.println(LARNIA_WS_PATH);

    _ws.begin(host.c_str(), port, LARNIA_WS_PATH);
    _ws.onEvent(LarniaNetwork::wsEvent);
    _ws.setReconnectInterval(3000);
}

bool LarniaNetwork::connected() const { return _connected; }

void LarniaNetwork::loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        if (_connected && _ui) _ui->setOnline(false);
        _connected = false;

        bool autoReconnect = !_storage || _storage->settings().autoReconnect;
        if (autoReconnect && millis() - _lastReconnectTry > 15000) {
            _lastReconnectTry = millis();
            reconnect();
        }
        return;
    }

    _ws.loop();
}

void LarniaNetwork::sendPanelReady()
{
    if (!_connected) return;

    DynamicJsonDocument doc(512);
    doc["type"] = "panel-ready";
    doc["device"] = _storage ? _storage->settings().deviceName : "CrowPanel ESP32-P4 7inch";
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
}

void LarniaNetwork::sendChatQuestion(const String& teacherId, const String& question)
{
    if (!_connected) {
        if (_ui) _ui->showPanelAnswer("Panel nie je pripojený k backendu. Skontroluj Wi‑Fi, Backend IP a spustený backend.", "");
        Serial.println("[Chat] Cannot send, WebSocket is not connected.");
        return;
    }

    DynamicJsonDocument doc(1024);
    doc["type"] = "chat-question";
    doc["teacherId"] = teacherId;
    doc["question"] = question;
    String out;
    serializeJson(doc, out);
    _ws.sendTXT(out);
    Serial.println("[Chat] Question sent to backend.");
}

void LarniaNetwork::wsEvent(WStype_t type, uint8_t* payload, size_t length)
{
    if (!instance) return;

    if (type == WStype_CONNECTED) {
        instance->_connected = true;
        Serial.println("[Backend] WebSocket connected.");
        if (instance->_ui) instance->_ui->setOnline(true);
        instance->sendPanelReady();

    } else if (type == WStype_DISCONNECTED) {
        instance->_connected = false;
        Serial.println("[Backend] WebSocket disconnected.");
        if (instance->_ui) instance->_ui->setOnline(false);

    } else if (type == WStype_TEXT) {
        DynamicJsonDocument doc(4096);
        if (deserializeJson(doc, payload, length)) return;
        String eventType = doc["type"] | "";
        if (instance->_ui) instance->_ui->handleEvent(eventType, doc.as<JsonVariant>());
    }
}
