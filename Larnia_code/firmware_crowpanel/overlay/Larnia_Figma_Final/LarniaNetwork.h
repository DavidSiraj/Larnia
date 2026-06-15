#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include "LarniaUi.h"
#include "LarniaStorage.h"

class LarniaNetwork {
public:
    void begin(LarniaUi* ui, LarniaStorage* storage);
    void loop();
    bool connected() const;
    void sendPanelReady();
    void reconnect();
    void sendChatQuestion(const String& teacherId, const String& question);
private:
    LarniaUi* _ui = nullptr;
    LarniaStorage* _storage = nullptr;
    WebSocketsClient _ws;
    bool _connected = false;
    uint32_t _lastReconnectTry = 0;
    bool connectWifi();
    void connectBackend();
    static void wsEvent(WStype_t type, uint8_t* payload, size_t length);
    static LarniaNetwork* instance;
};
