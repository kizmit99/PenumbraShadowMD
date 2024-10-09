#pragma once
#include <Arduino.h>
#include <ReelTwo.h>
#include <core/StringUtils.h>

#define PANEL_COUNT 10                // Number of panels

#ifndef SHADOW_VERBOSE
#define SHADOW_VERBOSE(...) printf(__VA_ARGS__);
#endif

#ifndef SHADOW_DEBUG
#define SHADOW_DEBUG(...) printf(__VA_ARGS__);
#endif

// ---------------------------------------------------------------------------------------
//                          MarcDuino Button Settings
// ---------------------------------------------------------------------------------------
// Std MarcDuino Function Codes:
//     1 = Close All Panels
//     2 = Scream - all panels open
//     3 = Wave, One Panel at a time
//     4 = Fast (smirk) back and forth wave
//     5 = Wave 2, Open progressively all panels, then close one by one
//     6 = Beep cantina - w/ marching ants panel action
//     7 = Faint / Short Circuit
//     8 = Cantina Dance - orchestral, rhythmic panel dance
//     9 = Leia message
//    10 = Disco
//    11 = Quite mode reset (panel close, stop holos, stop sounds)
//    12 = Full Awake mode reset (panel close, rnd sound, holo move,holo lights off)
//    13 = Mid Awake mode reset (panel close, rnd sound, stop holos)
//    14 = Full Awake+ reset (panel close, rnd sound, holo move, holo lights on)
//    15 = Scream, with all panels open (NO SOUND)
//    16 = Wave, one panel at a time (NO SOUND)
//    17 = Fast (smirk) back and forth (NO SOUND)
//    18 = Wave 2 (Open progressively, then close one by one) (NO SOUND)
//    19 = Marching Ants (NO SOUND)
//    20 = Faint/Short Circuit (NO SOUND)
//    21 = Rhythmic cantina dance (NO SOUND)
//    22 = Random Holo Movement On (All) - No other actions
//    23 = Holo Lights On (All)
//    24 = Holo Lights Off (All)
//    25 = Holo reset (motion off, lights off)
//    26 = Volume Up
//    27 = Volume Down
//    28 = Volume Max
//    29 = Volume Mid
//    30 = Open All Dome Panels
//    31 = Open Top Dome Panels
//    32 = Open Bottom Dome Panels
//    33 = Close All Dome Panels
//    34 = Open Dome Panel #1
//    35 = Close Dome Panel #1
//    36 = Open Dome Panel #2
//    37 = Close Dome Panel #2
//    38 = Open Dome Panel #3
//    39 = Close Dome Panel #3
//    40 = Open Dome Panel #4
//    41 = Close Dome Panel #4
//    42 = Open Dome Panel #5
//    43 = Close Dome Panel #5
//    44 = Open Dome Panel #6
//    45 = Close Dome Panel #6
//    46 = Open Dome Panel #7
//    47 = Close Dome Panel #7
//    48 = Open Dome Panel #8
//    49 = Close Dome Panel #8
//    50 = Open Dome Panel #9
//    51 = Close Dome Panel #9
//    52 = Open Dome Panel #10
//    53 = Close Dome Panel #10
//   *** BODY PANEL OPTIONS ASSUME SECOND MARCDUINO MASTER BOARD ON MEGA ADK SERIAL #3 ***
//    54 = Open All Body Panels
//    55 = Close All Body Panels
//    56 = Open Body Panel #1
//    57 = Close Body Panel #1
//    58 = Open Body Panel #2
//    59 = Close Body Panel #2
//    60 = Open Body Panel #3
//    61 = Close Body Panel #3
//    62 = Open Body Panel #4
//    63 = Close Body Panel #4
//    64 = Open Body Panel #5
//    65 = Close Body Panel #5
//    66 = Open Body Panel #6
//    67 = Close Body Panel #6
//    68 = Open Body Panel #7
//    69 = Close Body Panel #7
//    70 = Open Body Panel #8
//    71 = Close Body Panel #8
//    72 = Open Body Panel #9
//    73 = Close Body Panel #9
//    74 = Open Body Panel #10
//    75 = Close Body Panel #10
//   *** MAGIC PANEL LIGHTING COMMANDS
//    76 = Magic Panel ON
//    77 = Magic Panel OFF
//    78 = Magic Panel Flicker (10 seconds) 
//
// Std MarcDuino Logic Display Functions (For custom functions)
//     1 = Display normal random sequence
//     2 = Short circuit (10 second display sequence)
//     3 = Scream (flashing light display sequence)
//     4 = Leia (34 second light sequence)
//     5 = Display “Star Wars”
//     6 = March light sequence
//     7 = Spectrum, bar graph display sequence
//     8 = Display custom text
//
// Std MarcDuino Panel Functions (For custom functions)
//     1 = Panels stay closed (normal position)
//     2 = Scream sequence, all panels open
//     3 = Wave panel sequence
//     4 = Fast (smirk) back and forth panel sequence
//     5 = Wave 2 panel sequence, open progressively all panels, then close one by one)
//     6 = Marching ants panel sequence
//     7 = Faint / short circuit panel sequence
//     8 = Rhythmic cantina panel sequence
//     9 = Custom Panel Sequence

// Marcduino Action Syntax:
// #<1-76> Standard Marcduino Functions
// MP3=<182->,LD=<1-8>,LDText="Hello World",Panel=M<1-8>,Panel<1-10>[delay=1,open=5]

static const char* DEFAULT_MARCDUINO_COMMANDS[] = {
#include "MarcduinoCommands.h"
};

bool handleMarcduinoAction(const char* action);
void sendMarcCommand(const char* cmd);
void sendBodyMarcCommand(const char* cmd);

class MarcduinoButtonAction
{
public:
    MarcduinoButtonAction(const char* name, const char* action) :
        fNext(NULL),
        fName(name),
        fDefaultAction(action),
        fAction(action)
    {
        if (*head() == NULL)
            *head() = this;
        if (*tail() != NULL)
            (*tail())->fNext = this;
        *tail() = this;

        if (strlen(fName) > 15) {
            String key = fName;
            key = key.substring(0, 15);
            prefKey = key.c_str();
        } else {
            prefKey = fName;
        }
    }

    static MarcduinoButtonAction* findAction(String name)
    {
        for (MarcduinoButtonAction* btn = *head(); btn != NULL; btn = btn->fNext)
        {
            if (name.equalsIgnoreCase(btn->name()))
                return btn;
        }
        return nullptr;
    }

    static void listActions()
    {
        for (MarcduinoButtonAction* btn = *head(); btn != NULL; btn = btn->fNext)
        {
            printf("%s: %s\n", btn->name().c_str(), btn->action().c_str());
        }
    }

    const char* getKey() {
        return prefKey;
    }

    void reset()
    {
        fAction = fDefaultAction;
    }

    void trigger() {
        handleMarcduinoAction(fAction);
    }

    void setAction(String newAction)
    {
        fAction = newAction.c_str();
    }

    String name()
    {
        return fName;
    }

    String action()
    {
        return fAction;
    }

private:
    MarcduinoButtonAction* fNext;
    const char* fName;
    const char *fDefaultAction;
    const char* fAction;
    const char *prefKey;


    static MarcduinoButtonAction** head()
    {
        static MarcduinoButtonAction* sHead;
        return &sHead;
    }

    static MarcduinoButtonAction** tail()
    {
        static MarcduinoButtonAction* sTail;
        return &sTail;
    }
};

#define MARCDUINO_ACTION(var,act) \
MarcduinoButtonAction var(#var,act);

//----------------------------------------------------
// CONFIGURE: The FOOT Navigation Controller Buttons
//----------------------------------------------------

MARCDUINO_ACTION(btnUP_MD, "#12")
MARCDUINO_ACTION(btnLeft_MD, "#13")
MARCDUINO_ACTION(btnRight_MD, "#14")
MARCDUINO_ACTION(btnDown_MD, "#11")
MARCDUINO_ACTION(btnUP_CROSS_MD, "#26")
MARCDUINO_ACTION(btnLeft_CROSS_MD, "#23")
MARCDUINO_ACTION(btnRight_CROSS_MD, "#24")
MARCDUINO_ACTION(btnDown_CROSS_MD, "#27")
MARCDUINO_ACTION(btnUP_CIRCLE_MD, "#2")
MARCDUINO_ACTION(btnLeft_CIRCLE_MD, "#4")
MARCDUINO_ACTION(btnRight_CIRCLE_MD, "#7")
MARCDUINO_ACTION(btnDown_CIRCLE_MD, "#10")
MARCDUINO_ACTION(btnUP_PS_MD, "$71,LD=5")
MARCDUINO_ACTION(btnLeft_PS_MD, "$81,LD=1")
MARCDUINO_ACTION(btnRight_PS_MD, "$83,LD=1")
MARCDUINO_ACTION(btnDown_PS_MD, "$82,LD=1")
MARCDUINO_ACTION(btnUP_L1_MD, "#8")
MARCDUINO_ACTION(btnLeft_L1_MD, "#3")
MARCDUINO_ACTION(btnRight_L1_MD, "#5")
MARCDUINO_ACTION(btnDown_L1_MD, "#9")

//----------------------------------------------------
// CONFIGURE: The DOME Navigation Controller Buttons
//----------------------------------------------------
MARCDUINO_ACTION(FTbtnUP_MD, "#58")             // Arrow Up
MARCDUINO_ACTION(FTbtnLeft_MD, "#56")           // Arrow Left
MARCDUINO_ACTION(FTbtnRight_MD, "#57")          // Arrow Right
MARCDUINO_ACTION(FTbtnDown_MD, "#59")           // Arrow Down
MARCDUINO_ACTION(FTbtnUP_CROSS_MD, "#28")       // Arrow UP + CROSS
MARCDUINO_ACTION(FTbtnLeft_CROSS_MD, "#33")     // Arrow Left + CROSS
MARCDUINO_ACTION(FTbtnRight_CROSS_MD, "#30")    // Arrow Right + CROSS
MARCDUINO_ACTION(FTbtnDown_CROSS_MD, "#29")     // Arrow Down + CROSS
MARCDUINO_ACTION(FTbtnUP_CIRCLE_MD, "#22")      // Arrow Up + CIRCLE
MARCDUINO_ACTION(FTbtnLeft_CIRCLE_MD, "#23")    // Arrow Left + CIRCLE
MARCDUINO_ACTION(FTbtnRight_CIRCLE_MD, "#24")   // Arrow Right + CIRCLE
MARCDUINO_ACTION(FTbtnDown_CIRCLE_MD, "#25")    // Arrow Down + CIRCLE
MARCDUINO_ACTION(FTbtnUP_PS_MD, "#38")          // Arrow UP + PS
MARCDUINO_ACTION(FTbtnLeft_PS_MD, "#40")        // Arrow Left + PS
MARCDUINO_ACTION(FTbtnRight_PS_MD, "#41")       // Arrow Right + PS
MARCDUINO_ACTION(FTbtnDown_PS_MD, "#39")        // Arrow Down + PS
MARCDUINO_ACTION(FTbtnUP_L1_MD, "#34")          // Arrow UP + L1
MARCDUINO_ACTION(FTbtnLeft_L1_MD, "#36")        // Arrow Left + L1
MARCDUINO_ACTION(FTbtnRight_L1_MD, "#37")       // Arrow Right + L1
MARCDUINO_ACTION(FTbtnDown_L1_MD, "#35")        // Arrow Down + L1

// ---------------------------------------------------------------------------------------
//                    Panel Management Variables
// ---------------------------------------------------------------------------------------
static bool sRunningCustRoutine = false;

struct PanelStatus
{
    uint8_t fStatus = 0;
    uint32_t fStartTime = 0;
    uint8_t fStartDelay = 1;
    uint8_t fDuration = 5;
};

PanelStatus sPanelStatus[PANEL_COUNT];

inline bool handleMarcduinoAction(const char* action)
{
    String LD_text = "";
    bool panelTypeSelected = false;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s", action);
    char* cmd = buffer;
    if (*cmd == '#')
    {
        // Std Marcduino Function Call Configured
        uint32_t seq = strtolu(cmd+1, &cmd);
        if (*cmd == '\0')
        {
            if (seq >= 1 && seq <= SizeOfArray(DEFAULT_MARCDUINO_COMMANDS))
            {
                // If the commands starts with "BM" we direct it to the body marc controller
                const char* marcCommand = DEFAULT_MARCDUINO_COMMANDS[seq-1];
                if (marcCommand[0] == 'B' && marcCommand[1] == 'M')
                {
                    sendBodyMarcCommand(&marcCommand[2]);
                }
                else
                {
                    // Otherwise we send it to the dome Marcduino
                    sendMarcCommand(marcCommand);
                }
                return true;
            }
            else
            {
                SHADOW_DEBUG("Marcduino sequence range is 1-%d in action command \"%s\"\n", SizeOfArray(DEFAULT_MARCDUINO_COMMANDS), action)
                return false;
            }
        }
        SHADOW_DEBUG("Excepting number after # in action command \"%s\"\n", action)
        return false;
    }
    for (;;)
    {
        char buf[100];
        if (*cmd == '"')
        {
            // Skip the quote
            cmd++;
            char* marcCommand = cmd;
            char* nextCmd = strchr(cmd, ',');
            if (nextCmd != nullptr)
            {
                size_t len = nextCmd - marcCommand;
                strncpy(buf, marcCommand, len);
                buf[len] = '\0';
                cmd = nextCmd;
                marcCommand = buf;
            }
            else
            {
                cmd += strlen(marcCommand);
            }
            // If the commands starts with "BM" we direct it to the body marc controller
            if (marcCommand[0] == 'B' && marcCommand[1] == 'M')
            {
                sendBodyMarcCommand(&marcCommand[2]);
            }
            else
            {
                sendMarcCommand(marcCommand);
            }
        }
        else if (*cmd == '$')
        {
            char* mp3Cmd = cmd;
            char* nextCmd = strchr(cmd, ',');
            if (nextCmd != nullptr)
            {
                size_t len = nextCmd - mp3Cmd;
                strncpy(buf, mp3Cmd, len);
                buf[len] = '\0';
                cmd = nextCmd;
                mp3Cmd = buf;
            }
            else
            {
                cmd += strlen(mp3Cmd);
            }
            sendMarcCommand(mp3Cmd);
        }
        else if (startswith(cmd, "MP3="))
        {
            char* mp3Cmd = cmd;
            char* nextCmd = strchr(cmd, ',');
            if (nextCmd != nullptr)
            {
                size_t len = nextCmd - mp3Cmd;
                buf[0] = '$';
                strncpy(&buf[1], mp3Cmd, len);
                buf[len+1] = '\0';
                cmd = nextCmd;
                mp3Cmd = buf;
            }
            else
            {
                cmd += strlen(mp3Cmd);
            }
            sendMarcCommand(mp3Cmd);
        }
        else if (startswith(cmd, "Panel=M"))
        {
            static const char* sCommands[] = {
                ":CL00",
                ":SE51",
                ":SE52",
                ":SE53",
                ":SE54",
                ":SE55",
                ":SE56",
                ":SE57"
            };
            uint32_t num = strtolu(cmd, &cmd);
            if (num >= 1 && num <= SizeOfArray(sCommands))
            {
                if (num > 1)
                {
                    sendMarcCommand(":CL00");  // close all the panels prior to next custom routine
                    delay(50); // give panel close command time to process before starting next panel command 
                }
                sendMarcCommand(sCommands[num-1]);
                panelTypeSelected = true;
            }
            else
            {
                SHADOW_DEBUG("Marc Panel range is 1 - %d in action command \"%s\"\n", SizeOfArray(sCommands), action)
                return false;
            }
        }
        else if (startswith(cmd, "Panel"))
        {
            uint32_t num = strtolu(cmd, &cmd);
            if (num >= 1 && num <= SizeOfArray(sPanelStatus))
            {
                PanelStatus &panel = sPanelStatus[num-1];
                panel.fStatus = 1;
                if (*cmd == '[')
                {
                    cmd++;
                    do
                    {
                        if (startswith(cmd, "delay="))
                        {
                            uint32_t delayNum = strtolu(cmd, &cmd);
                            if (delayNum < 31)
                            {
                                panel.fStartDelay = delayNum;
                            }
                            else
                            {
                                panel.fStatus = 0;
                            }
                        }
                        else if (startswith(cmd, "dur="))
                        {
                            uint32_t duration = strtolu(cmd, &cmd);
                            if (duration < 31)
                            {
                                panel.fDuration = duration;
                            }
                            else
                            {
                                panel.fStatus = 0;
                            }
                        }
                        else if (*cmd == ',')
                        {
                            cmd++;
                        }
                    }
                    while (*cmd != '\0' && *cmd != ']');
                    if (*cmd == ']')
                        cmd++;
                }
                if (panel.fStatus)
                {
                    panelTypeSelected = true;
                    panel.fStartTime = millis();
                }
            }
            else
            {
                SHADOW_DEBUG("Panel range is 1 - %d in action command \"%s\"\n", SizeOfArray(sPanelStatus), action)
                return false;
            }
        }
        else if (startswith(cmd, "LDText=\""))
        {
            char* startString = ++cmd;
            while (*cmd != '\0' && *cmd != '"')
                cmd++;
            if (*cmd == '"')
                *cmd = '\0';
            LD_text = startString;
        }
        else if (startswith(cmd, "LD="))
        {
            uint32_t num = strtolu(cmd, &cmd);
            if (num >= 1 && num < 8)
            {
                // If a custom panel movement was selected - need to briefly pause before changing light sequence to avoid conflict)
                if (panelTypeSelected)
                {
                    delay(30);
                }
                switch (num)
                {
                    case 1:
                        sendMarcCommand("@0T1");
                        break;              
                    case 2:
                        sendMarcCommand("@0T4");
                        break;              
                    case 3:
                        sendMarcCommand("@0T5");
                        break;
                    case 4:
                        sendMarcCommand("@0T6");
                        break;
                    case 5:
                        sendMarcCommand("@0T10");
                        break;
                    case 6:
                        sendMarcCommand("@0T11");
                        break;
                    case 7:
                        sendMarcCommand("@0T92");
                        break;
                    case 8:
                        sendMarcCommand("@0T100");
                        delay(50);
                        String custString = "@0M";
                        custString += LD_text;
                        sendMarcCommand(custString.c_str());
                        break;
                }
            }
            else
            {
                SHADOW_DEBUG("LD range is 1 - 8 in action command \"%s\"\n", action)
                return false;
            }
        }
        if (*cmd != ',')
            break;
        cmd++;
    }
    if (*cmd != '\0')
    {
        SHADOW_DEBUG("Ignoring unknown trailing \"%s\" in action \"%s\"\n", cmd, action);
    }
    if (panelTypeSelected)
    {
        printf("panelTypeSelected\n");
        sRunningCustRoutine = true;
    }
    return true;
}

// =======================================================================================
// This function handles the processing of custom MarcDuino panel routines
// =======================================================================================
inline void custMarcDuinoPanel()
{
    if (!sRunningCustRoutine)
        return;
    sRunningCustRoutine = false;
    for (int i = 0; i < SizeOfArray(sPanelStatus); i++)
    {
        PanelStatus &panel = sPanelStatus[i];
        if (panel.fStatus == 1)
        {
            if (panel.fStartTime + panel.fStartDelay * 1000 < millis())
            {
                char cmd[10];
                snprintf(cmd, sizeof(cmd), ":OP%02d", i+1);
                sendMarcCommand(cmd);
                panel.fStatus = 2;
            }
        }
        else if (panel.fStatus == 2)
        {
            if (panel.fStartTime + (panel.fStartDelay + panel.fDuration) * 1000 < millis())
            {
                char cmd[10];
                snprintf(cmd, sizeof(cmd), ":CL%02d", i+1);
                sendMarcCommand(cmd);
                panel.fStatus = 0;
            }
        }
        if (panel.fStatus != 0)
            sRunningCustRoutine = true;
    }
}

