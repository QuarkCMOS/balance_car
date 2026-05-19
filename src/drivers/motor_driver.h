#pragma once

#include <Arduino.h>
#include <ESP32_MCPWM.h>
#include "../config/constants.h"

// Hardware config cho một motor
struct MotorPinConfig {
    int                 pinIN1;   // LPWM
    int                 pinIN2;   // RPWM
    mcpwm_unit_t        unit;
    mcpwm_timer_t       timer;
    mcpwm_io_signals_t  sigA;
    mcpwm_io_signals_t  sigB;
};

class MotorDriver {
public:
    explicit MotorDriver(const MotorPinConfig& cfg);

    // Khởi tạo MCPWM với PWM_FREQ_HZ và PWM_MAX từ constants.h
    void begin();

    // speed: -PWM_MAX → +PWM_MAX
    // dương = CW, âm = CCW
    // Tự apply PWM_DEADBAND — dưới ngưỡng thì hardBrake()
    void setSpeed(int speed);

    // Hard brake — short circuit IN1=IN2=HIGH, dừng ngay
    void hardBrake();

    // Soft brake — dither nhẹ, dừng êm
    void softBrake();

    // Coast — thả tự do
    void coast();

    int getCurrentSpeed() const { return _currentSpeed; }

private:
    MotorPinConfig _cfg;
    HBridgeMotor   _motor;
    int            _currentSpeed = 0;
};