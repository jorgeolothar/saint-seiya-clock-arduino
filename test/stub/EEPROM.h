#pragma once
#include <string.h>
struct StubEEPROM {
  uint8_t mem[1024] = {0};
  template <typename T> void put(int a, const T &v) { memcpy(mem + a, &v, sizeof(T)); }
  template <typename T> void get(int a, T &v) { memcpy(&v, mem + a, sizeof(T)); }
};
extern StubEEPROM EEPROM;
