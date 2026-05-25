// ================================================================
// pid_controller.cpp — PID Controller implementation
// ================================================================

#include "pid_controller.h"

// ────────────────────────────────────────────────────────────────
// Cấu hình
// ────────────────────────────────────────────────────────────────

void PIDController::setTunings(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PIDController::setOutputLimits(float minOut, float maxOut) {
    if (minOut >= maxOut) return;  // invalid range — bỏ qua
    _minOut = minOut;
    _maxOut = maxOut;
}

void PIDController::setIntegralLimits(float minI, float maxI) {
    if (minI >= maxI) return;
    _minI = minI;
    _maxI = maxI;
}

// ────────────────────────────────────────────────────────────────
// Compute
// ────────────────────────────────────────────────────────────────

float PIDController::compute(float setpoint, float measurement, float dt) {
    // Guard: dt không hợp lệ → giữ nguyên output trước
    if (dt <= 0.0f) return _lastOutput;

    // Sai số hiện tại
    float error = setpoint - measurement;

    // ── Proportional ──────────────────────────────────────────
    float pTerm = _kp * error;

    // ── Integral với anti-windup clamp ────────────────────────
    _integral += error * dt;
    _integral  = clamp(_integral, _minI, _maxI);
    float iTerm = _ki * _integral;

    // ── Derivative — bỏ qua lần đầu để tránh derivative spike ─
    float dTerm = 0.0f;
    if (!_firstRun) {
        float dError = (error - _prevError) / dt;
        dTerm = _kd * dError;
    }
    _firstRun = false;

    // Lưu error cho lần sau
    _prevError = error;

    // ── Tổng hợp và clamp output ──────────────────────────────
    float output = pTerm + iTerm + dTerm;
    output = clamp(output, _minOut, _maxOut);

    _lastOutput = output;
    return output;
}

// ────────────────────────────────────────────────────────────────
// Reset
// ────────────────────────────────────────────────────────────────

void PIDController::reset() {
    _integral   = 0.0f;
    _prevError  = 0.0f;
    _lastOutput = 0.0f;
    _firstRun   = true;
}

// ────────────────────────────────────────────────────────────────
// Private helper
// ────────────────────────────────────────────────────────────────

float PIDController::clamp(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}
