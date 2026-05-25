// ================================================================
// balance_controller.cpp — Balance Controller implementation
// ================================================================

#include "balance_controller.h"

// ────────────────────────────────────────────────────────────────
// Constructor & begin
// ────────────────────────────────────────────────────────────────

BalanceController::BalanceController(MotorDriver& leftMotor, MotorDriver& rightMotor)
    : _leftMotor(leftMotor), _rightMotor(rightMotor)
{}

bool BalanceController::begin() {
    // Thiết lập PID từ hằng số trong constants.h
    _pid.setTunings(PID_KP, PID_KI, PID_KD);
    _pid.setOutputLimits(-PID_OUTPUT_MAX, PID_OUTPUT_MAX);
    _pid.setIntegralLimits(-PID_INTEGRAL_MAX, PID_INTEGRAL_MAX);
    _pid.reset();

    _state = BalanceState::IDLE;
    return true;
}

// ────────────────────────────────────────────────────────────────
// Vòng điều khiển chính
// ────────────────────────────────────────────────────────────────

void BalanceController::update(float filteredAngle, float dt) {
    _currentAngle = filteredAngle;

    // Bộ điều khiển không hoạt động → giữ motor dừng
    if (_state == BalanceState::IDLE || _state == BalanceState::ERROR) {
        return;
    }

    // Phát hiện ngã — vượt FALL_ANGLE
    if (filteredAngle > FALL_ANGLE || filteredAngle < -FALL_ANGLE) {
        handleFall();
        return;
    }

    // Nếu đang trong trạng thái FALLEN, chờ xe được dựng lại
    // (góc phải trở về vùng an toàn trước khi re-enable)
    if (_state == BalanceState::FALLEN) {
        // Góc trở về vùng an toàn → tự động phục hồi
        // Threshold nhỏ hơn FALL_ANGLE để có hysteresis
        if (filteredAngle < FALL_ANGLE * 0.5f && filteredAngle > -FALL_ANGLE * 0.5f) {
            _pid.reset();
            _state = BalanceState::RUNNING;
        }
        return;
    }

    // ── RUNNING: tính PID và xuất motor ──────────────────────────
    _pidOutput = _pid.compute(_targetAngle, filteredAngle, dt);

    applyMotorCommand(_pidOutput);
}

// ────────────────────────────────────────────────────────────────
// Cấu hình
// ────────────────────────────────────────────────────────────────

void BalanceController::setTargetAngle(float angle) {
    _targetAngle = angle;
}

void BalanceController::enable() {
    if (_state == BalanceState::IDLE) {
        _pid.reset();
        _state = BalanceState::RUNNING;
    }
}

void BalanceController::disable() {
    _leftMotor.hardBrake();
    _rightMotor.hardBrake();
    _leftPWM  = 0;
    _rightPWM = 0;
    _pid.reset();
    _state = BalanceState::IDLE;
}

void BalanceController::setPIDTunings(float kp, float ki, float kd) {
    _pid.setTunings(kp, ki, kd);
}

// ────────────────────────────────────────────────────────────────
// Private helpers
// ────────────────────────────────────────────────────────────────

void BalanceController::handleFall() {
    // Dừng ngay cả 2 motor
    _leftMotor.hardBrake();
    _rightMotor.hardBrake();
    _leftPWM  = 0;
    _rightPWM = 0;
    _pidOutput = 0.0f;

    // Reset PID để tránh integral windup tích lũy khi nằm dưới đất
    _pid.reset();

    _state = BalanceState::FALLEN;
}

void BalanceController::applyMotorCommand(float command) {
    // command: âm → lùi, dương → tiến
    // Cả 2 motor nhận cùng một lệnh (xe 2 bánh song song)
    // Nếu sau này có điều khiển hướng, offset sẽ thêm vào đây

    int pwm = static_cast<int>(command);

    // Clamp thêm một lần nữa cho chắc (MotorDriver cũng clamp)
    if (pwm >  PWM_MAX) pwm =  PWM_MAX;
    if (pwm < -PWM_MAX) pwm = -PWM_MAX;

    _leftPWM  = pwm;
    _rightPWM = pwm;

    _leftMotor.setSpeed(_leftPWM);
    _rightMotor.setSpeed(_rightPWM);
}
