#include <Arduino.h>
#include "GameController.h"
#include <PS3BT.h>
#include <usbhub.h>
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


class DualSonyMoveController : public GameController {
public:

    DualSonyMoveController(const char* preferenceNamespace) :
        Btd(&Usb),
        PS3NavFootImpl(&Btd),
        PS3NavDomeImpl(&Btd),
        preferences(preferenceNamespace)
    {
        if (DualSonyMoveController::instance != NULL) {
            SHADOW_DEBUG("\nFATAL Problem - constructor for DualSonyMoveController called more than once!\r\n");
            while (1);
        }
        DualSonyMoveController::instance = this;
        PS3NavFoot = &PS3NavFootImpl;
        PS3NavDome = &PS3NavDomeImpl;

        //Setup for PS3
        PS3NavFoot->attachOnInit(DualSonyMoveController::onInitPS3NavFootWrapper);
        PS3NavDome->attachOnInit(DualSonyMoveController::onInitPS3NavDomeWrapper);
    }

    //Implement the GameController Interface methods

    bool init() {
        PS3ControllerFootMAC = preferences.getString(PREFERENCE_PS3_FOOT_MAC, PS3_CONTROLLER_FOOT_MAC);
        PS3ControllerDomeMAC = preferences.getString(PREFERENCE_PS3_DOME_MAC, PS3_CONTROLLER_DOME_MAC);

        return (Usb.Init() == 0);
    }

    void task() {
        Usb.Task();
    }

    void setStatusChangedCallback(void (*callback)(GameController*)) {
        this->statusChangeCallback = callback;
    }


    bool isConnected() {
        return mainControllerConnected;
    }

    bool hasFault() {
        return isFaulted;
    }

    void clearFault() {
        badPS3Data = 0;
        badPS3DataDome = 0;
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

        if (!PS3NavFoot->PS3NavigationConnected) {
            return 0;
        }

        switch (controller) {
            case Drive:
                if (PS3NavFoot->getButtonPress(L1) || PS3NavFoot->getButtonPress(L2)) {
                    return 0;
                }
                switch (axis) {
                    case X:
                        return (PS3NavFoot->getAnalogHat(LeftHatY) - 128);

                    case Y:
                        return (PS3NavFoot->getAnalogHat(LeftHatX) - 128);

                    default:
                        return 0;
                }

            case Dome: {
                PS3BT* ps3 = PS3NavDome;
                if (!ps3->PS3NavigationConnected) {
                    if (PS3NavFoot->PS3NavigationConnected && (PS3NavFoot->getButtonPress(L2))) {
                        ps3 = PS3NavFoot;
                    } else {
                        return 0;
                    }
                }
                switch (axis) {
                    case X:
                        return (ps3->getAnalogHat(LeftHatY) - 128);

                    case Y:
                        return (ps3->getAnalogHat(LeftHatX) - 128);

                    default:
                        return 0;
                }
            }

            default:
                return 0;
        }
        return 0;
    }

    String getAction() {
    //------------------------------------ 
    // Send triggers for the base buttons 
    //------------------------------------
        if (PS3NavFoot->getButtonPress(UP) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS))
        {
            if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
            {
                // Skip this section
            }
            else
            {
                return "btnUP_MD";
            }
        }
    
        if (PS3NavFoot->getButtonPress(DOWN) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS))
        {
            if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
            {
                // Skip this section
            }
            else
            {     
                return "btnDown_MD";
            }
        }
    
        if (PS3NavFoot->getButtonPress(LEFT) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS))
        {
            if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
            {
                // Skip this section
            }
            else
            {           
                return "btnLeft_MD";
            }
        }

        if (PS3NavFoot->getButtonPress(RIGHT) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(L1) && !PS3NavFoot->getButtonPress(PS))
        {
            if (PS3NavDome->PS3NavigationConnected && (PS3NavDome->getButtonPress(CROSS) || PS3NavDome->getButtonPress(CIRCLE) || PS3NavDome->getButtonPress(PS)))
            {
                // Skip this section
            }
            else
            {     
                return "btnRight_MD";
            }
        }
    
    //------------------------------------ 
    // Send triggers for the CROSS + base buttons 
    //------------------------------------
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(CROSS))))
        {
            return "btnUP_CROSS_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(CROSS))))
        {      
            return "btnDown_CROSS_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(CROSS))))
        {
            return "btnLeft_CROSS_MD";
        }

        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CROSS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(CROSS))))
        {
            return "btnRight_CROSS_MD";
        }

    //------------------------------------ 
    // Send triggers for the CIRCLE + base buttons 
    //------------------------------------
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(CIRCLE))))
        {
            return "btnUP_CIRCLE_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(CIRCLE))))
        {
            return "btnDown_CIRCLE_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(CIRCLE))))
        {
            return "btnLeft_CIRCLE_MD";
        }

        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CIRCLE)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(CIRCLE))))
        {
            return "btnRight_CIRCLE_MD";
        }
    
    //------------------------------------ 
    // Send triggers for the L1 + base buttons 
    //------------------------------------
        if (PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(L1))
        {
            return "btnUP_L1_MD";
        }
        
        if (PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(L1))
        {
            return "btnDown_L1_MD";
        }
        
        if (PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(L1))
        {
            return "btnLeft_L1_MD";
        }

        if (PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(L1))
        {
            return "btnRight_L1_MD";
        }
    
    //------------------------------------ 
    // Send triggers for the PS + base buttons 
    //------------------------------------
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(UP) && PS3NavDome->getButtonPress(PS))))
        {
            return "btnUP_PS_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(DOWN) && PS3NavDome->getButtonPress(PS))))
        {
            return "btnDown_PS_MD";
        }
        
        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(LEFT) && PS3NavDome->getButtonPress(PS))))
        {
            return "btnLeft_PS_MD";
        }

        if (((!PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(PS)) || (PS3NavDome->PS3NavigationConnected && PS3NavFoot->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(PS))))
        {
            return "btnRight_PS_MD";
        }

    //------------------------------------ 
    // Send triggers for the base buttons 
    //------------------------------------
        if (PS3NavDome->getButtonPress(UP) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnUP_MD";
        }
        
        if (PS3NavDome->getButtonPress(DOWN) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnDown_MD";
        }
        
        if (PS3NavDome->getButtonPress(LEFT) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnLeft_MD";
        }

        if (PS3NavDome->getButtonPress(RIGHT) && !PS3NavDome->getButtonPress(CROSS) && !PS3NavDome->getButtonPress(CIRCLE) && !PS3NavDome->getButtonPress(L1) && !PS3NavDome->getButtonPress(PS) && !PS3NavFoot->getButtonPress(CROSS) && !PS3NavFoot->getButtonPress(CIRCLE) && !PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnRight_MD";
        }
    
    //------------------------------------ 
    // Send triggers for the CROSS + base buttons 
    //------------------------------------
        if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(CROSS))
        {
            return "FTbtnUP_CROSS_MD";
        }
        
        if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CROSS))
        {
            return "FTbtnDown_CROSS_MD";
        }
        
        if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CROSS))
        {
            return "FTbtnLeft_CROSS_MD";
        }

        if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CROSS))
        {
            return "FTbtnRight_CROSS_MD";
        }

    //------------------------------------ 
    // Send triggers for the CIRCLE + base buttons 
    //------------------------------------
        if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(CIRCLE))
        {
            return "FTbtnUP_CIRCLE_MD";
        }
        
        if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(CIRCLE))
        {
            return "FTbtnDown_CIRCLE_MD";
        }
        
        if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(CIRCLE))
        {
            return "FTbtnLeft_CIRCLE_MD";
        }

        if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(CIRCLE))
        {
            return "FTbtnRight_CIRCLE_MD";
        }
    
    //------------------------------------ 
    // Send triggers for the L1 + base buttons 
    //------------------------------------
        if (PS3NavDome->getButtonPress(UP) && PS3NavDome->getButtonPress(L1))
        {
            return "FTbtnUP_L1_MD";
        }
        
        if (PS3NavDome->getButtonPress(DOWN) && PS3NavDome->getButtonPress(L1))
        {
            return "FTbtnDown_L1_MD";
        }
        
        if (PS3NavDome->getButtonPress(LEFT) && PS3NavDome->getButtonPress(L1))
        {
            return "FTbtnLeft_L1_MD";
        }

        if (PS3NavDome->getButtonPress(RIGHT) && PS3NavDome->getButtonPress(L1))
        {
            return "FTbtnRight_L1_MD";
        }
    
    //------------------------------------ 
    // Send triggers for the PS + base buttons 
    //------------------------------------
        if (PS3NavDome->getButtonPress(UP) && PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnUP_PS_MD";
        }
        
        if (PS3NavDome->getButtonPress(DOWN) && PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnDown_PS_MD";
        }
        
        if (PS3NavDome->getButtonPress(LEFT) && PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnLeft_PS_MD";
        }

        if (PS3NavDome->getButtonPress(RIGHT) && PS3NavFoot->getButtonPress(PS))
        {
            return "FTbtnRight_PS_MD";
        }

    //------------------------------------
    // Send triggers for command toggles
    //------------------------------------
        // enable / disable drive stick
        if (PS3NavFoot->getButtonPress(PS) && PS3NavFoot->getButtonClick(CROSS))
        {
            return "stickDisable";
        }
        
        if(PS3NavFoot->getButtonPress(PS) && PS3NavFoot->getButtonClick(CIRCLE))
        {
            return "stickEnable";
        }
        
        // Enable and Disable Overspeed
        if (PS3NavFoot->getButtonPress(L3) && PS3NavFoot->getButtonPress(L1))
        {
            return "toggleSpeed";
        }
    
        // Enable Disable Dome Automation
        if(PS3NavFoot->getButtonPress(L2) && PS3NavFoot->getButtonClick(CROSS))
        {
            return "autoDomeOff";
        } 

        if(PS3NavFoot->getButtonPress(L2) && PS3NavFoot->getButtonClick(CIRCLE))
        {
            return "autoDomeOn";
        } 

        return String("");
    }


    //Methods required to wrap the original PS3 code

    void onInitPS3NavFoot() {
        String btAddress = getLastConnectedBtMAC();
        PS3NavFoot->setLedOn(LED1);
        isPS3NavigatonInitialized = true;
        badPS3Data = 0;

        SHADOW_DEBUG("\nBT Address of Last connected Device when FOOT PS3 Connected: %s\n", btAddress.c_str());
        
        if (btAddress == PS3ControllerFootMAC || btAddress == PS3ControllerBackupFootMAC)
        {
            SHADOW_DEBUG("\nWe have our FOOT controller connected.\n")
            
            mainControllerConnected = true;
            WaitingforReconnect = true;
        }
        else if (PS3ControllerFootMAC[0] == 'X')
        {
            SHADOW_DEBUG("\nAssigning %s as FOOT controller.\n", btAddress.c_str());
            
            preferences.putString(PREFERENCE_PS3_FOOT_MAC, btAddress.c_str());
            PS3ControllerFootMAC = btAddress;
            mainControllerConnected = true;
            WaitingforReconnect = true;
        }
        else
        {
            // Prevent connection from anything but the MAIN controllers          
            SHADOW_DEBUG("\nWe have an invalid controller trying to connect as the FOOT controller, it will be dropped.\n")

            PS3NavFoot->setLedOff(LED1);
            PS3NavFoot->disconnect();
        
            isPS3NavigatonInitialized = false;
            mainControllerConnected = false;        
        } 
        statusChangeCallback(this);
    }

    void onInitPS3NavDome() {
        String btAddress = getLastConnectedBtMAC();
        PS3NavDome->setLedOn(LED1);
        isSecondaryPS3NavigatonInitialized = true;
        badPS3Data = 0;
        
        if (btAddress == PS3ControllerDomeMAC || btAddress == PS3ControllerBackupDomeMAC)
        {
            SHADOW_DEBUG("\nWe have our DOME controller connected.\n")
            
            domeControllerConnected = true;
            WaitingforReconnectDome = true;
        }
        else if (PS3ControllerDomeMAC[0] == 'X')
        {
            SHADOW_DEBUG("\nAssigning %s as DOME controller.\n", btAddress.c_str());
            
            preferences.putString(PREFERENCE_PS3_DOME_MAC, btAddress.c_str());
            PS3ControllerDomeMAC = btAddress;

            domeControllerConnected = true;
            WaitingforReconnectDome = true;
        }
        else
        {
            // Prevent connection from anything but the DOME controllers          
            SHADOW_DEBUG("\nWe have an invalid controller trying to connect as the DOME controller, it will be dropped.\n")

            PS3NavDome->setLedOff(LED1);
            PS3NavDome->disconnect();
        
            isSecondaryPS3NavigatonInitialized = false;
            domeControllerConnected = false;        
        } 
        statusChangeCallback(this);
    }

    String getLastConnectedBtMAC() {
        char buffer[20];
        uint8_t* addr = Btd.disc_bdaddr;
        snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        return buffer;
    }

    void criticalFaultDetect()
    {
        if (PS3NavFoot->PS3NavigationConnected || PS3NavFoot->PS3Connected)
        {
            currentTime = millis();
            lastMsgTime = PS3NavFoot->getLastMessageTime();
            msgLagTime = currentTime - lastMsgTime;            
            
            if (WaitingforReconnect)
            {
                if (msgLagTime < 200)
                {             
                    WaitingforReconnect = false; 
                }
                lastMsgTime = currentTime;            
            } 

            if (currentTime >= lastMsgTime)
            {
                msgLagTime = currentTime - lastMsgTime;
            }
            else
            {
                msgLagTime = 0;
            }
            
            if (sensitiveTimeoutEnabled && (msgLagTime > sensitiveTimeoutWindow))
            {
                isFaulted = true;
                statusChangeCallback(this);
            }
            
            if (msgLagTime > timeoutWindow)
            {
                isFaulted = true;
                statusChangeCallback(this);
                PS3NavFoot->disconnect();
                WaitingforReconnect = true;
                return;
            }

            //Check PS3 Signal Data
            if(!PS3NavFoot->getStatus(Plugged) && !PS3NavFoot->getStatus(Unplugged))
            {
                //We don't have good data from the controller.
                //Wait 15ms if no second controller - 100ms if some controller connected, Update USB, and try again
                if (PS3NavDome->PS3NavigationConnected)
                {
                    delay(100);     
                } else
                {
                    delay(15);
                }
                
                lastMsgTime = PS3NavFoot->getLastMessageTime();
                
                if(!PS3NavFoot->getStatus(Plugged) && !PS3NavFoot->getStatus(Unplugged))
                {
                    badPS3Data++;
                    SHADOW_DEBUG("\n**Invalid data from PS3 FOOT Controller. - Resetting Data**\n")
                    return;
                }
            }
            else if (badPS3Data > 0)
            {

                badPS3Data = 0;
            }
            
            if ( badPS3Data > 10 )
            {
                SHADOW_DEBUG("Too much bad data coming from the PS3 FOOT Controller\n")
                SHADOW_DEBUG("Disconnecting the controller and stop motors.\n")

                isFaulted = true;
                statusChangeCallback(this);
                PS3NavFoot->disconnect();
                WaitingforReconnect = true;
                return;
            }
        }
        else if (sensitiveTimeoutEnabled)
        {
            SHADOW_DEBUG("No foot controller was found\n")
            SHADOW_DEBUG("Shuting down motors and watching for a new PS3 foot message\n")

            isFaulted = true;
            statusChangeCallback(this);
            WaitingforReconnect = true;
            return;
        }
    }

    void criticalFaultDetectDome()
    {
        if (PS3NavDome->PS3NavigationConnected || PS3NavDome->PS3Connected)
        {
            currentTime = millis();
            lastMsgTime = PS3NavDome->getLastMessageTime();
            msgLagTime = currentTime - lastMsgTime;            
            
            if (WaitingforReconnectDome)
            {
                if (msgLagTime < 200)
                {
                    WaitingforReconnectDome = false; 
                }            
                lastMsgTime = currentTime;
            }
            
            if (currentTime >= lastMsgTime)
            {
                msgLagTime = currentTime - lastMsgTime;
                
            }
            else
            {
                msgLagTime = 0;
            }
            
            if (msgLagTime > timeoutWindow)
            {
                SHADOW_DEBUG("It has been 10s since we heard from the PS3 Dome Controller\nmsgLagTime:%u  lastMsgTime:%u  millis: %lu\n",
                            msgLagTime, lastMsgTime, millis())
                SHADOW_DEBUG("Disconnecting the Foot controller\n")
                
                isFaulted = true;
                statusChangeCallback(this);
                PS3NavDome->disconnect();
                WaitingforReconnectDome = true;
                return;
            }

            //Check PS3 Signal Data
            if (!PS3NavDome->getStatus(Plugged) && !PS3NavDome->getStatus(Unplugged))
            {
                // We don't have good data from the controller.
                //Wait 100ms, Update USB, and try again
                delay(100);
                
                // Usb.Task();
                lastMsgTime = PS3NavDome->getLastMessageTime();
                
                if(!PS3NavDome->getStatus(Plugged) && !PS3NavDome->getStatus(Unplugged))
                {
                    badPS3DataDome++;
                    SHADOW_DEBUG("\n**Invalid data from PS3 Dome Controller. - Resetting Data**\n")
                    return;
                }
            }
            else if (badPS3DataDome > 0)
            {
                badPS3DataDome = 0;
            }
            
            if (badPS3DataDome > 10)
            {
                SHADOW_DEBUG("Too much bad data coming from the PS3 DOME Controller\n")
                SHADOW_DEBUG("Disconnecting the controller and stop motors.\n")

                isFaulted = true;
                statusChangeCallback(this);
                PS3NavDome->disconnect();
                WaitingforReconnectDome = true;
                return;
            }
        }
    }


private:
    ///////Setup for USB and Bluetooth Devices////////////////////////////
    USB Usb;
    BTD Btd;
    PS3BT PS3NavFootImpl;
    PS3BT PS3NavDomeImpl;
    PS3BT* PS3NavFoot;
    PS3BT* PS3NavDome;

    LocalPreferences preferences;

    static DualSonyMoveController* instance;
    void (*statusChangeCallback)(GameController*);

    bool isPS3NavigatonInitialized = false;
    bool isSecondaryPS3NavigatonInitialized = false;
    bool mainControllerConnected = false;
    bool domeControllerConnected = false;
    bool WaitingforReconnect = false;
    bool WaitingforReconnectDome = false;

    //Used for PS3 Fault Detection
    uint32_t msgLagTime = 0;
    uint32_t lastMsgTime = 0;
    uint32_t currentTime = 0;
    uint32_t lastLoopTime = 0;
    int badPS3Data = 0;
    int badPS3DataDome = 0;
    bool isFaulted = false;
    uint32_t timeoutWindow = 10000;
    uint32_t sensitiveTimeoutWindow = 0;
    bool sensitiveTimeoutEnabled = false;

    String PS3ControllerFootMAC = PS3_CONTROLLER_FOOT_MAC;
    String PS3ControllerDomeMAC = PS3_CONTROLLER_DOME_MAC;
    String PS3ControllerBackupFootMAC = PS3_CONTROLLER_BACKUP_FOOT_MAC;
    String PS3ControllerBackupDomeMAC = PS3_CONTROLLER_BACKUP_DOME_MAC;

    static void onInitPS3NavFootWrapper() {
        instance->onInitPS3NavFoot();
    }
    static void onInitPS3NavDomeWrapper() {
        instance->onInitPS3NavDome();
    }
};

DualSonyMoveController* DualSonyMoveController::instance = NULL;
