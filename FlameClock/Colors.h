// =====================================================================
//  COLORS
// =====================================================================
//
//  Named colors, shared by every look: the clock's flames in Flames.h,
//  and the scenes' looks in SceneLooks.h. Changing a color here changes
//  every look that uses it.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called Colors.h.
//
// =====================================================================

#pragma once
#include "LookTypes.h"   // The building blocks. Do not change this line.

// Each color is CRGB(red, green, blue), with numbers from 0 to 255.
// Tip: search the web for "RGB color picker" to find the numbers you
// want. LEDs often look a bit different from a screen, so use
// MODE_COLOR_TEST (PART 2 of FlameClock.ino) to see your colors on the
// real LEDs.

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
const CRGB CINDER      = CRGB(40, 6, 0);       // a dim, glowing red
const CRGB DARK_RED    = CRGB(45, 0, 2);       // a deep, dim red
const CRGB GOLD        = CRGB(255, 170, 20);
const CRGB SEA_BLUE    = CRGB(0, 40, 160);
const CRGB VIOLET      = CRGB(150, 40, 255);
const CRGB DEEP_PURPLE = CRGB(40, 0, 70);
const CRGB BLACK       = CRGB(0, 0, 0);        // off. As a last color: no trail

// Colors for RAINBOW_COLORS looks, written as:
//
//   rainbowColor(brightness %, paleness %)
//
//   brightness .. 0 = off, 100 = full brightness
//   paleness .... 0 = full color, 100 = white
//
// There is no hue to pick: each LED takes its hue from its place in the
// group (on the clock: its place on the ring). Use these only in
// RAINBOW_COLORS looks (BLACK works there too).
const CRGB RAINBOW_PALE = rainbowColor(100, 40);   // bright and pale
const CRGB RAINBOW_DEEP = rainbowColor(80, 0);     // full color, a bit dim
const CRGB RAINBOW_FULL = rainbowColor(100, 0);    // full color, full brightness
