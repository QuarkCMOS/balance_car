#pragma once

// ================================================================
// balance_controller.h — "Brain" module của xe cân bằng
//
// Điều phối toàn bộ vòng điều khiển:
//   1. Lấy góc đã lọc từ ComplementaryFilter
//   2. Chạy PID để tính motor command
//   3. Ghi setSpeed() ra cả 2 MotorDriver
//   4. Emergency stop khi góc vượt FALL_ANGLE
// ================================================================

#include "pid_controller.h"
#include "../drivers/motor_driver.h"
#include "../config/constants.h"

// Trạng thái hoạt động của bộ điều khiển
enum class BalanceState : uint8_t {
    IDLE,       // Chưa khởi động hoặc đã dừng thủ công
    RUNNING,    // Đang điều khiển cân bằng bình thường
    FALLEN,     // Phát hiện ngã — emergency stop, chờ dựng lại
    ERROR       // Lỗi khởi tạo sensor
};

class BalanceController {
public:
    // Inject 2 motor driver từ ngoài (dependency injection)
    // leftMotor  — MotorDriver cho bánh trái
    // rightMotor — MotorDriver cho bánh phải
    BalanceController(MotorDriver& leftMotor, MotorDriver& rightMotor);

    // Khởi tạo: set PID tuning từ constants.h, reset trạng thái
    // Trả true nếu OK
    bool begin();

    // Vòng điều khiển chính — gọi mỗi LOOP_INTERVAL_US
    //   filteredAngle — góc đầu ra của ComplementaryFilter (độ)
    //                   âm = nghiêng ra trước, dương = nghiêng ra sau
    //   dt            — bước thời gian (giây)
    void update(float filteredAngle, float dt);

    // Đặt góc mục tiêu cân bằng (độ)
    // Dùng để chỉnh offset cơ học hoặc nhận lệnh từ remote
    void setTargetAngle(float angle);

    // Kích hoạt / dừng bộ điều khiển thủ công
    void enable();
    void disable();

    // Tune lại PID runtime (WebSocket command)
    void setPIDTunings(float kp, float ki, float kd);

    // Lấy thông tin để gửi telemetry
    BalanceState getState()        const { return _state; }
    float        getTargetAngle()  const { return _targetAngle; }
    float        getCurrentAngle() const { return _currentAngle; }
    float        getPIDOutput()    const { return _pidOutput; }
    int          getLeftPWM()      const { return _leftPWM; }
    int          getRightPWM()     const { return _rightPWM; }

private:
    // Motor drivers (tham chiếu — không sở hữu)
    MotorDriver& _leftMotor;
    MotorDriver& _rightMotor;

    // PID controller
    PIDController _pid;

    // Trạng thái
    BalanceState _state        = BalanceState::IDLE;
    float        _targetAngle  = TARGET_ANGLE;   // từ constants.h
    float        _currentAngle = 0.0f;
    float        _pidOutput    = 0.0f;
    int          _leftPWM      = 0;
    int          _rightPWM     = 0;

    // Xử lý emergency fall
    void handleFall();

    // Áp dụng motor command sau khi clamp và deadband (đã xử lý trong MotorDriver)
    void applyMotorCommand(float command);
};
