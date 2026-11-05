#pragma once
#include <stdint.h>
#include <string.h>
extern unsigned long fakeMillis;
inline unsigned long millis() { return fakeMillis; }
inline void delay(unsigned long) {}
#define LOW 0
#define HIGH 1
#define INPUT_PULLUP 2
inline int digitalRead(int) { return HIGH; }
inline void pinMode(int, int) {}
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
inline long map(long x, long a, long b, long c, long d) { return (x - a) * (d - c) / (b - a) + c; }
#define F(x) x
const uint8_t A0 = 14, A1 = 15, A2 = 16, A3 = 17;   // The analog pins, as digital pins
struct StubSerial {
  void begin(long) {}
  template <typename T> void print(T) {}
  template <typename T> void println(T) {}
};
extern StubSerial Serial;
