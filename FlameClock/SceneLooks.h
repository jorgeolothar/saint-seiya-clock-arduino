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

// Lightning: a dim stormy blue sky, struck now and then by 1 to 4 quick
// white flashes. With a single sky color, its cycle ms doesn't matter:
// use 0.
const ColorStop LIGHTNING_COLORS[] = { { SNAP, ICE_WHITE }, { EARLY_FADE, STORM_BLUE } };
const Look STORM_SKY              = { LIGHTNING, LIGHTNING_COLORS, 30, FIXED_COLORS };

// A freeze ray: a dim teal that charges up to ice blue, throbbing faster
// and faster, bursts in ice white, and fades back down. Its cycle ms is
// how long one charge and burst take.
const ColorStop FREEZE_RAY_COLORS[] = { { SMOOTH_FADE, DIM_TEAL }, { STEADY_FADE, ICE_BLUE }, { LATE_FADE, ICE_WHITE } };
const Look FREEZE_RAY             = { BUILDUP, FREEZE_RAY_COLORS, 60, FIXED_COLORS };

// A fireball: a white-hot flash that cools through orange and red to
// smoldering cinders, crackling and flaring up as it cools. Its cycle ms
// is how long it takes to cool.
const ColorStop FIREBALL_COLORS[]  = { { SNAP, SPARK_WHITE }, { EARLY_FADE, ORANGE }, { STEADY_FADE, RED }, { SMOOTH_FADE, CINDER } };
const Look FIREBALL               = { EXPLOSION, FIREBALL_COLORS, 60, FIXED_COLORS };

// A thunderstorm: dark clouds drifting along the group, struck now and
// then by 1 to 4 quick white flashes. Its cycle ms is how long the
// clouds take to drift along the group: give it a few LEDs in a row.
const ColorStop THUNDERSTORM_COLORS[] = { { SNAP, ICE_WHITE }, { SMOOTH_FADE, STORM_BLUE }, { SMOOTH_FADE, CLOUD_GRAY } };
const Look THUNDERSTORM           = { LIGHTNING, THUNDERSTORM_COLORS, 30, FIXED_COLORS };

// A strobe: quick flashes of cold white. Each color gets a quarter of
// the cycle: white for one, dark for three. Its cycle ms is the rate:
// 200 = 5 flashes a second. The LEDs are updated every 30 ms, so below
// 120, some flashes get skipped (hard to see at that speed).
const ColorStop STROBE_COLORS[]   = { { SNAP, ICE_WHITE }, { SNAP, BLACK }, { SNAP, BLACK }, { SNAP, BLACK } };
const Look STROBE                 = { GRADIENT, STROBE_COLORS, 0, FIXED_COLORS };

// A rainbow that moves along the group. Give it 6 LEDs or more.
const ColorStop MOVING_RAINBOW_COLORS[] = {
  { STEADY_FADE, SPECTRUM_RED },  { STEADY_FADE, SPECTRUM_ORANGE }, { STEADY_FADE, SPECTRUM_YELLOW },
  { STEADY_FADE, SPECTRUM_GREEN }, { STEADY_FADE, SPECTRUM_AQUA },  { STEADY_FADE, SPECTRUM_BLUE },
  { STEADY_FADE, SPECTRUM_PURPLE }, { STEADY_FADE, SPECTRUM_PINK } };
const Look MOVING_RAINBOW         = { FLOW, MOVING_RAINBOW_COLORS, 100, FIXED_COLORS };

// Coolant: pulses of ice blue running through a dim teal tube. Its
// cycle ms is how long a pulse takes to run along the group.
const ColorStop COOLANT_COLORS[]  = { { SMOOTH_FADE, ICE_BLUE }, { SMOOTH_FADE, DIM_TEAL }, { HOLD, DIM_TEAL }, { HOLD, DIM_TEAL } };
const Look COOLANT                = { FLOW, COOLANT_COLORS, 100, FIXED_COLORS };

// Deep sea blue that drifts into teal and back. For Poseidon's temples.
const ColorStop SEA_COLORS[]      = { { SMOOTH_FADE, DIM_TEAL }, { SMOOTH_FADE, SEA_BLUE } };
const Look SEA                    = { GRADIENT, SEA_COLORS, 0, FIXED_COLORS };

// ---------------------------------------------------------------------
//  THE SANCTUARY'S TEMPLES
//  One look per fighter, from Aries to Pisces. The temples also use
//  FIRE, CANDLE and COOL (Flames.h), GOLD_GLOW, STORM_SKY and FREEZE_RAY.
// ---------------------------------------------------------------------

// Aries. Mu's hammer: 3 white-gold clinks, resting on his pale gold aura.
const ColorStop MU_HAMMER_COLORS[]          = { { SNAP, SPARK_WHITE }, { EARLY_FADE, PALE_GOLD } };
const Look MU_HAMMER                        = { PULSES, MU_HAMMER_COLORS, 3, FIXED_COLORS };

// Taurus. Aldebaran's Great Horn: a gold-white blast, cooling through
// amber to a dim orange, crackling a little.
const ColorStop GREAT_HORN_COLORS[]         = { { SNAP, SPARK_WHITE }, { EARLY_FADE, GOLD }, { STEADY_FADE, AMBER }, { SMOOTH_FADE, DIM_ORANGE } };
const Look GREAT_HORN                       = { EXPLOSION, GREAT_HORN_COLORS, 30, FIXED_COLORS };

// Taurus. Seiya's Pegasus meteors: 8 quick blue-white flashes, resting
// on a sea blue.
const ColorStop PEGASUS_METEORS_COLORS[]    = { { SNAP, ICE_WHITE }, { EARLY_FADE, SEA_BLUE } };
const Look PEGASUS_METEORS                  = { PULSES, PEGASUS_METEORS_COLORS, 8, FIXED_COLORS };

// Gemini. The Galaxian Explosion: dark red charging through violet,
// bursting in starlight white.
const ColorStop GALAXIAN_EXPLOSION_COLORS[] = { { SMOOTH_FADE, DARK_RED }, { STEADY_FADE, VIOLET }, { LATE_FADE, SPARK_WHITE } };
const Look GALAXIAN_EXPLOSION               = { BUILDUP, GALAXIAN_EXPLOSION_COLORS, 60, FIXED_COLORS };

// Gemini. For an LED aimed at the mirrors: dark until the burst, so
// the mirrors flash with the Galaxian Explosion. Give it the same
// cycle ms and repeat as GALAXIAN_EXPLOSION, and it fires with it.
const ColorStop GALAXIAN_FLASH_COLORS[]     = { { SMOOTH_FADE, BLACK }, { STEADY_FADE, DEEP_PURPLE }, { LATE_FADE, SPARK_WHITE } };
const Look GALAXIAN_FLASH                   = { BUILDUP, GALAXIAN_FLASH_COLORS, 60, FIXED_COLORS };

// Gemini. Shun's Nebula Stream: soft pink drifting to magenta and back.
const ColorStop NEBULA_STREAM_COLORS[]      = { { SMOOTH_FADE, SOFT_PINK }, { SMOOTH_FADE, NEON_PINK } };
const Look NEBULA_STREAM                    = { GRADIENT, NEBULA_STREAM_COLORS, 0, FIXED_COLORS };

// Cancer. Will-o'-wisps: a ghost green that stutters and goes out, like
// souls fading into Yomotsu Hirasaka.
const ColorStop WILL_O_WISP_COLORS[]        = { { SNAP, GHOST_GREEN } };
const Look WILL_O_WISP                      = { FAILING, WILL_O_WISP_COLORS, 30, FIXED_COLORS };

// Cancer. Shiryu's faint heartbeat, clinging to the edge.
const ColorStop FAINT_HEART_COLORS[]        = { { EARLY_FADE, BLOOD_RED }, { SMOOTH_FADE, DARK_RED } };
const Look FAINT_HEART                      = { HEARTBEAT, FAINT_HEART_COLORS, 40, FIXED_COLORS };

// Cancer. For an LED inside a translucent flame: Deathmask's ghost fire.
const ColorStop GHOST_FLAME_COLORS[]        = { { SMOOTH_FADE, GHOST_GREEN }, { SMOOTH_FADE, DIM_TEAL } };
const Look GHOST_FLAME                      = { FLAME, GHOST_FLAME_COLORS, 70, FIXED_COLORS };

// Leo. Aioria's Lightning Plasma: a dark red-violet sky (the Genma Ken
// controlling him), struck by ice-white bolts.
const ColorStop LIGHTNING_PLASMA_COLORS[]   = { { SNAP, ICE_WHITE }, { EARLY_FADE, GENMA_VIOLET } };
const Look LIGHTNING_PLASMA                 = { LIGHTNING, LIGHTNING_PLASMA_COLORS, 40, FIXED_COLORS };

// Leo. Cassios fading: rose-violet dimming to dark. Give it a pause, so
// it rests in the dark before rising again.
const ColorStop CASSIOS_FADING_COLORS[]     = { { SMOOTH_FADE, ROSE_VIOLET }, { STEADY_FADE, BLACK } };
const Look CASSIOS_FADING                   = { GRADIENT, CASSIOS_FADING_COLORS, 0, FIXED_COLORS };

// Virgo. Ikki's phoenix flames: a white-gold flare, cooling to a crimson
// that keeps crackling and flaring.
const ColorStop PHOENIX_FLAMES_COLORS[]     = { { SNAP, SPARK_WHITE }, { EARLY_FADE, ORANGE }, { STEADY_FADE, RED }, { SMOOTH_FADE, CRIMSON } };
const Look PHOENIX_FLAMES                   = { EXPLOSION, PHOENIX_FLAMES_COLORS, 100, FIXED_COLORS };

// Virgo. Shaka's serenity: gold breathing slowly to a warm white.
const ColorStop SHAKA_SERENITY_COLORS[]     = { { SMOOTH_FADE, GOLD }, { SMOOTH_FADE, WARM_WHITE } };
const Look SHAKA_SERENITY                   = { GRADIENT, SHAKA_SERENITY_COLORS, 0, FIXED_COLORS };

// Libra. The Libra sword: one gold-white gleam, resting on Shiryu's dark
// emerald.
const ColorStop LIBRA_SWORD_COLORS[]        = { { SNAP, SPARK_WHITE }, { EARLY_FADE, DARK_EMERALD } };
const Look LIBRA_SWORD                      = { PULSES, LIBRA_SWORD_COLORS, 1, FIXED_COLORS };

// Scorpio. Milo's Scarlet Needle: dark red throbbing faster and faster,
// the 14 stings, then bursting in scarlet: Antares.
const ColorStop ANTARES_COLORS[]            = { { SMOOTH_FADE, DARK_RED }, { STEADY_FADE, BLOOD_RED }, { LATE_FADE, RED } };
const Look ANTARES                          = { BUILDUP, ANTARES_COLORS, 80, FIXED_COLORS };

// Sagittarius. For an LED over Aiolos saving baby Athena: a soft lilac
// heartbeat.
const ColorStop BABY_HEARTBEAT_COLORS[]     = { { EARLY_FADE, LILAC }, { SMOOTH_FADE, DIM_LILAC } };
const Look BABY_HEARTBEAT                   = { HEARTBEAT, BABY_HEARTBEAT_COLORS, 40, FIXED_COLORS };

// Capricorn. Shiryu's cosmos: emerald, flickering hard.
const ColorStop DRAGON_COSMOS_COLORS[]      = { { SMOOTH_FADE, EMERALD }, { SMOOTH_FADE, DARK_EMERALD } };
const Look DRAGON_COSMOS                    = { FLAME, DRAGON_COSMOS_COLORS, 70, FIXED_COLORS };

// Capricorn. Shura's Excalibur: a sharp silver-white flash, vanishing
// into dark steel, with no crackle.
const ColorStop EXCALIBUR_COLORS[]          = { { SNAP, ICE_WHITE }, { EARLY_FADE, STEEL } };
const Look EXCALIBUR                        = { EXPLOSION, EXCALIBUR_COLORS, 0, FIXED_COLORS };

// Capricorn. For an LED under the dragon: the Last Dragon, emerald
// charging up and bursting white as it rises into heaven.
const ColorStop LAST_DRAGON_COLORS[]        = { { SMOOTH_FADE, DARK_EMERALD }, { STEADY_FADE, EMERALD }, { LATE_FADE, SPARK_WHITE } };
const Look LAST_DRAGON                      = { BUILDUP, LAST_DRAGON_COLORS, 60, FIXED_COLORS };

// Aquarius. The clashing Aurora Executions: aurora colors chasing each
// other along the group. On 2 LEDs, they stay half a cycle apart.
const ColorStop AURORA_CLASH_COLORS[]       = { { SMOOTH_FADE, AURORA_GREEN }, { SMOOTH_FADE, TEAL }, { SMOOTH_FADE, VIOLET }, { SMOOTH_FADE, ICE_WHITE } };
const Look AURORA_CLASH                     = { FLOW, AURORA_CLASH_COLORS, 50, FIXED_COLORS };

// Aquarius. For an LED over Camus: a slow, cold ice violet.
const ColorStop CAMUS_COLD_COLORS[]         = { { SMOOTH_FADE, ICE_VIOLET }, { SMOOTH_FADE, ICE_BLUE } };
const Look CAMUS_COLD                       = { GRADIENT, CAMUS_COLD_COLORS, 0, FIXED_COLORS };

// Pisces. Aphrodite's Royal Demon Rose: rose red swelling to soft pink.
const ColorStop ROYAL_DEMON_ROSE_COLORS[]   = { { SMOOTH_FADE, ROSE_RED }, { SMOOTH_FADE, SOFT_PINK } };
const Look ROYAL_DEMON_ROSE                 = { GRADIENT, ROYAL_DEMON_ROSE_COLORS, 0, FIXED_COLORS };

// Pisces. Shun's Nebula Storm, in a cloud: pink and violet drifting,
// struck by white flashes.
const ColorStop NEBULA_STORM_COLORS[]       = { { SNAP, SPARK_WHITE }, { SMOOTH_FADE, DIM_PINK }, { SMOOTH_FADE, DEEP_PURPLE } };
const Look NEBULA_STORM                     = { LIGHTNING, NEBULA_STORM_COLORS, 40, FIXED_COLORS };
