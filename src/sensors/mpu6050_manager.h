#ifndef MPU6050_MANAGER_H
#define MPU6050_MANAGER_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

struct IMUData {
    float accX, accY, accZ;
    float gyroX, gyroY, gyroZ;
    float accAngle;
};

class MPU6050Manager {
public:
    bool begin();
    void update();
    IMUData getData();
private:
    IMUData imuData;
    Adafruit_MPU6050 mpu;
};

#endif