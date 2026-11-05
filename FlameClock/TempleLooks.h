// =====================================================================
//  TEMPLE SCENES: THE LOOKS
// =====================================================================
//
//  Looks for the temples in Scenes.h. They are written the same way as
//  the flames in Flames.h, and a temple can use any look from either
//  file. The colors are named in Colors.h.
//
//  A temple plays its look over one cycle (its "cycle ms" in Scenes.h).
//  Looping, each cycle starts where the last one ended, so a look whose
//  colors go back and forth (like ICE_WHITE then ICE_BLUE) pulses
//  smoothly.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called TempleLooks.h.
//
// =====================================================================

#pragma once
#include "LookTypes.h"   // The building blocks. Do not change these two lines.
#include "Colors.h"

// Off: dark. The usual off look.
const ColorStop OFF_COLORS[]      = { { SNAP, BLACK } };
const Look OFF                    = { GRADIENT, OFF_COLORS, 0, FIXED_COLORS };

// Cinders: a dim red glow that flickers. An off look for a fire temple.
const ColorStop CINDERS_COLORS[]  = { { SNAP, CINDER } };
const Look CINDERS                = { FLAME, CINDERS_COLORS, 60, FIXED_COLORS };

// A gold glow that swells and settles. For Athena's temple.
const ColorStop GOLD_GLOW_COLORS[] = { { SMOOTH_FADE, GOLD }, { SMOOTH_FADE, AMBER } };
const Look GOLD_GLOW              = { FLAME, GOLD_GLOW_COLORS, 15, FIXED_COLORS };

// A deep purple flame with a dim violet glow. For Hades' temples.
const ColorStop UNDERWORLD_COLORS[] = { { SMOOTH_FADE, VIOLET }, { SMOOTH_FADE, DEEP_PURPLE } };
const Look UNDERWORLD             = { FLAME, UNDERWORLD_COLORS, 60, FIXED_COLORS };

// Deep sea blue that drifts into teal and back. For Poseidon's temples.
const ColorStop SEA_COLORS[]      = { { SMOOTH_FADE, DIM_TEAL }, { SMOOTH_FADE, SEA_BLUE } };
const Look SEA                    = { GRADIENT, SEA_COLORS, 0, FIXED_COLORS };
