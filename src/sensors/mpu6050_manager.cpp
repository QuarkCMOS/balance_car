#include "mpu6050_manager.h"
#include "../config/pins.h"

bool MPU6050Manager::begin() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!mpu.begin()) return false;
    
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // Lọc nhiễu
    return true;
}

void MPU6050Manager::update() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    imuData.accX = a.acceleration.x;
    imuData.accY = a.acceleration.y;
    imuData.accZ = a.acceleration.z;
    imuData.gyroX = g.gyro.x;
    imuData.gyroY = g.gyro.y;
    imuData.gyroZ = g.gyro.z;

    // Góc Pitch (trục Y)
    imuData.accAngle = atan2(imuData.accY, imuData.accZ) * 180.0 / PI;
}

IMUData MPU6050Manager::getData() { return imuData; }