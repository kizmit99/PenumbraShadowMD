#include <Arduino.h>
#include "GameController.h"
#include <PS3BT.h>
#include <usbhub.h>

#ifndef SHADOW_DEBUG
#define SHADOW_DEBUG(...)
#endif

//Forward Declarations
void onInitPS3NavFootCB();
void onInitPS3NavDomeCB();

class DualSonyMoveController : public GameController {
public:

    DualSonyMoveController() :
        Btd(&Usb),
        PS3NavFootImpl(&Btd),
        PS3NavDomeImpl(&Btd)
    {
        PS3NavFoot = &PS3NavFootImpl;
        PS3NavDome = &PS3NavDomeImpl;

        //Setup for PS3
        PS3NavFoot->attachOnInit(onInitPS3NavFootCB); // called upon a new connection
        PS3NavDome->attachOnInit(onInitPS3NavDomeCB);
    }

    //Implement the GameController Interface methods

    bool init() {
        return (Usb.Init() == 0);
    }

    void task() {
        //TODO
    }

    bool isConnected() {
        //TODO
        return false;
    }

    bool hasFault() {
        //TODO
        return true;
    }

    void clearFault() {
        //TODO
    }

    int8_t getJoystick(Controller controller, Axis axis) {
        //Return the requested joystick, but if returning drive stick need to ensure that Neither L1 nor L2 is pressed on the stick controller
        //If only the drive controller is connected (no dome controller), then L2 pressed on drive controller means dome joystick
        //Note that (apparently) the PS3 joystick X/Y axis designations are reversed from GameController X/Y
        //Also note that this method returns an int, not a uint!  Range is -128 to 127, not 0 to 255!

        if (!PS3NavFoot) {
            return 0;
        }

        switch (controller) {
            case Drive:
                if (PS3NavFoot->getButtonPress(L1) || PS3NavFoot->getButtonPress(L2)) {
                    return 0;
                }
                switch (axis) {
                    case X:
                        return PS3NavFoot->getAnalogHat(LeftHatY);

                    case Y:
                        return PS3NavFoot->getAnalogHat(LeftHatX);

                    default:
                        return 0;
                }

            case Dome: {
                PS3BT* ps3 = PS3NavDome;
                if (ps3 == NULL) {
                    if ((PS3NavFoot != NULL) && (PS3NavFoot->getButtonPress(L2))) {
                        ps3 = PS3NavFoot;
                    } else {
                        return 0;
                    }
                }
                switch (axis) {
                    case X:
                        return ps3->getAnalogHat(LeftHatY);

                    case Y:
                        return ps3->getAnalogHat(LeftHatX);

                    default:
                        return 0;
                }
            }

            default:
                return 0;
        }
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

    PS3BT* getPS3NavFoot() {
        return PS3NavFoot;
    }

    PS3BT* getPS3NavDome() {
        return PS3NavDome;
    }

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
            
//TODO - Preferences
            //preferences.putString(PREFERENCE_PS3_FOOT_MAC, btAddress.c_str());
            PS3ControllerFootMAC = btAddress;
            mainControllerConnected = true;
            WaitingforReconnect = true;
        }
        else
        {
            // Prevent connection from anything but the MAIN controllers          
            SHADOW_DEBUG("\nWe have an invalid controller trying to connect as the FOOT controller, it will be dropped.\n")

//TODO - Talk to Motors
            // FootMotor->stop();
            // DomeMotor->stop();
            // isFootMotorStopped = true;
            // footDriveSpeed = 0;
            PS3NavFoot->setLedOff(LED1);
            PS3NavFoot->disconnect();
        
            isPS3NavigatonInitialized = false;
            mainControllerConnected = false;        
        } 
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
            
//TODO - Preferences
            //preferences.putString(PREFERENCE_PS3_DOME_MAC, btAddress.c_str());
            PS3ControllerDomeMAC = btAddress;

            domeControllerConnected = true;
            WaitingforReconnectDome = true;
        }
        else
        {
            // Prevent connection from anything but the DOME controllers          
            SHADOW_DEBUG("\nWe have an invalid controller trying to connect as the DOME controller, it will be dropped.\n")

//TODO - Talk to Motors
            // FootMotor->stop();
            // DomeMotor->stop();
            // isFootMotorStopped = true;
            // footDriveSpeed = 0;
            PS3NavDome->setLedOff(LED1);
            PS3NavDome->disconnect();
        
            isSecondaryPS3NavigatonInitialized = false;
            domeControllerConnected = false;        
        } 
    }

    String getLastConnectedBtMAC() {
        char buffer[20];
        uint8_t* addr = Btd.disc_bdaddr;
        snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        return buffer;
    }

private:
    ///////Setup for USB and Bluetooth Devices////////////////////////////
    USB Usb;
    BTD Btd;
    PS3BT PS3NavFootImpl;
    PS3BT PS3NavDomeImpl;
    PS3BT* PS3NavFoot;
    PS3BT* PS3NavDome;

    bool isPS3NavigatonInitialized = false;
    bool isSecondaryPS3NavigatonInitialized = false;
    bool mainControllerConnected = false;
    bool domeControllerConnected = false;
    bool WaitingforReconnect = false;
    bool WaitingforReconnectDome = false;

    int badPS3Data = 0;
    int badPS3DataDome = 0;

    String PS3ControllerFootMAC;
    String PS3ControllerDomeMAC;
    String PS3ControllerBackupFootMAC;
    String PS3ControllerBackupDomeMAC;
};

extern DualSonyMoveController sonyControllers;

// =======================================================================================
//           PS3 Controller Device callback Functions
// =======================================================================================

void onInitPS3NavFootCB() {
    sonyControllers.onInitPS3NavFoot();
}

void onInitPS3NavDomeCB() {
    sonyControllers.onInitPS3NavDome();
}
