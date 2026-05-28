#include "mpu6050_manager.h"
#include "../config/pins.h"
       
#include "../config/constants.h"   // ALPHA = 0.98f

bool MPU6050Manager::begin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    if (!mpu.begin()) return false;

    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Lấy góc ban đầu từ acc để filter không bắt đầu từ 0
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    imuData.accAngle = atan2(a.acceleration.y,
                             a.acceleration.z) * RAD_TO_DEG;

    _lastUs = micros();
    return true;
}

void MPU6050Manager::update() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // ── Raw data ─────────────────────────────────────────────────
    imuData.accX  = a.acceleration.x;
    imuData.accY  = a.acceleration.y;
    imuData.accZ  = a.acceleration.z;
    imuData.gyroX = g.gyro.x;   // rad/s
    imuData.gyroY = g.gyro.y;
    imuData.gyroZ = g.gyro.z;

    // ── dt chính xác bằng micros() ───────────────────────────────
    uint32_t now = micros();
    float dt = (now - _lastUs) / 1e6f;   // đổi sang giây
    _lastUs = now;

    // Tránh dt bất thường khi mới khởi động hoặc overflow
    if (dt <= 0.0f || dt > 0.5f) return;

    // ── Góc từ accelerometer (trục pitch) ────────────────────────
    float accAngle = atan2(imuData.accY, imuData.accZ) * RAD_TO_DEG;

    // ── Complementary filter ─────────────────────────────────────
    // ALPHA từ constants.h — 98% gyro tích phân + 2% acc
    imuData.accAngle = ALPHA * (imuData.accAngle + imuData.gyroX * dt * RAD_TO_DEG)
                     + (1.0f - ALPHA) * accAngle;
}

IMUData MPU6050Manager::getData() {
    return imuData;
}