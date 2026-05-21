#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Callback khi nhận lệnh từ dashboard (PID tuning, target angle...)
using WSCommandCallback = std::function<void(const String& key,
                                             float value)>;

class WebSocketManager {
public:
    explicit WebSocketManager(uint16_t port = 81);

    // Gọi sau khi WiFi đã connected
    void begin();

    // Gọi trong loop() — xử lý incoming events
    void handleClient();

    // Gửi JSON telemetry đến tất cả client đang kết nối
    void sendTelemetry(const String& json);

    // Đăng ký callback khi nhận command từ client
    void onCommand(WSCommandCallback cb);

    bool hasClient() const;

private:
    WebSocketsServer _ws;
    WSCommandCallback _commandCallback = nullptr;
    bool _hasClient = false;

    // Handler nội bộ — được forward từ static callback
    void _onEvent(uint8_t clientId,
                  WStype_t type,
                  uint8_t* payload,
                  size_t length);

    // WebSocketsServer yêu cầu static function làm event handler
    // Dùng pointer-to-instance để bridge vào member function
    static WebSocketManager* _instance;
    static void _staticEventHandler(uint8_t clientId,
                                    WStype_t type,
                                    uint8_t* payload,
                                    size_t length);
};
