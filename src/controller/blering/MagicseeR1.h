#pragma once
#include <Arduino.h>

class MagicseeR1 {
public:
    enum Button {
        A = 0, 
        B = 1, 
        C = 2, 
        D = 3, 
        L1 = 4, 
        L2 = 5, 
        UP = 6, 
        DOWN = 7,
        LEFT = 8, 
        RIGHT = 9
    };

    enum Direction {
        X, Y
    };

    enum Mode {MODE_A, MODE_B, MODE_C, MODE_D, MODE_UNKNOWN};

    bool isConnected = false;

    void handleReport(uint8_t *report, int length);
    void disconnect();
    bool isButtonPressed(Button button);
    bool isButtonClicked(Button button);
    const char* getAdvertisedName() {return "Magicsee R1";}
    Mode getMode() {return currentMode;}
    void printState();
    void unpress(Button button);

private:
    static const uint8_t buttonCount = ((uint8_t) RIGHT) + 1;

    bool pressedButtons[buttonCount];
    bool clickedButtons[buttonCount];
    Mode currentMode = MODE_UNKNOWN;

    const char *modeString(Mode mode);
    void unexpected(volatile void *report, size_t length);
    void testForModeChange(volatile uint8_t *report, int length);
    void clearAllButtons();
    void press(Button button);
    void click(Button button);
    void unclick(Button button);
};
