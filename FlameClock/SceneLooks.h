// =====================================================================
//  SCENES: THE LOOKS
// =====================================================================
//
//  Looks for the groups in Scenes.h. They are written the same way as
//  the flames in Flames.h, and a group can use any look from either
//  file. The colors are named in Colors.h.
//
//  A group plays its look over one cycle (its "cycle ms" in Scenes.h).
//  Looping, each cycle starts where the last one ended, so a look whose
//  colors go back and forth (like ICE_WHITE then ICE_BLUE) pulses
//  smoothly.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called SceneLooks.h.
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

// A heartbeat: two quick red beats, resting on a dark red glow. Its
// cycle ms is the heart rate: 1000 = 60 beats a minute.
const ColorStop HEART_COLORS[]    = { { EARLY_FADE, RED }, { SMOOTH_FADE, DARK_RED } };
const Look HEART                  = { HEARTBEAT, HEART_COLORS, 60, FIXED_COLORS };

// A beacon: a burst of 3 short white flashes, then dark until the next
// burst. Its cycle ms is how often the burst comes.
const ColorStop BEACON_COLORS[]   = { { SNAP, SPARK_WHITE }, { EARLY_FADE, BLACK } };
const Look BEACON                 = { PULSES, BEACON_COLORS, 3, FIXED_COLORS };

// A faulty light: a cold white that goes out for a moment or stutters
// now and then, like a bad wire. Its cycle ms doesn't matter: use 0.
const ColorStop FAULTY_LIGHT_COLORS[] = { { SNAP, ICE_WHITE } };
const Look FAULTY_LIGHT           = { FAILING, FAULTY_LIGHT_COLORS, 30, FIXED_COLORS };

// Deep sea blue that drifts into teal and back. For Poseidon's temples.
const ColorStop SEA_COLORS[]      = { { SMOOTH_FADE, DIM_TEAL }, { SMOOTH_FADE, SEA_BLUE } };
const Look SEA                    = { GRADIENT, SEA_COLORS, 0, FIXED_COLORS };
