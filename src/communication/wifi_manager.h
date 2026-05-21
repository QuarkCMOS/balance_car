#pragma once

#include <Arduino.h>
#include <WiFi.h>

// Callback khi trạng thái WiFi thay đổi
using WiFiStatusCallback = std::function<void(bool connected)>;

class WiFiManagerCustom {
public:
    // Khởi tạo và kết nối WiFi
    // Trả về true nếu kết nối thành công trong timeout
    bool begin(const char* ssid,
               const char* password,
               uint32_t timeoutMs = 10000);

    // Gọi trong loop() — tự reconnect nếu mất kết nối
    void update();

    bool isConnected() const;

    // Lấy IP hiện tại (trả về chuỗi rỗng nếu chưa kết nối)
    String getIP() const;

    // Đăng ký callback khi connect/disconnect
    void onStatusChange(WiFiStatusCallback cb);

    // Thông tin debug
    int8_t getRSSI() const;

private:
    const char* _ssid     = nullptr;
    const char* _password = nullptr;

    bool _lastConnected   = false;
    uint32_t _reconnectInterval = 5000;   // ms giữa các lần retry
    uint32_t _lastReconnectMs   = 0;

    WiFiStatusCallback _statusCallback = nullptr;

    void _connect();
    void _notifyStatus(bool connected);
};
