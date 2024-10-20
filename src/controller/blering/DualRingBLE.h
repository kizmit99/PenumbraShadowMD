#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "Ring.h"
#include "../../LocalPreferences.h"


/**
 * This class encapsulates access to two physical Magicsee R1 remote controllers.
 * 
 */
class DualRingBLE {
public:
    enum Modifier {
        A, B};

    enum Button {
        Up, Down, Left, Right};

    enum Clicker {
        C, D, L1};

    enum Controller {
        Drive, Dome};

    enum Axis {
        X, Y};

    DualRingBLE(const char* preferenceNamespace) :
        preferences(preferenceNamespace) {}

    DualRingBLE() {}

    void init(const char *name);
    void setNamespace(const char *);
    void task();

    bool isConnected();
    bool hasFault();

    bool isModifierPressed(Controller controller, Modifier button);
    bool isButtonPressed(Controller controller, Button button);
    bool isButtonClicked(Controller controller, Clicker button);
    int8_t getJoystick(Controller controller, Axis direction);

    Ring* getRing(Controller controller);
    void clearMACMap();

private:
    Ring driveRing;
    Ring domeRing;
    const char* name;
    LocalPreferences preferences;
};

// Reference to the single instance of this class
extern DualRingBLE rings;
