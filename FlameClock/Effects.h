// =====================================================================
//  LOOKS: HOW THEY ARE DRAWN
// =====================================================================
//
//  Turns a look into LED colors, for the clock and the scenes.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include "LookTypes.h"

// How fast flames flicker, in %. Set in PART 3 of FlameClock.ino.
extern const uint16_t FLICKER_SPEED_PERCENT;

// How far a fade has got (0.0 to 1.0) when 'progress' (0.0 to 1.0) of
// its time has passed
inline float fadeAmount(Fade fade, float progress) {
  switch (fade) {
    case SNAP:        return 1.0f;
    case HOLD:        return 0.0f;
    case SMOOTH_FADE: return progress * progress * (3.0f - 2.0f * progress);
    case EARLY_FADE:  return 1.0f - (1.0f - progress) * (1.0f - progress);
    case LATE_FADE:   return progress * progress;
    default:          return progress;   // STEADY_FADE
  }
}

// Mixes color 'from' into color 'to'. level 0 = all 'from', level 1 = all 'to'.
// Red, green and blue each move in a straight line from start to end.
inline CRGB fadeBetween(CRGB from, CRGB to, float level) {
  level = constrain(level, 0.0f, 1.0f);
  CRGB out;
  out.r = from.r + (int)((to.r - from.r) * level);
  out.g = from.g + (int)((to.g - from.g) * level);
  out.b = from.b + (int)((to.b - from.b) * level);
  return out;
}

// A look's color on one LED of a group of 'ledCount' LEDs. A rainbow color
// holds its brightness in red and its paleness in green (see rainbowColor),
// and takes its hue from the LED's place in the group.
inline CRGB paint(const Look &look, CRGB color, uint8_t led, uint8_t ledCount) {
  if (look.colors == FIXED_COLORS) return color;
  uint8_t hue = led * 256 / ledCount;
  return CRGB(CHSV(hue, 255 - color.g, color.r));
}

// The first and last colors of a look's color list, on one LED
inline CRGB firstColor(const Look &look, uint8_t led, uint8_t ledCount) {
  return paint(look, look.stops[0].color, led, ledCount);
}
inline CRGB endColor(const Look &look, uint8_t led, uint8_t ledCount) {
  return paint(look, look.stops[look.stopCount - 1].color, led, ledCount);
}

// Where a look is at 'phase' (0.0 to 1.0 through its cycle): which color
// of the list it is fading into, and how far it has got (0.0 to 1.0)
inline void findStop(const Look &look, float phase, uint8_t &stop, float &level) {
  float scaled = phase * look.stopCount;
  int part = (int)scaled;
  if (part >= look.stopCount) {           // The very end of the cycle
    stop = look.stopCount - 1;
    level = 1.0f;
    return;
  }
  stop = part;
  level = fadeAmount(look.stops[part].fade, scaled - part);
}

// ---- HEARTBEAT timing, in % of the cycle ----
const uint8_t BEAT_LENGTH_PERCENT = 15;         // How long each beat takes
const uint8_t SECOND_BEAT_START_PERCENT = 25;   // When the 2nd beat (dub) starts

// ---- PULSES timing, in % of the cycle ----
const uint8_t PULSE_LENGTH_PERCENT = 8;         // How long each pulse takes
const uint8_t PULSE_GAP_PERCENT = 4;            // The rest between two pulses of a burst
const uint8_t MOST_PULSES = 8;                  // The most pulses a burst can have

static_assert(SECOND_BEAT_START_PERCENT >= BEAT_LENGTH_PERCENT &&
              SECOND_BEAT_START_PERCENT + BEAT_LENGTH_PERCENT <= 100,
              "HEARTBEAT: the 2nd beat must start after the 1st one ends, and end by 100%");
static_assert(MOST_PULSES * PULSE_LENGTH_PERCENT + (MOST_PULSES - 1) * PULSE_GAP_PERCENT <= 100,
              "PULSES: MOST_PULSES pulses and their gaps must fit in 100% of the cycle");

// The same, as parts of the cycle (0.0 to 1.0), for the drawing below
const float BEAT_LENGTH = BEAT_LENGTH_PERCENT / 100.0f;
const float SECOND_BEAT_START = SECOND_BEAT_START_PERCENT / 100.0f;
const float PULSE_LENGTH = PULSE_LENGTH_PERCENT / 100.0f;
const float PULSE_GAP = PULSE_GAP_PERCENT / 100.0f;

// How many pulses a PULSES look has: its amount, from 1 to MOST_PULSES
inline uint8_t pulseCount(const Look &look) {
  return (look.amount < 1) ? 1 : (look.amount > MOST_PULSES) ? MOST_PULSES : look.amount;
}

// How far a look has taken over from the LED's old color at 'phase':
// 0.0 = still the old color, 1.0 = past its first color
inline float arrival(const Look &look, float phase) {
  float first = (look.effect == HEARTBEAT) ? BEAT_LENGTH     // It takes over during
              : (look.effect == PULSES)    ? PULSE_LENGTH    // its 1st beat or pulse
              : 1.0f;
  phase = (phase < first) ? phase / first : 1.0f;
  uint8_t stop;
  float level;
  findStop(look, phase, stop, level);
  return (stop == 0) ? level : 1.0f;
}

// The color list played once, from 'from', at 'phase'
inline CRGB listColor(const Look &look, CRGB from, float phase, uint8_t led, uint8_t ledCount) {
  uint8_t stop;
  float level;
  findStop(look, phase, stop, level);
  CRGB start = (stop == 0) ? from : paint(look, look.stops[stop - 1].color, led, ledCount);
  return fadeBetween(start, paint(look, look.stops[stop].color, led, ledCount), level);
}

// A heartbeat's color at 'phase': the color list played quickly twice,
// the 2nd time weaker, then its last color until the cycle ends
inline CRGB heartbeatColor(const Look &look, CRGB from, float phase, uint8_t led, uint8_t ledCount) {
  CRGB rest = endColor(look, led, ledCount);
  if (phase < BEAT_LENGTH) {                                    // Lub
    return listColor(look, from, phase / BEAT_LENGTH, led, ledCount);
  }
  float inSecond = phase - SECOND_BEAT_START;
  if (inSecond >= 0.0f && inSecond < BEAT_LENGTH) {             // Dub
    uint8_t strength = (look.amount > 100) ? 100 : look.amount;
    CRGB full = listColor(look, rest, inSecond / BEAT_LENGTH, led, ledCount);
    return fadeBetween(rest, full, strength / 100.0f);
  }
  return rest;                                                  // Resting
}

// A burst of pulses' color at 'phase': the color list played quickly,
// once per pulse, then its last color until the cycle ends
inline CRGB pulsesColor(const Look &look, CRGB from, float phase, uint8_t led, uint8_t ledCount) {
  CRGB rest = endColor(look, led, ledCount);
  const float slot = PULSE_LENGTH + PULSE_GAP;                  // One pulse and its gap
  uint8_t pulse = (uint8_t)(phase / slot);                      // Which pulse, from 0
  float inPulse = phase - pulse * slot;
  if (pulse < pulseCount(look) && inPulse < PULSE_LENGTH) {     // Pulsing
    return listColor(look, (pulse == 0) ? from : rest, inPulse / PULSE_LENGTH, led, ledCount);
  }
  return rest;                                                  // Between pulses, or resting
}

// A look's color at 'phase', before flicker. 'from' is the LED's color
// before the look started: the look fades from it into its first color.
inline CRGB lookColor(const Look &look, CRGB from, float phase, uint8_t led, uint8_t ledCount) {
  if (look.effect == HEARTBEAT) return heartbeatColor(look, from, phase, led, ledCount);
  if (look.effect == PULSES) return pulsesColor(look, from, phase, led, ledCount);
  return listColor(look, from, phase, led, ledCount);
}

// How much a look flickers, in %. Only flames flicker.
inline uint8_t lookFlicker(const Look &look) {
  return (look.effect == FLAME) ? look.amount : 0;
}

// Where the flicker is in its noise pattern: at 100%, one step every
// 3 ms. Worked out in two parts so it never overflows; the result wraps
// around every 65536 steps, as a uint16_t does.
inline uint16_t flickerTime() {
  uint32_t ms = millis();
  return (ms / 300) * FLICKER_SPEED_PERCENT + (ms % 300) * FLICKER_SPEED_PERCENT / 300;
}

// ---- FAILING timing, in real time ----
// Time is cut into windows of FAIL_WINDOW_STEPS steps. Each window, each
// group of LEDs (a scene's group, or a ring) has a chance (the look's
// amount, in %) of one fault: a stutter of 1 to FAIL_LONGEST_STEPS
// steps, out on the first step and on about half of the rest.
const uint8_t FAIL_STEP_MS = 25;          // The shortest dropout
const uint8_t FAIL_WINDOW_STEPS = 32;     // 32 steps = 0.8 s: at 100%, a fault every 0.8 s
const uint8_t FAIL_LONGEST_STEPS = 8;     // 8 steps = 0.2 s: the longest stutter
static_assert(FAIL_LONGEST_STEPS >= 1 && FAIL_LONGEST_STEPS <= 8 && FAIL_LONGEST_STEPS < FAIL_WINDOW_STEPS,
              "FAILING: a stutter is 1 to 8 steps and must fit in its window");

// Mixes the bits of a number, to make the same "random" number from the
// same input every time
inline uint32_t mixBits(uint32_t x) {
  x ^= x >> 16;
  x *= 0x45D9F3BUL;
  x ^= x >> 16;
  x *= 0x45D9F3BUL;
  x ^= x >> 16;
  return x;
}

// Is a failing look out right now? 'spot' picks the group of LEDs that
// fail together, and 'amount' is how often it fails, in %.
inline bool failedNow(uint16_t spot, uint8_t amount) {
  if (amount == 0) return false;
  uint32_t step = millis() / FAIL_STEP_MS;
  uint32_t window = step / FAIL_WINDOW_STEPS;
  uint32_t dice = mixBits(window * 2654435761UL + spot * 40503UL);

  if ((dice & 0xFF) * 100 / 256 >= amount) return false;                   // No fault this window
  uint8_t start = (dice >> 8) % (FAIL_WINDOW_STEPS - FAIL_LONGEST_STEPS + 1);
  uint8_t length = 1 + (dice >> 16) % FAIL_LONGEST_STEPS;
  uint8_t into = step % FAIL_WINDOW_STEPS;
  if (into < start || into >= start + length) return false;                // Before or after the fault
  return into == start || ((dice >> 24) >> (into - start)) & 1;            // Out, or a stutter
}

// How dark a look's dropouts make the LED right now, in %. Only failing
// looks have them.
inline uint8_t lookDropout(const Look &look, uint16_t spot) {
  return (look.effect == FAILING && failedNow(spot, look.amount)) ? 100 : 0;
}

// Brightness factor (255 = full) for one LED. Each LED reads its own
// 'spot' in a smooth noise pattern, so LEDs flicker independently.
// flicker: in %, 0 = steady, 100 = deepest dips.
inline uint8_t flickerScale(uint16_t spot, uint8_t flicker) {
  if (flicker == 0) return 255;
  if (flicker > 100) flicker = 100;
  uint16_t t = flickerTime();
  uint8_t n = inoise8(spot, t);
  n = constrain(n, 40, 215);
  uint8_t lowest = 255 - (uint16_t)flicker * 255 / 100;
  return (uint8_t)map(n, 40, 215, lowest, 255);
}

// Draws one LED: 'look' at 'phase', taking over from 'before'. The LED
// fades from the last color of 'before' into the look's colors, and the
// flicker and the dropouts change over from one look's to the other's as
// it does. To show a look on its own, give it as 'before' too.
// 'spot' picks the LED's place in the flicker pattern: give each LED a
// different one. 'groupSpot' picks when a failing look goes out: LEDs
// with the same one (a scene's group, or a ring) fail together.
// steady = true turns the flicker and the dropouts off.
inline CRGB drawLook(const Look &look, const Look &before, float phase, uint8_t led,
                     uint8_t ledCount, uint16_t spot, uint16_t groupSpot, bool steady) {
  CRGB color = lookColor(look, endColor(before, led, ledCount), phase, led, ledCount);
  if (!steady) {
    float arrived = arrival(look, phase);
    uint8_t flicker = (uint8_t)(lookFlicker(before) + (lookFlicker(look) - lookFlicker(before)) * arrived);
    color.nscale8(flickerScale(spot, flicker));
    uint8_t darkBefore = lookDropout(before, groupSpot);
    uint8_t dark = (uint8_t)(darkBefore + (lookDropout(look, groupSpot) - darkBefore) * arrived);
    if (dark > 0) color.nscale8(255 - (uint16_t)dark * 255 / 100);
  }
  return color;
}
