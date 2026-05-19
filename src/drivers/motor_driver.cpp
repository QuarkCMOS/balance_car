#include "motor_driver.h"

MotorDriver::MotorDriver(const MotorPinConfig& cfg)
    : _cfg(cfg) {}

void MotorDriver::begin() {
    MotorMCPWMConfig hw{
        _cfg.pinIN1,
        _cfg.pinIN2,
        -1,                      // EN pin — DRV8833 không có
        _cfg.unit,
        _cfg.timer,
        _cfg.sigA,
        _cfg.sigB
    };
    hw.pwm_freq_hz  = PWM_FREQ_HZ;          // 20000 Hz từ constants.h
    hw.input_max    = PWM_MAX;              // 1023 từ constants.h
    hw.counter      = MCPWM_UP_DOWN_COUNTER; // center-aligned
    hw.use_deadtime = false;                // BẮT BUỘC false với DRV8833

    // Aggregate init đúng thứ tự 5 field của MotorBehaviorConfig:
    // { freewheel_mode, soft_brake_hz, dither_pwm, min_phase_us, dither_coast_hi_z }
    MotorBehaviorConfig beh{
        FreewheelMode::HiZ_Awake,  // freewheel_mode
        60,                         // soft_brake_hz
        0,                          // dither_pwm
        600,                        // min_phase_us
        false                       // dither_coast_hi_z — false cho DRV8833
    };

    _motor.setup(hw, beh);
    _motor.start();
}

void MotorDriver::setSpeed(int speed) {
    speed = constrain(speed, -PWM_MAX, PWM_MAX);  // clamp theo PWM_MAX

    // Deadband — dưới ngưỡng PWM_DEADBAND motor không đủ torque để quay
    // brake thay vì coast để giữ vị trí
    if (abs(speed) < PWM_DEADBAND) {
        hardBrake();
        return;
    }

    _currentSpeed = speed;

    if (speed > 0) {
        _motor.setSpeed(speed, Dir::CW);
    } else {
        _motor.setSpeed(-speed, Dir::CCW);
    }
}

void MotorDriver::hardBrake() {
    _currentSpeed = 0;
    _motor.setHardBrake();
}

void MotorDriver::softBrake() {
    _currentSpeed = 0;
    _motor.setFreewheelMode(FreewheelMode::DitherBrake);
    _motor.setSoftBrakePWM(PWM_MAX * 3 / 100);  // 3% của PWM_MAX
    _motor.setSpeed(0, Dir::CW);
}

void MotorDriver::coast() {
    _currentSpeed = 0;
    _motor.setFreewheel();
}