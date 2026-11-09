// A minimal stand-in for FastLED, so the sketch compiles on a computer.
//
// It does not need to match FastLED's own math: the regression test only
// compares two versions of the sketch built with this same stand-in. Only
// what the sketch uses is here.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "Arduino.h"

struct CHSV {
  uint8_t h, s, v;
  CHSV(uint8_t H, uint8_t S, uint8_t V) : h(H), s(S), v(V) {}
};

struct CRGB {
  uint8_t r, g, b;
  CRGB() : r(0), g(0), b(0) {}
  constexpr CRGB(uint8_t R, uint8_t G, uint8_t B) : r(R), g(G), b(B) {}
  CRGB(const CHSV &c) {   // Not FastLED's rainbow: any fixed mapping will do
    r = (uint8_t)((c.h * 3 + c.s) * c.v / 255);
    g = (uint8_t)((c.h * 5 + 7) * c.v / 255);
    b = (uint8_t)((255 - c.h + c.s / 2) * c.v / 255);
  }
  CRGB &nscale8(uint8_t s) {   // Same formula as FastLED's scale8
    r = (uint16_t)r * (1 + s) >> 8;
    g = (uint16_t)g * (1 + s) >> 8;
    b = (uint16_t)b * (1 + s) >> 8;
    return *this;
  }
  bool operator==(const CRGB &o) const { return r == o.r && g == o.g && b == o.b; }
};

// Not FastLED's noise: a fixed hash, so flicker is repeatable
inline uint8_t inoise8(uint16_t x, uint16_t t) {
  uint32_t h = x * 2654435761u ^ (t * 40503u + 0x9E37u);
  h ^= h >> 13;
  h *= 0x5bd1e995;
  h ^= h >> 15;
  return (uint8_t)h;
}

enum EOrder { RGB, GRB, BRG };
struct WS2812B {};
struct WS2812 {};
struct PL9823 {};
struct APA106 {};

struct StubFastLED {
  uint8_t brightness = 255;
  template <typename CHIP, int PIN, EOrder ORDER> void addLeds(CRGB *, int) {}
  void setBrightness(uint8_t b) { brightness = b; }
  uint8_t getBrightness() { return brightness; }
  unsigned long shows = 0;       // How many times show() was called: the frames sent
  void show() { shows++; }
  void clear() {}
};
extern StubFastLED FastLED;

// Names the real FastLED takes as macros when built for the Uno or Nano.
// They are defined here the same way, so a sketch that uses one of them as
// a name fails here as it would in the Arduino IDE. LOOP comes from
// platforms/avr/clockless_avr.h and LEDS from FastLED.h.
#define LOOP asm __volatile__("");   // As in FastLED, less its assembly
#define LEDS FastLED
// And a constant FastLED declares at global scope (fl/system/fastpin.h)
const uint8_t NO_PIN = 255;
