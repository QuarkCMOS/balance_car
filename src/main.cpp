#include <Arduino.h>

// Bật TEST_MODE trong platformio.ini:
// build_flags = -D TEST_MODE
#ifdef TEST_MODE
  #include "test/test_motor_pwm.cpp"
#endif