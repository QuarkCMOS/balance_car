#include "websocket_manager.h"

// Định nghĩa static instance pointer
WebSocketManager* WebSocketManager::_instance = nullptr;

// ─── Constructor ─────────────────────────────────────────────

WebSocketManager::WebSocketManager(uint16_t port)
    : _ws(port) {
    _instance = this;
}

// ─── Public ──────────────────────────────────────────────────

void WebSocketManager::begin() {
    _ws.begin();
    _ws.onEvent(_staticEventHandler);
    Serial.println("[WS] WebSocket server started on port 81");
}

void WebSocketManager::handleClient() {
    _ws.loop();
}

void WebSocketManager::sendTelemetry(const String& json) {
    if (!_hasClient) return;
    _ws.broadcastTXT(json.c_str());
}

void WebSocketManager::onCommand(WSCommandCallback cb) {
    _commandCallback = cb;
}

bool WebSocketManager::hasClient() const {
    return _hasClient;
}

// ─── Private ─────────────────────────────────────────────────

void WebSocketManager::_onEvent(uint8_t clientId,
                                 WStype_t type,
                                 uint8_t* payload,
                                 size_t length) {
    switch (type) {

        case WStype_CONNECTED:
            _hasClient = true;
            Serial.printf("[WS] Client #%u kết nối  IP: %s\n",
                          clientId,
                          _ws.remoteIP(clientId).toString().c_str());

            // Gửi ACK về cho client biết đã connect
            _ws.sendTXT(clientId, R"({"status":"connected"})");
            break;

        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u ngắt kết nối\n", clientId);
            // Kiểm tra còn client nào không
            // (WebSocketsServer không expose count, dùng flag đơn giản)
            _hasClient = false;
            break;

        case WStype_TEXT: {
            // Parse JSON lệnh từ dashboard
            // Ví dụ: {"cmd":"set_kp","value":1.5}
            //        {"cmd":"set_target","value":0.0}
            StaticJsonDocument<128> doc;
            DeserializationError err = deserializeJson(
                doc, payload, length);

            if (err) {
                Serial.printf("[WS] JSON parse error: %s\n",
                              err.c_str());
                break;
            }

            const char* cmd = doc["cmd"];
            float value     = doc["value"] | 0.0f;

            if (cmd && _commandCallback) {
                _commandCallback(String(cmd), value);
            }
            break;
        }

        case WStype_ERROR:
            Serial.printf("[WS] Error client #%u\n", clientId);
            break;

        default:
            break;
    }
}

void WebSocketManager::_staticEventHandler(uint8_t clientId,
                                            WStype_t type,
                                            uint8_t* payload,
                                            size_t length) {
    if (_instance) {
        _instance->_onEvent(clientId, type, payload, length);
    }
}
