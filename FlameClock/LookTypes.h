// =====================================================================
//  LOOKS: THE BUILDING BLOCKS
// =====================================================================
//
//  This file describes what a "look" is: how a group of LEDs changes
//  color over one cycle. The clock's flames are looks (see Flames.h),
//  and the temple scenes will use the same looks.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include <FastLED.h>

// ---- How a change to the next color is spread over its part of the cycle ----
enum Fade : uint8_t {
  SNAP,           // Switches to the new color at once, at the start of its part
  HOLD,           // Keeps the old color, and switches at the end of its part
  STEADY_FADE,    // Changes at an even pace
  SMOOTH_FADE,    // Gentle at the start and at the end
  EARLY_FADE,     // Most of the change happens early
  LATE_FADE       // Most of the change happens late
};

// ---- One color in a look's color list, and how to fade into it ----
struct ColorStop {
  Fade fade;      // How the LED gets to this color from the one before
  CRGB color;     // The color
};

// ---- What a look does with its colors ----
enum Effect : uint8_t {
  GRADIENT,       // Fades through the colors, steady. The amount is not used.
                  // With a single color, this is a solid color.
  FLAME,          // Fades through the colors, flickering. Amount = flicker in %:
                  // 0 = steady, 100 = strongest.
  HEARTBEAT       // Beats twice, like a heart (lub, dub), then rests on the last
                  // color until the cycle ends. Each beat plays the color list
                  // quickly. Amount = how strong the 2nd beat is, in %.
};

// ---- Where a look's colors come from ----
enum ColorStyle : uint8_t {
  FIXED_COLORS,   // Exactly the colors you wrote
  WHEEL_COLORS    // A rainbow across the LEDs: each LED takes its hue from its
                  // place in the group. Use colors made with wheelColor().
};

// ---- A color for WHEEL_COLORS looks ----
// Brightness and paleness in % (paleness: 0 = full color, 100 = white).
// The hue comes from the LED's place in the group. It is stored in an
// ordinary CRGB (red = brightness, green = paleness) that only wheel
// looks read this way, so it shows a meaningless color in a
// FIXED_COLORS look.
inline CRGB wheelColor(uint8_t brightness, uint8_t paleness) {
  if (brightness > 100) brightness = 100;
  if (paleness > 100) paleness = 100;
  return CRGB(brightness * 255 / 100, paleness * 255 / 100, 0);
}

// ---- A look: how LEDs change color over one cycle ----
// Written as { effect, color list, amount, color style }, for example:
//
//   const ColorStop FIRE_COLORS[] = { { SMOOTH_FADE, ORANGE }, { SMOOTH_FADE, RED } };
//   const Look FIRE = { FLAME, FIRE_COLORS, 70, FIXED_COLORS };
//
// The color list can be any length. Its colors share the cycle equally,
// and the last one is where the look ends.
struct Look {
  Effect effect;              // GRADIENT, FLAME or HEARTBEAT
  const ColorStop *stops;     // The color list
  uint8_t stopCount;          // How many colors are in the list (counted for you)
  uint8_t amount;             // What it means depends on the effect (FLAME: flicker %,
                              // HEARTBEAT: strength of the 2nd beat %)
  ColorStyle colors;          // FIXED_COLORS or WHEEL_COLORS

  template <size_t N>
  constexpr Look(Effect e, const ColorStop (&list)[N], uint8_t amt, ColorStyle style)
    : effect(e), stops(list), stopCount(N), amount(amt), colors(style) {
    static_assert(N <= 255, "A color list can hold up to 255 colors");
  }
};
