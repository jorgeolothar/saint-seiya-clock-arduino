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
//                 1. ALWAYS = on all the time. FIRST_HALF_OF_CYCLE and
//                 SECOND_HALF_OF_CYCLE = on for that half of the cycle.
//                 The cycle is as long as the highest hour (twice that
//                 with FILL_AND_EMPTY), and starts again after it.
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
//    ..._ORDER ..... RGB for 5mm LEDs, GRB for LED strips and rings.
//                    If red and green look swapped, change it
//    ..._POWER ..... the scene's power supply: { most milliamps,
//                    milliamps per color }, as for RING_POWER in PART 3
//                    of FlameClock.ino
//    ..._LIGHTING .. SPOTLIGHT = only the current hour's groups are on
//                    FILL = each group stays on from its hour until
//                           the end of the cycle
//                    FILL_AND_EMPTY = like the hours ring: each group
//                           stays on for half the cycle from its hour,
//                           so they light up one by one, then go out
//                           one by one. The cycle is twice the highest
//                           hour: 12 temples light up from midnight to
//                           noon, and go out from noon to midnight
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
//  DEMO: one LED for each effect, and 4 for FLOW, always on: 12 LEDs.
//  Wire a 12-LED ring or strip to A3 and switch it on to see them all.
// ---------------------------------------------------------------------
// #define USE_DEMO
#define DEMO_PIN   A3
#define DEMO_CHIP  WS2812B
#define DEMO_ORDER GRB      // GRB for a ring or strip, RGB for 5mm LEDs
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
  { ALWAYS, 4,    MOVING_RAINBOW, OFF,    3000,     REPEAT },                       // FLOW, along 4 LEDs
};

// ---------------------------------------------------------------------
//  SANCTUARY: the 12 zodiac temples, lit like the hours ring. In the
//  morning they light up one per hour, from Aries at midnight to Pisces
//  at 11; in the afternoon they go out in the same order, so each temple
//  burns for 12 hours. Athena's temple burns from noon to midnight, as
//  the temples go out.
//
//  Wired from Athena's temple (closest to the Arduino) to Pisces, then
//  down to Aries, and left to right inside each temple. One LED over
//  each fighter, each with a look of its own (SceneLooks.h). The lines
//  with slashes are extra LEDs for later: remove the slashes once the
//  LED is wired in, in its place on the chain.
// ---------------------------------------------------------------------
// #define USE_SANCTUARY
#define SANCTUARY_PIN   10
#define SANCTUARY_CHIP  WS2812B
#define SANCTUARY_ORDER RGB
const PowerLimit SANCTUARY_POWER = { 2000, 20 };   // A 5V 10A supply: 2 A covers every LED at full
const Lighting SANCTUARY_LIGHTING = FILL_AND_EMPTY;
const uint32_t SANCTUARY_START = clockTime(0, 0, 0);

constexpr Group SANCTUARY[] = {
//  hour                  LEDs  on look             off look  cycle ms  repeat                   amount
//{ SECOND_HALF_OF_CYCLE, 2,    GOLD_GLOW,          OFF,      4000,     REPEAT },                        // Athena's temple
  { 12,                   1,    ROYAL_DEMON_ROSE,   OFF,      6000,     REPEAT },                        // Pisces: Aphrodite
  { 12,                   1,    NEBULA_STORM,       OFF,      8000,     REPEAT },                        // Pisces: Shun's cloud
//{ 12,                   1,    NEBULA_STORM,       OFF,      8000,     REPEAT },                        // Pisces: a 2nd LED in the cloud
//{ 11,                   1,    CAMUS_COLD,         OFF,      8000,     REPEAT },                        // Aquarius: over Camus
  { 11,                   2,    AURORA_CLASH,       OFF,      4000,     REPEAT },                        // Aquarius: the clash, center
//{ 11,                   1,    FREEZE_RAY,         OFF,      4000,     pauseThenRepeat(4000) },         // Aquarius: over Hyoga
  { 10,                   1,    DRAGON_COSMOS,      OFF,      2000,     REPEAT },                        // Capricorn: Shiryu
  { 10,                   1,    EXCALIBUR,          OFF,      800,      pauseThenRepeat(4000) },         // Capricorn: Shura
//{ 10,                   1,    LAST_DRAGON,        OFF,      5000,     pauseThenRepeat(7000) },         // Capricorn: under the dragon, on the roof
  { 9,                    1,    GOLD_GLOW,          OFF,      4000,     REPEAT },                        // Sagittarius: Aiolos's wing
  { 9,                    1,    CANDLE,             OFF,      2000,     REPEAT },                        // Sagittarius: the message
//{ 9,                    1,    BABY_HEARTBEAT,     OFF,      600,      REPEAT },                        // Sagittarius: Aiolos saving baby Athena, on the roof
  { 8,                    1,    STORM_SKY,          OFF,      0,        REPEAT,                  100 },  // Scorpio: Hyoga's Diamond Dust
  { 8,                    1,    ANTARES,            OFF,      4000,     pauseThenRepeat(5000) },         // Scorpio: Milo
  { 7,                    1,    LIBRA_SWORD,        OFF,      4000,     REPEAT },                        // Libra: Shiryu
  { 7,                    1,    COOL,               OFF,      2000,     REPEAT,                  10 },   // Libra: the Freezing Coffin
  { 6,                    1,    PHOENIX_FLAMES,     OFF,      4000,     pauseThenRepeat(4000) },         // Virgo: Ikki and the phoenix
  { 6,                    1,    SHAKA_SERENITY,     OFF,      8000,     REPEAT },                        // Virgo: Shaka
  { 5,                    1,    LIGHTNING_PLASMA,   OFF,      0,        REPEAT },                        // Leo: Aioria
  { 5,                    1,    CASSIOS_FADING,     OFF,      10000,    pauseThenRepeat(3000) },         // Leo: Cassios and Shaina
  { 4,                    1,    WILL_O_WISP,        OFF,      0,        REPEAT },                        // Cancer: behind, left
//{ 4,                    1,    GHOST_FLAME,        OFF,      2000,     REPEAT },                        // Cancer: inside the green flame
  { 4,                    1,    FAINT_HEART,        OFF,      1500,     REPEAT },                        // Cancer: behind, right
  { 3,                    1,    GALAXIAN_EXPLOSION, OFF,      4000,     pauseThenRepeat(6000) },         // Gemini: Gemini
//{ 3,                    1,    GALAXIAN_FLASH,     OFF,      4000,     pauseThenRepeat(6000) },         // Gemini: front, aimed at the mirrors
  { 3,                    1,    NEBULA_STREAM,      OFF,      6000,     REPEAT },                        // Gemini: Shun
  { 2,                    1,    GREAT_HORN,         OFF,      2500,     pauseThenRepeat(4000) },         // Taurus: Aldebaran
  { 2,                    1,    PEGASUS_METEORS,    OFF,      1500,     pauseThenRepeat(3500) },         // Taurus: Seiya
  { 1,                    1,    MU_HAMMER,          OFF,      3000,     REPEAT },                        // Aries: Mu, front
  { 1,                    1,    FIRE,               OFF,      2000,     REPEAT },                        // Aries: the forge, back
};

// ---------------------------------------------------------------------
//  ASGARD: 7 temples, one per hour, then Odin's throne in hour 8, once
//  all the temples are done. 8 hours a cycle, 3 cycles a day.
// ---------------------------------------------------------------------
// #define USE_ASGARD
#define ASGARD_PIN   11
#define ASGARD_CHIP  WS2812B
#define ASGARD_ORDER RGB
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
#define POSEIDON_ORDER RGB
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
#define HADES_ORDER RGB
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
