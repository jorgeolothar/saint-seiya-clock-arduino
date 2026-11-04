// =====================================================================
//  FLAME CLOCK: THE CLOCK'S BUILDING BLOCKS
// =====================================================================
//
//  This file describes what a "clock" (one LED ring) is, so that
//  FlameClock.ino can list your clocks right at the top.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include "LookTypes.h"

// ---- What a ring shows ----
enum Unit {
  SHOW_HOURS,     // 1 LED per hour.      1st flame: midnight to noon.         2nd flame: noon to midnight.
  SHOW_MINUTES,   // 1 LED per 5 minutes. 1st flame: even hours (12, 2, 4...). 2nd flame: odd hours.
  SHOW_SECONDS    // 1 LED per 5 seconds. 1st flame: even minutes.             2nd flame: odd minutes.
};

// ---- A clock: one LED ring ----
struct Clock {
  Unit unit;                // SHOW_HOURS, SHOW_MINUTES or SHOW_SECONDS
  const Look *flame[2];     // 1st and 2nd flame
  uint8_t topLed;           // Which LED sits at 12 o'clock (1 to 12)

  Clock(Unit shows, const Look &first, const Look &second, uint8_t top)
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
