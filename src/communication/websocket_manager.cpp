#include "websocket_manager.h"
 
WebSocketManager* WebSocketManager::_instance = nullptr;
 
// ─── Constructor ─────────────────────────────────────────────────
WebSocketManager::WebSocketManager(uint16_t port)
    : _ws(port) {
    _instance = this;
}
 
// ─── Public ──────────────────────────────────────────────────────
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
 
// ─── Private ─────────────────────────────────────────────────────
void WebSocketManager::_onEvent(uint8_t clientId,
                                WStype_t type,
                                uint8_t* payload,
                                size_t   length)
{
    switch (type) {
 
        case WStype_CONNECTED:
            _hasClient = true;
            Serial.printf("[WS] Client #%u connected  IP: %s\n",
                          clientId,
                          _ws.remoteIP(clientId).toString().c_str());
            _ws.sendTXT(clientId, R"({"status":"connected"})");
            break;
 
        case WStype_DISCONNECTED:
            _hasClient = false;
            Serial.printf("[WS] Client #%u disconnected\n", clientId);
            break;
 
        case WStype_TEXT: {
            // ── Parse JSON từ dashboard ──────────────────────────
            // Format PID:  {"type":"PID",  "p":25.0, "i":0.5, "d":1.2}
            // Format MOVE: {"type":"MOVE", "dir":"FORWARD"}
            StaticJsonDocument<128> doc;
            DeserializationError err = deserializeJson(doc, payload, length);
 
            if (err) {
                Serial.printf("[WS] JSON parse error: %s\n", err.c_str());
                break;
            }
 
            const char* type = doc["type"];
            if (!type) break;
 
            if (strcmp(type, "PID") == 0) {
                if (!_commandCallback) break;
                _commandCallback("set_kp", doc["p"] | 0.0f);
                _commandCallback("set_ki", doc["i"] | 0.0f);
                _commandCallback("set_kd", doc["d"] | 0.0f);
                Serial.printf("[WS] PID  Kp=%.2f  Ki=%.2f  Kd=%.2f\n",
                              (float)doc["p"], (float)doc["i"], (float)doc["d"]);
            }
            else if (strcmp(type, "MOVE") == 0) {
                if (!_commandCallback) break;
                const char* dir = doc["dir"] | "STOP";
 
                // Encode hướng thành float để giữ nguyên interface callback(key, value)
                // 0=STOP  1=FORWARD  2=BACKWARD  3=LEFT  4=RIGHT
                float code = 0;
                if      (strcmp(dir, "FORWARD" ) == 0) code = 1;
                else if (strcmp(dir, "BACKWARD") == 0) code = 2;
                else if (strcmp(dir, "LEFT"    ) == 0) code = 3;
                else if (strcmp(dir, "RIGHT"   ) == 0) code = 4;
 
                _commandCallback("move", code);
                Serial.printf("[WS] MOVE  dir=%s\n", dir);
            }
            else {
                Serial.printf("[WS] Unknown type: %s\n", type);
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
                                           size_t   length)
{
    if (_instance) {
        _instance->_onEvent(clientId, type, payload, length);
    }
}