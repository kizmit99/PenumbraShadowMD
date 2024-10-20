#pragma once
#include <Arduino.h>

#define REPORT_BUFFER_SIZE 4

typedef struct {
    uint8_t report[32];
    size_t report_len;
    bool isNotify;
} ReportRecord;

class ReportQueue {
public:
    int8_t head = 0;
    int8_t tail = 0;
    int8_t count = 0;
    ReportRecord buffer[REPORT_BUFFER_SIZE];

    bool isFull() {return count == REPORT_BUFFER_SIZE;}
    bool isEmpty() {return count == 0;}
    ReportRecord *getNextFreeReportBuffer();
    ReportRecord *getNextReport();
    void releaseReportBuffer();
};