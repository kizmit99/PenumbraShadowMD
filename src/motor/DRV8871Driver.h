#pragma once

#include "MotorDriver.h"

#ifndef SHADOW_DEBUG
#define SHADOW_DEBUG(...) printf(__VA_ARGS__);
#endif

/**
 * @file DRV8871Driver.h
 * @author kizmit99 (kizmit99@gmail.com)
 * @brief   This class implements a driver to control the DRV8871 motor driver chip.
 * @version 1.0
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

class DRV8871Driver : public MotorDriver 
{
public:

    /** \brief Constructor
      *
      * Construct a new instance of DRV8871Driver.
      *
      * \param pwm1 The pin number connected to the PWM1 input on the motor driver carrier
      * \param pwm2 The pin number connected to the PWM2 input on the motor driver carrier
      */
    DRV8871Driver(uint8_t pwmPin1, uint8_t pwmPin2) :
        fPWM1(pwmPin1),
        fPWM2(pwmPin2) {

        pinMode(fPWM1, OUTPUT);
        pinMode(fPWM2, OUTPUT);
        stop();
    }

    void setDeadband(uint8_t value) {
        deadband = min((int) value, 127);
    }

    void setRamping(float_t value) {
        rampPowerPerMs = abs(value);
    }

    void stop() {
        motor(0);
    }


    void motor(int8_t speed) {
        SHADOW_DEBUG("DRV8871.motor speed = %d\n", speed);
        lastCommandMs = millis();
        if (abs(speed) <= deadband) {
            speed = 0;
            requestedPWM = 0;
        } else {
            requestedPWM = (int16_t) (((float) speed) * 512.0 / 256.0);
        }
        SHADOW_DEBUG("DRV8871.motor requestedPWM = %d\n", requestedPWM);
        task();
    }

    void drive(int8_t power) {
        //This is a single motor driver
        motor(power);
    }

    void turn(int8_t power) {
        //This is a single motor driver, so stop instead.
        stop();
    }

    void setBaudRate(long baudRate) {
        //Not needed
    }

    //motor will automatically stop this many milliseconds after last motor() command
    //Note, that this feature requires the heartbeat method to be called periodically
    void setTimeout(int milliseconds) {
        if (milliseconds < 1) {
            milliseconds = 1;
        }
        timeoutMs = milliseconds;
    }
    
    void task() {
        ulong now = millis();
        if ((requestedPWM != 0) && (now > (lastCommandMs + timeoutMs))) {
            SHADOW_DEBUG("DRV8871.task timeout happened now=%d, lastCmd=%d, timeout=%d\n", now, lastCommandMs, timeoutMs);
            requestedPWM = 0;
            lastCommandMs = now;
        }
        if (lastUpdateMs > now) {
            lastUpdateMs = now;
        }
        if (requestedPWM != currentPWM) {
            SHADOW_DEBUG("DRV8871.task requestPWM=%d, currentPWM=%d\n", requestedPWM, currentPWM);
            int16_t delta = abs(currentPWM - requestedPWM);
            int16_t maxDelta = (int16_t) ((now - lastUpdateMs) * rampPowerPerMs);
            if (delta > maxDelta) {
                delta = maxDelta;
            }
            SHADOW_DEBUG("DRV8871.task delta=%d\n", delta);
            if (delta > 0) {
                if (currentPWM > requestedPWM) {
                    currentPWM = max(-255, currentPWM - delta);
                } else {
                    currentPWM = min(255, currentPWM + delta);
                }
                setMotorSpeed(currentPWM);
            }
        }
        lastUpdateMs = now;
    }



private:
    // Pin Numbers
    uint8_t fPWM1;  // PWM for OUT1 (PWM1)
    uint8_t fPWM2;  // PWM for OUT2 (PWM2)

    uint16_t timeoutMs = 100;
    ulong lastCommandMs = 0;
    ulong lastUpdateMs = 0;
    uint8_t deadband = 3;
    float_t rampPowerPerMs = 0.1;
    int16_t requestedPWM = 0;
    int16_t currentPWM = 0;

    void setMotorSpeed(int16_t speed) {
        SHADOW_DEBUG("DRV8871.setMotorSpeed %d\n", speed);
        if (speed == 0) {
            analogWrite(fPWM1, 0);
            analogWrite(fPWM2, 0);
        } else if (speed > 0) {
            analogWrite(fPWM2, 0);
            analogWrite(fPWM1, (abs(speed)));
        } else {
            analogWrite(fPWM1, 0);
            analogWrite(fPWM2, (abs(speed)));
        }
    }
};
