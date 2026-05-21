#include "wifi_manager.h"

bool WiFiManagerCustom::begin(const char* ssid,
                              const char* password,
                              uint32_t timeoutMs) {
    _ssid     = ssid;
    _password = password;

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);  // Tự quản lý reconnect trong update()

    _connect();

    // Chờ kết nối trong khoảng timeoutMs
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > timeoutMs) {
            Serial.println("[WiFi] Timeout — sẽ retry trong loop");
            return false;
        }
        delay(200);
        Serial.print(".");
    }

    Serial.println();
    Serial.printf("[WiFi] Đã kết nối: %s  IP: %s\n",
                  _ssid, WiFi.localIP().toString().c_str());

    _lastConnected = true;
    _notifyStatus(true);
    return true;
}

void WiFiManagerCustom::update() {
    bool connected = (WiFi.status() == WL_CONNECTED);

    // Phát hiện thay đổi trạng thái
    if (connected != _lastConnected) {
        _lastConnected = connected;
        _notifyStatus(connected);

        if (!connected) {
            Serial.println("[WiFi] Mất kết nối");
        } else {
            Serial.printf("[WiFi] Đã kết nối lại  IP: %s\n",
                          WiFi.localIP().toString().c_str());
        }
    }

    // Thử reconnect theo interval nếu mất kết nối
    if (!connected) {
        uint32_t now = millis();
        if (now - _lastReconnectMs >= _reconnectInterval) {
            _lastReconnectMs = now;
            Serial.println("[WiFi] Đang kết nối lại...");
            _connect();
        }
    }
}

bool WiFiManagerCustom::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManagerCustom::getIP() const {
    if (!isConnected()) return "";
    return WiFi.localIP().toString();
}

int8_t WiFiManagerCustom::getRSSI() const {
    return WiFi.RSSI();
}

void WiFiManagerCustom::onStatusChange(WiFiStatusCallback cb) {
    _statusCallback = cb;
}

// ─── Private ───────────────────────────────────────────────

void WiFiManagerCustom::_connect() {
    WiFi.disconnect(false);
    WiFi.begin(_ssid, _password);
}

void WiFiManagerCustom::_notifyStatus(bool connected) {
    if (_statusCallback) {
        _statusCallback(connected);
    }
}
