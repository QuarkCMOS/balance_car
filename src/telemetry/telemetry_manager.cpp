#include "telemetry_manager.h"

void TelemetryManager::update(const TelemetryPayload& payload) {
    _payload = payload;
}

String TelemetryManager::buildTelemetryJSON() const {
    // StaticJsonDocument — không dynamic alloc, an toàn trên ESP32
    StaticJsonDocument<256> doc;

    doc["angle"]       = _payload.angle;
    doc["gyro"]        = _payload.gyro;
    doc["pid"]         = _payload.pidOutput;
    doc["pwm_l"]       = _payload.pwmLeft;
    doc["pwm_r"]       = _payload.pwmRight;
    doc["rpm_l"]       = _payload.rpmLeft;
    doc["rpm_r"]       = _payload.rpmRight;
    doc["target"]      = _payload.targetAngle;
    doc["ts"]          = millis();   // timestamp để client tính latency

    String out;
    serializeJson(doc, out);
    return out;
}