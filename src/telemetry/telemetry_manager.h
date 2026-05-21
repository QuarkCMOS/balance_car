#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// Forward declare để tránh include vòng
struct IMUData;
struct EncoderData;

struct TelemetryPayload {
    float angle;        // góc nghiêng (filtered)
    float gyro;         // gyro rate thô
    float pidOutput;    // PID output hiện tại
    int   pwmLeft;      // PWM motor trái
    int   pwmRight;     // PWM motor phải
    float rpmLeft;      // RPM encoder trái
    float rpmRight;     // RPM encoder phải
    float targetAngle;  // setpoint hiện tại
};

class TelemetryManager {
public:
    // Cập nhật data mới nhất — gọi từ BalanceController
    void update(const TelemetryPayload& payload);

    // Serialize thành JSON string gửi qua WebSocket
    String buildTelemetryJSON() const;

private:
    TelemetryPayload _payload{};
};
