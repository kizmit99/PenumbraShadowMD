#include "ReportQueue.h"
#include <Arduino.h>
#include "debug.h"

ReportRecord *ReportQueue::getNextFreeReportBuffer() {
    if (isFull()) {
        DBG_println("Report Buffer overrun!");
        return NULL;
    }
    int8_t index = head;
    head = (head + 1) % REPORT_BUFFER_SIZE;
    count++;
    return &(buffer[index]);
}

ReportRecord *ReportQueue::getNextReport() {
    if (isEmpty()) {
        DBG_println("getNextReport called on EMPTY queue!");
        return NULL;
    }
    return &buffer[tail];
}

void ReportQueue::releaseReportBuffer() {
    if (isEmpty()) {
        DBG_println("Report Buffer released when empty!");
    } else {
        tail = (tail + 1) % REPORT_BUFFER_SIZE;
        count--;
    }
}
