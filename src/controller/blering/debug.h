#pragma once
#include <Arduino.h>

#define BLE_DEBUG 1

#if BLE_DEBUG
#define DBG_begin(...)    Serial.begin(__VA_ARGS__)
#define DBG_end(...)      Serial.end(__VA_ARGS__)
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#define DBG_printf(...)   Serial.printf(__VA_ARGS__)
#else
#define DBG_begin(...)
#define DBG_end(...)
#define DBG_print(...)
#define DBG_println(...)
#define DBG_printf(...)
#endif

void DBG_printHex(const void *p, size_t length);