#pragma once
#include <Arduino.h>

/**
 * @file GameController.h
 * @author kizmit99 (kizmit99@gmail.com)
 * @brief   This class represents an interface to a generic Game Controller implementation.
 *          Specific implementations (or wrappers for existing drivers) should
 *          extend this class as a way of implementing the interface.
 * @version 1.0
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

class GameController 
{
public:
    enum Controller {
        Drive, Dome};

    enum Axis {
        X, Y};

    virtual bool init() = 0;
    virtual void task() = 0;
    virtual bool isConnected() = 0;
    virtual bool hasFault() = 0;
    virtual void clearFault() = 0;
    virtual int8_t getJoystick(Controller controller, Axis axis) = 0;
    virtual String getAction() = 0;
    virtual void setStatusChangedCallback(void (*callback)(GameController*)) = 0;
    virtual void setTimeoutWindow(uint32_t timeoutMs) = 0;
    virtual void setSensitiveTimeoutWindow(uint32_t timeoutMs) = 0;
    virtual void enableSensitiveTimeout(bool enable = true) = 0;
    virtual void printState() = 0;
};
