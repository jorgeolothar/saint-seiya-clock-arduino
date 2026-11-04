// Regression test: draws every pair of flames from FLAME_LIST on every
// ring, at many times of day, with and without flicker, plus the color
// test, and prints the color of every LED, one frame per line.
//
// run.sh builds this once with an older version of the sketch and once
// with the current one, and compares the two printouts.
//
// It relies only on names both versions have: leds, currentFlame,
// FLAME_LIST, FLAME_COUNT, NUM_CLOCKS, LED_PER_RING, drawClock and
// drawColorTest.
#include <stdio.h>
#include "FastLED.h"
#include "EEPROM.h"

unsigned long fakeMillis = 0;
StubSerial Serial;
StubEEPROM EEPROM;
StubFastLED FastLED;

#include SKETCH   // The sketch's FlameClock.ino, set by run.sh

static uint32_t seed = 12345;
static uint32_t nextRandom() {
  seed = seed * 1664525u + 1013904223u;
  return seed;
}

static void printFrame() {
  for (int ring = 0; ring < NUM_CLOCKS; ring++)
    for (int i = 0; i < LED_PER_RING; i++)
      printf("%02x%02x%02x", leds[ring][i].r, leds[ring][i].g, leds[ring][i].b);
  putchar('\n');
}

int main() {
  printf("rings %d, flames %d\n", NUM_CLOCKS, FLAME_COUNT);
  const uint32_t stepLengths[3] = { 3600000UL, 300000UL, 5000UL };   // hour, 5 minutes, 5 seconds

  for (int a = 0; a < FLAME_COUNT; a++) {
    for (int b = 0; b < FLAME_COUNT; b++) {
      for (int ring = 0; ring < NUM_CLOCKS; ring++) {
        currentFlame[ring][0] = FLAME_LIST[a];
        currentFlame[ring][1] = FLAME_LIST[b];
      }
      printf("pair %d %d\n", a + 1, b + 1);

      for (int ring = 0; ring < NUM_CLOCKS; ring++) drawColorTest(ring);
      printFrame();

      // Every step start and each eighth of a step, over two laps, for
      // each kind of ring. Odd eighths are drawn without flicker.
      for (int unit = 0; unit < 3; unit++)
        for (int step = 0; step < 48; step++)
          for (int eighth = 0; eighth < 8; eighth++) {
            uint32_t ms = (step * stepLengths[unit] + eighth * stepLengths[unit] / 8) % DAY_MS;
            fakeMillis = nextRandom();
            for (int ring = 0; ring < NUM_CLOCKS; ring++) drawClock(ring, ms, eighth & 1);
            printFrame();
          }

      // Random times of day
      for (int n = 0; n < 300; n++) {
        uint32_t ms = nextRandom() % DAY_MS;
        fakeMillis = nextRandom();
        for (int ring = 0; ring < NUM_CLOCKS; ring++) drawClock(ring, ms, n & 1);
        printFrame();
      }
    }
  }
  return 0;
}
