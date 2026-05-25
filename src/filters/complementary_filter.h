#pragma once

// ================================================================
// complementary_filter.h — Bộ lọc bù góc nghiêng cho xe cân bằng
//
// Kết hợp:
//   • Accelerometer  → góc tuyệt đối, nhưng nhiễu nhiều ở tần số cao
//   • Gyroscope      → tốc độ góc chính xác ngắn hạn, drift dài hạn
//
// Công thức:
//   angle = alpha * (angle + gyroRate * dt) + (1 - alpha) * accelAngle
//
// alpha ∈ [0, 1]:
//   • Gần 1 → tin gyro nhiều hơn (nhạy, ít nhiễu tần số cao)
//   • Gần 0 → tin accel nhiều hơn (ổn định dài hạn, dễ nhiễu)
//   • Mặc định: 0.98 (từ ALPHA trong constants.h)
// ================================================================

class ComplementaryFilter {
public:
    // Khởi tạo với hệ số alpha tùy chọn
    // Nếu không truyền, dùng ALPHA từ constants.h (0.98)
    explicit ComplementaryFilter(float alpha = 0.98f);

    // Đặt lại hệ số alpha sau khi khởi tạo
    void setAlpha(float alpha);

    // Reset trạng thái về góc ban đầu
    // Gọi sau emergency stop hoặc khi xe được đặt lại tư thế
    void reset(float initialAngleDeg = 0.0f);

    // ── Cập nhật chính ────────────────────────────────────────────
    // Tính góc nghiêng từ dữ liệu IMU thô
    //
    // Tham số:
    //   accelX, accelZ  — gia tốc trên trục X và Z (g hoặc m/s², đơn vị nhất quán)
    //                     accelAngle = atan2(accelX, accelZ) * (180/π)
    //   gyroRateDegPerS — tốc độ quay đo từ gyro (độ/giây), quanh trục pitch
    //   dt              — bước thời gian (giây), phải > 0
    //
    // Trả về:
    //   Góc nghiêng đã lọc (độ):
    //     0   → thẳng đứng
    //     +   → nghiêng về phía trước
    //     −   → nghiêng về phía sau
    float update(float accelX, float accelZ, float gyroRateDegPerS, float dt);

    // ── Variant: nhận accelAngle trực tiếp (đã tính bên ngoài) ───
    // Hữu ích khi sensor driver đã tính góc từ accelerometer rồi
    float updateWithAngle(float accelAngleDeg, float gyroRateDegPerS, float dt);

    // ── Getters ───────────────────────────────────────────────────
    float getAngle()        const { return _angle; }
    float getAlpha()        const { return _alpha; }
    bool  isInitialized()   const { return _initialized; }

private:
    float _alpha;           // trọng số gyro (0..1)
    float _angle;           // góc nghiêng hiện tại (độ)
    bool  _initialized;     // false → lần đầu update dùng 100% accel để seed

    // Helper: clamp alpha về [0, 1]
    static float clampAlpha(float a);

    // Helper: atan2 wrapper trả về độ
    static float accelToDeg(float accelX, float accelZ);
};