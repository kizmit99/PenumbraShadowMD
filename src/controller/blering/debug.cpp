#include "debug.h"
#include <Arduino.h>

void DBG_printHex(const void *p, size_t length) {
  const uint8_t *data = (uint8_t *)p;
  for (size_t i = 0; i < length; i++) {
    DBG_printf(" %02x", data[i]);
  }
}