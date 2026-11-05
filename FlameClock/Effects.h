// =====================================================================
//  LOOKS: HOW THEY ARE DRAWN
// =====================================================================
//
//  Turns a look into LED colors. The clock uses these, and so will the
//  temple scenes.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include "LookTypes.h"

// Flicker speed: lower = faster. Set in PART 3 of FlameClock.ino.
extern const uint8_t FLICKER_TIME_DIV;

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

// A look's color on one LED of a group of 'ledCount' LEDs. A wheel color
// holds its brightness in red and its paleness in green (see wheelColor),
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

// ---- HEARTBEAT timing, as parts of the cycle (0.0 to 1.0) ----
const float BEAT_LENGTH = 0.15f;          // How long each beat takes
const float SECOND_BEAT_START = 0.25f;    // When the 2nd beat (dub) starts

// How far a look has taken over from the LED's old color at 'phase':
// 0.0 = still the old color, 1.0 = past its first color
inline float arrival(const Look &look, float phase) {
  if (look.effect == HEARTBEAT) {         // It takes over during the 1st beat
    phase = (phase < BEAT_LENGTH) ? phase / BEAT_LENGTH : 1.0f;
  }
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

// A look's color at 'phase', before flicker. 'from' is the LED's color
// before the look started: the look fades from it into its first color.
inline CRGB lookColor(const Look &look, CRGB from, float phase, uint8_t led, uint8_t ledCount) {
  if (look.effect == HEARTBEAT) return heartbeatColor(look, from, phase, led, ledCount);
  return listColor(look, from, phase, led, ledCount);
}

// How much a look flickers, in %. Only flames flicker.
inline uint8_t lookFlicker(const Look &look) {
  return (look.effect == FLAME) ? look.amount : 0;
}

// Brightness factor (255 = full) for one LED. Each LED reads its own
// 'spot' in a smooth noise pattern, so LEDs flicker independently.
// flicker: in %, 0 = steady, 100 = deepest dips.
inline uint8_t flickerScale(uint16_t spot, uint8_t flicker) {
  if (flicker == 0) return 255;
  if (flicker > 100) flicker = 100;
  uint16_t t = millis() / FLICKER_TIME_DIV;
  uint8_t n = inoise8(spot, t);
  n = constrain(n, 40, 215);
  uint8_t lowest = 255 - (uint16_t)flicker * 255 / 100;
  return (uint8_t)map(n, 40, 215, lowest, 255);
}

// Draws one LED: 'look' at 'phase', taking over from 'before'. The LED
// fades from the last color of 'before' into the look's colors, and the
// flicker changes over from one look's to the other's as it does.
// To show a look on its own, give it as 'before' too.
// 'spot' picks the LED's place in the flicker pattern: give each LED a
// different one. steady = true turns the flicker off.
inline CRGB drawLook(const Look &look, const Look &before, float phase,
                     uint8_t led, uint8_t ledCount, uint16_t spot, bool steady) {
  CRGB color = lookColor(look, endColor(before, led, ledCount), phase, led, ledCount);
  if (!steady) {
    float arrived = arrival(look, phase);
    uint8_t flicker = (uint8_t)(lookFlicker(before) + (lookFlicker(look) - lookFlicker(before)) * arrived);
    color.nscale8(flickerScale(spot, flicker));
  }
  return color;
}
