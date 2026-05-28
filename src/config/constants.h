#pragma once
#include <cstdint>

// constants.h — hằng số toàn hệ thống

// WiFi
constexpr const char* WIFI_SSID         = "TP-LINK_2B99";
constexpr const char* WIFI_PASSWORD     = "xincamon";

// WebSocket
constexpr uint16_t  WS_PORT             = 81;
constexpr uint32_t  TELEMETRY_INTERVAL_MS = 50;    // 20 Hz

// Motor / PWM
constexpr int       PWM_FREQ_HZ         = 20000;   // 20kHz, trên ngưỡng nghe
constexpr int       PWM_MAX             = 1023;    // độ phân giải 10-bit
constexpr int       PWM_DEADBAND        = 30;      // dưới ngưỡng này motor không quay

// Encoder
constexpr int       ENC_PPR             = 26;      // pulses per revolution (GA25-370 raw)
constexpr float     GEAR_RATIO          = 18.8f;   // tỉ số truyền GA25-370
constexpr float     WHEEL_DIAMETER_M    = 0.065f;  // đường kính bánh xe (m), chỉnh theo thực tế
constexpr float     ENC_TICKS_PER_REV   = ENC_PPR * GEAR_RATIO * 4; // x4 vì quadrature

// PID
// Giá trị khởi đầu — tune lại khi test thực tế
constexpr float     PID_KP              = 20.0f;
constexpr float     PID_KI              = 0.5f;
constexpr float     PID_KD              = 0.8f;
constexpr float     PID_OUTPUT_MAX      = PWM_MAX;
constexpr float     PID_INTEGRAL_MAX    = 200.0f;  // anti-windup clamp

// Balance
constexpr float     TARGET_ANGLE        = 0.0f;    // góc cân bằng (độ), chỉnh offset theo thực tế
constexpr float     FALL_ANGLE          = 30.0f;   // quá góc này → emergency brake
constexpr float     ALPHA               = 0.98f;   // complementary filter weight

// Timing
constexpr uint32_t  LOOP_INTERVAL_US    = 5000;    // 200 Hz control loop