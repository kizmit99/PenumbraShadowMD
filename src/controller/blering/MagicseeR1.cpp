#include "MagicseeR1.h"
#include "debug.h"

const char* MagicseeR1::modeString(Mode mode) {
    switch (mode) {
        case MODE_A:    return "A";
        case MODE_B:    return "B";
        case MODE_C:    return "C";
        case MODE_D:    return "D";
        case MODE_UNKNOWN:  return "Unknown";
        default:        return "Undefined";
    }
}

void unexpected(volatile void *report, size_t length, char *mode) {
    DBG_printf("Unexpected message in Mode-%s: ", mode);
    DBG_printHex((void *) report, length);
    DBG_println();
}

void MagicseeR1::clearAllButtons() {
    //Unpress and Unclick all buttons
    for (int i = 0; i < buttonCount; i++) {
        pressedButtons[i] = false;
    }
    for (int i = 0; i < buttonCount; i++) {
        clickedButtons[i] = false;
    }
}

void MagicseeR1::press(Button button) {
    pressedButtons[button] = true;
}

void MagicseeR1::unpress(Button button) {
    pressedButtons[button] = false;
}

void MagicseeR1::click(Button button) {
    clickedButtons[button] = true;
}

void MagicseeR1::unclick(Button button) {
    clickedButtons[button] = false;
}

void MagicseeR1::handleReport(uint8_t *report, int length) {
    testForModeChange(report, length);

    switch (currentMode) {
        case MODE_UNKNOWN:
            //Mode is unknown - do not process the message
            clearAllButtons();
            //Do handle unambiguous joystick reports if we happen to actually be in Mode B or C
            if (length == 2) {  //Report type 4
                if ((report[0] == 0x00) && (report[1] == 0x10)) {
                    press(UP);
                } else if ((report[0] == 0x00) && (report[1] == 0x90)) {
                    press(DOWN);
                } else if ((report[0] == 0x00) && (report[1] == 0x40)) {
                    press(LEFT);
                } else if ((report[0] == 0x00) && (report[1] == 0x60)) {
                    press(RIGHT);
                }
            }
            break;

        case MODE_A:
            if (length == 1) {  // Report Type 2
                if (report[0] == 0x00) {
                    unpress(A);
                    unpress(C);
                    unpress(D);
                    unpress(UP);
                    unpress(DOWN);
                    unpress(LEFT);
                    unpress(RIGHT);
                } else if (report[0] == 0x01) {
                    press(D);
                    click(D);
                    press(UP);
                } else if (report[0] == 0x02) {
                    press(C);
                    click(C);
                    press(DOWN);
                } else if (report[0] == 0x04) {
                    press(A);
                    click(A);
                } else if (report[0] == 0x10) {
                    press(LEFT);
                } else if (report[0] == 0x20) {
                    press(RIGHT);
                } else {
                    unexpected(report, length, getMode());
                }
            } else if (length == 2) {   // Report Type 4
                if ((report[0] == 0x00) && (report[1] == 0x50)) {
                    unpress(L1);
                } else if ((report[0] == 0x02) && (report[1] == 0x50)) {
                    press(L1);
                    click(L1);
                } else {
                    unexpected(report, length, getMode());
                }
            } else {
                unexpected(report, length, getMode());
            }
            break;

        case (MODE_B):
            if (length == 2) {  // Report type 4
                if ((report[0] == 0x00) && (report[1] == 0x50)) {
                    unpress(A);
                    unpress(B);
                    unpress(C);
                    unpress(D);
                    unpress(L1);
                    unpress(L2);
                    unpress(UP);
                    unpress(DOWN);
                    unpress(LEFT);
                    unpress(RIGHT);
                } else if ((report[0] == 0x10) && (report[1] == 0x52)) {
                    press(A);
                } else if ((report[0] == 0x01) && (report[1] == 0x50)) {
                    press(B);
                } else if ((report[0] == 0x08) && (report[1] == 0x51)) {
                    press(C);
                } else if ((report[0] == 0x02) && (report[1] == 0x50)) {
                    press(D);
                } else if ((report[0] == 0x80) && (report[1] == 0x50)) {
                    press(L1);
                } else if ((report[0] == 0x40) && (report[1] == 0x50)) {
                    press(L2);
                } else if ((report[0] == 0x00) && (report[1] == 0x10)) {
                    press(UP);
                } else if ((report[0] == 0x00) && (report[1] == 0x90)) {
                    press(DOWN);
                } else if ((report[0] == 0x00) && (report[1] == 0x40)) {
                    press(LEFT);
                } else if ((report[0] == 0x00) && (report[1] == 0x60)) {
                    press(RIGHT);
                } else {
                    unexpected(report, length, getMode());
                }
            } else {
                unexpected(report, length, getMode());
            }
            break;

        case (MODE_C):
            if (length == 1) {  // Report type 2
                if (report[0] == 0x00) {
                    unpress(C);
                } else if (report[0] == 0x02) {
                    press(C);
                    click(C);
                } else {
                    unexpected(report, length, getMode());
                }
            } else if (length == 2) {   // Report type 4
                if ((report[0] == 0x00) && (report[1] == 0x50)) {
                    unpress(UP);
                    unpress(DOWN);
                    unpress(LEFT);
                    unpress(RIGHT);
                } else if ((report[0] == 0x00) && (report[1] == 0x10)) {
                    press(UP);
                } else if ((report[0] == 0x00) && (report[1] == 0x90)) {
                    press(DOWN);
                } else if ((report[0] == 0x00) && (report[1] == 0x40)) {
                    press(LEFT);
                } else if ((report[0] == 0x00) && (report[1] == 0x60)) {
                    press(RIGHT);
                } else {
                    unexpected(report, length, getMode());
                }
            } else {
                unexpected(report, length, getMode());
            }
            break;
            
        case (MODE_D):
            if (length == 1) {  // Report type 2
                if (report[0] == 0x00) {
                    unpress(C);
                    unpress(D);
                } else if (report[0] == 0x01) {
                    press(C);
                    click(C);
                } else if (report[0] == 0x02) {
                    press(D);
                    click(D);
                } else {
                    unexpected(report, length, getMode());
                }
            } else if (length == 2) {   // Report type 4
                if ((report[0] == 0x00) && (report[1] == 0x50)) {
                    unpress(A);
                    unpress(B);
                } else if ((report[0] == 0x01) && (report[1] == 0x50)) {
                    press(A);
                } else if ((report[0] == 0x02) && (report[1] == 0x50)) {
                    press(B);
                } else {
                    unexpected(report, length, getMode());
                }
            } else if (length == 4) {   // Report type 1
                if ((report[0] == 0x00) && (report[1] == 0x00) && (report[2] == 0x00) && (report[3] == 0x00)) {
                    unpress(L1);
                    unpress(L2);
                    unpress(UP);
                    unpress(DOWN);
                    unpress(LEFT);
                    unpress(RIGHT);
                } else if ((report[0] == 0x02) && (report[1] == 0x00) && (report[2] == 0x00) && (report[3] == 0x00)) {
                    press(L1);
                    click(L1);
                } else if ((report[0] == 0x01) && (report[1] == 0x00) && (report[2] == 0x00) && (report[3] == 0x00)) {
                    press(L2);
                    click(L2);
                } else {
                    if (report[0] == 0x01) {
                        press(L2);
                    } else {
                        unpress(L2);
                    }
                    if (report[1] == 0xe4) {
                        unpress(UP);
                        unpress(DOWN);
                        press(LEFT);
                        unpress(RIGHT);
                    } else if (report[1] == 0x1c) {
                        unpress(UP);
                        unpress(DOWN);
                        unpress(LEFT);
                        press(RIGHT);
                    } else if (report[2] == 0xe4) {
                        press(UP);
                        unpress(DOWN);
                        unpress(LEFT);
                        unpress(RIGHT);
                    } else if (report[2] == 0x1c) {
                        unpress(UP);
                        press(DOWN);
                        unpress(LEFT);
                        unpress(RIGHT);
                    } else {
                        unexpected(report, length, getMode());
                    }
                }
            } else {
                unexpected(report, length, getMode());
            }
            break;
    }
}

void MagicseeR1::disconnect() {
    clearAllButtons();
    currentMode = MODE_UNKNOWN;
}

bool MagicseeR1::isButtonPressed(Button button) {
    return pressedButtons[button];
}

bool MagicseeR1::isButtonClicked(Button button) {
    return clickedButtons[button];
    unclick(button);
}

char *MagicseeR1::getMode() {
    return (char *) modeString(currentMode);
}

void MagicseeR1::testForModeChange(volatile uint8_t *report, int length) {

    Mode newMode = currentMode;

    // Tests for unique messages in ANY mode
    if (length == 4) {  //Report Type 1
        newMode = MODE_D;
    } else if (length == 1) {  //Report Type 2
        if ((report[0] == 0x04) ||
            (report[0] == 0x10) ||
            (report[0] == 0x20)) {
            newMode = MODE_A;
        }
    } else if (length == 2) {  // Report Type 4
        if (((report[0] == 0x10) && (report[1] == 0x52)) ||
            ((report[0] == 0x08) && (report[1] == 0x51)) ||
            ((report[0] == 0x80) && (report[1] == 0x50)) ||
            ((report[0] == 0x40) && (report[1] == 0x50))) {
            newMode = MODE_B;
        }
    }

    if (newMode != currentMode) {
        clearAllButtons();
        currentMode = newMode;
        return;
    }

    switch (currentMode) {
        case MODE_A:
            if (length == 2) {  // Report Type 4
                if (((report[0] == 0x01) && (report[1] == 0x50)) ||
                    ((report[0] == 0x00) && (report[1] == 0x10)) ||
                    ((report[0] == 0x00) && (report[1] == 0x90)) ||
                    ((report[0] == 0x00) && (report[1] == 0x40)) ||
                    ((report[0] == 0x00) && (report[1] == 0x60))) {
                    newMode = MODE_UNKNOWN;
                }
            }
            break;

        case MODE_B:
            if (length == 1) {  // Report Type 2
                if ((report[0] == 0x02) || 
                    (report[0] == 0x01)) {
                    newMode = MODE_UNKNOWN;
                }
            }
            break;

        case MODE_C:
            if (length == 1) {  // Report Type 2
                if (report[0] == 0x01) {
                    newMode = MODE_UNKNOWN;
                }
            }
            if (length == 2) {  //Report Type 4
                if (((report[0] == 0x01) && (report[1] == 0x50)) ||
                    ((report[0] == 0x02) && (report[1] == 0x50))) {
                        newMode = MODE_UNKNOWN;
                    }
            }
            break;

        case MODE_D:
            if (length == 2) {  // Report Type 4
                if (((report[0] == 0x00) && (report[1] == 0x10)) ||
                    ((report[0] == 0x00) && (report[1] == 0x90)) ||
                    ((report[0] == 0x00) && (report[1] == 0x40)) ||
                    ((report[0] == 0x00) && (report[1] == 0x60))) {
                    newMode = MODE_UNKNOWN;
                }
            }
            break;
    }
    if (newMode != currentMode) {
        clearAllButtons();
        currentMode = newMode;
    }
}

void MagicseeR1::printState() {
    DBG_printf("Ring state: MODE-%s : ", getMode());
    if (isButtonPressed(MagicseeR1::A)) DBG_print("A."); else DBG_print(" .");
    if (isButtonPressed(MagicseeR1::B)) DBG_print("B."); else DBG_print(" .");
    if (isButtonPressed(MagicseeR1::C)) DBG_print("C."); else DBG_print(" .");
    if (isButtonPressed(MagicseeR1::D)) DBG_print("D."); else DBG_print(" .");
    if (isButtonPressed(MagicseeR1::L1)) DBG_print("L1."); else DBG_print("  .");
    if (isButtonPressed(MagicseeR1::L2)) DBG_print("L2."); else DBG_print("  .");
    if (isButtonPressed(MagicseeR1::UP)) DBG_print("UP."); else DBG_print("  .");
    if (isButtonPressed(MagicseeR1::DOWN)) DBG_print("DOWN."); else DBG_print("    .");
    if (isButtonPressed(MagicseeR1::LEFT)) DBG_print("LEFT."); else DBG_print("    .");
    if (isButtonPressed(MagicseeR1::RIGHT)) DBG_print("RIGHT"); else DBG_print("     ");
    DBG_println();
}
