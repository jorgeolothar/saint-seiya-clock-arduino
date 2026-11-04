// Checks for the parts that have no "before" to compare with: color lists
// of other lengths, the power limit and the test strip. run.sh builds it
// with the current sketch, test strip switched on.
#include <stdio.h>
#include <stdlib.h>
#include "FastLED.h"
#include "EEPROM.h"

unsigned long fakeMillis = 0;
StubSerial Serial;
StubEEPROM EEPROM;
StubFastLED FastLED;

#include SKETCH   // The sketch's FlameClock.ino, set by run.sh

static int failures = 0;

static void check(bool ok, const char *what) {
  printf("  %s  %s\n", ok ? "ok  " : "FAIL", what);
  if (!ok) failures++;
}

static bool near(CRGB c, CRGB want, int tolerance) {
  return abs(c.r - want.r) <= tolerance && abs(c.g - want.g) <= tolerance && abs(c.b - want.b) <= tolerance;
}

const CRGB R = CRGB(255, 0, 0), G = CRGB(0, 255, 0), B = CRGB(0, 0, 255);

int main() {
  setup();

  printf("A 3-color list (SNAP red, STEADY_FADE green, HOLD blue), starting from black:\n");
  const ColorStop THREE[] = { { SNAP, R }, { STEADY_FADE, G }, { HOLD, B } };
  const Look three = { GRADIENT, THREE, 0, FIXED_COLORS };
  const CRGB black = CRGB(0, 0, 0);
  check(three.stopCount == 3, "the list's length is counted");
  check(lookColor(three, black, 0.0f, 0, 1) == R, "SNAP: red at once, at the very start");
  check(lookColor(three, black, 0.30f, 0, 1) == R, "red for the whole first third");
  check(lookColor(three, black, 0.5f, 0, 1) == CRGB(128, 127, 0), "halfway between red and green in the middle");
  check(lookColor(three, black, 0.9f, 0, 1) == G, "HOLD: still green near the end");
  check(lookColor(three, black, 1.0f, 0, 1) == B, "blue at the very end");
  check(firstColor(three, 0, 1) == R && endColor(three, 0, 1) == B, "first and last colors");

  printf("A 1-color list:\n");
  const ColorStop ONE[] = { { SMOOTH_FADE, G } };
  const Look one = { GRADIENT, ONE, 0, FIXED_COLORS };
  check(lookColor(one, R, 0.0f, 0, 1) == R, "starts from the old color");
  check(lookColor(one, R, 0.5f, 0, 1) == CRGB(128, 127, 0), "fades into its color");
  check(near(lookColor(one, R, 0.999f, 0, 1), G, 1), "ends on its color");

  printf("Power limit:\n");
  CRGB white[12];
  FastLED.setBrightness(255);
  for (int i = 0; i < 12; i++) white[i] = CRGB(255, 255, 255);
  const PowerLimit limit = { 400, 20 };
  check(estimateMilliamps(white, 12, limit) == 732, "12 white LEDs at full: 12 x 3 x 20 mA + 12 mA idle = 732 mA");
  limitPower(white, 12, limit);
  uint32_t after = estimateMilliamps(white, 12, limit);
  check(after <= 400 && after >= 380, "dimmed to just under 400 mA");
  FastLED.setBrightness(60);
  for (int i = 0; i < 12; i++) white[i] = CRGB(255, 255, 255);
  limitPower(white, 12, limit);
  check(white[0] == CRGB(255, 255, 255), "left alone when already under the limit (brightness 60)");

  printf("Test strip:\n");
  const Look &look = TEST_STRIP_LOOK;
  CRGB from = endColor(look, 0, TEST_STRIP_LEDS);
  check(near(lookColor(look, from, 0.0f, 0, TEST_STRIP_LEDS), lookColor(look, from, 0.9999f, 0, TEST_STRIP_LEDS), 2),
        "the loop has no jump: the end of a cycle matches the start");
  fakeMillis = 500;
  drawTestStrip();
  check(!(stripLeds[0] == CRGB(0, 0, 0)), "the strip lights up");

  printf(failures ? "%d check(s) FAILED\n" : "All checks passed\n", failures);
  return failures ? 1 : 0;
}
