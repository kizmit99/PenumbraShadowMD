#include <Arduino.h>
#include "GameController.h"
#include "blering/DualRingBLE.h"
#include "../LocalPreferences.h"

#ifndef SHADOW_DEBUG
#define SHADOW_DEBUG(...)
#endif

#define PREFERENCE_PS3_FOOT_MAC        "ps3footmac"
#define PREFERENCE_PS3_DOME_MAC        "ps3domemac"
#define PS3_CONTROLLER_DEFAULT_MAC     "XX:XX:XX:XX:XX:XX"
#define PS3_CONTROLLER_FOOT_MAC        PS3_CONTROLLER_DEFAULT_MAC
#define PS3_CONTROLLER_DOME_MAC        PS3_CONTROLLER_DEFAULT_MAC
#define PS3_CONTROLLER_BACKUP_FOOT_MAC PS3_CONTROLLER_DEFAULT_MAC
#define PS3_CONTROLLER_BACKUP_DOME_MAC PS3_CONTROLLER_DEFAULT_MAC


class DualRingController : public GameController {
public:

    DualRingController(const char* preferenceNamespace) :
        preferences(preferenceNamespace)
    {
        if (DualRingController::instance != NULL) {
            SHADOW_DEBUG("\nFATAL Problem - constructor for DualRingController called more than once!\r\n");
            while (1);
        }
        rings.setNamespace(preferenceNamespace);
        DualRingController::instance = this;
    }

    //Implement the GameController Interface methods

    bool init() {
        rings.init("DualRings");
        return true;
    }

    void task() {
        bool beforeConnected = rings.isConnected();
        bool beforeFault = rings.hasFault();
        rings.task();
        bool afterConnected = rings.isConnected();
        bool afterFault = rings.hasFault();
        if ((beforeConnected != afterConnected) ||
            (beforeFault != afterFault)) {
            statusChangeCallback(this);
        }
    }

    void setStatusChangedCallback(void (*callback)(GameController*)) {
        this->statusChangeCallback = callback;
    }


    bool isConnected() {
        return rings.isConnected();
    }

    bool hasFault() {
        return rings.hasFault();
    }

    void clearFault() {
        isFaulted = false;
    }

    void setTimeoutWindow(uint32_t timeoutMs) {
        timeoutWindow = timeoutMs;
    }

    void setSensitiveTimeoutWindow(uint32_t timeoutMs) {
        sensitiveTimeoutWindow = timeoutMs;
    }

    void enableSensitiveTimeout(bool enable = true) {
        sensitiveTimeoutEnabled = enable;
    }

    int8_t getJoystick(GameController::Controller controller, GameController::Axis axis) {
        //Return the requested joystick, but if returning drive stick need to ensure that Neither L1 nor L2 is pressed on the stick controller
        //If only the drive controller is connected (no dome controller), then L2 pressed on drive controller means dome joystick
        //Note that (apparently) the PS3 joystick X/Y axis designations are reversed from GameController X/Y
        //Also note that this method returns an int, not a uint!  Range is -128 to 127, not 0 to 255!

        DualRingBLE::Controller mappedController;
        if (controller == GameController::Drive) {
            mappedController = DualRingBLE::Drive;
        } else {
            mappedController = DualRingBLE::Dome;
        }
        DualRingBLE::Axis mappedAxis;
        if (axis == GameController::X) {
            mappedAxis = DualRingBLE::X;
        } else {
            mappedAxis = DualRingBLE::Y;
        }
        return rings.getJoystick(mappedController, mappedAxis);
    }

    String getAction() {
        //TODO - Note, returns the triggers defined in terms of Sony Buttons!
        return String("");
    }

private:

    LocalPreferences preferences;

    static DualRingController* instance;
    void (*statusChangeCallback)(GameController*);

    bool isFaulted = false;
    uint32_t timeoutWindow = 10000;
    uint32_t sensitiveTimeoutWindow = 0;
    bool sensitiveTimeoutEnabled = false;

};

DualRingController* DualRingController::instance = NULL;
DualRingBLE rings;