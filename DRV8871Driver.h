#pragma once

#include "ReelTwo.h"
#define SHADOW_VERBOSE(...) printf(__VA_ARGS__);
/**
  * \ingroup Motor
  *
  * \class DRV8871Driver
  *
  * \brief Implements support for the Adafruit DRV8871 MotorDriver Breakout board
*/
class DRV8871Driver {
public:

    /** \brief Constructor
      *
      * Construct a new instance of DRV8871Driver.
      *
      * \param pwm1 The pin number connected to the PWM1 input on the motor driver carrier
      * \param pwm2 The pin number connected to the PWM2 input on the motor driver carrier
      */
    DRV8871Driver(
            uint8_t pwm1,
            uint8_t pwm2) :
       fPWM1(pwm1),
       fPWM2(pwm2)
    {
        pinMode(fPWM1, OUTPUT);
        pinMode(fPWM2, OUTPUT);
        stop();
    }

    void setDeadband(uint8_t value) {
        deadband = min((int) value, 127);
    }

    void setRamping(float_t value) {
        rampPWMperMs = abs(value);
    }

    void stop() {
        motor(0);
    }

    //Set motor speed, -127 <= speed <= 127
    void motor(int8_t speed) {
        SHADOW_VERBOSE("DRV8871.motor speed = %d\n", speed);
        lastCommandMs = millis();
        if (abs(speed) <= deadband) {
            speed = 0;
            requestedPWM = 0;
        } else {
            requestedPWM = (int16_t) (((float) speed) * 512.0 / 256.0);
        }
        SHADOW_VERBOSE("DRV8871.motor requestedPWM = %d\n", requestedPWM);
        task();
    }

    //motor will automatically stop this many (hundredsOfMillis * 100) milliseconds after last motor() command
    //Note, that this feature requires the heartbeat method to be called periodically
    void setTimeout(int hundredsOfMillis) {
        if (hundredsOfMillis < 1) {
            hundredsOfMillis = 1;
        }
        timeoutMs = hundredsOfMillis * 100;
    }
    
    void task() {
        ulong now = millis();
        if ((requestedPWM != 0) && (now > (lastCommandMs + timeoutMs))) {
            SHADOW_VERBOSE("DRV8871.task timeout happened now=%d, lastCmd=%d, timeout=%d\n", now, lastCommandMs, timeoutMs);
            requestedPWM = 0;
            lastCommandMs = now;
        }
        if (requestedPWM != currentPWM) {
            SHADOW_VERBOSE("DRV8871.task requestPWM=%d, currentPWM=%d\n", requestedPWM, currentPWM);
            int16_t delta = abs(currentPWM - requestedPWM);
            int16_t maxDelta = (int16_t) ((now - lastUpdateMs) * rampPWMperMs);
            if (delta > maxDelta) {
                delta = maxDelta;
            }
            SHADOW_VERBOSE("DRV8871.task delta=%d\n", delta);
            if (delta > 0) {
                if (currentPWM > requestedPWM) {
                    currentPWM = max(-255, currentPWM - delta);
                } else {
                    currentPWM = min(255, currentPWM + delta);
                }
                setMotorSpeed(currentPWM);
                lastUpdateMs = now;
            }
        }
    }


private:
    // Pin values
    uint8_t fPWM1;  // PWM for OUT1 (PWM1)
    uint8_t fPWM2;  // PWM for OUT2 (PWM2)
    uint16_t timeoutMs = 100;
    ulong lastCommandMs = 0;
    ulong lastUpdateMs = 0;
    uint8_t deadband = 3;
    float_t rampPWMperMs = 0.1;
    int16_t requestedPWM = 0;
    int16_t currentPWM = 0;

    void setMotorSpeed(int16_t speed) {
        SHADOW_VERBOSE("DRV8871.setMotorSpeed %d\n", speed);
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
