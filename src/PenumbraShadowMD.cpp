// =======================================================================================
//                      Penumbra Shadow MD: Derived from SHADOW_MD
// =======================================================================================
//        SHADOW_MD:  Small Handheld Arduino Droid Operating Wand + MarcDuino
// =======================================================================================
//                          Last Revised Date: 01/08/2023
//                             Revised By: skelmir
//                        Previously Revised Date: 08/23/2015
//                             Revised By: vint43
//                Inspired by the PADAWAN / KnightShade SHADOW effort
// =======================================================================================
//
//         This program is free software: you can redistribute it and/or modify it for
//         your personal use and the personal use of other astromech club members.  
//
//         This program is distributed in the hope that it will be useful 
//         as a courtesy to fellow astromech club members wanting to develop
//         their own droid control system.
//
//         IT IS OFFERED WITHOUT ANY WARRANTY; without even the implied warranty of
//         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//         You are using this software at your own risk and, as a fellow club member, it is
//         expected you will have the proper experience / background to handle and manage that 
//         risk appropriately.  It is completely up to you to insure the safe operation of
//         your droid and to validate and test all aspects of your droid control system.
//
// =======================================================================================
//   Note: You will need an ESP32 with a USB Host MAX3421 chip.
//
//   Required Libraries:
///
//   https://github.com/reeltwo/Reeltwo
//   https://github.com/rimim/espsoftwareserial
//   https://github.com/felis/USB_Host_Shield_2.0
//
// =======================================================================================
//
//   Sabertooth (Foot Drive):
//         Set Sabertooth 2x32 or 2x25 Dip Switches: 1 and 2 Down, All Others Up
//
//   SyRen 10 Dome Drive:
//         For SyRen packetized Serial Set Switches: 1, 2 and 4 Down, All Others Up
//
// =======================================================================================
//
//   Cytron SmartDriveDuo MDDS30 (Foot Drive):
//         Set Dip Switches 1,2,3 Up. 4,5,6 Down.
//
//   Cytron SmartDriveDuo MDDS10 (Dome Drive):
//         Set Dip Switches 1,2,3,6 Up. 4,5 Down.
//
//   Set Dip Switches 7 and 8 for your battery type. 0 Down 1 Up.
//      7:0 8:0 - LiPo
//      7:0 8:1 - NiMH
//      7:1 8:0 - SLA (Lead Acid)
//      7:1 8:1 - No battery monitoring
//
// =======================================================================================
//
// ---------------------------------------------------------------------------------------
//                        General User Settings
// ---------------------------------------------------------------------------------------

//#define SHADOW_DEBUG(...)       //uncomment this for console DEBUG output
//#define SHADOW_VERBOSE(...)   //uncomment this for console VERBOSE output

#define SHADOW_DEBUG(...) printf(__VA_ARGS__);
#define SHADOW_VERBOSE(...) printf(__VA_ARGS__);
#define USE_DEBUG                     // Define to enable debug diagnostic

#include <Arduino.h>
#include "pin-map.h"
#include "motor/MotorDriver.h"
#include "controller/GameController.h"
#include "controller/DualSonyMoveController.h"
#include "MarcduinoTriggers.h"

//#define USE_MP3_TRIGGER
//#define USE_DFMINI_PLAYER
#define USE_HCR_VOCALIZER
//#define ENABLE_BODY_MD_SERIAL

//For Speed Setting (Normal): set this to whatever speeds works for you. 0-stop, 127-full speed.
#define DEFAULT_DRIVE_SPEED_NORMAL          70
//For Speed Setting (Over Throttle): set this for when needing extra power. 0-stop, 127-full speed.
#define DEFAULT_DRIVE_SPEED_OVER_THROTTLE   100

// the higher this number the faster it will spin in place, lower - the easier to control.
// Recommend beginner: 40 to 50, experienced: 50+, I like 75
#define DEFAULT_TURN_SPEED                  50
                         
// If using a speed controller for the dome, sets the top speed. Use a number up to 127
#define DEFAULT_DOME_SPEED                  100
                         
// Ramping- the lower this number the longer R2 will take to speedup or slow down,
// change this by increments of 1
#define DEFAULT_RAMPING                     1
                 
// For controllers that centering problems, use the lowest number with no drift
#define DEFAULT_JOYSTICK_FOOT_DEADBAND      15

// For controllers that centering problems, use the lowest number with no drift
#define DEFAULT_JOYSTICK_DOME_DEADBAND      10

// Used to set the Sabertooth DeadZone for foot motors
#define DEFAULT_DRIVE_DEADBAND              10

//This may need to be set to true for some configurations
#define DEFAULT_INVERT_TURN_DIRECTION       false

// Speed used when dome automation is active - Valid Values: 50 - 100
#define DEFAULT_AUTO_DOME_SPEED             70

// milliseconds for dome to complete 360 turn at domeAutoSpeed - Valid Values: 2000 - 8000 (2000 = 2 seconds)
#define DEFAULT_AUTO_DOME_TURN_TIME         2500

// Motor serial communication baud rate. Default 9600
#define DEFAULT_MOTOR_BAUD                  9600

// Marcduino serial communication baud rate. Default 9600
#define DEFAULT_MARCDUINO_BAUD              9600

byte drivespeed1 = DEFAULT_DRIVE_SPEED_NORMAL;
byte drivespeed2 = DEFAULT_DRIVE_SPEED_OVER_THROTTLE;
byte turnspeed = DEFAULT_TURN_SPEED;
byte domespeed = DEFAULT_DOME_SPEED;
byte ramping = DEFAULT_RAMPING;

byte joystickFootDeadZoneRange = DEFAULT_JOYSTICK_FOOT_DEADBAND;
byte joystickDomeDeadZoneRange = DEFAULT_JOYSTICK_DOME_DEADBAND;

byte driveDeadBandRange = DEFAULT_DRIVE_DEADBAND;

bool invertTurnDirection = DEFAULT_INVERT_TURN_DIRECTION;

byte domeAutoSpeed = DEFAULT_AUTO_DOME_SPEED;
int time360DomeTurn = DEFAULT_AUTO_DOME_TURN_TIME;

#define USE_PREFERENCES
#define PREFERENCE_NAMESPACE "penumbrashadow"
#include "LocalPreferences.h"
LocalPreferences preferences(PREFERENCE_NAMESPACE);

#define PREFERENCE_MARCSOUND                "msound"
#define PREFERENCE_MARCSOUND_VOLUME         "mvolume"
#define PREFERENCE_MARCSOUND_STARTUP        "msoundstart"
#define PREFERENCE_MARCSOUND_RANDOM         "mrandom"
#define PREFERENCE_MARCSOUND_RANDOM_MIN     "mrandommin"
#define PREFERENCE_MARCSOUND_RANDOM_MAX     "mrandommax"
#define PREFERENCE_SPEED_NORMAL             "smspeednorm"
#define PREFERENCE_SPEED_OVER_THROTTLE      "smspeedmax"
#define PREFERENCE_TURN_SPEED               "smspeedturn"
#define PREFERENCE_DOME_SPEED               "smspeeddome"
#define PREFERENCE_RAMPING                  "smramping"
#define PREFERENCE_FOOTSTICK_DEADBAND       "smfootdband"
#define PREFERENCE_DOMESTICK_DEADBAND       "smdomedband"
#define PREFERENCE_DRIVE_DEADBAND           "smdrivedband"
#define PREFERENCE_INVERT_TURN_DIRECTION    "sminvertturn"
#define PREFERENCE_DOME_AUTO_SPEED          "smdomeautospeed"
#define PREFERENCE_DOME_DOME_TURN_TIME      "smdometurntime"
#define PREFERENCE_MOTOR_BAUD               "smmotorbaud"
#define PREFERENCE_MARCDUINO_BAUD           "smmarcbaud"

// ---------------------------------------------------------------------------------------
//               SYSTEM VARIABLES - USER CONFIG SECTION COMPLETED
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
//                          Drive Controller Settings
// ---------------------------------------------------------------------------------------
#include "motor/MotorDriver.h"
#include "motor/DRV8871Driver.h"
#include "motor/SabertoothWrapper.h"
//#include "motor/CytronSmartDriveDuoWrapper.h"

int motorControllerBaudRate = DEFAULT_MOTOR_BAUD;
int marcDuinoBaudRate = DEFAULT_MARCDUINO_BAUD;

#define FOOT_MOTOR_ADDRESS 128
SabertoothWrapper MotorDriverImpl(FOOT_MOTOR_ADDRESS, MOTOR_SERIAL);
//CytronSmartDriveDuoMDDS30Wrapper MotorDriverImpl(FOOT_MOTOR_ADDRESS, MOTOR_SERIAL);
MotorDriver *FootMotor = &MotorDriverImpl;

#define DOME_MOTOR_ADDRESS 129
//SabertoothWrapper DomeDriverImpl(DOME_MOTOR_ADDRESS, MOTOR_SERIAL);
//CytronSmartDriveDuoMDDS30Wrapper DomeDriverImpl(DOME_MOTOR_ADDRESS, MOTOR_SERIAL);
DRV8871Driver DomeDriverImpl(DOUT1_PIN, DOUT2_PIN);
MotorDriver *DomeMotor = &DomeDriverImpl;

#define ENABLE_UHS_DEBUGGING 1

// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <ReelTwo.h>
#include <core/SetupEvent.h>
#include <core/StringUtils.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif

#define CONSOLE_BUFFER_SIZE     300
static unsigned sPos;
static char sBuffer[CONSOLE_BUFFER_SIZE];


// ---------------------------------------------------------------------------------------
//                          Variables
// ---------------------------------------------------------------------------------------

long previousDomeMillis = millis();
long previousFootMillis = millis();
long previousMarcDuinoMillis = millis();
long previousDomeToggleMillis = millis();
long previousSpeedToggleMillis = millis();
long currentMillis = millis();

int serialLatency = 25;   //This is a delay factor in ms to prevent queueing of the Serial data.
                          //25ms seems approprate for HardwareSerial, values of 50ms or larger are needed for Softare Emulation
                          
int speedToggleButtonCounter = 0;

//Forward declarations
void footMotorDrive();
void domeDrive();
void custMarcDuinoPanel();
void autoDome();
void triggerActions();
void statusChangeCallback(GameController* controller);

// ///////Setup for Conrollers////////////////////////////

DualSonyMoveController sonyController(PREFERENCE_NAMESPACE);
GameController* controller = &sonyController;

bool isFootMotorStopped = true;
bool isDomeMotorStopped = true;

bool overSpeedSelected = false;
bool isStickEnabled = true;

// Dome Automation Variables
bool domeAutomation = false;
int domeTurnDirection = 1;  // 1 = positive turn, -1 negative turn
float domeTargetPosition = 0; // (0 - 359) - degrees in a circle, 0 = home
unsigned long domeStopTurnTime = 0;    // millis() when next turn should stop
unsigned long domeStartTurnTime = 0;  // millis() when next turn should start
int domeStatus = 0;  // 0 = stopped, 1 = prepare to turn, 2 = turning

int footDriveSpeed = 0;

// =======================================================================================

// =======================================================================================
//                          Main Program
// =======================================================================================

#if defined(USE_HCR_VOCALIZER) || defined(USE_MP3_TRIGGER) || defined(USE_DFMINI_PLAYER)
#define SOUND_DEBUG(...) printf(__VA_ARGS__);
#define MARC_SOUND_VOLUME               500     // 0 - 1000
#define MARC_SOUND_RANDOM               true    // Play random sounds
#define MARC_SOUND_RANDOM_MIN           1000    // Min wait until random sound
#define MARC_SOUND_RANDOM_MAX           10000   // Max wait until random sound
#define MARC_SOUND_STARTUP              255     // Startup sound
#define MARC_SOUND_PLAYER               MarcSound::kHCR
#include "MarcduinoSound.h"
#define MARC_SOUND
#endif

// =======================================================================================
//                          Initialize - Setup Function
// =======================================================================================
void setup()
{
    //Setup for GameController
    controller->setStatusChangedCallback(statusChangeCallback);
    controller->setTimeoutWindow(10000);
    controller->setSensitiveTimeoutWindow(300);
    controller->enableSensitiveTimeout(false);

    REELTWO_READY();

    drivespeed1 = preferences.getInt(PREFERENCE_SPEED_NORMAL, DEFAULT_DRIVE_SPEED_NORMAL);
    drivespeed2 = preferences.getInt(PREFERENCE_SPEED_OVER_THROTTLE, DEFAULT_DRIVE_SPEED_OVER_THROTTLE);
    turnspeed = preferences.getInt(PREFERENCE_TURN_SPEED, DEFAULT_TURN_SPEED);
    domespeed = preferences.getInt(PREFERENCE_DOME_SPEED, DEFAULT_DOME_SPEED);
    ramping = preferences.getInt(PREFERENCE_RAMPING, DEFAULT_RAMPING);
    joystickFootDeadZoneRange = preferences.getInt(PREFERENCE_FOOTSTICK_DEADBAND, DEFAULT_JOYSTICK_FOOT_DEADBAND);
    joystickDomeDeadZoneRange = preferences.getInt(PREFERENCE_DOMESTICK_DEADBAND, DEFAULT_JOYSTICK_DOME_DEADBAND);
    driveDeadBandRange = preferences.getInt(PREFERENCE_DRIVE_DEADBAND, DEFAULT_DRIVE_DEADBAND);
    invertTurnDirection = preferences.getBool(PREFERENCE_INVERT_TURN_DIRECTION, DEFAULT_INVERT_TURN_DIRECTION);
    domeAutoSpeed = preferences.getInt(PREFERENCE_DOME_AUTO_SPEED, DEFAULT_AUTO_DOME_SPEED);
    time360DomeTurn = preferences.getInt(PREFERENCE_DOME_DOME_TURN_TIME, DEFAULT_AUTO_DOME_TURN_TIME);
    motorControllerBaudRate = preferences.getInt(PREFERENCE_MOTOR_BAUD, DEFAULT_MOTOR_BAUD);
    marcDuinoBaudRate = preferences.getInt(PREFERENCE_MARCDUINO_BAUD, DEFAULT_MARCDUINO_BAUD);

    PrintReelTwoInfo(Serial, "Penumbra Shadow MD");

    DEBUG_PRINTLN("Bluetooth Library Started");

    //Setup for Motor Controllers
    MOTOR_SERIAL_INIT(motorControllerBaudRate);
    // If your controller is set to something other than 9600Baud call setBaudRate(9600) below
    // FootMotor->setBaudRate(9600);
    FootMotor->setTimeout(1000);      //DMB:  How low can we go for safety reasons?
    FootMotor->setDeadband(driveDeadBandRange);
    FootMotor->stop();
    DomeMotor->setTimeout(2000);      //DMB:  How low can we go for safety reasons?
    DomeMotor->setRamping(0.8);
    DomeMotor->stop();

    // //Setup for MD_SERIAL MarcDuino Dome Control Board
    MD_SERIAL_INIT(marcDuinoBaudRate);

    //Setup for BODY_MD_SERIAL Optional MarcDuino Control Board for Body Panels
#if defined(ENABLE_BODY_MD_SERIAL)
    BODY_MD_SERIAL_INIT(marcDuinoBaudRate);
#endif

    // randomSeed(analogRead(0));  // random number seed for dome automation   

    SetupEvent::ready();

#if defined(MARC_SOUND_PLAYER)
    SOUND_SERIAL_INIT(SOUND_SERIAL_BAUD);
    MarcSound::Module soundPlayer = (MarcSound::Module) preferences.getInt(PREFERENCE_MARCSOUND, MARC_SOUND_PLAYER);
    int soundStartup = preferences.getInt(PREFERENCE_MARCSOUND_STARTUP, MARC_SOUND_STARTUP);
    if (!sMarcSound.begin(soundPlayer, SOUND_SERIAL, soundStartup))
    {
        DEBUG_PRINTLN("FAILED TO INITALIZE SOUND MODULE");
    }
    sMarcSound.setVolume(preferences.getInt(PREFERENCE_MARCSOUND_VOLUME, MARC_SOUND_VOLUME) / 1000.0);
#endif

    if (!controller->init())
    {
        DEBUG_PRINTLN("OSC did not start");
        while (1); //halt
    }
#if defined(MARC_SOUND_PLAYER)
    sMarcSound.playStartSound();
    sMarcSound.setRandomMin(preferences.getInt(PREFERENCE_MARCSOUND_RANDOM_MIN, MARC_SOUND_RANDOM_MIN));
    sMarcSound.setRandomMax(preferences.getInt(PREFERENCE_MARCSOUND_RANDOM_MAX, MARC_SOUND_RANDOM_MAX));
    if (preferences.getBool(PREFERENCE_MARCSOUND_RANDOM, MARC_SOUND_RANDOM))
        sMarcSound.startRandomInSeconds(13);
#endif
}

void sendMarcCommand(const char* cmd)
{
    SHADOW_VERBOSE("Sending MARC: \"%s\"\n", cmd)
    MD_SERIAL.print(cmd); MD_SERIAL.print("\r");
#if defined(MARC_SOUND_PLAYER)
    sMarcSound.handleCommand(cmd);
#endif
}

void sendBodyMarcCommand(const char* cmd)
{
#if defined(ENABLE_BODY_MD_SERIAL)
    SHADOW_VERBOSE("Sending BODYMARC: \"%s\"\n", cmd)
    BODY_MD_SERIAL.print(cmd); BODY_MD_SERIAL.print("\r");
#endif
}

////////////////////////////////
// This function is called when settings have been changed and needs a reboot
void reboot()
{
    DEBUG_PRINTLN("Restarting...");
    delay(1000);
    ESP.restart();
}

// =======================================================================================
//           Main Program Loop - This is the recurring check loop for entire sketch
// =======================================================================================

void loop()
{
    DomeMotor->task();
    FootMotor->task();
    controller->enableSensitiveTimeout(!isFootMotorStopped);
    controller->task();

    footMotorDrive();
    domeDrive();
    triggerActions();
    custMarcDuinoPanel();     
#if defined(MARC_SOUND_PLAYER)
    sMarcSound.idle();
#endif

    // If dome automation is enabled - Call function
    if (domeAutomation && time360DomeTurn > 1999 && time360DomeTurn < 8001 && domeAutoSpeed > 49 && domeAutoSpeed < 101)  
    {
       autoDome(); 
    }

    if (Serial.available())
    {
        int ch = Serial.read();
        MD_SERIAL.print((char)ch);
        if (ch == 0x0A || ch == 0x0D)
        {
            char* cmd = sBuffer;
            if (startswith(cmd, "#SMZERO"))
            {
                preferences.clear();
                DEBUG_PRINT("Clearing preferences. ");
                reboot();
            }
            else if (startswith(cmd, "#SMRESTART"))
            {
                reboot();
            }
            else if (startswith(cmd, "#SMLIST"))
            {
                printf("Button Actions\n");
                printf("-----------------------------------\n");
                MarcduinoButtonAction::listActions();
            }
            else if (startswith(cmd, "#SMDEL"))
            {
                String key(cmd);
                key.trim();
                MarcduinoButtonAction* btn = MarcduinoButtonAction::findAction(key);
                if (btn != nullptr)
                {
                    btn->reset();
                    preferences.remove(btn->getKey());
                    printf("Trigger: %s reset to default %s\n", btn->name().c_str(), btn->action().c_str());
                }
                else
                {
                    printf("Trigger Not Found: %s\n", key.c_str());
                }
            }
            else if (startswith(cmd, "#SMVOLUME"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val > 1000)
                {
                    printf("Value out of range. 0 - 1000\n");
                }
                else
                {
                    preferences.putInt(PREFERENCE_MARCSOUND_VOLUME, val);
                    printf("Sound Volume: %d\n", val);
                    sMarcSound.setVolume(1000.0 / val);
                }
            }
            else if (startswith(cmd, "#SMSOUND"))
            {
                bool invalid = false;
                uint32_t val = strtolu(cmd, &cmd);
                switch (val)
                {
                    case MarcSound::kDisabled:
                        printf("Sound Disabled.\n");
                        break;
                    case MarcSound::kMP3Trigger:
                        printf("MP3Trigger Enabled.\n");
                        break;
                    case MarcSound::kDFMini:
                        printf("DFMiniPlayer Enabled.\n");
                        break;
                    case MarcSound::kHCR:
                        printf("HCR Vocalizer Enabled.\n");
                        break;
                    default:
                        invalid = true;
                        printf("Unknown Sound Type: %d\n", val);
                        break;
                }
                if (!invalid)
                {
                    preferences.putInt(PREFERENCE_MARCSOUND, val);
                }
            }
            else if (startswith(cmd, "#SMCONFIG"))
            {
                printf("Drive Speed Normal:  %3d (#SMNORMALSPEED) [0..127]\n", drivespeed1);
                printf("Drive Speed Max:     %3d (#SMMAXSPEED)    [0..127]\n", drivespeed2);
                printf("Turn Speed:          %3d (#SMTURNSPEED)   [0..127]\n", turnspeed);
                printf("Dome Speed:          %3d (#SMDOMESPEED)   [0..127]\n", domespeed);
                printf("Ramping:             %3d (#SMRAMPING)     [0..10]\n", ramping);
                printf("Foot Stick Deadband: %3d (#SMFOOTDB)      [0..127]\n", joystickFootDeadZoneRange);
                printf("Dome Stick Deadband: %3d (#SMDOMEDB)      [0..127]\n", joystickDomeDeadZoneRange);
                printf("Drive Deadband:      %3d (#SMDRIVEDB)     [0..127]\n", driveDeadBandRange);
                printf("Invert Turn:         %3d (#SMINVERT)      [0..1]\n", invertTurnDirection);
                printf("Dome Auto Speed:     %3d (#SMAUTOSPEED)   [50..100]\n", domeAutoSpeed);
                printf("Dome Auto Time:     %4d (#SMAUTOTIME)    [2000..8000]\n", time360DomeTurn);
                printf("Marcduino Baud:   %6d (#SMMARCBAUD)\n", marcDuinoBaudRate);
                printf("Motor Baud:       %6d (#SMMOTORBAUD)\n", motorControllerBaudRate);
            }
            else if (startswith(cmd, "#SMSTARTUP"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                preferences.putInt(PREFERENCE_MARCSOUND_STARTUP, val);
                printf("Startup Sound: %d\n", val);
            }
            else if (startswith(cmd, "#SMRANDMIN"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                preferences.putInt(PREFERENCE_MARCSOUND_RANDOM_MIN, val);
                printf("Random Min: %d\n", val);
                sMarcSound.setRandomMin(val);
            }
            else if (startswith(cmd, "#SMRANDMAX"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                preferences.putInt(PREFERENCE_MARCSOUND_RANDOM_MAX, val);
                printf("Random Max: %d\n", val);
                sMarcSound.setRandomMax(val);
            }
            else if (startswith(cmd, "#SMRAND0"))
            {
                preferences.putInt(PREFERENCE_MARCSOUND_RANDOM, false);
                printf("Random Disabled.\n");
                sMarcSound.stopRandom();
            }
            else if (startswith(cmd, "#SMRAND1"))
            {
                preferences.putBool(PREFERENCE_MARCSOUND_RANDOM, true);
                printf("Random Enabled.\n");
                sMarcSound.startRandom();
            }
            else if (startswith(cmd, "#SMNORMALSPEED"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == drivespeed1)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    drivespeed1 = val;
                    preferences.putInt(PREFERENCE_SPEED_NORMAL, drivespeed1);
                    printf("Normal Speed Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMMAXSPEED"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == drivespeed2)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    drivespeed2 = val;
                    preferences.putInt(PREFERENCE_SPEED_OVER_THROTTLE, drivespeed2);
                    printf("Max Speed Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMTURNSPEED"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == turnspeed)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    turnspeed = val;
                    preferences.putInt(PREFERENCE_TURN_SPEED, turnspeed);
                    printf("Turn Speed Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMDOMESPEED"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == domespeed)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    domespeed = val;
                    preferences.putInt(PREFERENCE_DOME_SPEED, val);
                    printf("Dome Speed Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMRAMPING"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == ramping)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 10)
                {
                    ramping = val;
                    preferences.putInt(PREFERENCE_RAMPING, ramping);
                    printf("Ramping Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-10\n");
                }
            }
            else if (startswith(cmd, "#SMFOOTDB"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == joystickFootDeadZoneRange)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    joystickFootDeadZoneRange = val;
                    preferences.putInt(PREFERENCE_FOOTSTICK_DEADBAND, joystickFootDeadZoneRange);
                    printf("Foot Joystick Deadband Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMDOMEDB"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == joystickDomeDeadZoneRange)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    joystickDomeDeadZoneRange = val;
                    preferences.putInt(PREFERENCE_DOMESTICK_DEADBAND, joystickDomeDeadZoneRange);
                    printf("Dome Joystick Deadband Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMDRIVEDB"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == driveDeadBandRange)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 127)
                {
                    driveDeadBandRange = val;
                    preferences.putInt(PREFERENCE_DRIVE_DEADBAND, driveDeadBandRange);
                    printf("Drive Controller Deadband Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMINVERT"))
            {
                bool invert = (strtolu(cmd, &cmd) == 1);
                if (invert == invertTurnDirection)
                {
                    printf("Unchanged.\n");
                }
                else
                {
                    invertTurnDirection = invert;
                    preferences.putInt(PREFERENCE_INVERT_TURN_DIRECTION, invertTurnDirection);
                    if (invert)
                        printf("Invert Turn Direction Enabled.\n");
                    else
                        printf("Invert Turn Direction Disabled.\n");
                }
            }
            else if (startswith(cmd, "#SMAUTOSPEED"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == domeAutoSpeed)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 100)
                {
                    domeAutoSpeed = val;
                    preferences.putInt(PREFERENCE_DOME_AUTO_SPEED, domeAutoSpeed);
                    printf("Auto Dome Speed Changed.\n");
                }
                else
                {
                    printf("Must be in range 50-100\n");
                }
            }
            else if (startswith(cmd, "#SMAUTOTIME"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == time360DomeTurn)
                {
                    printf("Unchanged.\n");
                }
                else if (val <= 8000)
                {
                    time360DomeTurn = val;
                    preferences.putInt(PREFERENCE_DOME_DOME_TURN_TIME, time360DomeTurn);
                    printf("Auto Dome Turn Time Changed.\n");
                }
                else
                {
                    printf("Must be in range 0-127\n");
                }
            }
            else if (startswith(cmd, "#SMMOTORBAUD"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == motorControllerBaudRate)
                {
                    printf("Unchanged.\n");
                }
                else
                {
                    motorControllerBaudRate = val;
                    preferences.putInt(PREFERENCE_MOTOR_BAUD, motorControllerBaudRate);
                    printf("Motor Controller Serial Baud Rate Changed. Needs Reboot.\n");
                }
            }
            else if (startswith(cmd, "#SMMARCBAUD"))
            {
                uint32_t val = strtolu(cmd, &cmd);
                if (val == marcDuinoBaudRate)
                {
                    printf("Unchanged.\n");
                }
                else
                {
                    marcDuinoBaudRate = val;
                    preferences.putInt(PREFERENCE_MARCDUINO_BAUD, marcDuinoBaudRate);
                    printf("Marcduino Serial Baud Rate Changed. Needs Reboot.\n");
                }
            }
            else if (startswith(cmd, "#SMPLAY"))
            {
                String key(cmd);
                key.trim();
                MarcduinoButtonAction* btn = MarcduinoButtonAction::findAction(key);
                if (btn != nullptr)
                {
                    btn->trigger();
                }
                else
                {
                    printf("Trigger Not Found: %s\n", key.c_str());
                }
            }
            else if (startswith(cmd, "#SMSET"))
            {
                // Skip whitespace
                while (*cmd == ' ')
                    cmd++;
                char* keyp = cmd;
                char* valp = strchr(cmd, ' ');
                if (valp != nullptr)
                {
                    *valp++ = '\0';
                    String key(keyp);
                    key.trim();
                    MarcduinoButtonAction* btn = MarcduinoButtonAction::findAction(key);
                    if (btn != nullptr)
                    {
                        String action(valp);
                        action.trim();
                        btn->setAction(action);
                        preferences.putString(btn->getKey(), action.c_str());
                        printf("Trigger: %s set to %s\n", key.c_str(), action.c_str());
                    }
                    else
                    {
                        printf("Trigger Not Found: %s\n", key.c_str());
                    }
                }
            }
            else
            {
                printf("Unknown: %s\n", sBuffer);
            }
            sPos = 0;
        }
        else if (sPos < SizeOfArray(sBuffer)-1)
        {
            sBuffer[sPos++] = ch;
            sBuffer[sPos] = '\0';
        }
    }

    // Clear inbound buffer of any data sent from the MarcDuino board
    if (MD_SERIAL.available())
    {
        int ch = MD_SERIAL.read();
        Serial.print((char)ch);
    }
#if defined(ENABLE_BODY_MD_SERIAL)
    if (BODY_MD_SERIAL.available())
    {
        int ch = BODY_MD_SERIAL.read();
        Serial.print((char)ch);
    }
#endif
}

// =======================================================================================
//           footDrive Motor Control Section
// =======================================================================================

bool ps3FootMotorDrive()
{
    int stickSpeed = 0;
    int turnnum = 0;
  
    if (controller->isConnected())
    {    
         // Additional fault control.  Do NOT send additional commands to motord if no controllers have initialized.
        if (!isStickEnabled)
        {
        #ifdef SHADOW_VERBOSE
            if (abs(controller->getJoystick(GameController::Drive, GameController::Y)) > joystickFootDeadZoneRange)
            {
                SHADOW_VERBOSE("Drive Stick is disabled\n")
            }
        #endif

            if (!isFootMotorStopped)
            {
                FootMotor->stop();
                isFootMotorStopped = true;
                footDriveSpeed = 0;
              
                SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
            }
            return false;
        }
        else if (!controller->isConnected())
        {
            if (!isFootMotorStopped)
            {
                FootMotor->stop();
                isFootMotorStopped = true;
                footDriveSpeed = 0;

                SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
            }
          
            return false;
        }
        else
        {
            int joystickPosition = controller->getJoystick(GameController::Drive, GameController::X);
          
            if (overSpeedSelected) //Over throttle is selected
            {
                stickSpeed = (map(joystickPosition, -128, 127, -drivespeed2, drivespeed2));   
            }
            else 
            {
                stickSpeed = (map(joystickPosition, -128, 127, -drivespeed1, drivespeed1));
            }

            if ( abs(joystickPosition) < joystickFootDeadZoneRange)
            {
                // This is RAMP DOWN code when stick is now at ZERO but prior FootSpeed > 20
                if (abs(footDriveSpeed) > 50)
                {   
                    if (footDriveSpeed > 0)
                    {
                        footDriveSpeed -= 3;
                    }
                    else
                    {
                        footDriveSpeed += 3;
                    }
                    SHADOW_VERBOSE("ZERO FAST RAMP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                }
                else if (abs(footDriveSpeed) > 20)
                {   
                    if (footDriveSpeed > 0)
                    {
                        footDriveSpeed -= 2;
                    }
                    else
                    {
                        footDriveSpeed += 2;
                    }  
                    SHADOW_VERBOSE("ZERO MID RAMP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                }
                else
                {        
                    footDriveSpeed = 0;
                }
            }
            else 
            {
                isFootMotorStopped = false;
                if (footDriveSpeed < stickSpeed)
                {
                    if ((stickSpeed-footDriveSpeed)>(ramping+1))
                    {
                        footDriveSpeed+=ramping;
                        SHADOW_VERBOSE("RAMPING UP: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                    }
                    else
                    {
                        footDriveSpeed = stickSpeed;
                    }
                }
                else if (footDriveSpeed > stickSpeed)
                {
                    if ((footDriveSpeed-stickSpeed)>(ramping+1))
                    {
                        footDriveSpeed-=ramping;
                        SHADOW_VERBOSE("RAMPING DOWN: footSpeed: %d\nStick Speed: %d\n", footDriveSpeed, stickSpeed)
                    }
                    else
                    {
                        footDriveSpeed = stickSpeed;  
                    }
                }
                else
                {
                    footDriveSpeed = stickSpeed;  
                }
            }
            uint8_t joystick_Y = controller->getJoystick(GameController::Drive, GameController::Y);
            turnnum = joystick_Y;

            //TODO:  Is there a better algorithm here?  
            if ( abs(footDriveSpeed) > 50)
                turnnum = (map(joystick_Y, -74, 72, -(turnspeed/4), (turnspeed/4)));
            else if (turnnum <= 72 && turnnum >= -74)
                turnnum = (map(joystick_Y, -74, 72, -(turnspeed/3), (turnspeed/3)));
            else if (turnnum > 72)
                turnnum = (map(joystick_Y, 73, 127, turnspeed/3, turnspeed));
            else if (turnnum < -74)
                turnnum = (map(joystick_Y, -128, -75, -turnspeed, -(turnspeed/3)));
              
            if (abs(turnnum) > 5)
            {
                isFootMotorStopped = false;   
            }

            currentMillis = millis();
          
            if ((currentMillis - previousFootMillis) > serialLatency)
            {
                if (footDriveSpeed != 0 || abs(turnnum) > 5)
                {
                    SHADOW_VERBOSE("Motor: FootSpeed: %d\nTurnnum: %d\nTime of command: %lu\n", footDriveSpeed, turnnum, millis())              
                    FootMotor->turn(turnnum * (invertTurnDirection ? 1 : -1));
                    FootMotor->drive(footDriveSpeed);
                }
                else
                {    
                    if (!isFootMotorStopped)
                    {
                        FootMotor->stop();
                        isFootMotorStopped = true;
                        footDriveSpeed = 0;
                      
                        SHADOW_VERBOSE("\n***Foot Motor STOPPED***\n")
                    }
                }
              
                // The Sabertooth won't act on mixed mode packet serial commands until
                // it has received power levels for BOTH throttle and turning, since it
                // mixes the two together to get diff-drive power levels for both motors.
              
                previousFootMillis = currentMillis;
                return true; //we sent a foot command   
            }
        }
    }
    return false;
}

void footMotorDrive()
{
    //Flood control prevention
    if ((millis() - previousFootMillis) < serialLatency)
        return;
  
    if (controller->isConnected())
        ps3FootMotorDrive();
}  


// =======================================================================================
//           domeDrive Motor Control Section
// =======================================================================================

int ps3DomeDrive()
{
    int domeRotationSpeed = 0;
    int joystickPosition = controller->getJoystick(GameController::Dome, GameController::Y);
        
    domeRotationSpeed = (map(joystickPosition, -128, 127, -domespeed, domespeed));
    if ( abs(joystickPosition) < joystickDomeDeadZoneRange ) {
       domeRotationSpeed = 0;
    }
          
    if ((domeRotationSpeed != 0) && (domeAutomation == true))  // Turn off dome automation if manually moved
    {   
        domeAutomation = false; 
        domeStatus = 0;
        domeTargetPosition = 0; 
        
        SHADOW_VERBOSE("Dome Automation OFF\n")
    }    

    return domeRotationSpeed;
}

void rotateDome(int domeRotationSpeed, String mesg)
{
    //Constantly sending commands to the SyRen (Dome) is causing foot motor delay.
    //Lets reduce that chatter by trying 3 things:
    // 1.) Eliminate a constant stream of "don't spin" messages (isDomeMotorStopped flag)
    // 2.) Add a delay between commands sent to the SyRen (previousDomeMillis timer)
    // 3.) Switch to real UART on the MEGA (Likely the *CORE* issue and solution)
    // 4.) Reduce the timout of the SyRen - just better for safety!
    
    currentMillis = millis();
    if ((!isDomeMotorStopped || domeRotationSpeed != 0) &&
        ((currentMillis - previousDomeMillis) > (2*serialLatency)))
    {
        if (domeRotationSpeed != 0)
        {
            isDomeMotorStopped = false;
            SHADOW_VERBOSE("Dome rotation speed: %d\n", domeRotationSpeed)        
            DomeMotor->motor(domeRotationSpeed);
        }
        else
        {
            isDomeMotorStopped = true; 
            SHADOW_VERBOSE("\n***Dome motor is STOPPED***\n")
            
            DomeMotor->stop();
        }
        previousDomeMillis = currentMillis;      
    }
}

void domeDrive()
{
    //Flood control prevention
    //This is intentionally set to double the rate of the Dome Motor Latency
    if ((millis() - previousDomeMillis) < (2*serialLatency))
        return;
  
    if (controller->isConnected()) 
    {
        rotateDome(ps3DomeDrive(),"Controller Move");
    }
    else if (!isDomeMotorStopped)
    {
        DomeMotor->stop();
        isDomeMotorStopped = true;
    }  
}  

// =======================================================================================
//                               Toggle Control Section
// =======================================================================================

void handleToggleActions(String action)
{
    // enable / disable drive stick
    if (action.equals("stickDisable"))
    {
        SHADOW_DEBUG("Disabling the DriveStick\n")
        SHADOW_DEBUG("Stopping Motors\n")

        FootMotor->stop();
        isFootMotorStopped = true;
        isStickEnabled = false;
        footDriveSpeed = 0;
    }
    
    if(action.equals("stickEnable"))
    {
        SHADOW_DEBUG("Enabling the DriveStick\n");
        isStickEnabled = true;
    }
    
    // Enable and Disable Overspeed
    if (action.equals("toggleSpeed") && isStickEnabled)
    {
        if ((millis() - previousSpeedToggleMillis) > 1000)
        {
            speedToggleButtonCounter = 0;
            previousSpeedToggleMillis = millis();
        } 

        speedToggleButtonCounter += 1;       
        if (speedToggleButtonCounter == 1)
        {
            if (!overSpeedSelected)
            {
                overSpeedSelected = true;
                SHADOW_VERBOSE("Over Speed is now: ON\n");
            }
            else
            {      
                overSpeedSelected = false;
                SHADOW_VERBOSE("Over Speed is now: OFF\n")
            }
        }
    }
   
    // Enable Disable Dome Automation
    if(action.equals("autoDomeOff"))
    {
        domeAutomation = false;
        domeStatus = 0;
        domeTargetPosition = 0;
        DomeMotor->stop();
        isDomeMotorStopped = true;
        
        SHADOW_DEBUG("Dome Automation OFF\n")
    } 

    if(action.equals("autoDomeOn"))
    {
        domeAutomation = true;

        SHADOW_DEBUG("Dome Automation On\n")
    } 
}

String lastAction;
uint32_t lastActionTime = 0;

void triggerActions() {
    String action = controller->getAction();
    if ((action == NULL) ||
        (action.equals(""))) {
        //Nothing to do here
        return;
    }

    //Allow auto-repeat of actions
    uint32_t now = millis();
    if ((action == lastAction) &&
        ((now - lastActionTime) < 1000)) {
        //Not enough time has passed, skip it
        return;
    }
    lastAction = action;
    lastActionTime = now;

    MarcduinoButtonAction* mdAction = MarcduinoButtonAction::findAction(action);
    if (mdAction != NULL) {
        mdAction->trigger();
        return;
    }

    handleToggleActions(action);
}


// =======================================================================================
//                             Dome Automation Function
//
//    Features toggles 'on' via L2 + CIRCLE.  'off' via L2 + CROSS.  Default is 'off'.
//
//    This routines randomly turns the dome motor in both directions.  It assumes the 
//    dome is in the 'home' position when the auto dome feature is toggled on.  From
//    there it turns the dome in a random direction.  Stops for a random length of 
//    of time.  Then returns the dome to the home position.  This randomly repeats.
//
//    It is driven off the user variable - time360DomeTurn.  This records how long
//    it takes the dome to do a 360 degree turn at the given auto dome speed.  Tweaking
//    this parameter to be close provides the best results.
//
//    Activating the dome controller manually immediately cancels the auto dome feature
//    or you can toggle the feature off by pressing L2 + CROSS.
// =======================================================================================
void autoDome()
{
    long rndNum;
    int domeSpeed;
    
    if (domeStatus == 0)  // Dome is currently stopped - prepare for a future turn
    { 
        if (domeTargetPosition == 0)  // Dome is currently in the home position - prepare to turn away
        {
            domeStartTurnTime = millis() + (random(3, 10) * 1000);

            rndNum = random(5,354);

            domeTargetPosition = rndNum;  // set the target position to a random degree of a 360 circle - shaving off the first and last 5 degrees

            if (domeTargetPosition < 180)  // Turn the dome in the positive direction
            {
                domeTurnDirection = 1;
                domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);              
            }
            else  // Turn the dome in the negative direction
            {
                domeTurnDirection = -1;
                domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);              
            }
        }
        else  // Dome is not in the home position - send it back to home
        {
            domeStartTurnTime = millis() + (random(3, 10) * 1000);   
            if (domeTargetPosition < 180)
            {
                domeTurnDirection = -1;
                domeStopTurnTime = domeStartTurnTime + ((domeTargetPosition / 360) * time360DomeTurn);
            }
            else
            {
                domeTurnDirection = 1;
                domeStopTurnTime = domeStartTurnTime + (((360 - domeTargetPosition) / 360) * time360DomeTurn);
            }
            domeTargetPosition = 0;
        }
        domeStatus = 1;  // Set dome status to preparing for a future turn

        SHADOW_DEBUG("Dome Automation: Initial Turn Set\nCurrent Time: %lu\nNext Start Time: %lu\nNext Stop Time: %lu\nDome Target Position: %f\n",
            millis(), domeStartTurnTime, domeStopTurnTime, domeTargetPosition);
    }    

    if (domeStatus == 1)  // Dome is prepared for a future move - start the turn when ready
    {
        if (domeStartTurnTime < millis())
        {
            domeStatus = 2; 
            SHADOW_DEBUG("Dome Automation: Ready To Start Turn\n")
        }
    }
    
    if (domeStatus == 2) // Dome is now actively turning until it reaches its stop time
    {      
        if (domeStopTurnTime > millis())
        {
            domeSpeed = domeAutoSpeed * domeTurnDirection;
            DomeMotor->motor(domeSpeed);

            SHADOW_DEBUG("Turning Now!!\n")
        }
        else  // turn completed - stop the motor
        {
            domeStatus = 0;
            DomeMotor->stop();

            SHADOW_DEBUG("STOP TURN!!\n")
        }      
    }
}

void statusChangeCallback(GameController* controller) {
    if (!controller->isConnected()) {
        SHADOW_DEBUG("Controller is no longer connected\n")
        SHADOW_DEBUG("Stopping the motors.\n")
        FootMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;

        DomeMotor->stop();
    }
    if (controller->hasFault()) {
        SHADOW_DEBUG("Controller has indicated a fault\n")
        SHADOW_DEBUG("Stopping the motors.\n")

        FootMotor->stop();
        isFootMotorStopped = true;
        footDriveSpeed = 0;

        DomeMotor->stop();
    }
}
