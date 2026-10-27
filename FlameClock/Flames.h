// =====================================================================
//  FLAME CLOCK: THE COLORS AND FLAMES
// =====================================================================
//
//  A flame is a "look" for one lap around the ring. The clocks in
//  FlameClock.ino pick their flames by name from this file.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called Flames.h.
//
//  This file has three parts:
//    1. COLORS ........ named colors, used by the flames
//    2. FLAMES ........ the flames themselves
//    3. FLAME_LIST .... the flames the button steps through
//
// =====================================================================

#pragma once
#include "FlameTypes.h"   // The building blocks. Do not change this line.

// =====================================================================
//  1. COLORS
//  Each color is CRGB(red, green, blue), with numbers from 0 to 255.
//  Tip: search the web for "RGB color picker" to find the numbers you
//  want. LEDs often look a bit different from a screen, so use
//  MODE_COLOR_TEST (PART 2 of FlameClock.ino) to see your colors on the
//  real LEDs. Changing a color here changes every flame that uses it.
// =====================================================================

const CRGB ORANGE      = CRGB(255, 110, 0);
const CRGB DEEP_ORANGE = CRGB(255, 90, 0);
const CRGB RED         = CRGB(255, 30, 0);     // a warm red
const CRGB AMBER       = CRGB(255, 140, 40);
const CRGB CANDLE_GLOW = CRGB(50, 15, 0);      // a very faint amber
const CRGB SPARK_WHITE = CRGB(255, 230, 150);  // a hot, slightly yellow white
const CRGB ICE_WHITE   = CRGB(200, 245, 255);
const CRGB ICE_BLUE    = CRGB(0, 191, 255);
const CRGB NEON_PINK   = CRGB(255, 0, 160);
const CRGB DIM_TEAL    = CRGB(0, 60, 80);
const CRGB BLACK       = CRGB(0, 0, 0);        // off. As a trail color: no trail

// Colors for WHEEL_COLORS flames, written as:
//
//   wheelColor(brightness %, paleness %)
//
//   brightness .. 0 = off, 100 = full brightness
//   paleness .... 0 = full color, 100 = white
//
// There is no hue to pick: each LED takes its hue from its place on the
// ring. Use these only in WHEEL_COLORS flames (BLACK works there too).
const CRGB WHEEL_PALE  = wheelColor(100, 40);  // bright and pale
const CRGB WHEEL_DEEP  = wheelColor(80, 0);    // full color, a bit dim
const CRGB WHEEL_FULL  = wheelColor(100, 0);   // full color, full brightness

// =====================================================================
//  2. FLAMES
//  A flame is written in the order things happen on each LED, so it
//  reads like a sentence:
//
//    const Flame FIRE = { SMOOTH_FADE, ORANGE, SMOOTH_FADE, RED, 70, FIXED_COLORS };
//
//    "FIRE fades smoothly into ORANGE, then fades smoothly into a RED
//     trail, with 70% flicker, using the colors as written."
//
//  The six values, in order:
//    1. fade in ...... how the flame appears over the LED's old color
//    2. flame color .. the flame at its brightest, halfway through its step
//    3. fade out ..... how the flame turns into its trail
//    4. trail color .. what the flame leaves behind (BLACK = no trail)
//    5. flicker % .... 0 = steady, 100 = strongest
//    6. colors ....... FIXED_COLORS = the colors as written
//                      WHEEL_COLORS = a rainbow around the ring (each LED
//                      takes its hue from its place on the ring). Use the
//                      wheelColor() colors from part 1 with it.
//
//  The fades:
//    NO_FADE ....... no fade: the flame appears at once (fade in), or
//                    stays until its step ends (fade out)
//    STEADY_FADE ... changes at an even pace
//    SMOOTH_FADE ... gentle at the start and at the end
//    EARLY_FADE .... most of the change happens early
//    LATE_FADE ..... most of the change happens late
//
//  To make your own flame, copy a line and give it a new name. Add the
//  name to FLAME_LIST below if the button should be able to pick it.
// =====================================================================

//                          fade in      flame color  fade out     trail color  flicker% colors

// Orange flame with a red trail
const Flame FIRE          = { SMOOTH_FADE, ORANGE,      SMOOTH_FADE, RED,         70,      FIXED_COLORS };

// Icy white flame with an ice blue trail
const Flame COOL          = { SMOOTH_FADE, ICE_WHITE,   SMOOTH_FADE, ICE_BLUE,    70,      FIXED_COLORS };

// A bright flash that dies out quickly and leaves no trail
const Flame SPARK         = { NO_FADE,     SPARK_WHITE, EARLY_FADE,  BLACK,       90,      FIXED_COLORS };

// Steady amber, no fading and no flicker
const Flame SOLID         = { NO_FADE,     AMBER,       NO_FADE,     AMBER,        0,      FIXED_COLORS };

// A rainbow around the ring: pale flame, deep colored trail
const Flame HUE_WHEEL     = { SMOOTH_FADE, WHEEL_PALE,  SMOOTH_FADE, WHEEL_DEEP,  30,      WHEEL_COLORS };

// An orange flame that burns out quickly to black. After COOL, it is the
// countdown of the Sanctuary fire clock.
const Flame EMBER         = { SMOOTH_FADE, DEEP_ORANGE, EARLY_FADE,  BLACK,       80,      FIXED_COLORS };

// An icy flame that melts away to black, with a gentle cold shimmer
const Flame FROST         = { SMOOTH_FADE, ICE_WHITE,   SMOOTH_FADE, BLACK,       30,      FIXED_COLORS };

// A warm candle that leaves a faint glow behind it
const Flame CANDLE        = { SMOOTH_FADE, AMBER,       SMOOTH_FADE, CANDLE_GLOW, 50,      FIXED_COLORS };

// Pink neon that hardly flickers, holds its color, then turns into a dim teal trail
const Flame NEON          = { STEADY_FADE, NEON_PINK,   LATE_FADE,   DIM_TEAL,     5,      FIXED_COLORS };

// A rainbow around the ring where each LED burns its own color down to black
const Flame RAINBOW_EMBER = { SMOOTH_FADE, WHEEL_FULL,  STEADY_FADE, BLACK,       60,      WHEEL_COLORS };

// =====================================================================
//  3. FLAME_LIST: THE FLAMES THE BUTTON STEPS THROUGH
//  Holding the button moves to the next flame in this list, in this
//  order. The number a ring blinks is the flame's place in this list.
//  Up to 12 flames. Write each name with a & in front.
// =====================================================================

const Flame *const FLAME_LIST[] = { &FIRE, &COOL, &SPARK, &SOLID, &HUE_WHEEL,
                                    &EMBER, &FROST, &CANDLE, &NEON, &RAINBOW_EMBER };
