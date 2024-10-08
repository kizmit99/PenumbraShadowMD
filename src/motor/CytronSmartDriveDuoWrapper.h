#pragma once

#include "MotorDriver.h"
#include <motor/CytronSmartDriveDuoDriver.h>

/**
 * @file CytronSmartDriveDuoWrapper.h
 * @author kizmit99 (kizmit99@gmail.com)
 * @brief   This class implements a wrapper around the CytronSmartDriveDuo Motor Controller driver 
 *          provided by the ReelTwo library.  It conforms that driver to the MotorDriver interface.
 * @version 1.0
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

class CytronSmartDriveDuoWrapper : public MotorDriver 
{
public:

    /** \brief Constructor
      *
      * Construct a new instance of CytronSmartDriveDuoWrapper.
      *
    \param address The driver address.
    \param port    The port to use.
      */
    CytronSmartDriveDuoWrapper(byte address, Stream& port, uint8_t initialByte = 0x80) : wrapped(address, port, initialByte) {}

    inline void setDeadband(uint8_t value) {
        wrapped.setDeadband((byte) value);
    }

    void setRamping(float_t value) {
        //Not supported
    }

    void stop() {
        wrapped.stop();
    }

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
        //Not supported
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
    CytronSmartDriveDuoDriver wrapped;
};

class CytronSmartDriveDuoMDDS10Wrapper : public CytronSmartDriveDuoWrapper
{
public:
	CytronSmartDriveDuoMDDS10Wrapper(byte address, Stream& port) :
		CytronSmartDriveDuoWrapper(address, port, 0x55) {}
};

class CytronSmartDriveDuoMDDS30Wrapper : public CytronSmartDriveDuoWrapper
{
public:
	CytronSmartDriveDuoMDDS30Wrapper(byte address, Stream& port) :
		CytronSmartDriveDuoWrapper(address, port, 0x80) {}
};

class CytronSmartDriveDuoMDDS60Wrapper : public CytronSmartDriveDuoWrapper
{
public:
	CytronSmartDriveDuoMDDS60Wrapper(byte address, Stream& port) :
		CytronSmartDriveDuoWrapper(address, port, 0x55) {}
};