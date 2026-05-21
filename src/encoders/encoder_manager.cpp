#include "encoder_manager.h"
#include "../config/pins.h"
#include "../config/constants.h"

volatile int global_ticks = 0; 

// ISR đếm xung
void IRAM_ATTR encoderISR() {
    if (digitalRead(ENCODER_M1_B) == HIGH) global_ticks++;
    else global_ticks--;
}

void EncoderManager::begin() {
    pinMode(ENCODER_M1_A, INPUT);
    pinMode(ENCODER_M1_B, INPUT);
    attachInterrupt(digitalPinToInterrupt(ENCODER_M1_A), encoderISR, RISING);
    
    ticks = lastTicks = 0;
    lastUpdateTime = millis();
}

void EncoderManager::update() {
    noInterrupts(); 
    ticks = global_ticks;
    interrupts();

    uint32_t now = millis();
    float dt = (now - lastUpdateTime) / 1000.0f; // s
    
    if (dt > 0) {
        int delta = ticks - lastTicks;
        encData.rpm = ((float)delta / TICKS_PER_REV) / (dt / 60.0f);
        encData.velocity = (encData.rpm * 2.0f * PI / 60.0f) * WHEEL_RADIUS;
        
        lastTicks = ticks;
        lastUpdateTime = now;
    }
    encData.ticks = ticks;
}

EncoderData EncoderManager::getData() { return encData; }