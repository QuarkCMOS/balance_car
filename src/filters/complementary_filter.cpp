// ================================================================
// complementary_filter.cpp — Bộ lọc bù góc nghiêng
// ================================================================

#include "complementary_filter.h"

// Arduino build: có <math.h> qua Arduino.h
// Native test build: cần include trực tiếp
#ifndef ARDUINO
    #include <cmath>
    #ifndef M_PI
        #define M_PI 3.14159265358979323846f
    #endif
#else
    #include <math.h>
#endif

// ────────────────────────────────────────────────────────────────
// Constructor & cấu hình
// ────────────────────────────────────────────────────────────────

ComplementaryFilter::ComplementaryFilter(float alpha)
    : _alpha(clampAlpha(alpha))
    , _angle(0.0f)
    , _initialized(false)
{}

void ComplementaryFilter::setAlpha(float alpha) {
    _alpha = clampAlpha(alpha);
}

void ComplementaryFilter::reset(float initialAngleDeg) {
    _angle       = initialAngleDeg;
    _initialized = false;
}

// ────────────────────────────────────────────────────────────────
// Cập nhật từ dữ liệu IMU thô
// ────────────────────────────────────────────────────────────────

float ComplementaryFilter::update(float accelX, float accelZ,
                                  float gyroRateDegPerS, float dt) {
    float accelAngleDeg = accelToDeg(accelX, accelZ);
    return updateWithAngle(accelAngleDeg, gyroRateDegPerS, dt);
}

// ────────────────────────────────────────────────────────────────
// Cập nhật khi đã có accelAngle tính sẵn
// ────────────────────────────────────────────────────────────────

float ComplementaryFilter::updateWithAngle(float accelAngleDeg,
                                           float gyroRateDegPerS,
                                           float dt) {
    // dt không hợp lệ → giữ góc cũ
    if (dt <= 0.0f) {
        return _angle;
    }

    // Lần đầu tiên: seed bằng 100% accelerometer để tránh drift từ 0
    if (!_initialized) {
        _angle       = accelAngleDeg;
        _initialized = true;
        return _angle;
    }

    // Bước tích phân gyro
    float gyroAngle = _angle + gyroRateDegPerS * dt;

    // Trộn: tin gyro (alpha) + tin accel (1 - alpha)
    _angle = _alpha * gyroAngle + (1.0f - _alpha) * accelAngleDeg;

    return _angle;
}

// ────────────────────────────────────────────────────────────────
// Private helpers
// ────────────────────────────────────────────────────────────────

float ComplementaryFilter::clampAlpha(float a) {
    if (a < 0.0f) return 0.0f;
    if (a > 1.0f) return 1.0f;
    return a;
}

float ComplementaryFilter::accelToDeg(float accelX, float accelZ) {
    // atan2 trả về radian, chuyển sang độ
    // Trục: X = chiều tiến/lùi của xe, Z = thẳng đứng
    // accelAngle = 0 khi xe thẳng đứng (accelX=0, accelZ=1g)
    return static_cast<float>(atan2(accelX, accelZ)) * (180.0f / static_cast<float>(M_PI));
}