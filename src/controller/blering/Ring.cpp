#include "Ring.h"
#include "MagicseeR1.h"
#include "ReportQueue.h"
#include "debug.h"

void Ring::onConnect() {
    //TODO
}

void Ring::onDisconnect() {
    DBG_printf("Ring.onDisconnect: %s\r\n", address);
    myRing.disconnect();
    waitingFor = true;
    connectTo = false;
}

void Ring::onReport(uint8_t* pData, size_t length) {
    if (reportQueue.isFull()) {
          DBG_printf("Report Buffer is full(%s), dropping a report, current dropped count: %u\r\n", address, ++droppedReports);
    } else {
        volatile ReportRecord* newRecord = reportQueue.getNextFreeReportBuffer();
        newRecord->report_len = length;
        memcpy((void *) newRecord->report, pData, min(length, sizeof(newRecord->report)));
    }
}

void Ring::init() {
    address[0] = 0;
    advertisedDevice = NULL;
    waitingFor = true;
    connectTo = false;
}

void Ring::unpress(MagicseeR1::Button button) {
    myRing.unpress(button);
}

void Ring::task() {
    if (!reportQueue.isEmpty()) {
        ReportRecord *newReport = reportQueue.getNextReport();

        DBG_printf("%s:", __func__);
        for (size_t i = 0; i < newReport->report_len; i++) {
           DBG_printf(" %02x", newReport->report[i]);
        }
        //DBG_printf(", head: %d, tail: %d, count: %d, empty: %d, full: %d", reportQueueHead, reportQueueTail, reportQueueCount, reportQueueEmpty(), reportQueueFull());
        DBG_println();

        bool l2Before = myRing.isButtonPressed(MagicseeR1::L2);
        myRing.handleReport(newReport->report, newReport->report_len);
        reportQueue.releaseReportBuffer();
        bool l2After = myRing.isButtonPressed(MagicseeR1::L2);
        if (l2Before && !l2After) {
            otherRing->unpress(MagicseeR1::A);
            otherRing->unpress(MagicseeR1::B);
        }
//        myRing.printState();
    }
}

bool Ring::isButtonPressed(MagicseeR1::Button button) {
    if ((myRing.getMode() != MagicseeR1::MODE_D) ||
        (otherRing->getMode() != MagicseeR1::MODE_D)) {
        return false;
    }
    return myRing.isButtonPressed(button);
}

bool Ring::isButtonClicked(MagicseeR1::Button button) {
    if ((myRing.getMode() != MagicseeR1::MODE_D) ||
        (otherRing->getMode() != MagicseeR1::MODE_D)) {
        return false;
    }
    return myRing.isButtonClicked(button);
}

#define SLOW 32
#define FAST 127
#define MID (SLOW + (FAST - SLOW)/2)

int8_t Ring::getJoystick(MagicseeR1::Direction direction) {
    if ((myRing.getMode() != MagicseeR1::MODE_D) ||
        (otherRing->getMode() != MagicseeR1::MODE_D)) {
        return 0;
    }
    if (!isButtonPressed(MagicseeR1::L2)) {
        return 0;
    }
    int8_t value = 0;
    int8_t range = MID;
    if (otherRing->isButtonPressed(MagicseeR1::A)) {
        range = SLOW;
    } else if (otherRing->isButtonPressed(MagicseeR1::B)) {
        range = FAST;
    }
    switch (direction) {
        case MagicseeR1::X:
            if (isButtonPressed(MagicseeR1::LEFT) && !isButtonPressed(MagicseeR1::RIGHT)) {
                value = -range;
            } else if (isButtonPressed(MagicseeR1::RIGHT) && !isButtonPressed(MagicseeR1::LEFT)) {
                value = range;
            }
            break;

        case MagicseeR1::Y:
            if (isButtonPressed(MagicseeR1::UP) && !isButtonPressed(MagicseeR1::DOWN)) {
                value = range;
            } else if (isButtonPressed(MagicseeR1::DOWN) && !isButtonPressed(MagicseeR1::UP)) {
                value = -range;
            }
            break;

        default:
            value = 0;
    }
    if (value != 0) {
        printState();
    }
    return value;
}

void Ring::printState() {
    myRing.printState();
}