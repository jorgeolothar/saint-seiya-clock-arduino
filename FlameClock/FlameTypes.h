// =====================================================================
//  FLAME CLOCK: THE BUILDING BLOCKS
// =====================================================================
//
//  This file describes what a "flame" and a "clock" are, so that
//  FlameClock.ino can list your flames and clocks right at the top.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a second tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include <FastLED.h>

// ---- What a ring shows ----
enum Unit {
  SHOW_HOURS,     // 1 LED per hour.      1st flame: midnight to noon.         2nd flame: noon to midnight.
  SHOW_MINUTES,   // 1 LED per 5 minutes. 1st flame: even hours (12, 2, 4...). 2nd flame: odd hours.
  SHOW_SECONDS    // 1 LED per 5 seconds. 1st flame: even minutes.             2nd flame: odd minutes.
};

// ---- How a color change is spread over its half of the step ----
enum Fade {
  NO_FADE,        // No fade. Fading in: the flame appears at once.
                  //          Fading out: the flame stays until the step ends.
  STEADY_FADE,    // Changes at an even pace
  SMOOTH_FADE,    // Gentle at the start and at the end
  EARLY_FADE,     // Most of the change happens early
  LATE_FADE       // Most of the change happens late
};

// ---- Where a flame's colors come from ----
enum ColorStyle {
  FIXED_COLORS,   // Exactly the colors you wrote
  WHEEL_COLORS    // A rainbow around the ring: each LED takes its hue from its
                  // place on the ring. Use colors made with wheelColor().
};

// ---- A color for WHEEL_COLORS flames ----
// Brightness and paleness in % (paleness: 0 = full color, 100 = white).
// The hue comes from the LED's place on the ring. It is stored in an
// ordinary CRGB (red = brightness, green = paleness) that only wheel
// flames read this way, so it shows a meaningless color in a
// FIXED_COLORS flame.
inline CRGB wheelColor(uint8_t brightness, uint8_t paleness) {
  if (brightness > 100) brightness = 100;
  if (paleness > 100) paleness = 100;
  return CRGB(brightness * 255 / 100, paleness * 255 / 100, 0);
}

// ---- A flame: the look of one lap around the ring ----
// The values are in the order things happen on an LED, so a flame
// reads like a sentence (see Flames.h).
struct Flame {
  Fade fadeIn;          // From the LED's old color to the flame color (1st half of the step)
  CRGB flameColor;      // The flame at its brightest, halfway through its step
  Fade fadeOut;         // From the flame color to the trail color (2nd half of the step)
  CRGB trailColor;      // What the flame leaves behind. BLACK = no trail
  uint8_t flicker;      // Flicker in %: 0 = steady, 100 = strongest
  ColorStyle colors;    // FIXED_COLORS or WHEEL_COLORS
};

// ---- A clock: one LED ring ----
struct Clock {
  Unit unit;                // SHOW_HOURS, SHOW_MINUTES or SHOW_SECONDS
  const Flame *flame[2];    // 1st and 2nd flame
  uint8_t topLed;           // Which LED sits at 12 o'clock (1 to 12)

  Clock(Unit shows, const Flame &first, const Flame &second, uint8_t top)
    : unit(shows), flame{&first, &second}, topLed(top) {}
};

// ---- What the whole clock does (see PART 2 of FlameClock.ino) ----
enum Behavior {
  MODE_REGULAR,       // Shows the real time
  MODE_ACCELERATED,   // A fast pretend day
  MODE_COLOR_TEST,    // Holds still to check colors and the top LED
  MODE_FREEZE         // Holds still at one moment
};

// ---- Time of day ----
// The program counts time in milliseconds since midnight.
const uint32_t DAY_MS = 24UL * 60 * 60 * 1000;

// Turns a time into milliseconds since midnight, for example
// clockTime(15, 45, 0) for 3:45 in the afternoon. Seconds can have decimals.
constexpr uint32_t clockTime(int hour, int minute, float second) {
  return (hour * 3600UL + minute * 60UL) * 1000UL + (uint32_t)(second * 1000.0f);
}
