#pragma once

// ================================================================
// pid_controller.h — PID Controller cho xe cân bằng
//
// Tính toán output = Kp*e + Ki*∫e*dt + Kd*(de/dt)
// Có anti-windup integral clamp và derivative filter
// ================================================================

class PIDController {
public:
    PIDController() = default;

    // Thiết lập hệ số PID
    // Gọi trước lần compute() đầu tiên hoặc khi tune lại
    void setTunings(float kp, float ki, float kd);

    // Đặt giới hạn output [minOut, maxOut]
    // Mặc định: [-PID_OUTPUT_MAX, +PID_OUTPUT_MAX]
    void setOutputLimits(float minOut, float maxOut);

    // Đặt giới hạn integral để chống windup
    // Mặc định: [-PID_INTEGRAL_MAX, +PID_INTEGRAL_MAX]
    void setIntegralLimits(float minI, float maxI);

    // Tính PID output
    //   setpoint    — giá trị mong muốn (độ)
    //   measurement — giá trị đo thực tế (độ)
    //   dt          — bước thời gian (giây), phải > 0
    // Trả về: motor command trong phạm vi [_minOut, _maxOut]
    float compute(float setpoint, float measurement, float dt);

    // Reset trạng thái nội bộ (integral, previousError)
    // Gọi khi xe bị ngã / re-enable sau emergency
    void reset();

    // --- Getters để debug / telemetry ---
    float getKp()            const { return _kp; }
    float getKi()            const { return _ki; }
    float getKd()            const { return _kd; }
    float getIntegral()      const { return _integral; }
    float getPreviousError() const { return _prevError; }
    float getLastOutput()    const { return _lastOutput; }

private:
    // Hệ số PID
    float _kp = 0.0f;
    float _ki = 0.0f;
    float _kd = 0.0f;

    // Trạng thái nội bộ
    float _integral    = 0.0f;
    float _prevError   = 0.0f;
    float _lastOutput  = 0.0f;
    bool  _firstRun    = true;   // bỏ qua derivative spike lần đầu

    // Giới hạn
    float _minOut  = -1023.0f;
    float _maxOut  =  1023.0f;
    float _minI    = -200.0f;
    float _maxI    =  200.0f;

    // Helper: clamp giá trị vào [lo, hi]
    static float clamp(float val, float lo, float hi);
};
