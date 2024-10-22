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
        //Note that this method returns an int, not a uint!  Range is -128 to 127, not 0 to 255!

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
        //This method also needs to handle turning continuous button presses into a single action (possibly repeating)
        //Ok, that's not true.  Apparently the calling code handles that.

        const char* rawAction = getRawAction();
        // if (rawAction != "") {
        //     SHADOW_DEBUG("Action: %s\n", rawAction);
        //     rings.printState();
        // }
        return rawAction;
    }

    void printState() {
        rings.printState();
    }

    const char* getRawAction() {
        if (rings.isButtonClicked(DualRingBLE::Drive, DualRingBLE::L1)) return "FTbtnDown_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Up)) return "btnUP_CROSS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Down)) return "btnDown_CROSS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Left)) return "FTbtnDown_CROSS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Right)) return "FTbtnUP_CROSS_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Up)) return "FTbtnUP_L1_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Down)) return "FTbtnDown_L1_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Left)) return "btnDown_L1_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Right)) return "";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Up)) return "btnUP_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Down)) return "btnDown_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Left)) return "btnLeft_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::L2) &&
            rings.isButtonPressed(DualRingBLE::Dome, DualRingBLE::Right)) return "btnRight_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Up)) return "FTbtnUP_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Down)) return "btnUP_L1_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Left)) return "btnDown_L1_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Right)) return "btnUP_CIRCLE_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Up)) return "btnDown_CIRCLE_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Down)) return "btnRight_CIRCLE_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Left)) return "btnLeft_CIRCLE_MD";

        if (!rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Right)) return "btnLeft_L1_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Up)) return "FTbtnUP_L1_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Down)) return "FTbtnDown_L1_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Left)) return "FTbtnLeft_L1_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Right)) return "FTbtnRight_L1_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Up)) return "FTbtnUP_PS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Down)) return "FTbtnDown_PS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Left)) return "FTbtnLeft_PS_MD";

        if (rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::B) &&
            !rings.isModifierPressed(DualRingBLE::Drive, DualRingBLE::L2) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::A) &&
            !rings.isModifierPressed(DualRingBLE::Dome, DualRingBLE::B) &&
            rings.isButtonPressed(DualRingBLE::Drive, DualRingBLE::Right)) return "FTbtnRight_PS_MD";

        return "";
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