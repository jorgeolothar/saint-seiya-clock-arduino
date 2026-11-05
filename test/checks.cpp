// Checks for the parts that have no "before" to compare with: color lists
// of other lengths, the power limit and the temple scenes. run.sh builds
// it with the current sketch, every scene switched on.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
const CRGB K = CRGB(0, 0, 0);

// ---- Scenes: a test scene of their own, so editing Scenes.h does not break these ----
const ColorStop RED_BLUE[] = { { STEADY_FADE, R }, { STEADY_FADE, B } };
const Look ON_LOOK = { GRADIENT, RED_BLUE, 0, FIXED_COLORS };
const ColorStop GREEN[] = { { SNAP, G } };
const Look GREEN_OFF = { GRADIENT, GREEN, 0, FIXED_COLORS };

constexpr Temple TEST_TEMPLES[] = {
//  hour    LEDs  on       off        cycle  repeat         LEDs on the chain
  { 1,      2,    ON_LOOK, OFF,       1000,  REPEAT },      // 0, 1
  { 2,      1,    ON_LOOK, GREEN_OFF, 1000,  ONCE_HOLD },   // 2
  { 3,      3,    ON_LOOK, OFF,       1000,  ONCE_OFF },    // 3, 4, 5
  { 12,     1,    ON_LOOK, OFF,       1000,  REPEAT },      // 6
  { ALWAYS, 2,    ON_LOOK, OFF,       0,     REPEAT },      // 7, 8
};
CRGB testLeds[ledsIn(TEST_TEMPLES)];
const PowerLimit TEST_POWER = { 1000, 20 };
const Scene spotlight(SCENE_TEMPLES(TEST_TEMPLES), testLeds, SPOTLIGHT, clockTime(1, 0, 0), TEST_POWER, 0);
const Scene fill(SCENE_TEMPLES(TEST_TEMPLES), testLeds, FILL, clockTime(1, 0, 0), TEST_POWER, 0);

constexpr Temple EIGHT_TEMPLES[] = {
  { 1, 1, ON_LOOK, OFF, 1000, REPEAT }, { 2, 1, ON_LOOK, OFF, 1000, REPEAT }, { 3, 1, ON_LOOK, OFF, 1000, REPEAT },
  { 4, 1, ON_LOOK, OFF, 1000, REPEAT }, { 5, 1, ON_LOOK, OFF, 1000, REPEAT }, { 6, 1, ON_LOOK, OFF, 1000, REPEAT },
  { 7, 1, ON_LOOK, OFF, 1000, REPEAT }, { 8, 1, ON_LOOK, OFF, 1000, REPEAT },   // the throne
};
CRGB eightLeds[ledsIn(EIGHT_TEMPLES)];
const Scene eight(SCENE_TEMPLES(EIGHT_TEMPLES), eightLeds, SPOTLIGHT, clockTime(0, 0, 0), TEST_POWER, 0);

// Two lamps that fail as often as they can: 3 LEDs, then 2
const ColorStop RED_LAMP[] = { { SNAP, R } };
const Look FAILING_LAMP = { FAILING, RED_LAMP, 100, FIXED_COLORS };
constexpr Temple LAMPS[] = {
  { ALWAYS, 3, FAILING_LAMP, OFF, 0, REPEAT },
  { ALWAYS, 2, FAILING_LAMP, OFF, 0, REPEAT },
};
CRGB lampLeds[ledsIn(LAMPS)];
const Scene lamps(SCENE_TEMPLES(LAMPS), lampLeds, SPOTLIGHT, clockTime(0, 0, 0), TEST_POWER, 0);

// Which temples of a scene are on at a time of day, as a string like "1....": 1 = on
static const char *onAt(const Scene &scene, uint32_t ms, bool allOn = false) {
  static char text[32];
  uint8_t i = 0;
  for (; i < scene.templeCount; i++) {
    uint32_t onFor;
    text[i] = templeOn(scene, scene.temples[i].get(), ms, allOn, onFor) ? '1' : '.';
  }
  text[i] = 0;
  return text;
}

static bool onAtIs(const Scene &scene, uint32_t ms, const char *want, bool allOn = false) {
  const char *got = onAt(scene, ms, allOn);
  bool ok = strcmp(got, want) == 0;
  if (!ok) printf("        got %s, wanted %s\n", got, want);
  return ok;
}

static bool ledsAre(const Scene &scene, const CRGB *want) {
  bool ok = true;
  for (uint16_t i = 0; i < scene.ledCount; i++) ok = ok && scene.leds[i] == want[i];
  if (!ok) {
    printf("        got");
    for (uint16_t i = 0; i < scene.ledCount; i++) printf(" %02x%02x%02x", scene.leds[i].r, scene.leds[i].g, scene.leds[i].b);
    printf("\n");
  }
  return ok;
}

static void checkScenes() {
  printf("Scene layout (temples of 2, 1, 3, 1 and 2 LEDs):\n");
  check(spotlight.templeCount == 5, "5 temples");
  check(spotlight.ledCount == 9 && ledsIn(TEST_TEMPLES) == 9, "9 LEDs, counted for you");
  check(spotlight.hours == 12, "12 hours a cycle: the highest temple hour");
  check(eight.hours == 8, "7 temples and a throne: 8 hours a cycle");

  printf("SPOTLIGHT, hour 1 starting at 1 o'clock (temples: hours 1, 2, 3, 12, ALWAYS):\n");
  check(onAtIs(spotlight, clockTime(1, 30, 0), "1...1"), "01:30: hour 1, and ALWAYS");
  check(onAtIs(spotlight, clockTime(2, 0, 0), ".1..1"), "02:00: hour 2 starts on the dot");
  check(onAtIs(spotlight, clockTime(4, 0, 0), "....1"), "04:00: hours 4 to 11 have no temple");
  check(onAtIs(spotlight, clockTime(0, 30, 0), "...11"), "00:30: hour 12");
  check(onAtIs(spotlight, clockTime(12, 30, 0), "...11"), "12:30: hour 12 again");
  check(onAtIs(spotlight, clockTime(13, 30, 0), "1...1"), "13:30: the second cycle of the day");
  check(onAtIs(spotlight, clockTime(1, 30, 0), "11111", true), "ALL_TEMPLES_ON turns every temple on");

  printf("FILL:\n");
  check(onAtIs(fill, clockTime(2, 30, 0), "11..1"), "02:30: hours 1 and 2 stay on");
  check(onAtIs(fill, clockTime(5, 0, 0), "111.1"), "05:00: hours 1 to 3");
  check(onAtIs(fill, clockTime(0, 59, 59), "11111"), "00:59:59: the last hour, all on");
  check(onAtIs(fill, clockTime(1, 0, 0), "1...1"), "01:00: the cycle starts again");

  printf("8 hours a cycle, from midnight:\n");
  check(onAtIs(eight, clockTime(0, 30, 0), "1......."), "00:30: temple 1");
  check(onAtIs(eight, clockTime(7, 30, 0), ".......1"), "07:30: the throne, once the 7 temples are done");
  check(onAtIs(eight, clockTime(8, 30, 0), "1......."), "08:30: temple 1 again");
  check(onAtIs(eight, clockTime(23, 59, 0), ".......1"), "23:59: the throne, then midnight starts cleanly");

  printf("Drawing (GRADIENT red then blue, 1 second a cycle, no flicker):\n");
  drawScene(spotlight, clockTime(1, 0, 0.5), 1, true, false, false);
  const CRGB first[9] = { R, R, G, K, K, K, K, B, B };
  check(ledsAre(spotlight, first),
        "01:00:00.5: hour 1 on (red), the others show their off looks, ALWAYS with cycle 0 holds blue");
  drawScene(spotlight, clockTime(2, 0, 0.5), 1, true, false, false);
  const CRGB second[9] = { K, K, R, K, K, K, K, B, B };
  check(ledsAre(spotlight, second), "02:00:00.5: only the LED of hour 2 changes");

  check(drawTemple(TEST_TEMPLES[1], true, 0, 0, 0, 0, true) == G, "turning on fades in from the off look");
  check(drawTemple(TEST_TEMPLES[0], true, 1000, 0, 0, 0, true) == B, "REPEAT: the 2nd cycle starts where the 1st ended");
  check(drawTemple(TEST_TEMPLES[0], true, 1500, 0, 0, 0, true) == R, "REPEAT: and plays again");
  check(drawTemple(TEST_TEMPLES[1], true, 5000, 0, 0, 0, true) == B, "ONCE_HOLD: stays on the last color");
  check(drawTemple(TEST_TEMPLES[2], true, 5000, 0, 0, 0, true) == K, "ONCE_OFF: turns to the off look");

  drawScene(spotlight, clockTime(1, 0, 30), 60, true, false, false);
  check(spotlight.leds[0] == R, "sped up 60 times: 30 clock seconds play as half a real second");
  drawScene(spotlight, clockTime(5, 0, 0), 1, true, true, false);
  check(spotlight.leds[3] == R && spotlight.leds[6] == R, "color test: every temple shows its on look's first color");

  printf("The scenes in Scenes.h:\n");
  check(NUM_SCENES == 4, "all four scenes compile (run.sh switches them all on)");
  const Scene &sanctuaryScene = *SCENES[0];
  const Temple aries = sanctuaryScene.temples[0].get(), athena = sanctuaryScene.temples[12].get();
  check(sanctuaryScene.templeCount == 13 && sanctuaryScene.ledCount == 26 && sanctuaryScene.hours == 12,
        "the Sanctuary: 13 temples, 26 LEDs, 12 hours");
  check(aries.hour == 1 && aries.ledCount == 2 && aries.on == SANCTUARY[0].on && aries.repeat == REPEAT,
        "its temples read back out of program space as written");
  check(athena.hour == ALWAYS && athena.cycleMs == 4000, "down to the last one, Athena's temple");
}

static void checkHeartbeat() {
  printf("HEARTBEAT (red then blue, 2nd beat 50%%), starting from green:\n");
  const Look beat = { HEARTBEAT, RED_BLUE, 50, FIXED_COLORS };
  const float mid1 = BEAT_LENGTH / 2, mid2 = SECOND_BEAT_START + BEAT_LENGTH / 2;
  check(lookColor(beat, G, 0.0f, 0, 1) == G, "starts from the old color");
  check(near(lookColor(beat, G, mid1, 0, 1), R, 2), "1st beat: red halfway through it");
  check(lookColor(beat, G, (BEAT_LENGTH + SECOND_BEAT_START) / 2, 0, 1) == B, "between the beats: the last color");
  check(near(lookColor(beat, G, mid2, 0, 1), CRGB(127, 0, 128), 2), "2nd beat: half as strong, halfway between blue and red");
  check(lookColor(beat, G, 0.6f, 0, 1) == B && lookColor(beat, G, 1.0f, 0, 1) == B, "rests on the last color to the end");
  check(arrival(beat, 0.0f) == 0.0f && arrival(beat, BEAT_LENGTH) == 1.0f, "it has taken over by the end of the 1st beat");
  check(drawLook(beat, beat, 0.6f, 0, 1, 0, 0, false) == B, "it doesn't flicker");
  const Look quiet = { HEARTBEAT, RED_BLUE, 0, FIXED_COLORS };
  check(lookColor(quiet, G, mid2, 0, 1) == B, "amount 0: no 2nd beat");

  const Temple heart(1, 1, beat, OFF, 1000, REPEAT);
  check(drawTemple(heart, true, 1000, 0, 0, 0, true) == B, "in a temple, each heartbeat starts from the resting color");
  check(near(drawTemple(heart, true, 1000 + 1000 * mid1, 0, 0, 0, true), R, 2), "and beats again");
}

static void checkPulses() {
  printf("PULSES (red then blue, 3 pulses), starting from green:\n");
  const Look burst = { PULSES, RED_BLUE, 3, FIXED_COLORS };
  const float slot = PULSE_LENGTH + PULSE_GAP, mid = PULSE_LENGTH / 2;
  check(lookColor(burst, G, 0.0f, 0, 1) == G, "starts from the old color");
  check(near(lookColor(burst, G, mid, 0, 1), R, 2), "1st pulse: red halfway through it");
  check(lookColor(burst, G, PULSE_LENGTH + PULSE_GAP / 2, 0, 1) == B, "between pulses: the last color");
  check(near(lookColor(burst, G, slot + mid, 0, 1), R, 2) && near(lookColor(burst, G, 2 * slot + mid, 0, 1), R, 2),
        "2nd and 3rd pulses");
  check(near(lookColor(burst, G, slot + 0.001f, 0, 1), B, 12), "each later pulse starts from the resting color");
  check(lookColor(burst, G, 3 * slot + mid, 0, 1) == B, "no 4th pulse");
  check(lookColor(burst, G, 0.7f, 0, 1) == B && lookColor(burst, G, 1.0f, 0, 1) == B, "rests on the last color to the end");
  check(arrival(burst, 0.0f) == 0.0f && arrival(burst, PULSE_LENGTH) == 1.0f, "it has taken over by the end of the 1st pulse");
  check(drawLook(burst, burst, mid, 0, 1, 0, 0, false) == lookColor(burst, B, mid, 0, 1), "it doesn't flicker");
  const Look none = { PULSES, RED_BLUE, 0, FIXED_COLORS }, many = { PULSES, RED_BLUE, 200, FIXED_COLORS };
  check(lookColor(none, G, slot + mid, 0, 1) == B, "amount 0 counts as 1 pulse");
  check(near(lookColor(many, G, 7 * slot + mid, 0, 1), R, 2) && lookColor(many, G, 8 * slot + mid, 0, 1) == B,
        "amount over 8 counts as 8 pulses");
}

// How many steps an LED is out in one window, and whether they all fall
// between the first and last step of one stutter
static int stutterLength = 0;   // From the first step out to the last, in the last window looked at

static uint8_t stepsOut(uint32_t window, uint16_t spot, uint8_t amount, bool &oneStutter) {
  int first = -1, last = -1, count = 0;
  for (uint8_t s = 0; s < FAIL_WINDOW_STEPS; s++) {
    fakeMillis = (window * FAIL_WINDOW_STEPS + s) * FAIL_STEP_MS;
    if (failedNow(spot, amount)) { count++; if (first < 0) first = s; last = s; }
  }
  oneStutter = (count == 0) || (last - first < FAIL_LONGEST_STEPS);
  stutterLength = (count == 0) ? 0 : last - first + 1;
  return count;
}

static void checkFailing() {
  printf("FAILING (red, steady, with dropouts):\n");
  const ColorStop RED_ONLY[] = { { SNAP, R } };
  const Look failing = { FAILING, RED_ONLY, 30, FIXED_COLORS };
  const Look never = { FAILING, RED_ONLY, 0, FIXED_COLORS }, always = { FAILING, RED_ONLY, 100, FIXED_COLORS };
  const int windows = 20000;

  bool none = true, eachShort = true, allFault = true, stepsAlike = true;
  int faults = 0, rare = 0, stutters = 0, out = 0, steps = 0, apart = 0, longest = 0, shortest = 99;
  for (uint32_t w = 0; w < windows; w++) {
    bool one;
    none = none && stepsOut(w, 7, 0, one) == 0;
    uint8_t n = stepsOut(w, 7, 100, one);
    allFault = allFault && n > 0;
    eachShort = eachShort && one;
    stutters += n < stutterLength;                      // Back on for a step, then out again
    rare += stepsOut(w, 7, 1, one) > 0;
    if (n > longest) longest = n;
    if (n < shortest) shortest = n;
    faults += stepsOut(w, 7, 30, one) > 0;
    if ((stepsOut(w, 7, 30, one) > 0) != (stepsOut(w, 7 + 97, 30, one) > 0)) apart++;
    for (uint8_t s = 0; s < FAIL_WINDOW_STEPS; s++) {    // Steady within a step
      fakeMillis = (w * FAIL_WINDOW_STEPS + s) * FAIL_STEP_MS;
      bool a = failedNow(7, 100);
      fakeMillis += FAIL_STEP_MS - 1;
      stepsAlike = stepsAlike && failedNow(7, 100) == a;
      out += a;
      steps++;
    }
  }
  check(none, "amount 0: never fails");
  check(allFault && shortest == 1 && longest == FAIL_LONGEST_STEPS, "amount 100: a fault every window, 1 to 8 steps out");
  check(eachShort, "each fault is one short stutter, inside its window");
  check(stutters * 100 / windows >= 20, "faults often stutter: out, on, out again");
  check(rare >= windows / 200 && rare <= windows * 3 / 200, "amount 1: a fault in about 1% of windows");
  check(stepsAlike, "an LED stays out or on for a whole step");
  check(out * 100 / steps < 15, "amount 100: still on most of the time (under 15% out)");
  printf("        amount 30: a fault in %d.%d%% of windows; groups 97 apart differ in %d.%d%%\n",
         faults * 100 / windows, faults * 1000 / windows % 10, apart * 100 / windows, apart * 1000 / windows % 10);
  check(faults * 100 / windows >= 28 && faults * 100 / windows <= 32, "amount 30: a fault in about 30% of windows");
  check(apart * 100 / windows >= 35, "different groups fail at different times");

  bool lit = true, dark = true, steadyLit = true, rarelyOut = true;
  for (uint32_t ms = 0; ms < 200000; ms += 5) {
    fakeMillis = ms;
    CRGB c = drawLook(failing, failing, 0.5f, 0, 1, 7, 7, false);
    if (failedNow(7, 30)) dark = dark && c == K; else lit = lit && c == R;
    steadyLit = steadyLit && drawLook(failing, failing, 0.5f, 0, 1, 7, 7, true) == R;
    rarelyOut = rarelyOut && drawLook(never, never, 0.5f, 0, 1, 7, 7, false) == R;
  }
  check(lit, "on: its own color, with no flicker");
  check(dark, "out: dark");
  check(steadyLit, "steady (MODE_FREEZE): never out");
  check(rarelyOut, "amount 0: always on");
  fakeMillis = 0;
  for (uint32_t s = 0; s < 2000 && !failedNow(7, 100); s++) fakeMillis += FAIL_STEP_MS;
  check(drawLook(always, OFF, 0.0f, 0, 1, 7, 7, false) == K, "a fault shows from the start when it SNAPs in");
  check(drawLook(ON_LOOK, always, 0.0f, 0, 1, 7, 7, false) == K && drawLook(ON_LOOK, always, 1.0f, 0, 1, 7, 7, false) == B,
        "taking over from a failing look: its faults fade out as the new look fades in");
  check(lookFlicker(failing) == 0, "it doesn't flicker");

  bool together = true, wentOut = false;
  int lampsApart = 0;
  for (uint32_t ms = 0; ms < 200000; ms += 5) {
    fakeMillis = ms;
    drawScene(lamps, 0, 1, false, false, false);
    together = together && lampLeds[0] == lampLeds[1] && lampLeds[1] == lampLeds[2] && lampLeds[3] == lampLeds[4];
    wentOut = wentOut || lampLeds[0] == K;
    lampsApart += !(lampLeds[0] == lampLeds[3]);
  }
  check(wentOut && together, "in a temple, all its LEDs go out together");
  check(lampsApart > 1000, "two temples fail at different times");
  fakeMillis = 0;
}

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

  printf("Flicker speed (FLICKER_SPEED_PERCENT = %u):\n", (unsigned)FLICKER_SPEED_PERCENT);
  if (FLICKER_SPEED_PERCENT == 100) {
    bool same = true;
    const uint32_t times[] = { 0, 1, 2, 299, 300, 301, 196607, 196608, 86400000UL, 4000000000UL, 4294967295UL };
    for (uint32_t ms : times) { fakeMillis = ms; same = same && flickerTime() == (uint16_t)(ms / 3); }
    for (uint32_t i = 0; i < 100000; i++) { fakeMillis = i * 42949UL + 7; same = same && flickerTime() == (uint16_t)(fakeMillis / 3); }
    check(same, "100%: one step every 3 ms, as before, from the start to 49 days");
  }
  fakeMillis = 0;

  checkHeartbeat();
  checkPulses();
  checkFailing();
  checkScenes();

  printf(failures ? "%d check(s) FAILED\n" : "All checks passed\n", failures);
  return failures ? 1 : 0;
}
