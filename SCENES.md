# 🏛️ Scenes

Besides the clock rings, the Arduino can light up to four scenes. Each scene is one chain of LEDs on its own pin, split into **groups**: a few LEDs that light up together, like a temple, part of a throne or a street lamp. Each group lights up in its hour.

The scenes are optional, and all four are switched off in the sketch as it comes. Switched off, they use no memory on the Arduino.

| Scene | Groups | Cycle | Pin |
|---|---|---|---|
| **Sanctuary** | 12 zodiac temples, one per hour, and Athena's temple, always on | 12 hours, from midnight and from noon: Aries 12–1 o'clock … Pisces 11–12 | D10 |
| **Asgard** | 7 temples, one per hour, then Odin's throne in hour 8 | 8 hours, 3 times a day | D11 |
| **Poseidon** | 7 temples, one per hour, then Poseidon's throne in hour 8 | 8 hours, 3 times a day | D12 |
| **Hades** | 7 temples, one per hour, then Hades' throne in hour 8 | 8 hours, 3 times a day | A0 |

## 🔛 Switching a scene on

Each scene has a `#define USE_...` line in **Scenes.h**. Remove the two slashes in front of it to switch the scene on:

```cpp
#define USE_SANCTUARY        // on
// #define USE_ASGARD        // off
```

## 🏗️ Setting up the groups

Each group is one line in **Scenes.h**, in the order its LEDs are wired: the group whose LEDs come first on the chain goes first.

```cpp
//  hour    LEDs  on look    off look  cycle ms  repeat
  { 1,      2,    COOL,      OFF,      2000,     REPEAT },   // Aries
  { ALWAYS, 2,    GOLD_GLOW, OFF,      4000,     REPEAT },   // Athena's temple
```

| Value | What it means |
|---|---|
| **Hour** | Which hour of the scene's cycle the group is on, from 1. `ALWAYS` = on all the time. The cycle is as long as the highest hour, then starts again |
| **LEDs** | How many LEDs the group has. To add an LED, change this number: nothing else moves |
| **On look** | The look while it's on: any look from `SceneLooks.h` or `Flames.h` |
| **Off look** | The look while it's off. `OFF` = dark, `CINDERS` = a dim flickering glow |
| **Cycle ms** | One cycle of the on look, in thousandths of a second (`2000` = 2 seconds, up to `65000`). `0` = fade in and hold |
| **Repeat** | What happens after one cycle: `REPEAT` = again and again, `ONCE_HOLD` = stay on the last color, `ONCE_OFF` = turn to the off look |
| **Amount** | Optional. The on look's amount for this group only: see below. Leave it out to use the look's own amount |

When a group turns on, its look fades in from the off look. With `REPEAT`, each cycle starts where the last one ended, so a look whose colors go back and forth pulses smoothly.

Groups can share an hour. A throne can be several groups, each with its own look, and lights that should act on their own, like street lamps, are one group each.

**A group's own amount.** Add an amount at the end of a line to change the on look's amount for that group only, so several groups can share one look but flicker, fail or strike more or less. It replaces whatever the amount means for that look: the flicker for a flame, how often a light fails, how many pulses, and so on.

```cpp
//  hour  LEDs  on look       off look  cycle ms  repeat  amount
  { 1,    2,    FAULTY_LIGHT, OFF,      0,        REPEAT },        // lamp 1: FAULTY_LIGHT's own 30%
  { 1,    2,    FAULTY_LIGHT, OFF,      0,        REPEAT, 10 },    // lamp 2: fails less often
  { 1,    2,    FAULTY_LIGHT, OFF,      0,        REPEAT, 60 },    // lamp 3: fails more often
```

This saves writing a new look for each amount, and each look you write takes a little memory, while an amount on a line takes none.

## ⚙️ The settings of each scene

| Setting | What it means |
|---|---|
| `..._PIN` | The Arduino pin the chain's DIN is wired to. A0 to A3 work as well as D2 to D13 |
| `..._CHIP` | The LED chip: `WS2812B`, or for 5mm LEDs often `PL9823` or `APA106` |
| `..._ORDER` | `GRB`, or `RGB` if red and green look swapped |
| `..._POWER` | The scene's power limit: `{ most milliamps, milliamps per color }`. See [Power limit](#-power-limit) |
| `..._LIGHTING` | `SPOTLIGHT` lights only the current hour's groups. `FILL` keeps each group on from its hour until the end of the cycle |
| `..._START` | When hour 1 starts, as `clockTime(hour, minute, 0)` |

The cycle should fit a day evenly (8 or 12 hours, for example). If it doesn't, it starts again at `..._START` every day.

`ALL_GROUPS_ON`, at the top of `Scenes.h`, turns every group of every scene on, whatever the hour. It's handy for checking the wiring. Set it back to `false` when you're done.

## 🎨 The scene looks

These come in **SceneLooks.h**. Any flame from `Flames.h` works in a scene too.

| Look | What it is |
|---|---|
| `OFF` | Dark. The usual off look |
| `CINDERS` | A dim red glow that flickers. An off look for a fire temple |
| `GOLD_GLOW` | A gold glow that swells and settles. For Athena's temple |
| `UNDERWORLD` | A deep purple flame with a dim violet glow. For Hades' temples |
| `SEA` | Deep sea blue that drifts into teal and back. For Poseidon's temples |
| `HEART` | A heartbeat: two quick red beats, resting on a dark red glow. A cycle ms of `1000` = 60 beats a minute |
| `BEACON` | A burst of 3 short white flashes, then dark. The cycle ms is how often the burst comes |
| `FAULTY_LIGHT` | A cold white light that goes out for a moment or stutters now and then, like a bad wire |
| `STORM_SKY` | A dim stormy blue sky, struck now and then by 1 to 4 quick white flashes |

Scene looks are written the same way as flames: see [Making your own flame](README.md#️-making-your-own-flame) in the README.

> 🌈 **Rainbow looks** (`RAINBOW_COLORS`, like `RAINBOW_GLOW` and `RAINBOW_EMBER`) spread the rainbow across each group's own LEDs, and every group starts it over at red. A 1-LED group is always red, and a 2-LED group is red and aqua. The hues don't move over time.

> 🔅 **Dim colors** are dimmed again by `BRIGHTNESS_PERCENT` in PART 3 of `FlameClock.ino` (25% as the sketch comes). At that setting, a dim rainbow color needs about 30% or more to show, for example `rainbowColor(35, 0)`. A color that's mostly one of red, green or blue, like `DARK_RED`, can go dimmer.

### 💓 Heartbeat looks

A look with the `HEARTBEAT` effect beats twice in each cycle, like a heart, then rests until the cycle ends. Each beat plays the look's color list quickly, and its **last color** is the resting color between heartbeats. The amount is how strong the 2nd beat is, in %:

```cpp
const ColorStop HEART_COLORS[] = { { EARLY_FADE, RED }, { SMOOTH_FADE, DARK_RED } };
const Look HEART               = { HEARTBEAT, HEART_COLORS, 60, FIXED_COLORS };
```

| Part of the cycle | What the LED does |
|---|---|
| 0 – 15% | 1st beat: plays the color list |
| 15 – 25% | Rests on the last color |
| 25 – 40% | 2nd beat, at the amount's strength |
| 40 – 100% | Rests until the next heartbeat |

The group's **cycle ms** sets the heart rate: `1000` = 60 beats a minute, `600` = 100. Use `REPEAT`, so it keeps beating. A heartbeat doesn't flicker.

### 🔦 Pulse looks

A look with the `PULSES` effect gives a quick burst of short pulses at the start of each cycle, then rests until the cycle ends, like a beacon. Each pulse plays the look's color list quickly, and its **last color** is the resting color. The amount is how many pulses, from 1 to 8:

```cpp
const ColorStop BEACON_COLORS[] = { { SNAP, SPARK_WHITE }, { EARLY_FADE, BLACK } };
const Look BEACON               = { PULSES, BEACON_COLORS, 3, FIXED_COLORS };
```

Each pulse takes 8% of the cycle, with 4% between pulses. With a cycle ms of `2000` and 3 pulses:

| Time | What the LED does |
|---|---|
| 0 – 160 ms | 1st pulse |
| 160 – 240 ms | Rests on the last color |
| 240 – 400 ms | 2nd pulse |
| 400 – 480 ms | Rests |
| 480 – 640 ms | 3rd pulse |
| 640 – 2000 ms | Rests until the next burst |

So a longer cycle ms makes each pulse longer too. Use `REPEAT`, so the bursts keep coming. Pulses don't flicker.

### 💡 Failing lights

A look with the `FAILING` effect plays its color list like `GRADIENT`, so with one color it's a steady light. Now and then it goes out for a moment, or stutters: out, on, out again, all within a fifth of a second. The amount is how often it fails, in %:

```cpp
const ColorStop FAULTY_LIGHT_COLORS[] = { { SNAP, ICE_WHITE } };
const Look FAULTY_LIGHT               = { FAILING, FAULTY_LIGHT_COLORS, 30, FIXED_COLORS };
```

| Amount | How often each LED fails |
|---|---|
| `100` | Every 0.8 seconds. It's still on most of the time |
| `30` | About every 2.7 seconds, at random |
| `10` | About every 8 seconds, at random |

**A group's LEDs fail together,** like one light: the whole room goes dark at once. For lights that fail on their own, like two street lamps, make each light its own group with the same hour. Different groups fail at different moments, and with [their own amounts](#️-setting-up-the-groups) they fail at different rates.

The failures follow real time, not the group's cycle, so they're the same whatever the cycle ms. With one color, the cycle ms doesn't matter: use `0`. A failing light doesn't flicker. `MODE_FREEZE` and `MODE_COLOR_TEST` show it steady.

### ⛈️ Lightning

A look with the `LIGHTNING` effect rests on its **last color**, and now and then lightning strikes: 1 to 4 quick flashes, each playing the color list. The first flash is at full strength, and the others at 40 to 100%. The amount is how often it strikes, in %:

```cpp
const ColorStop LIGHTNING_COLORS[] = { { SNAP, ICE_WHITE }, { EARLY_FADE, STORM_BLUE } };
const Look STORM_SKY               = { LIGHTNING, LIGHTNING_COLORS, 30, FIXED_COLORS };
```

| Amount | How often it strikes |
|---|---|
| `100` | Every 2 seconds |
| `30` | About every 7 seconds, at random |
| `10` | About every 20 seconds, at random |

Each flash takes 150 ms, and the next one follows 60 to 180 ms after it starts. A strike is over in under 0.7 seconds.

**A group's LEDs strike together,** like one sky. For clouds that strike on their own, make each cloud its own group with the same hour. To make one cloud strike more often than the others, give its line its own amount.

Like a failing light, lightning follows real time, so the cycle ms doesn't matter: use `0`. It doesn't flicker. `MODE_FREEZE` shows its resting color, and `MODE_COLOR_TEST` shows its first color.

## 🧪 Test modes

The scenes follow the clock's `BEHAVIOR` setting in **PART 2** of `FlameClock.ino`:

| Setting | What the scenes do |
|---|---|
| `MODE_ACCELERATED` | Runs through their hours faster. The looks keep their own speed |
| `MODE_COLOR_TEST` | Every group shows its on look's first color |
| `MODE_FREEZE` | Holds them still |

## ⚡ Power limit

Each scene estimates how much current its LEDs draw, and dims them if it's more than its `..._POWER` limit:

```cpp
const PowerLimit SANCTUARY_POWER = { 300, 20 };   // most milliamps, milliamps per color
```

**Milliamps per color** is what red, green or blue of one LED draws at full (about 20 for WS2812B; check your LEDs' datasheet). This is an estimate, not a measurement: keep the fuses in your wiring.

## 🔌 Wiring

| Connection | Pin |
|---|---|
| DIN — Sanctuary | D10 |
| DIN — Asgard | D11 |
| DIN — Poseidon | D12 |
| DIN — Hades | A0 |

- Give each scene its own 5V power supply, and **join its GND to the Arduino's GND**.
- Put a 330 Ω resistor in the data line, close to the Arduino pin, to protect the first LED.
- 5mm through-hole LEDs need a 100 nF capacitor right at the legs of each LED. LED strips and rings have them built in.

## 💾 Memory

The Uno and Nano have 2 KB of memory. Each scene switched on takes some of it for its LEDs. Its list of groups is kept in program space, so that part costs no memory.

| Switched on | Memory used |
|---|---|
| 1 ring, no scenes | 487 bytes (23%) |
| 1 ring and the Sanctuary | 660 bytes (32%) |
| 3 rings and all four scenes | 1,181 bytes (57%) |

If the clock resets by itself or the LEDs show garbage, memory may be running out: switch off a scene you don't need.

## 🩺 Troubleshooting

- **A scene stays dark:** check that its `#define USE_...` line has no slashes in front. Then set `ALL_GROUPS_ON = true` to rule out the hour.
- **Red and green look swapped:** 5mm LEDs are often `RGB`. Change the scene's `..._ORDER`. A group showing `SOLID` should look amber, not green.
- **Colors flicker or the chain shows random colors:** check the 100 nF capacitors, the 330 Ω resistor and that the scene's GND is joined to the Arduino's GND. Try `PL9823` or `APA106` as the scene's `..._CHIP`.
- **The wrong group lights up:** the lines in `Scenes.h` must be in the order the LEDs are wired, with the right number of LEDs per group.
