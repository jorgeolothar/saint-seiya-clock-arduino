# 🏛️ Temple Scenes

Besides the clock rings, the Arduino can light up to four scenes of temples. Each scene is one chain of LEDs on its own pin, split into temples, and each temple lights up in its hour.

The scenes are optional, and all four are switched off in the sketch as it comes. Switched off, they use no memory on the Arduino.

| Scene | Temples | Cycle | Pin |
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

## 🏗️ Setting up the temples

Each temple is one line in **Scenes.h**, in the order its LEDs are wired: the temple whose LEDs come first on the chain goes first.

```cpp
//  hour    LEDs  on look    off look  cycle ms  repeat
  { 1,      2,    COOL,      OFF,      2000,     REPEAT },   // Aries
  { ALWAYS, 2,    GOLD_GLOW, OFF,      4000,     REPEAT },   // Athena's temple
```

| Value | What it means |
|---|---|
| **Hour** | Which hour of the scene's cycle the temple is on, from 1. `ALWAYS` = on all the time. The cycle is as long as the highest hour, then starts again |
| **LEDs** | How many LEDs the temple has. To add an LED, change this number: nothing else moves |
| **On look** | The look while it's on: any look from `TempleLooks.h` or `Flames.h` |
| **Off look** | The look while it's off. `OFF` = dark, `CINDERS` = a dim flickering glow |
| **Cycle ms** | One cycle of the on look, in thousandths of a second (`2000` = 2 seconds, up to `65000`). `0` = fade in and hold |
| **Repeat** | What happens after one cycle: `REPEAT` = again and again, `ONCE_HOLD` = stay on the last color, `ONCE_OFF` = turn to the off look |

When a temple turns on, its look fades in from the off look. With `REPEAT`, each cycle starts where the last one ended, so a look whose colors go back and forth pulses smoothly.

## ⚙️ The settings of each scene

| Setting | What it means |
|---|---|
| `..._PIN` | The Arduino pin the chain's DIN is wired to. A0 to A3 work as well as D2 to D13 |
| `..._CHIP` | The LED chip: `WS2812B`, or for 5mm LEDs often `PL9823` or `APA106` |
| `..._ORDER` | `GRB`, or `RGB` if red and green look swapped |
| `..._POWER` | The scene's power limit: `{ most milliamps, milliamps per color }`. See [Power limit](#-power-limit) |
| `..._LIGHTING` | `SPOTLIGHT` lights only the current hour's temple. `FILL` keeps each temple on from its hour until the end of the cycle |
| `..._START` | When hour 1 starts, as `clockTime(hour, minute, 0)` |

The cycle should fit a day evenly (8 or 12 hours, for example). If it doesn't, it starts again at `..._START` every day.

`ALL_TEMPLES_ON`, at the top of `Scenes.h`, turns every temple of every scene on, whatever the hour. It's handy for checking the wiring. Set it back to `false` when you're done.

## 🎨 The temple looks

These come in **TempleLooks.h**. Any flame from `Flames.h` works as a temple look too.

| Look | What it is |
|---|---|
| `OFF` | Dark. The usual off look |
| `CINDERS` | A dim red glow that flickers. An off look for a fire temple |
| `GOLD_GLOW` | A gold glow that swells and settles. For Athena's temple |
| `UNDERWORLD` | A deep purple flame with a dim violet glow. For Hades' temples |
| `SEA` | Deep sea blue that drifts into teal and back. For Poseidon's temples |

Temple looks are written the same way as flames: see [Making your own flame](README.md#️-making-your-own-flame) in the README.

## 🧪 Test modes

The scenes follow the clock's `BEHAVIOR` setting in **PART 2** of `FlameClock.ino`:

| Setting | What the scenes do |
|---|---|
| `MODE_ACCELERATED` | Runs through their hours faster. The looks keep their own speed |
| `MODE_COLOR_TEST` | Every temple shows its on look's first color |
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

The Uno and Nano have 2 KB of memory. Each scene switched on takes some of it for its LEDs. Its list of temples is kept in program space, so that part costs no memory.

| Switched on | Memory used |
|---|---|
| 1 ring, no scenes | 487 bytes (23%) |
| 1 ring and the Sanctuary | 660 bytes (32%) |
| 3 rings and all four scenes | 1,181 bytes (57%) |

If the clock resets by itself or the LEDs show garbage, memory may be running out: switch off a scene you don't need.

## 🩺 Troubleshooting

- **A scene stays dark:** check that its `#define USE_...` line has no slashes in front. Then set `ALL_TEMPLES_ON = true` to rule out the hour.
- **Red and green look swapped:** 5mm LEDs are often `RGB`. Change the scene's `..._ORDER`. A temple showing `SOLID` should look amber, not green.
- **Colors flicker or the chain shows random colors:** check the 100 nF capacitors, the 330 Ω resistor and that the scene's GND is joined to the Arduino's GND. Try `PL9823` or `APA106` as the scene's `..._CHIP`.
- **The wrong temple lights up:** the lines in `Scenes.h` must be in the order the LEDs are wired, with the right number of LEDs per temple.
