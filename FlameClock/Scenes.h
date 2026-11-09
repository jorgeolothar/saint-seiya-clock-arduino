// =====================================================================
//  SCENES
// =====================================================================
//
//  Each scene is a chain of LEDs on its own data pin, split into
//  groups: a few LEDs that light up together, like a temple, part of a
//  throne or a street lamp. A group lights up in its hour of the
//  scene's cycle, and plays its look. All the scenes are switched off as
//  the sketch comes: see SCENES.md, next to the README. To see every
//  effect first, switch on the DEMO.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab called Scenes.h.
//
//  HOW A SCENE IS WRITTEN
//
//    One line per group, in the order the LEDs are wired: the group
//    whose LEDs come first on the chain goes first. To give a group an
//    extra LED, change its LEDs number. Nothing else moves. Groups can
//    share an hour: a throne can be several groups, each with its own
//    look, and street lamps that fail on their own are one group each.
//
//    { hour, LEDs, on look, off look, cycle ms, repeat },
//    { hour, LEDs, on look, off look, cycle ms, repeat, amount },
//
//    hour ....... which hour of the scene's cycle the group is on, from
//                 1. ALWAYS = on all the time. The cycle is as long as
//                 the highest hour, and starts again after it.
//    LEDs ....... how many LEDs the group has
//    on look .... the look while it is on: any look from SceneLooks.h
//                 or Flames.h
//    off look ... the look while it is off. OFF = dark. It shows its
//                 last color, with its flicker.
//    cycle ms ... one cycle of the on look, in thousandths of a second
//                 (2000 = 2 seconds, up to 65000). 0 = no cycle: the
//                 look just fades in and holds its last color.
//    repeat ..... after one cycle of the on look:
//                   REPEAT ..... play it again, and again
//                   pauseThenRepeat(6000) .. wait 6 seconds on its
//                                end color, then play it again
//                   ONCE_HOLD .. stay on its last color
//                   ONCE_OFF ... turn to the off look
//    amount ..... optional: the on look's amount for this group only,
//                 so groups can share a look but flicker, fail or
//                 strike more or less (see SCENES.md). Leave it out to
//                 use the look's own amount.
//
//    When a group turns on, its look fades in from the off look.
//
//  THE SETTINGS OF EACH SCENE
//
//    USE_... ....... remove the two slashes to switch the scene on
//    ..._PIN ....... the Arduino pin the chain's DIN is wired to
//                    (A0 to A3 work as well as D2 to D13)
//    ..._CHIP ...... the LED chip: WS2812B, or for 5mm LEDs often
//                    PL9823 or APA106
//    ..._ORDER ..... GRB, or RGB if red and green look swapped (5mm
//                    LEDs are often RGB)
//    ..._POWER ..... the scene's power supply: { most milliamps,
//                    milliamps per color }, as for RING_POWER in PART 3
//                    of FlameClock.ino
//    ..._LIGHTING .. SPOTLIGHT = only the current hour's groups are on
//                    FILL = each group stays on from its hour until
//                           the end of the cycle
//    ..._START ..... when hour 1 starts, as clockTime(hour, minute, 0)
//
//  The cycle should fit a day evenly (8 or 12 hours, for example), or
//  it starts again at ..._START every day.
//
// =====================================================================

#pragma once
#include "SceneTypes.h"   // The building blocks. Do not change these lines.
#include "ClockTypes.h"
#include "Flames.h"
#include "SceneLooks.h"

// Turn every group of every scene on, whatever the hour? Handy for
// checking the wiring. true = yes, false = no (the normal setting)
const bool ALL_GROUPS_ON = false;

// ---------------------------------------------------------------------
//  DEMO: one LED for each effect, always on. Wire 8 LEDs (a strip, or
//  the first 8 of a ring) to A3 and switch it on to see them all.
// ---------------------------------------------------------------------
// #define USE_DEMO
#define DEMO_PIN   A3
#define DEMO_CHIP  WS2812B
#define DEMO_ORDER GRB
const PowerLimit DEMO_POWER = { 300, 20 };
const Lighting DEMO_LIGHTING = SPOTLIGHT;
const uint32_t DEMO_START = clockTime(0, 0, 0);

constexpr Group DEMO[] = {
//  hour    LEDs  on look       off look  cycle ms  repeat                   amount
  { ALWAYS, 1,    SEA,          OFF,      5000,     REPEAT },                       // GRADIENT
  { ALWAYS, 1,    FIRE,         OFF,      2000,     REPEAT },                       // FLAME
  { ALWAYS, 1,    HEART,        OFF,      1000,     pauseThenRepeat(2000) },        // HEARTBEAT, every 3 s
  { ALWAYS, 1,    BEACON,       OFF,      3000,     REPEAT },                       // PULSES
  { ALWAYS, 1,    FAULTY_LIGHT, OFF,      0,        REPEAT },                       // FAILING
  { ALWAYS, 1,    STORM_SKY,    OFF,      0,        REPEAT,                  60 },  // LIGHTNING, more often
  { ALWAYS, 1,    FREEZE_RAY,   OFF,      4000,     pauseThenRepeat(3000) },        // BUILDUP, every 7 s
  { ALWAYS, 1,    FIREBALL,     OFF,      3000,     pauseThenRepeat(3000) },        // EXPLOSION, every 6 s
};

// ---------------------------------------------------------------------
//  SANCTUARY: the 12 zodiac temples, one per hour, and Athena's temple,
//  always on. Hour 1 starts at midnight (and again at noon): Aries from
//  12 to 1 o'clock, while the hours ring's flame is on its first LED,
//  through Pisces from 11 to 12 o'clock.
// ---------------------------------------------------------------------
// #define USE_SANCTUARY
#define SANCTUARY_PIN   10
#define SANCTUARY_CHIP  WS2812B
#define SANCTUARY_ORDER GRB
const PowerLimit SANCTUARY_POWER = { 300, 20 };
const Lighting SANCTUARY_LIGHTING = SPOTLIGHT;
const uint32_t SANCTUARY_START = clockTime(0, 0, 0);

constexpr Group SANCTUARY[] = {
//  hour    LEDs  on look    off look  cycle ms  repeat
  { 1,      2,    COOL,      OFF,      2000,     REPEAT },   // Aries
  { 2,      2,    COOL,      OFF,      2000,     REPEAT },   // Taurus
  { 3,      2,    COOL,      OFF,      2000,     REPEAT },   // Gemini
  { 4,      2,    COOL,      OFF,      2000,     REPEAT },   // Cancer
  { 5,      2,    COOL,      OFF,      2000,     REPEAT },   // Leo
  { 6,      2,    COOL,      OFF,      2000,     REPEAT },   // Virgo
  { 7,      2,    COOL,      OFF,      2000,     REPEAT },   // Libra
  { 8,      2,    COOL,      OFF,      2000,     REPEAT },   // Scorpio
  { 9,      2,    COOL,      OFF,      2000,     REPEAT },   // Sagittarius
  { 10,     2,    COOL,      OFF,      2000,     REPEAT },   // Capricorn
  { 11,     2,    COOL,      OFF,      2000,     REPEAT },   // Aquarius
  { 12,     2,    COOL,      OFF,      2000,     REPEAT },   // Pisces
  { ALWAYS, 2,    GOLD_GLOW, OFF,      4000,     REPEAT },   // Athena's temple
};

// ---------------------------------------------------------------------
//  ASGARD: 7 temples, one per hour, then Odin's throne in hour 8, once
//  all the temples are done. 8 hours a cycle, 3 cycles a day.
// ---------------------------------------------------------------------
// #define USE_ASGARD
#define ASGARD_PIN   11
#define ASGARD_CHIP  WS2812B
#define ASGARD_ORDER GRB
const PowerLimit ASGARD_POWER = { 300, 20 };
const Lighting ASGARD_LIGHTING = SPOTLIGHT;
const uint32_t ASGARD_START = clockTime(0, 0, 0);

constexpr Group ASGARD[] = {
//  hour  LEDs  on look  off look  cycle ms  repeat
  { 1,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 1
  { 2,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 2
  { 3,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 3
  { 4,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 4
  { 5,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 5
  { 6,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 6
  { 7,    2,    FROST,   OFF,      3000,     REPEAT },   // Temple 7
  { 8,    2,    COOL,    OFF,      3000,     REPEAT },   // Odin's throne
};

// ---------------------------------------------------------------------
//  POSEIDON: 7 temples, one per hour, then Poseidon's throne in hour 8,
//  once all the temples are done. 8 hours a cycle, 3 cycles a day.
// ---------------------------------------------------------------------
// #define USE_POSEIDON
#define POSEIDON_PIN   12
#define POSEIDON_CHIP  WS2812B
#define POSEIDON_ORDER GRB
const PowerLimit POSEIDON_POWER = { 300, 20 };
const Lighting POSEIDON_LIGHTING = SPOTLIGHT;
const uint32_t POSEIDON_START = clockTime(0, 0, 0);

constexpr Group POSEIDON[] = {
//  hour  LEDs  on look  off look  cycle ms  repeat
  { 1,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 1
  { 2,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 2
  { 3,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 3
  { 4,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 4
  { 5,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 5
  { 6,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 6
  { 7,    2,    SEA,     OFF,      5000,     REPEAT },   // Temple 7
  { 8,    2,    GOLD_GLOW, OFF,    5000,     REPEAT },   // Poseidon's throne
};

// ---------------------------------------------------------------------
//  HADES: 7 temples, one per hour, then Hades' throne in hour 8, once
//  all the temples are done. 8 hours a cycle, 3 cycles a day.
// ---------------------------------------------------------------------
// #define USE_HADES
#define HADES_PIN   A0
#define HADES_CHIP  WS2812B
#define HADES_ORDER GRB
const PowerLimit HADES_POWER = { 300, 20 };
const Lighting HADES_LIGHTING = SPOTLIGHT;
const uint32_t HADES_START = clockTime(0, 0, 0);

constexpr Group HADES[] = {
//  hour  LEDs  on look     off look  cycle ms  repeat
  { 1,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 1
  { 2,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 2
  { 3,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 3
  { 4,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 4
  { 5,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 5
  { 6,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 6
  { 7,    2,    UNDERWORLD, OFF,      3000,     REPEAT },   // Temple 7
  { 8,    2,    FROST,      OFF,      3000,     REPEAT },   // Hades' throne
};
