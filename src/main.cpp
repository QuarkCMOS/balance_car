// ================================================================
// main.cpp — CHẾ ĐỘ TEST THỰC TẾ
//
// OUTPUT CSV: time_ms, angle, accRaw, pid_out, L_pwm, R_pwm, state
//
// Phím lệnh:
//   e/d/r     — enable / disable / reset
//   p         — in PID hiện tại
//   P kp ki kd — set PID trực tiếp, vd: P 35.0 0.0 0.5
//   x 0..5    — chọn cặp trục tính góc (0=XZ 1=YZ 2=XY 3=-XZ 4=-YZ 5=-XY)
//   h         — help
// ================================================================

#include <Arduino.h>
#include "config/constants.h"
#include "config/pins.h"
#include "sensors/mpu6050_manager.h"
#include "encoders/encoder_manager.h"
#include "filters/complementary_filter.h"
#include "control/balance_controller.h"
#include "drivers/motor_driver.h"

// Biến tick volatile được định nghĩa trong encoder_manager.cpp
extern volatile int32_t g_ticksLeft;
extern volatile int32_t g_ticksRight;

// ── Hardware ──────────────────────────────────────────────────────
MPU6050Manager      imu;
ComplementaryFilter filter(ALPHA);

// ESP32 MCPWM: cả 2 motor dùng UNIT_0, khác TIMER
// UNIT_0 TIMER_0 → MCPWM0A/0B → GPIO26, GPIO25  (motor trái)
// UNIT_0 TIMER_1 → MCPWM1A/1B → GPIO33, GPIO32  (motor phải)
MotorPinConfig cfgLeft = {
    PIN_MOTOR_L1, PIN_MOTOR_L2,
    MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, MCPWM0B
};
MotorPinConfig cfgRight = {
    PIN_MOTOR_R1, PIN_MOTOR_R2,
    MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM1A, MCPWM1B
};

MotorDriver       motorLeft(cfgLeft);
MotorDriver       motorRight(cfgRight);
BalanceController balancer(motorLeft, motorRight);

// ── Encoder ───────────────────────────────────────────────────────
EncoderManager encLeft (PIN_ENC_L_A, PIN_ENC_L_B, &g_ticksLeft);
EncoderManager encRight(PIN_ENC_R_A, PIN_ENC_R_B, &g_ticksRight);

// ── Timing ────────────────────────────────────────────────────────
uint32_t lastLoopUs  = 0;
uint32_t lastPrintMs = 0;

// ── Runtime PID ───────────────────────────────────────────────────
float runtimeKp = PID_KP;
float runtimeKi = PID_KI;
float runtimeKd = PID_KD;

// ── Chọn trục tính góc ────────────────────────────────────────────
// MPU6050 có thể gắn theo nhiều hướng — cần thử để tìm đúng
// Cặp trục  : (a1, a2) cho atan2(a1, a2), và gyro axis
// 0 = XZ / gyroY  (mặc định)
// 1 = YZ / gyroX
// 2 = XY / gyroZ
// 3 = -XZ / gyroY (X âm)
// 4 = -YZ / gyroX (Y âm)
// 5 = -XY / gyroZ (Z âm)
uint8_t axisMode = 0;

const char* axisModeStr[] = {
    "atan2(accX, accZ) / gyroY  [DEFAULT]",
    "atan2(accY, accZ) / gyroX",
    "atan2(accX, accY) / gyroZ",
    "atan2(-accX, accZ) / gyroY",
    "atan2(-accY, accZ) / gyroX",
    "atan2(-accX, accY) / gyroZ"
};

// ── Serial input buffer (cho lệnh nhập số) ────────────────────────
char    inputBuf[48];
uint8_t inputLen = 0;

// ─────────────────────────────────────────────────────────────────
const char* stateToStr(BalanceState s) {
    switch (s) {
        case BalanceState::IDLE:    return "IDLE";
        case BalanceState::RUNNING: return "RUN";
        case BalanceState::FALLEN:  return "FALL";
        case BalanceState::ERROR:   return "ERR";
    }
    return "?";
}

void applyPID() {
    balancer.setPIDTunings(runtimeKp, runtimeKi, runtimeKd);
    Serial.print(F("[PID] Kp="));  Serial.print(runtimeKp, 2);
    Serial.print(F("  Ki="));      Serial.print(runtimeKi, 3);
    Serial.print(F("  Kd="));      Serial.println(runtimeKd, 3);
}

void printHelp() {
    Serial.println(F("\n========= BALANCE TEST MODE ========="));
    Serial.println(F("Lệnh đơn phím:"));
    Serial.println(F("  e       — Enable PID"));
    Serial.println(F("  d       — Disable / dừng motor"));
    Serial.println(F("  r       — Reset filter+PID, re-enable"));
    Serial.println(F("  p       — Hiển thị PID hiện tại"));
    Serial.println(F("  h       — Help"));
    Serial.println(F(""));
    Serial.println(F("Lệnh nhập số (gõ xong nhấn Enter):"));
    Serial.println(F("  P <kp> <ki> <kd>   vd: P 35.0 0.0 0.8"));
    Serial.println(F("  x <0-5>            chọn cặp trục tính góc"));
    Serial.println(F("     0 = atan2(X,Z)/gyroY  [mặc định]"));
    Serial.println(F("     1 = atan2(Y,Z)/gyroX"));
    Serial.println(F("     2 = atan2(X,Y)/gyroZ"));
    Serial.println(F("     3 = atan2(-X,Z)/gyroY"));
    Serial.println(F("     4 = atan2(-Y,Z)/gyroX"));
    Serial.println(F("     5 = atan2(-X,Y)/gyroZ"));
    Serial.println(F(""));
    Serial.println(F("CSV: time_ms, angle, accAngle, pid_out, L_pwm, R_pwm, state"));
    Serial.println(F("=====================================\n"));
}

// Xử lý một dòng lệnh hoàn chỉnh (sau Enter)
void processLine(char* line) {
    // Trim trailing whitespace/CR
    int len = strlen(line);
    while (len > 0 && (line[len-1] == '\r' || line[len-1] == ' ')) {
        line[--len] = 0;
    }
    if (len == 0) return;

    char cmd = line[0];

    // ── Lệnh đơn ─────────────────────────────────────────────
    if (len == 1) {
        switch (cmd) {
            case 'e': case 'E':
                filter.reset(0.0f);
                balancer.enable();
                Serial.println(F("[CMD] ENABLED"));
                break;
            case 'd': case 'D':
                balancer.disable();
                Serial.println(F("[CMD] DISABLED"));
                break;
            case 'r': case 'R':
                filter.reset(0.0f);
                balancer.disable();
                delay(50);
                balancer.enable();
                Serial.println(F("[CMD] RESET + ENABLED"));
                break;
            case 'p': case 'P':
                applyPID();
                break;
            case 'h': case 'H':
                printHelp();
                break;
            default:
                Serial.print(F("[?] Lệnh không rõ: "));
                Serial.println(line);
                break;
        }
        return;
    }

    // ── Lệnh nhập số ─────────────────────────────────────────
    if (cmd == 'P' && line[1] == ' ') {
        // P kp ki kd
        float kp, ki, kd;
        int n = sscanf(line + 2, "%f %f %f", &kp, &ki, &kd);
        if (n == 3 && kp >= 0 && ki >= 0 && kd >= 0) {
            runtimeKp = kp;
            runtimeKi = ki;
            runtimeKd = kd;
            applyPID();
        } else {
            Serial.println(F("[ERR] Dùng: P <kp> <ki> <kd>  vd: P 35.0 0.0 0.8"));
        }
        return;
    }

    if ((cmd == 'x' || cmd == 'X') && line[1] == ' ') {
        // x 0..5
        int mode = atoi(line + 2);
        if (mode >= 0 && mode <= 5) {
            axisMode = (uint8_t)mode;
            filter.reset(0.0f);
            Serial.print(F("[AXIS] Mode "));
            Serial.print(axisMode);
            Serial.print(F(": "));
            Serial.println(axisModeStr[axisMode]);
            Serial.println(F("  → Filter reset"));
        } else {
            Serial.println(F("[ERR] mode phải từ 0 đến 5"));
        }
        return;
    }

    Serial.print(F("[?] Lệnh không rõ: "));
    Serial.println(line);
}

// ═══════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    delay(500);
    printHelp();

    // DRV8833 enable
    pinMode(PIN_DRV_ULT, OUTPUT);
    digitalWrite(PIN_DRV_ULT, HIGH);
    Serial.println(F("[INIT] DRV8833 nEn = HIGH"));

    // IMU
    Serial.print(F("[INIT] MPU6050... "));
    if (!imu.begin()) {
        while (true) {
            Serial.println(F("[ERR] MPU6050 không phản hồi!"));
            delay(2000);
        }
    }
    Serial.println(F("OK"));

    // Motors
    Serial.print(F("[INIT] Motor trái... "));
    motorLeft.begin();
    Serial.println(F("OK"));

    Serial.print(F("[INIT] Motor phải... "));
    motorRight.begin();
    Serial.println(F("OK"));

    Serial.print(F("[INIT] Encoder trái (GPIO "));
    Serial.print(PIN_ENC_L_A); Serial.print(F(", "));
    Serial.print(PIN_ENC_L_B); Serial.print(F(")... "));
    encLeft.begin();
    Serial.println(F("OK"));

    Serial.print(F("[INIT] Encoder phải (GPIO "));
    Serial.print(PIN_ENC_R_A); Serial.print(F(", "));
    Serial.print(PIN_ENC_R_B); Serial.print(F(")... "));
    encRight.begin();
    Serial.println(F("OK"));

    balancer.begin();

    Serial.println(F("[INIT] Sẵn sàng. Gõ 'e' để bắt đầu.\n"));
    Serial.println(F("time_ms,angle,accAngle,pid_out,L_pwm,R_pwm,L_rpm,R_rpm,L_ticks,R_ticks,state"));

    lastLoopUs  = micros();
    lastPrintMs = millis();
}

// ═══════════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════════
void loop() {
    // ── Đọc Serial từng ký tự, xử lý khi gặp Enter ──────────
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputLen > 0) {
                inputBuf[inputLen] = 0;
                processLine(inputBuf);
                inputLen = 0;
            }
        } else {
            if (inputLen < sizeof(inputBuf) - 1) {
                inputBuf[inputLen++] = c;
            }
        }
    }

    // ── Control loop 200 Hz ───────────────────────────────────
    uint32_t nowUs    = micros();
    uint32_t elapsed  = nowUs - lastLoopUs;
    if (elapsed < LOOP_INTERVAL_US) return;
    lastLoopUs = nowUs;

    float dt = elapsed * 1e-6f;

    // ── Đọc IMU ───────────────────────────────────────────────
    imu.update();
    IMUData d = imu.getData();

    // ── Tính góc theo axisMode ────────────────────────────────
    float a1, a2, gyroRate;
    switch (axisMode) {
        default:
        case 0: a1 =  d.accX; a2 = d.accZ; gyroRate =  d.gyroY; break;
        case 1: a1 =  d.accY; a2 = d.accZ; gyroRate =  d.gyroX; break;
        case 2: a1 =  d.accX; a2 = d.accY; gyroRate =  d.gyroZ; break;
        case 3: a1 = -d.accX; a2 = d.accZ; gyroRate =  d.gyroY; break;
        case 4: a1 = -d.accY; a2 = d.accZ; gyroRate =  d.gyroX; break;
        case 5: a1 = -d.accX; a2 = d.accY; gyroRate =  d.gyroZ; break;
    }

    float accAngleDeg  = atan2f(a1, a2) * (180.0f / PI);
    float gyroRateDegS = gyroRate * (180.0f / PI);   // rad/s → deg/s
    float filteredAngle = filter.updateWithAngle(accAngleDeg, gyroRateDegS, dt);

    balancer.update(filteredAngle, dt);

    // ── Encoder update ────────────────────────────────────────
    encLeft.update(dt);
    encRight.update(dt);

    // ── CSV 20 Hz ─────────────────────────────────────────────
    uint32_t nowMs = millis();
    if (nowMs - lastPrintMs >= 50) {
        lastPrintMs = nowMs;
        EncoderData el = encLeft.getData();
        EncoderData er = encRight.getData();
        Serial.print(nowMs);
        Serial.print(','); Serial.print(filteredAngle, 2);
        Serial.print(','); Serial.print(accAngleDeg, 2);
        Serial.print(','); Serial.print(balancer.getPIDOutput(), 1);
        Serial.print(','); Serial.print(balancer.getLeftPWM());
        Serial.print(','); Serial.print(balancer.getRightPWM());
        Serial.print(','); Serial.print(el.rpm, 1);
        Serial.print(','); Serial.print(er.rpm, 1);
        Serial.print(','); Serial.print(el.ticks);
        Serial.print(','); Serial.print(er.ticks);
        Serial.print(','); Serial.println(stateToStr(balancer.getState()));
    }
}