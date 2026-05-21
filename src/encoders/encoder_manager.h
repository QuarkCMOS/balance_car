#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include <Arduino.h>

struct EncoderData {
    int ticks;
    float rpm;
    float velocity;
};

class EncoderManager {
public:
    void begin();
    void update();
    EncoderData getData();
private:
    volatile int ticks;
    EncoderData encData;
    uint32_t lastUpdateTime;
    int lastTicks;
};

#endif