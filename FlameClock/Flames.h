// =====================================================================
//  FLAME CLOCK: THE FLAMES
// =====================================================================
//
//  A flame is the look of one lap around the ring. The clocks in
//  FlameClock.ino pick their flames by name from this file. The colors
//  they use are named in Colors.h.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called Flames.h.
//
//  This file has two parts:
//    1. FLAMES ........ the flames themselves
//    2. FLAME_LIST .... the flames the button steps through
//
// =====================================================================

#pragma once
#include "LookTypes.h"   // The building blocks. Do not change these two lines.
#include "Colors.h"

// =====================================================================
//  1. FLAMES
//  A flame is written in two lines. The first is its color list, in the
//  order the colors show on each LED, and how to fade into each one:
//
//    const ColorStop FIRE_COLORS[] = { { SMOOTH_FADE, ORANGE }, { SMOOTH_FADE, RED } };
//
//  The second puts the colors to use:
//
//    const Look FIRE = { FLAME, FIRE_COLORS, 70, FIXED_COLORS };
//
//    "FIRE fades smoothly into ORANGE, then fades smoothly into a RED
//     trail, with 70% flicker, using the colors as written."
//
//  The color list:
//    - The LED fades from its old color into the 1st color, then into
//      the 2nd, and so on. Each color gets an equal part of the step.
//    - The last color is the trail: what the flame leaves behind.
//      BLACK = no trail.
//    - It can be any length. With 2 colors, the 1st is the flame at its
//      brightest, halfway through the step. With 1, the LED just
//      changes into that color.
//
//  The four values of the second line, in order:
//    1. effect ....... FLAME = flickers, GRADIENT = steady. The other
//                      effects are made for scenes: switch on the DEMO
//                      in Scenes.h to see them all.
//    2. color list ... the name of the first line
//    3. flicker % .... 0 = steady, 100 = strongest. GRADIENT ignores it.
//    4. colors ....... FIXED_COLORS = the colors as written
//                      RAINBOW_COLORS = a rainbow around the ring (each LED
//                      takes its hue from its place on the ring). Use the
//                      rainbowColor() colors from Colors.h with it.
//
//  The fades:
//    SNAP .......... switches to the color at once, at the start of its part
//    HOLD .......... keeps the color before, and switches at the end of its part
//    STEADY_FADE ... changes at an even pace
//    SMOOTH_FADE ... gentle at the start and at the end
//    EARLY_FADE .... most of the change happens early
//    LATE_FADE ..... most of the change happens late
//
//  To make your own flame, copy two lines and give them a new name. Add
//  the name to FLAME_LIST below if the button should be able to pick it.
// =====================================================================

// Orange flame with a red trail
const ColorStop FIRE_COLORS[]          = { { SMOOTH_FADE, ORANGE },       { SMOOTH_FADE, RED } };
const Look FIRE                        = { FLAME, FIRE_COLORS, 70, FIXED_COLORS };

// Icy white flame with an ice blue trail
const ColorStop COOL_COLORS[]          = { { SMOOTH_FADE, ICE_WHITE },    { SMOOTH_FADE, ICE_BLUE } };
const Look COOL                        = { FLAME, COOL_COLORS, 70, FIXED_COLORS };

// A bright flash that dies out quickly and leaves no trail
const ColorStop SPARK_COLORS[]         = { { SNAP, SPARK_WHITE },         { EARLY_FADE, BLACK } };
const Look SPARK                       = { FLAME, SPARK_COLORS, 90, FIXED_COLORS };

// Steady amber, no fading and no flicker
const ColorStop SOLID_COLORS[]         = { { SNAP, AMBER } };
const Look SOLID                       = { GRADIENT, SOLID_COLORS, 0, FIXED_COLORS };

// A rainbow around the ring: pale flame, deep colored trail
const ColorStop RAINBOW_GLOW_COLORS[]  = { { SMOOTH_FADE, RAINBOW_PALE }, { SMOOTH_FADE, RAINBOW_DEEP } };
const Look RAINBOW_GLOW                = { FLAME, RAINBOW_GLOW_COLORS, 30, RAINBOW_COLORS };

// An orange flame that burns out quickly to black. After COOL, it is the
// countdown of the Sanctuary fire clock.
const ColorStop EMBER_COLORS[]         = { { SMOOTH_FADE, DEEP_ORANGE },  { EARLY_FADE, BLACK } };
const Look EMBER                       = { FLAME, EMBER_COLORS, 80, FIXED_COLORS };

// An icy flame that melts away to black, with a gentle cold shimmer
const ColorStop FROST_COLORS[]         = { { SMOOTH_FADE, ICE_WHITE },    { SMOOTH_FADE, BLACK } };
const Look FROST                       = { FLAME, FROST_COLORS, 30, FIXED_COLORS };

// A warm candle that leaves a faint glow behind it
const ColorStop CANDLE_COLORS[]        = { { SMOOTH_FADE, AMBER },        { SMOOTH_FADE, CANDLE_GLOW } };
const Look CANDLE                      = { FLAME, CANDLE_COLORS, 50, FIXED_COLORS };

// Pink neon that hardly flickers, holds its color, then turns into a dim teal trail
const ColorStop NEON_COLORS[]          = { { STEADY_FADE, NEON_PINK },    { LATE_FADE, DIM_TEAL } };
const Look NEON                        = { FLAME, NEON_COLORS, 5, FIXED_COLORS };

// A rainbow around the ring where each LED burns its own color down to black
const ColorStop RAINBOW_EMBER_COLORS[] = { { SMOOTH_FADE, RAINBOW_FULL }, { STEADY_FADE, BLACK } };
const Look RAINBOW_EMBER               = { FLAME, RAINBOW_EMBER_COLORS, 60, RAINBOW_COLORS };

// =====================================================================
//  2. FLAME_LIST: THE FLAMES THE BUTTON STEPS THROUGH
//  Holding the button moves to the next flame in this list, in this
//  order. The number a ring blinks is the flame's place in this list.
//  Up to 12 flames. Write each name with a & in front.
// =====================================================================

const Look *const FLAME_LIST[] = { &FIRE, &COOL, &SPARK, &SOLID, &RAINBOW_GLOW,
                                   &EMBER, &FROST, &CANDLE, &NEON, &RAINBOW_EMBER };
