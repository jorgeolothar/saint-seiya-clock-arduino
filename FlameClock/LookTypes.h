// =====================================================================
//  LOOKS: THE BUILDING BLOCKS
// =====================================================================
//
//  This file describes what a "look" is: how a group of LEDs changes
//  color over one cycle. The clock's flames are looks (see Flames.h),
//  and the scenes use the same looks.
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
// To see them all, switch on the DEMO scene in Scenes.h: one LED each.
enum Effect : uint8_t {
  GRADIENT,       // Fades through the colors, steady. The amount is not used.
                  // With a single color, this is a solid color.
  FLAME,          // Fades through the colors, flickering. Amount = flicker in %:
                  // 0 = steady, 100 = strongest.
  HEARTBEAT,      // Beats twice, like a heart (lub, dub), then rests on the last
                  // color until the cycle ends. Each beat plays the color list
                  // quickly. Amount = how strong the 2nd beat is, in %.
  PULSES,         // A quick burst of short pulses at the start of the cycle,
                  // then rests on the last color until the cycle ends. Each
                  // pulse plays the color list quickly. Amount = how many
                  // pulses, 1 to 8.
  FAILING,        // Fades through the colors, steady, like GRADIENT, but goes
                  // out for a moment or stutters at random times, like a bad
                  // wire. A group's LEDs fail together. Amount = how often
                  // it fails, in %.
  LIGHTNING,      // A sky, struck at random times by 1 to 4 quick flashes. The
                  // first color is the flash; the others are the sky. With
                  // more than one sky color, they drift along a scene group
                  // like clouds, one wave along it per cycle. A group's LEDs
                  // strike together. Amount = how often it strikes, in %.
  BUILDUP,        // Charges up through the colors, throbbing faster and faster,
                  // bursts on the last color, then fades back to the first.
                  // Amount = how deep the throbs are, in %.
  EXPLOSION,      // Flashes on the first color at once, then cools through the
                  // rest, quickly at first. Amount = how much the embers
                  // crackle and flare back up as they cool, in %.
  FLOW            // Fades through the colors, steady, like GRADIENT, but in a
                  // scene group each LED is a little behind the one before,
                  // so the colors travel from the group's first LED to its
                  // last. Amount = how far behind the last LED is, in % of
                  // the cycle: 100 = one full wave along the group.
};

// ---- Where a look's colors come from ----
enum ColorStyle : uint8_t {
  FIXED_COLORS,   // Exactly the colors you wrote
  RAINBOW_COLORS  // A rainbow across the LEDs: each LED takes its hue from its
                  // place in the group. Use colors made with rainbowColor().
};

// ---- A color for RAINBOW_COLORS looks ----
// Brightness and paleness in % (paleness: 0 = full color, 100 = white).
// The hue comes from the LED's place in the group. It is stored in an
// ordinary CRGB (red = brightness, green = paleness) that only rainbow
// looks read this way, so it shows a meaningless color in a
// FIXED_COLORS look.
inline CRGB rainbowColor(uint8_t brightness, uint8_t paleness) {
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
  Effect effect;              // GRADIENT, FLAME, HEARTBEAT, PULSES, FAILING, LIGHTNING, BUILDUP,
                              // EXPLOSION or FLOW
  const ColorStop *stops;     // The color list
  uint8_t stopCount;          // How many colors are in the list (counted for you)
  uint8_t amount;             // What it means depends on the effect (FLAME: flicker %,
                              // HEARTBEAT: strength of the 2nd beat %, PULSES: how many,
                              // FAILING: how often it fails %, LIGHTNING: how often it strikes %,
                              // BUILDUP: how deep the throbs are %, EXPLOSION: ember crackle %,
                              // FLOW: how far behind the last LED is, in % of the cycle)
  ColorStyle colors;          // FIXED_COLORS or RAINBOW_COLORS

  template <size_t N>
  constexpr Look(Effect e, const ColorStop (&list)[N], uint8_t amt, ColorStyle style)
    : effect(e), stops(list), stopCount(N), amount(amt), colors(style) {
    static_assert(N <= 255, "A color list can hold up to 255 colors");
  }
};
