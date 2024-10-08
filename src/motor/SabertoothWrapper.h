#pragma once

#include "MotorDriver.h"
#include <motor/SabertoothDriver.h>

/**
 * @file SabertoothWrapper.h
 * @author kizmit99 (kizmit99@gmail.com)
 * @brief   This class implements a wrapper around the Sabertooth Motor Controller driver 
 *          provided by the ReelTwo library.  It conforms that driver to the MotorDriver interface.
 * @version 1.0
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

class SabertoothWrapper : public MotorDriver 
{
public:

    /** \brief Constructor
      *
      * Construct a new instance of SabertoothWrapper.
      *
    \param address The driver address.
    \param port    The port to use.
      */
    SabertoothWrapper(byte address, Stream& port) : wrapped(address, port) {}

    inline void setDeadband(uint8_t value) {
        wrapped.setDeadband((byte) value);
    }

    void setRamping(float_t value) {
        //This is a somewhat bogus mapping of incoming float value (specified in power/ms) to 
        //whatever values the Sabertooth driver is actually expecting.
        //Use with caution.
        wrapped.setRamping((byte) max(127, (int) value));
    }

    void stop() {
        wrapped.stop();
    }

    //Set motor speed, -127 <= speed <= 127
    void motor(int8_t speed) {
        wrapped.motor(speed);
    }

    void drive(int8_t power) {
        wrapped.drive(power);
    }

    void turn(int8_t power) {
        wrapped.turn(power);
    }

    void setBaudRate(long baudRate) {
        wrapped.setBaudRate(baudRate);
    }

    //motor will automatically stop this many milliseconds after last motor() command
    //Note, that this feature requires the heartbeat method to be called periodically
    void setTimeout(int milliseconds) {
        wrapped.setTimeout(milliseconds);
    }
    
    void task() {
        //NOOP
    }

private:
    SabertoothDriver wrapped;
};
