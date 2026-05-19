#include "pwm_manager.h"

PWMManager::PWMManager()
    : _left(MotorPinConfig{
          PIN_MOTOR_L1,        // 26 — DRV8833 IN3
          PIN_MOTOR_L2,        // 25 — DRV8833 IN4
          MCPWM_UNIT_0,
          MCPWM_TIMER_0,
          MCPWM0A,
          MCPWM0B
      }),
      _right(MotorPinConfig{
          PIN_MOTOR_R1,        // 33 — DRV8833 IN1
          PIN_MOTOR_R2,        // 32 — DRV8833 IN2
          MCPWM_UNIT_0,
          MCPWM_TIMER_1,       // Timer khác để 2 motor hoàn toàn độc lập
          MCPWM1A,
          MCPWM1B
      })
{}

void PWMManager::begin() {
    _left.begin();
    _right.begin();
    Serial.printf("[PWM] MCPWM ready — %d Hz, max %d\n",
                  PWM_FREQ_HZ, PWM_MAX);
}

void PWMManager::write(int leftSpeed, int rightSpeed) {
    _left.setSpeed(leftSpeed);
    _right.setSpeed(rightSpeed);
}

void PWMManager::emergencyBrake() {
    _left.hardBrake();
    _right.hardBrake();
    Serial.println("[PWM] EMERGENCY BRAKE");
}

void PWMManager::softBrake() {
    _left.softBrake();
    _right.softBrake();
}