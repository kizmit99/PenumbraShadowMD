#pragma once
#include <Arduino.h>

/**
 * @file MotorDriver.h
 * @author kizmit99 (kizmit99@gmail.com)
 * @brief   This class represents an interface to a generic motor driver implementation.
 *          Specific motor driver implementations (or wrappers for those drivers) should
 *          extend this class as a way of implementing the interface.
 * @version 1.0
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

class MotorDriver 
{
public:
    virtual void task() = 0;
    virtual void motor(int8_t power) = 0;
    virtual void drive(int8_t power) = 0;
    virtual void turn(int8_t power) = 0;
    virtual void stop() = 0;
    virtual void setDeadband(uint8_t value) = 0;
    virtual void setRamping(float_t value) = 0;
    virtual void setTimeout(int milliseconds) = 0;
    virtual void setBaudRate(long baudRate) = 0;
};
