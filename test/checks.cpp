// Checks for the parts that have no "before" to compare with: color lists
// of other lengths, the power limit and the scenes. run.sh builds
// it with the current sketch, every scene switched on.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
const ColorStop JUST_GREEN[] = { { SNAP, G } };
const Look GREEN_OFF = { GRADIENT, JUST_GREEN, 0, FIXED_COLORS };

constexpr Group TEST_GROUPS[] = {
//  hour    LEDs  on       off        cycle  repeat         LEDs on the chain
  { 1,      2,    ON_LOOK, OFF,       1000,  REPEAT },      // 0, 1
  { 2,      1,    ON_LOOK, GREEN_OFF, 1000,  ONCE_HOLD },   // 2
  { 3,      3,    ON_LOOK, OFF,       1000,  ONCE_OFF },    // 3, 4, 5
  { 12,     1,    ON_LOOK, OFF,       1000,  REPEAT },      // 6
  { ALWAYS, 2,    ON_LOOK, OFF,       0,     REPEAT },      // 7, 8
};
CRGB testLeds[ledsIn(TEST_GROUPS)];
const PowerLimit TEST_POWER = { 1000, 20 };
const Scene spotlight(SCENE_GROUPS(TEST_GROUPS), testLeds, SPOTLIGHT, clockTime(1, 0, 0), TEST_POWER, 0);
const Scene fill(SCENE_GROUPS(TEST_GROUPS), testLeds, FILL, clockTime(1, 0, 0), TEST_POWER, 0);

constexpr Group EIGHT_GROUPS[] = {
  { 1, 1, ON_LOOK, OFF, 1000, REPEAT }, { 2, 1, ON_LOOK, OFF, 1000, REPEAT }, { 3, 1, ON_LOOK, OFF, 1000, REPEAT },
  { 4, 1, ON_LOOK, OFF, 1000, REPEAT }, { 5, 1, ON_LOOK, OFF, 1000, REPEAT }, { 6, 1, ON_LOOK, OFF, 1000, REPEAT },
  { 7, 1, ON_LOOK, OFF, 1000, REPEAT }, { 8, 1, ON_LOOK, OFF, 1000, REPEAT },   // the throne
};
CRGB eightLeds[ledsIn(EIGHT_GROUPS)];
const Scene eight(SCENE_GROUPS(EIGHT_GROUPS), eightLeds, SPOTLIGHT, clockTime(0, 0, 0), TEST_POWER, 0);

// Two lamps that fail as often as they can: 3 LEDs, then 2
const ColorStop RED_LAMP[] = { { SNAP, R } };
const Look FAILING_LAMP = { FAILING, RED_LAMP, 100, FIXED_COLORS };
constexpr Group LAMPS[] = {
  { ALWAYS, 3, FAILING_LAMP, OFF, 0, REPEAT },
  { ALWAYS, 2, FAILING_LAMP, OFF, 0, REPEAT },
};
CRGB lampLeds[ledsIn(LAMPS)];
const Scene lamps(SCENE_GROUPS(LAMPS), lampLeds, SPOTLIGHT, clockTime(0, 0, 0), TEST_POWER, 0);

// Two skies that strike as often as they can: 3 LEDs, then 2
const ColorStop WHITE_ON_BLUE[] = { { SNAP, CRGB(255, 255, 255) }, { STEADY_FADE, B } };
const Look STRIKING = { LIGHTNING, WHITE_ON_BLUE, 100, FIXED_COLORS };
constexpr Group SKIES[] = {
  { ALWAYS, 3, STRIKING, OFF, 0, REPEAT },
  { ALWAYS, 2, STRIKING, OFF, 0, REPEAT },
};
CRGB skyLeds[ledsIn(SKIES)];
const Scene skies(SCENE_GROUPS(SKIES), skyLeds, SPOTLIGHT, clockTime(0, 0, 0), TEST_POWER, 0);

// Which groups of a scene are on at a time of day, as a string like "1....": 1 = on
static const char *onAt(const Scene &scene, uint32_t ms, bool allOn = false) {
  static char text[32];
  uint8_t i = 0;
  for (; i < scene.groupCount; i++) {
    uint32_t onFor;
    text[i] = groupOn(scene, scene.groups[i].get(), ms, allOn, onFor) ? '1' : '.';
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
  printf("Scene layout (groups of 2, 1, 3, 1 and 2 LEDs):\n");
  check(spotlight.groupCount == 5, "5 groups");
  check(spotlight.ledCount == 9 && ledsIn(TEST_GROUPS) == 9, "9 LEDs, counted for you");
  check(spotlight.hours == 12, "12 hours a cycle: the highest group hour");
  check(eight.hours == 8, "7 groups and a throne: 8 hours a cycle");

  printf("SPOTLIGHT, hour 1 starting at 1 o'clock (groups: hours 1, 2, 3, 12, ALWAYS):\n");
  check(onAtIs(spotlight, clockTime(1, 30, 0), "1...1"), "01:30: hour 1, and ALWAYS");
  check(onAtIs(spotlight, clockTime(2, 0, 0), ".1..1"), "02:00: hour 2 starts on the dot");
  check(onAtIs(spotlight, clockTime(4, 0, 0), "....1"), "04:00: hours 4 to 11 have no group");
  check(onAtIs(spotlight, clockTime(0, 30, 0), "...11"), "00:30: hour 12");
  check(onAtIs(spotlight, clockTime(12, 30, 0), "...11"), "12:30: hour 12 again");
  check(onAtIs(spotlight, clockTime(13, 30, 0), "1...1"), "13:30: the second cycle of the day");
  check(onAtIs(spotlight, clockTime(1, 30, 0), "11111", true), "ALL_GROUPS_ON turns every group on");

  printf("FILL:\n");
  check(onAtIs(fill, clockTime(2, 30, 0), "11..1"), "02:30: hours 1 and 2 stay on");
  check(onAtIs(fill, clockTime(5, 0, 0), "111.1"), "05:00: hours 1 to 3");
  check(onAtIs(fill, clockTime(0, 59, 59), "11111"), "00:59:59: the last hour, all on");
  check(onAtIs(fill, clockTime(1, 0, 0), "1...1"), "01:00: the cycle starts again");

  printf("8 hours a cycle, from midnight:\n");
  check(onAtIs(eight, clockTime(0, 30, 0), "1......."), "00:30: group 1");
  check(onAtIs(eight, clockTime(7, 30, 0), ".......1"), "07:30: the throne, once the 7 groups are done");
  check(onAtIs(eight, clockTime(8, 30, 0), "1......."), "08:30: group 1 again");
  check(onAtIs(eight, clockTime(23, 59, 0), ".......1"), "23:59: the throne, then midnight starts cleanly");

  printf("Drawing (GRADIENT red then blue, 1 second a cycle, no flicker):\n");
  drawScene(spotlight, clockTime(1, 0, 0.5), 1, true, false, false);
  const CRGB first[9] = { R, R, G, K, K, K, K, B, B };
  check(ledsAre(spotlight, first),
        "01:00:00.5: hour 1 on (red), the others show their off looks, ALWAYS with cycle 0 holds blue");
  drawScene(spotlight, clockTime(2, 0, 0.5), 1, true, false, false);
  const CRGB second[9] = { K, K, R, K, K, K, K, B, B };
  check(ledsAre(spotlight, second), "02:00:00.5: only the LED of hour 2 changes");

  check(drawGroup(TEST_GROUPS[1], true, 0, 0, 0, 0, true) == G, "turning on fades in from the off look");
  check(drawGroup(TEST_GROUPS[0], true, 1000, 0, 0, 0, true) == B, "REPEAT: the 2nd cycle starts where the 1st ended");
  check(drawGroup(TEST_GROUPS[0], true, 1500, 0, 0, 0, true) == R, "REPEAT: and plays again");
  check(drawGroup(TEST_GROUPS[1], true, 5000, 0, 0, 0, true) == B, "ONCE_HOLD: stays on the last color");
  check(drawGroup(TEST_GROUPS[2], true, 5000, 0, 0, 0, true) == K, "ONCE_OFF: turns to the off look");

  drawScene(spotlight, clockTime(1, 0, 30), 60, true, false, false);
  check(spotlight.leds[0] == R, "sped up 60 times: 30 clock seconds play as half a real second");
  drawScene(spotlight, clockTime(5, 0, 0), 1, true, true, false);
  check(spotlight.leds[3] == R && spotlight.leds[6] == R, "color test: every group shows its on look's first color");

  printf("The scenes in Scenes.h:\n");
  check(NUM_SCENES == 5, "all five scenes compile (run.sh switches them all on)");
  const Scene &demoScene = *SCENES[0];
  bool allOn = true;
  int shown[FLOW + 1] = {};
  for (uint8_t i = 0; i < demoScene.groupCount; i++) {
    const Group group = demoScene.groups[i].get();
    allOn = allOn && group.hour == ALWAYS && (group.on->effect == FLOW ? group.ledCount > 1 : group.ledCount == 1);
    shown[group.on->effect]++;
  }
  bool eachOnce = true;
  for (int e = GRADIENT; e <= FLOW; e++) eachOnce = eachOnce && shown[e] == 1;
  check(demoScene.groupCount == FLOW + 1 && allOn, "the demo: 1 LED for each effect (a few for FLOW), always on");
  check(demoScene.ledCount == 12, "12 LEDs: a 12-LED ring shows it all");
  check(eachOnce, "it shows every effect, once");
  check(timesUsed(DEMO_PIN) == 1 && timesUsed(RING1_PIN) == 1 && timesUsed(UNUSED_PIN) == MAX_RINGS - NUM_CLOCKS,
        "every pin in use has one job, and the unused ring pins are left out");
  const Scene &sanctuaryScene = *SCENES[1];
  const Group aries = sanctuaryScene.groups[0].get(), athena = sanctuaryScene.groups[12].get();
  check(sanctuaryScene.groupCount == 13 && sanctuaryScene.ledCount == 26 && sanctuaryScene.hours == 12,
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

  const Group heart(1, 1, beat, OFF, 1000, REPEAT);
  check(drawGroup(heart, true, 1000, 0, 0, 0, true) == B, "in a group, each heartbeat starts from the resting color");
  check(near(drawGroup(heart, true, 1000 + 1000 * mid1, 0, 0, 0, true), R, 2), "and beats again");
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
  check(wentOut && together, "in a group, all its LEDs go out together");
  check(lampsApart > 1000, "two groups fail at different times");
  fakeMillis = 0;
}

// One window of a lightning look, 1 ms at a time: how many flashes it
// has (each starts on the first color, or brighter than the moment
// before), how many ms it is off its resting color, and whether the 1st
// flash starts at full strength
struct StrikeSeen { int flashes, msLit, firstLit, lastLit; bool fullFirst, weakerLater, overRest, someWeaker; };
static StrikeSeen watchStrike(const Look &look, uint32_t window, uint16_t spot) {
  StrikeSeen seen = { 0, 0, -1, -1, false, true, true, false };
  CRGB rest = endColor(look, 0, 1), before = rest;
  for (uint16_t t = 0; t < STRIKE_WINDOW_MS; t++) {
    fakeMillis = window * STRIKE_WINDOW_MS + t;
    CRGB c = drawLook(look, look, 1.0f, 0, 1, 7, spot, false);
    if (!(c == rest)) {
      seen.msLit++;
      if (seen.firstLit < 0) seen.firstLit = t;
      seen.lastLit = t;
    }
    if (c.r > before.r) {                                // A flash starts
      if (seen.flashes == 0) seen.fullFirst = (c == firstColor(look, 0, 1));
      else {
        seen.weakerLater = seen.weakerLater && c.r >= 255 * FLASH_WEAKEST_PERCENT / 100 - 1;
        seen.someWeaker = seen.someWeaker || c.r < 250;
      }
      seen.flashes++;
    }
    seen.overRest = seen.overRest && c.b >= rest.b && c.r >= rest.r;
    before = c;
  }
  return seen;
}

static void checkLightning() {
  printf("LIGHTNING (white flashes, resting on blue):\n");
  const Look sometimes = { LIGHTNING, WHITE_ON_BLUE, 30, FIXED_COLORS };
  const Look never = { LIGHTNING, WHITE_ON_BLUE, 0, FIXED_COLORS };
  const int windows = 3000;
  const Look rarely = { LIGHTNING, WHITE_ON_BLUE, 1, FIXED_COLORS };
  bool every = true, fit = true, fullFirst = true, weaker = true, overRest = true, quiet = true, toEnd = true, anyWeaker = false;
  int counts[MOST_FLASHES + 2] = { 0 }, struck = 0, rare = 0, lit = 0, apart = 0, oneFlash = -1, oneStart = 0;
  for (int w = 0; w < windows; w++) {
    StrikeSeen a = watchStrike(STRIKING, w, 7);
    every = every && a.flashes >= 1;
    fit = fit && a.flashes <= MOST_FLASHES && a.lastLit - a.firstLit < LONGEST_STRIKE_MS;
    toEnd = toEnd && a.lastLit < STRIKE_WINDOW_MS - 1;
    if (a.flashes == 1 && oneFlash < 0) { oneFlash = w; oneStart = a.firstLit; }
    fullFirst = fullFirst && a.fullFirst;
    weaker = weaker && a.weakerLater;
    anyWeaker = anyWeaker || a.someWeaker;
    overRest = overRest && a.overRest;
    counts[a.flashes > MOST_FLASHES ? MOST_FLASHES + 1 : a.flashes]++;
    lit += a.msLit;
    StrikeSeen b = watchStrike(sometimes, w, 7);
    struck += b.flashes > 0;
    apart += (b.flashes > 0) != (watchStrike(sometimes, w, 7 + 97).flashes > 0);
    quiet = quiet && watchStrike(never, w, 7).msLit == 0;
    rare += watchStrike(rarely, w, 7).flashes > 0;
  }
  check(quiet, "amount 0: never strikes");
  check(every, "amount 100: a strike every window");
  check(fit, "1 to 4 flashes, all inside the window");
  check(counts[1] > 0 && counts[2] > 0 && counts[3] > 0 && counts[4] > 0, "strikes of 1, 2, 3 and 4 flashes");
  check(toEnd, "each strike is over before its window ends");
  check(fullFirst, "the 1st flash starts on the first color, at full strength");
  fakeMillis = oneFlash * STRIKE_WINDOW_MS + oneStart + FLASH_MS / 4;
  CRGB early = drawLook(STRIKING, STRIKING, 1.0f, 0, 1, 7, 7, false);
  fakeMillis += FLASH_MS / 2;
  CRGB late = drawLook(STRIKING, STRIKING, 1.0f, 0, 1, 7, 7, false);
  check(early == CRGB(255, 255, 255) && near(late, fadeBetween(CRGB(255, 255, 255), B, 0.75f), 4),
        "a flash: white, then fading back into blue, quickly at first");
  check(weaker && anyWeaker, "later flashes are weaker, but at least FLASH_WEAKEST_PERCENT strong");
  check(overRest, "flashes only brighten the resting color");
  check(lit * 100 / (windows * STRIKE_WINDOW_MS) < 25, "amount 100: resting most of the time");
  printf("        amount 30: a strike in %d%% of windows; groups 97 apart differ in %d%%\n",
         struck * 100 / windows, apart * 100 / windows);
  check(struck * 100 / windows >= 26 && struck * 100 / windows <= 34, "amount 30: a strike in about 30% of windows");
  check(apart * 100 / windows >= 30, "different groups strike at different times");
  check(rare >= windows / 200 && rare <= windows * 3 / 200, "amount 1: a strike in about 1% of windows");

  bool steadyRest = true;
  for (uint32_t ms = 0; ms < 100000; ms += 3) {
    fakeMillis = ms;
    steadyRest = steadyRest && drawLook(STRIKING, STRIKING, 1.0f, 0, 1, 7, 7, true) == B;
  }
  check(steadyRest, "steady (MODE_FREEZE): rests on the last color");
  check(drawGroup(Group(1, 1, STRIKING, OFF, 0, REPEAT), true, 0, 0, 0, 0, true) == B,
        "with cycle 0, its sky takes over at once");
  check(lookColor(STRIKING, G, 0.5f, 0, 1) == fadeBetween(G, B, 0.5f) && arrival(STRIKING, 0.5f) == 0.5f,
        "with a cycle, its sky fades in like GRADIENT");
  check(lookFlicker(STRIKING) == 0, "it doesn't flicker");

  bool together = true, flashed = false;
  int skiesApart = 0;
  for (uint32_t ms = 0; ms < 200000; ms += 3) {
    fakeMillis = ms;
    drawScene(skies, 0, 1, false, false, false);
    together = together && skyLeds[0] == skyLeds[1] && skyLeds[1] == skyLeds[2] && skyLeds[3] == skyLeds[4];
    flashed = flashed || !(skyLeds[0] == B);
    skiesApart += !(skyLeds[0] == skyLeds[3]);
  }
  check(flashed && together, "in a group, all its LEDs strike together");
  check(skiesApart > 1000, "two groups strike at different times");
  fakeMillis = 0;
}

static void checkGroupAmount() {
  printf("A group's own amount (PULSES, red then blue, 3 pulses):\n");
  const Look burst = { PULSES, RED_BLUE, 3, FIXED_COLORS };
  const Group own(1, 1, burst, OFF, 1000, REPEAT), five(1, 1, burst, OFF, 1000, REPEAT, 5);
  const float slot = PULSE_LENGTH + PULSE_GAP, mid = PULSE_LENGTH / 2;
  const uint32_t fourth = 1000 + (uint32_t)(1000 * (3 * slot + mid) + 0.5f);
  check(own.amount == LOOK_AMOUNT, "left out: the look's own amount");
  check(drawGroup(own, true, fourth, 0, 0, 0, true) == B, "the look's own 3 pulses: no 4th");
  check(near(drawGroup(five, true, fourth, 0, 0, 0, true), R, 2), "amount 5 for this group: a 4th pulse");
  check(near(drawGroup(five, true, 1000 + (uint32_t)(1000 * (4 * slot + mid) + 0.5f), 0, 0, 0, true), R, 2) &&
        drawGroup(five, true, 1000 + (uint32_t)(1000 * (5 * slot + mid) + 0.5f), 0, 0, 0, true) == B, "and a 5th, but no 6th");
  check(near(drawGroup(five, true, (uint32_t)(1000 * (4 * slot + mid) + 0.5f), 0, 0, 0, true), R, 2),
        "in the first cycle too");
  check(burst.amount == 3, "the look itself is unchanged");

  const Group calm(ALWAYS, 1, STRIKING, OFF, 0, REPEAT, 0);
  bool still = true;
  for (uint32_t ms = 0; ms < 100000; ms += 3) {
    fakeMillis = ms;
    still = still && drawGroup(calm, true, ms, 0, 0, 7, false) == B;
  }
  check(still, "LIGHTNING at 100%, amount 0 for this group: never strikes");
  const Look steadyFlame = { FLAME, RED_BLUE, 0, FIXED_COLORS };
  const Group flickering(1, 1, steadyFlame, OFF, 1000, REPEAT, 100);
  bool flickers = false;
  CRGB first = CRGB(0, 0, 0);
  for (uint32_t ms = 0; ms < 20000; ms += 7) {
    fakeMillis = ms;
    CRGB c = drawGroup(flickering, true, 1000, 0, 0, 0, false);   // The 2nd cycle starting, still on blue
    if (ms == 0) first = c;
    flickers = flickers || !(c == first);
  }
  check(flickers, "FLAME at 0%, flicker 100 for this group: flickers, cycle after cycle");
  const Group own2 = SCENES[1]->groups[0].get();
  check(own2.amount == LOOK_AMOUNT, "the Sanctuary's groups read back with the look's own amount");
  fakeMillis = 0;
}

static void checkBuildup() {
  printf("BUILDUP (red then blue, throbs 60%% deep), starting from green:\n");
  const Look charge = { BUILDUP, RED_BLUE, 60, FIXED_COLORS };
  const Look smooth = { BUILDUP, RED_BLUE, 0, FIXED_COLORS };
  auto at = [](float charged) { return charged * CHARGE; };              // A point of the charge, as a phase
  auto top = [](float k) { return (float)sqrt(k / THROBS); };          // The top of throb k
  check(lookColor(charge, G, 0.0f, 0, 1) == G, "starts from the old color");
  check(near(lookColor(charge, G, at(top(2)), 0, 1), R, 2), "charging: red at the top of a throb halfway through");
  CRGB full = listColor(charge, G, top(2.5f), 0, 1), dimmed = full;
  dimmed.nscale8(255 - 60 * 255 / 100);
  check(near(lookColor(charge, G, at(top(2.5f)), 0, 1), dimmed, 2), "at the bottom of a throb: 60% darker");
  check(near(lookColor(smooth, G, at(top(2.5f)), 0, 1), full, 1), "amount 0: no throbs, a smooth rise");

  int firstHalf = 0, secondHalf = 0;
  bool high = true;
  for (int i = 0; i <= 4000; i++) {                                   // Count the dips: near full, then clearly darker
    float charged = i / 4000.0f;
    CRGB c = lookColor(charge, G, at(charged), 0, 1), plain = listColor(charge, G, charged, 0, 1);
    float level = (float)(c.r + c.g + c.b) / (plain.r + plain.g + plain.b);
    if (high && level < 0.7f) { if (charged < 0.5f) firstHalf++; else secondHalf++; high = false; }
    if (level > 0.85f) high = true;
  }
  check(firstHalf + secondHalf == THROBS, "8 throbs while charging");
  check(secondHalf > 2 * firstHalf, "faster and faster: most of them in the 2nd half");

  check(lookColor(charge, G, CHARGE, 0, 1) == B && lookColor(charge, G, CHARGE + BURST * 0.9f, 0, 1) == B,
        "the burst: the last color, at full");
  CRGB fading = lookColor(charge, G, (CHARGE + BURST + 1.0f) / 2, 0, 1);
  check(fading.r > 128 && fading.b < 128, "then fading back, quickly at first");
  check(lookColor(charge, G, 1.0f, 0, 1) == R && endColor(charge, 0, 1) == R, "it ends on its first color");
  check(arrival(smooth, 0.0f) == 0.0f && arrival(smooth, CHARGE / 2) == 1.0f, "it has taken over halfway through the charge");
  check(drawLook(charge, charge, CHARGE, 0, 1, 0, 0, false) == B, "it doesn't flicker");

  const Group ray(1, 1, charge, OFF, 1000, REPEAT);
  check(drawGroup(ray, true, 0, 0, 0, 0, true) == K, "in a group, the 1st cycle starts from the off look");
  check(drawGroup(ray, true, 1000, 0, 0, 0, true) == R && near(drawGroup(ray, true, 1999, 0, 0, 0, true), R, 2),
        "and each later cycle starts and ends on the first color");
}

static void checkExplosion() {
  printf("EXPLOSION (red, then green, then blue, crackle 100%%):\n");
  const ColorStop RGB_LIST[] = { { STEADY_FADE, R }, { STEADY_FADE, G }, { STEADY_FADE, B } };
  const Look boom = { EXPLOSION, RGB_LIST, 100, FIXED_COLORS };
  const Look calm = { EXPLOSION, RGB_LIST, 0, FIXED_COLORS };
  const ColorStop RED_ONLY[] = { { STEADY_FADE, R } };
  const Look single = { EXPLOSION, RED_ONLY, 100, FIXED_COLORS };
  auto cooling = [](float part) { return BLAST_FLASH + (1.0f - BLAST_FLASH) * part; };   // A point of the cooling, as a phase
  check(lookColor(boom, G, 0.0f, 0, 1) == R && arrival(boom, 0.0f) == 1.0f,
        "flashes on its first color at once, whatever its fade");
  check(lookColor(boom, G, BLAST_FLASH * 0.9f, 0, 1) == R, "the flash lasts the start of the cycle");
  CRGB quarter = lookColor(boom, G, cooling(0.25f), 0, 1);
  check(quarter.g > 200 && quarter.b == 0, "a quarter into the cooling, nearly at its 2nd color: quickly at first");
  check(lookColor(boom, G, 1.0f, 0, 1) == B && endColor(boom, 0, 1) == B, "it ends on its last color");
  check(lookColor(single, G, 0.5f, 0, 1) == R, "with one color, it stays on it");
  const ColorStop SNAPPING[] = { { STEADY_FADE, R }, { SNAP, G } };
  const Look snapping = { EXPLOSION, SNAPPING, 0, FIXED_COLORS };
  check(lookColor(snapping, K, BLAST_FLASH * 0.5f, 0, 1) == R && lookColor(snapping, K, cooling(0.01f), 0, 1) == G,
        "a 2nd color that SNAPs comes right after the flash, not during it");

  check(lookFlicker(boom, BLAST_FLASH * 0.5f) == 0 && lookFlicker(boom, cooling(0.5f)) == 50 &&
        lookFlicker(boom, 1.0f) == 100, "it crackles more and more as it cools: 0, 50, 100%");
  bool flashSteady = true, calmSteady = true;
  for (uint32_t ms = 0; ms < 20000; ms += 7) {
    fakeMillis = ms;
    flashSteady = flashSteady && drawLook(boom, boom, BLAST_FLASH * 0.5f, 0, 1, 0, 0, false) == R;
    calmSteady = calmSteady && drawLook(calm, calm, cooling(0.5f), 0, 1, 0, 0, false) == lookColor(calm, K, cooling(0.5f), 0, 1);
  }
  check(flashSteady, "the flash doesn't flicker or flare up");
  check(calmSteady, "amount 0: a smooth fade, no crackle");

  int windows = 0, flared = 0, flaredOften = 0;
  bool inRange = true, reachesFull = false, flashOrColdFlares = false, spotsDiffer = false, calmFlares = false;
  const Look sometimes = { EXPLOSION, RGB_LIST, 30, FIXED_COLORS };
  for (uint32_t w = 0; w < 2000; w++) {
    bool any = false, anyOften = false;
    for (uint32_t ms = w * FLARE_WINDOW_MS; ms < (w + 1) * FLARE_WINDOW_MS; ms += 5) {
      fakeMillis = ms;
      float f = emberFlare(boom, 0.5f, 7);
      inRange = inRange && f >= 0.0f && f <= 1.0f;
      reachesFull = reachesFull || f > 0.95f;
      anyOften = anyOften || f > 0.0f;
      any = any || emberFlare(sometimes, 0.5f, 7) > 0.0f;
      flashOrColdFlares = flashOrColdFlares || emberFlare(boom, BLAST_FLASH * 0.5f, 7) > 0.0f || emberFlare(boom, 1.0f, 7) > 0.0f;
      spotsDiffer = spotsDiffer || emberFlare(boom, 0.5f, 7) != emberFlare(boom, 0.5f, 300);
      calmFlares = calmFlares || emberFlare(calm, 0.5f, 7) > 0.0f;
    }
    windows++;
    if (any) flared++;
    if (anyOften) flaredOften++;
  }
  check(flaredOften == windows, "100%: a flare-up every window (0.5 s)");
  check(flared * 100 / windows >= 25 && flared * 100 / windows <= 35, "30%: a flare-up in about 30% of the windows");
  check(inRange && reachesFull, "a flare-up rises to full and goes back down");
  check(!flashOrColdFlares && !calmFlares, "no flare-ups during the flash, once cold, or at amount 0");
  check(spotsDiffer, "different groups flare up at different times");
  check(near(explosionColor(boom, 0.9999f, 1.0f, 0, 1), fadeBetween(G, B, 0.4f), 2),
        "a full flare-up takes it 30% of the way back toward the flash");
  check(drawLook(boom, boom, cooling(0.5f), 0, 1, 0, 0, true) == lookColor(boom, K, cooling(0.5f), 0, 1),
        "steady: no crackle or flare-ups");
  bool drawn = true;
  int flaring = 0;
  for (uint32_t ms = 0; ms < 20000; ms += 7) {
    fakeMillis = ms;
    float flare = emberFlare(boom, cooling(0.7f), 300);
    CRGB expected = explosionColor(boom, cooling(0.7f), flare, 0, 1);
    expected.nscale8(flickerScale(5, lookFlicker(boom, cooling(0.7f))));
    drawn = drawn && drawLook(boom, boom, cooling(0.7f), 0, 1, 5, 300, false) == expected;
    if (flare > 0.5f) flaring++;
  }
  check(drawn && flaring > 100, "drawn: the group's flare-ups, then each LED's own crackle");

  const Group blast(1, 1, boom, OFF, 1000, REPEAT), once(1, 1, calm, OFF, 1000, ONCE_HOLD);
  check(drawGroup(blast, true, 0, 0, 0, 0, true) == R && drawGroup(blast, true, 1000, 0, 0, 0, true) == R,
        "in a group, each cycle starts with the flash");
  bool holds = true;
  for (uint32_t ms = 0; ms < 5000; ms += 7) {
    fakeMillis = ms;
    holds = holds && drawGroup(once, true, 3000, 0, 0, 0, false) == B;
  }
  check(holds, "ONCE_HOLD: stays on its last color");
  fakeMillis = 0;
}

static void checkFlow() {
  printf("FLOW (red then blue, 1000 ms a cycle, 4 LEDs):\n");
  const Look flowing = { FLOW, RED_BLUE, 100, FIXED_COLORS };
  const Look gradient = { GRADIENT, RED_BLUE, 0, FIXED_COLORS };
  const Group wave(1, 4, flowing, OFF, 1000, REPEAT), two(1, 4, flowing, OFF, 1000, REPEAT, 200),
              together(1, 4, flowing, OFF, 1000, REPEAT, 0), half(1, 4, flowing, OFF, 1000, REPEAT, 50);
  check(lookColor(flowing, G, 0.3f, 0, 1) == lookColor(gradient, G, 0.3f, 0, 1) && lookFlicker(flowing) == 0,
        "each LED plays the colors like GRADIENT, without flicker");
  bool behind = true, behindTwice = true, same = true, behindHalf = true;
  for (uint32_t t = 0; t < 3000; t += 7) {
    for (uint8_t led = 1; led < 4; led++) {
      behind = behind && drawGroup(wave, true, t + 250 * led, led, 0, 0, true) == drawGroup(wave, true, t, 0, 0, 0, true);
      behindTwice = behindTwice && drawGroup(two, true, t + 500 * led, led, 0, 0, true) == drawGroup(two, true, t, 0, 0, 0, true);
      behindHalf = behindHalf && drawGroup(half, true, t + 125 * led, led, 0, 0, true) == drawGroup(half, true, t, 0, 0, 0, true);
      same = same && drawGroup(together, true, t, led, 0, 0, true) == drawGroup(together, true, t, 0, 0, 0, true);
    }
  }
  check(behind, "amount 100: each LED plays what the one before played 250 ms earlier");
  check(behindTwice, "amount 200: 500 ms earlier, two waves along the group");
  check(behindHalf, "the group's own amount, 50: 125 ms earlier");
  check(same, "amount 0: all the LEDs together");
  const Look heavy = { GRADIENT, RED_BLUE, 100, FIXED_COLORS };
  const Group still(1, 4, heavy, OFF, 1000, REPEAT);
  bool stillTogether = true;
  for (uint32_t t = 0; t < 3000; t += 7)
    for (uint8_t led = 1; led < 4; led++)
      stillTogether = stillTogether && drawGroup(still, true, t, led, 0, 0, true) == drawGroup(still, true, t, 0, 0, 0, true);
  check(stillTogether, "other effects, even with amount 100: the group's LEDs stay together");
  CRGB mid = drawGroup(wave, true, 1500, 0, 0, 0, true);
  check(!(mid == drawGroup(wave, true, 1500, 2, 0, 0, true)), "so the LEDs show different colors at once");

  check(drawGroup(wave, true, 100, 0, 0, 0, true) == lookColor(flowing, K, 0.1f, 0, 1) &&
        drawGroup(wave, true, 100, 1, 0, 0, true) == K && drawGroup(wave, true, 499, 2, 0, 0, true) == K &&
        !(drawGroup(wave, true, 600, 2, 0, 0, true) == K),
        "turning on, it spreads from the 1st LED: the others wait on the off look");
  const Group shot(1, 4, flowing, OFF, 1000, ONCE_OFF), resting(1, 4, flowing, OFF, 1000, pauseThenRepeat(1000));
  check(drawGroup(shot, true, 1100, 0, 0, 0, true) == K && !(drawGroup(shot, true, 1100, 3, 0, 0, true) == K) &&
        drawGroup(shot, true, 1800, 3, 0, 0, true) == K, "ONCE_OFF: one pulse runs along the group, then it's dark");
  bool restsBehind = true;
  for (uint32_t t = 0; t < 6000; t += 7)
    restsBehind = restsBehind && drawGroup(resting, true, t + 750, 3, 0, 0, true) == drawGroup(resting, true, t, 0, 0, 0, true);
  check(restsBehind, "with a pause, each LED rests in turn, 250 ms after the one before");

  const Look widest = { FLOW, RED_BLUE, 255, FIXED_COLORS };
  const Group longest(1, 255, widest, OFF, 65535, REPEAT);
  const uint32_t lag = (uint32_t)((uint64_t)65535 * 255 * 254 / (100 * 255));   // 166,458 ms
  check(drawGroup(longest, true, lag - 1, 254, 0, 0, true) == K && !(drawGroup(longest, true, lag + 1000, 254, 0, 0, true) == K),
        "the longest lag (65535 ms, 255%, 255 LEDs) doesn't overflow");
}

static void checkClouds() {
  printf("LIGHTNING over clouds (flash red, sky green then blue, strikes 100%%):\n");
  const ColorStop STORM_LIST[] = { { SNAP, R }, { STEADY_FADE, G }, { STEADY_FADE, B } };
  const Look storm = { LIGHTNING, STORM_LIST, 100, FIXED_COLORS };
  const Look still = { LIGHTNING, STORM_LIST, 0, FIXED_COLORS };
  check(lookColor(storm, K, 0.0f, 0, 1) == K && lookColor(storm, K, 0.5f, 0, 1) == G && lookColor(storm, K, 1.0f, 0, 1) == B,
        "the clouds: from the old color into green, then blue");
  check(firstColor(storm, 0, 1) == R && endColor(storm, 0, 1) == B, "the flash is its first color, and it ends on its last");
  check(arrival(storm, 0.25f) == 0.5f && arrival(storm, 0.5f) == 1.0f, "its clouds take over during their first color");
  const Group sky(1, 1, storm, OFF, 1000, REPEAT);
  check(drawGroup(sky, true, 1000, 0, 0, 0, true) == B && drawGroup(sky, true, 1500, 0, 0, 0, true) == G,
        "with REPEAT, the clouds go round: blue, green, blue");

  const Group drifting(1, 4, storm, OFF, 1000, REPEAT), calm(1, 4, still, OFF, 1000, REPEAT);
  bool drifts = true;
  for (uint32_t t = 0; t < 3000; t += 7)
    for (uint8_t led = 1; led < 4; led++)
      drifts = drifts && drawGroup(drifting, true, t + 250 * led, led, 0, 0, true) == drawGroup(drifting, true, t, 0, 0, 0, true) &&
               drawGroup(calm, true, t + 250 * led, led, 0, 0, true) == drawGroup(calm, true, t, 0, 0, 0, true);
  check(drifts, "the clouds drift one wave along the group, whatever the amount");

  bool fullFlash = false, shapeRight = true, quiet = true, frozen = true, together = true, apart = false;
  int flashing = 0;
  const Group lit(ALWAYS, 4, storm, OFF, 0, REPEAT);
  for (uint32_t ms = 0; ms < 60000; ms += 3) {
    fakeMillis = ms;
    CRGB c = drawLook(storm, storm, 1.0f, 0, 1, 0, 7, false);
    float progress = 0, strength = 0;
    if (flashingNow(storm.amount, 7, progress, strength)) {
      flashing++;
      CRGB full = (progress < 0.5f) ? R : fadeBetween(R, B, fadeAmount(EARLY_FADE, progress * 2.0f - 1.0f));
      shapeRight = shapeRight && c == ((strength >= 1.0f) ? full : fadeBetween(B, full, strength));
      fullFlash = fullFlash || c == R;
    } else {
      shapeRight = shapeRight && c == B;
    }
    quiet = quiet && drawLook(still, still, 1.0f, 0, 1, 0, 7, false) == B;
    frozen = frozen && drawLook(storm, storm, 1.0f, 0, 1, 0, 7, true) == B;
    together = together && drawGroup(lit, true, 0, 0, 0, 7, false) == drawGroup(lit, true, 0, 3, 3, 7, false);
    apart = apart || !(drawGroup(lit, true, 0, 0, 0, 7, false) == drawGroup(lit, true, 0, 0, 0, 300, false));
  }
  check(flashing > 1000 && fullFlash, "it strikes: the 1st flash on the full flash color");
  check(shapeRight, "each flash: into the flash color, then back to the clouds; later ones weaker");
  check(quiet && frozen, "no strikes at amount 0, or steady");
  check(together && apart, "a group's LEDs strike together; different groups at different times");
  fakeMillis = 0;
}

static void checkStrobe() {
  printf("The STROBE look (200 ms a cycle):\n");
  const Group strobe(1, 1, STROBE, OFF, 200, REPEAT);
  bool flashes = true, dark = true;
  int flashCount = 0;
  bool wasOn = false;
  for (uint32_t ms = 0; ms < 1000; ms++) {
    CRGB c = drawGroup(strobe, true, ms, 0, 0, 0, true);
    bool on = !(c == K);
    uint32_t inCycle = ms % 200;
    if (inCycle < 50) flashes = flashes && c == ICE_WHITE;
    else dark = dark && c == K;
    if (on && !wasOn) flashCount++;
    wasOn = on;
  }
  check(flashes && dark, "cold white for the first quarter of the cycle, dark for the rest");
  check(flashCount == 5, "5 flashes a second");
}

// Runs the sketch's loop() for 'ms' milliseconds. Each pass takes 1 ms,
// and a pass that draws a frame takes 'drawMs' more. Returns how many
// frames were sent.
static unsigned long framesIn(uint32_t ms, uint32_t drawMs) {
  unsigned long before = FastLED.shows;
  uint32_t end = fakeMillis + ms;
  while (fakeMillis < end) {
    unsigned long shown = FastLED.shows;
    loop();
    fakeMillis += 1 + ((FastLED.shows > shown) ? drawMs : 0);
  }
  return FastLED.shows - before;
}

static void checkFrames() {
  printf("Frames (FRAME_MS = %lu):\n", (unsigned long)FRAME_MS);
  fakeMillis = 100000;
  auto about = [](unsigned long frames, unsigned long expected) { return frames + 1 >= expected && frames <= expected + 1; };
  framesIn(1000, 0);                                       // Settle into the rhythm
  check(about(framesIn(3000, 0), 3000 / FRAME_MS), "a frame every FRAME_MS");
  check(about(framesIn(3000, 10), 3000 / FRAME_MS), "the same when each frame takes 10 ms to draw");
  unsigned long slow = framesIn(3000, 50);                 // 1 + 50 ms a frame
  check(about(slow, 3000 / 51), "drawing slower than FRAME_MS: frames back to back, no catching up");
  check(about(framesIn(3000, 0), 3000 / FRAME_MS), "and back to a frame every FRAME_MS after");
  fakeMillis = 0;
}

static void checkPause() {
  printf("A pause between cycles (BUILDUP of 1000 ms, then 500 ms of pause):\n");
  const Look charge = { BUILDUP, RED_BLUE, 60, FIXED_COLORS };
  const Group plain(1, 1, charge, OFF, 1000, REPEAT), paused(1, 1, charge, OFF, 1000, pauseThenRepeat(500));
  check(plain.pauseMs == 0 && Group(1, 1, charge, OFF, 1000, ONCE_HOLD).pauseMs == 0, "REPEAT and ONCE_HOLD: no pause");
  bool firstSame = true, secondLater = true, thirdLater = true;
  for (uint32_t ms = 0; ms < 1000; ms += 7) {
    CRGB c = drawGroup(plain, true, ms, 0, 0, 0, true);
    firstSame = firstSame && drawGroup(paused, true, ms, 0, 0, 0, true) == c;
    CRGB again = drawGroup(plain, true, 1000 + ms, 0, 0, 0, true);
    secondLater = secondLater && drawGroup(paused, true, 1500 + ms, 0, 0, 0, true) == again;
    thirdLater = thirdLater && drawGroup(paused, true, 3000 + ms, 0, 0, 0, true) == again;
  }
  check(firstSame, "the 1st cycle is unchanged");
  check(drawGroup(paused, true, 1000, 0, 0, 0, true) == R && drawGroup(paused, true, 1250, 0, 0, 0, true) == R &&
        drawGroup(paused, true, 1499, 0, 0, 0, true) == R, "the pause rests on the look's end color");
  check(secondLater, "the 2nd cycle plays as without a pause, 500 ms later");
  check(thirdLater, "and the 3rd, 1000 ms later");
  check(drawGroup(paused, true, 2999, 0, 0, 0, true) == R && !(drawGroup(paused, true, 2400, 0, 0, 0, true) == R),
        "each cycle is followed by a pause");

  const Look gradient = { GRADIENT, RED_BLUE, 0, FIXED_COLORS };
  const Group fade(1, 1, gradient, OFF, 1000, pauseThenRepeat(2000));
  check(drawGroup(fade, true, 1000, 0, 0, 0, true) == B && drawGroup(fade, true, 2999, 0, 0, 0, true) == B &&
        drawGroup(fade, true, 3000, 0, 0, 0, true) == B &&
        near(drawGroup(fade, true, 3500, 0, 0, 0, true), lookColor(gradient, B, 0.5f, 0, 1), 1),
        "a gradient rests on its last color, then plays from it again");

  const Group smooth(1, 1, charge, OFF, 1000, pauseThenRepeat(500), 0);
  const Look noThrobs = { BUILDUP, RED_BLUE, 0, FIXED_COLORS };
  check(smooth.amount == 0 && smooth.pauseMs == 500 &&
        near(drawGroup(smooth, true, 1500 + 300, 0, 0, 0, true), lookColor(noThrobs, R, 0.3f, 0, 1), 1),
        "with its own amount too");
  const Group own = SCENES[1]->groups[0].get();
  check(own.pauseMs == 0, "the Sanctuary's groups read back with no pause");
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
  checkLightning();
  checkBuildup();
  checkExplosion();
  checkFlow();
  checkClouds();
  checkStrobe();
  checkFrames();
  checkPause();
  checkGroupAmount();
  checkScenes();

  printf(failures ? "%d check(s) FAILED\n" : "All checks passed\n", failures);
  return failures ? 1 : 0;
}
