# 🔥 Flame Clock for the Saint Seiya Zodiac Clock Tower

[🧩 **View the 3D Model on MakerWorld**](https://makerworld.com/en/models/930419-saint-seiya-clock?from=search#profileId-1965434)

## ✨ What It Does

Click below for video:

[![Saint Seiya Clock Arduino](https://img.youtube.com/vi/aKjive5c41w/0.jpg)](https://www.youtube.com/watch?v=aKjive5c41w)

Each ring of 12 LEDs is a **clock face**. A flickering **"flame"** moves around it, one LED at a time.

Each ring can show something different:

| Ring shows | One LED per | 1st flame | 2nd flame |
|---|---|---|---|
| 🕐 **Hours** | hour | 🌙 midnight → noon | ☀️ noon → midnight |
| ⏱️ **Minutes** | 5 minutes | even hours (12, 2, 4 …) | odd hours |
| ⏲️ **Seconds** | 5 seconds | even minutes | odd minutes |

The flame goes around the ring twice, with a different flame each time.

### 🕯️ How a flame moves

When the flame reaches an LED, it slowly lights up. Halfway through, it's at full flame. Then it fades into its **trail**, the color it leaves behind, and moves on to the next LED.

| Arrives | | Halfway | | Leaves |
|:---:|:---:|:---:|:---:|:---:|
| ⬛ | 🔸 | 🔥 | 🔸 | 🟥 |
| old color | fading in | full flame | fading out | trail |

A **black trail** leaves the ring dark behind the flame.

## 🏛️ The Sanctuary Fire Clock (the default)

> The clock from the **Sanctuary chapter**: twelve blue flames that burn out one per hour while the Bronze Saints race through the twelve zodiac temples.

This is how the hours ring is set up when you first upload the sketch:

```cpp
{ SHOW_HOURS, COOL, EMBER, 1 },
```

### 🌙 Morning (midnight → noon): the flames are lit

A white flame lights one blue flame per hour. By noon, all twelve are burning.

At about 9:30 in the morning:

| 12 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🔥 | ⬛ | ⬛ |

### ☀️ Afternoon (noon → midnight): the countdown

Every hour, one blue flame flares up and burns out. By midnight the ring is dark, and the morning lights it again.

At about 3:30 in the afternoon:

| 12 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| ⬛ | ⬛ | ⬛ | 🔥 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 | 🟦 |

- ⬛ **Behind the flame:** the flames that have burned out.
- 🔥 **The flame:** the one burning out right now, on the 3 o'clock LED.
- 🟦 **Ahead of the flame:** the blue flames still burning.

> 🟧 The orange glow as each flame burns out is an extra touch, not from the series. For an all-blue countdown, change `DEEP_ORANGE` to `ICE_BLUE` in the `EMBER` line of `Flames.h`.

## 🔘 Button Controls

- **Short press** → turns the clock **on or off**.
- **Hold for 1 second** → changes the flame you see moving to the **next flame**.
  - Each ring shows the new flame's number like the hours on a clock: flame 3 lights the LEDs at 1, 2 and 3 o'clock.
  - Hold again for the next one.
  - The clock remembers your choice, even when unplugged.

## 🚀 Getting Started

1. Download the **[FlameClock](FlameClock/)** folder. Keep its three files together:

   | File | What's in it |
   |---|---|
   | [FlameClock.ino](FlameClock/FlameClock.ino) | Your clocks and settings. Open this one. |
   | [Flames.h](FlameClock/Flames.h) | The colors and flames. |
   | [FlameTypes.h](FlameClock/FlameTypes.h) | The building blocks. No need to open it. |

2. Install the libraries (see [Required Libraries](#-required-libraries)).
3. Open **FlameClock.ino** in the Arduino IDE. The other two files show up as tabs.
4. Set up your rings in **PART 1** (see below), then upload.

## ⏰ Setting Up Your Clocks

In **PART 1** of `FlameClock.ino`, write one line for each LED ring (1 to 4 rings), ring 1 first:

```cpp
const Clock CLOCKS[] = {
  { SHOW_HOURS,   COOL,      EMBER,         1 },   // the Sanctuary Fire Clock
  { SHOW_MINUTES, SPARK,     SPARK,         1 },   // a spark jumps every 5 minutes
  { SHOW_SECONDS, HUE_WHEEL, RAINBOW_EMBER, 4 },   // a rainbow fills and burns out every 2 minutes
};
```

The sketch comes with the minutes and seconds rings turned off (two slashes in front of their lines). Remove the slashes to use them.

Each line is: **what it shows**, **1st flame**, **2nd flame**, **top LED**.

- **What it shows:** `SHOW_HOURS`, `SHOW_MINUTES` or `SHOW_SECONDS`.
- **1st and 2nd flame:** any flame name from `Flames.h`. Both can be the same flame.
- **Top LED:** which LED is at 12 o'clock. Usually `1`. If your ring is turned, see [Finding the top LED](#-finding-the-top-led).

## 🎨 The Flames

These flames come built in. The number is what a ring shows when you pick it with the button.

| # | Flame | Trail, then flame ➡️ | Description |
|:---:|---|:---:|---|
| 1 | 🔥 `FIRE` | <small>🟥🟥</small>🟧 | Orange flame, red trail |
| 2 | 🧊 `COOL` | <small>🟦🟦</small>⬜ | Icy white flame, ice blue trail |
| 3 | ⚡ `SPARK` | <small>⬛⬛</small>🟨 | A bright flash that dies out quickly, no trail |
| 4 | 🟠 `SOLID` | <small>🟧🟧</small>🟧 | Steady amber, no fading, no flicker |
| 5 | 🌈 `HUE_WHEEL` | <small>🟥🟨</small>🌈 | A rainbow around the ring: pale flame, deep colored trail |
| 6 | 🪵 `EMBER` | <small>⬛⬛</small>🟧 | Orange flame that burns out quickly to black. The countdown in the Sanctuary Fire Clock (see above) |
| 7 | ❄️ `FROST` | <small>⬛⬛</small>⬜ | Icy flame that melts away to black |
| 8 | 🕯️ `CANDLE` | <small>🟫🟫</small>🟧 | Warm flame that leaves a faint glow |
| 9 | 💡 `NEON` | <small>🩵🩵</small>🩷 | Pink neon that hardly flickers, dim teal trail |
| 10 | 🌈 `RAINBOW_EMBER` | <small>⬛⬛</small>🌈 | Each LED burns its own rainbow color down to black |

### ✏️ Making your own flame

Flames live in **Flames.h**. Each one reads like a sentence, in the order things happen on an LED:

```cpp
//                          fade in      flame color  fade out     trail color  flicker% colors
const Flame FIRE          = { SMOOTH_FADE, ORANGE,      SMOOTH_FADE, RED,         70,      FIXED_COLORS };
```

> "FIRE fades smoothly into ORANGE, then fades smoothly into a RED trail, with 70% flicker, using the colors as written."

| Value | What it means |
|---|---|
| **Fade in** | How the flame appears over the LED's old color |
| **Flame color** | The flame at its brightest, halfway through |
| **Fade out** | How the flame turns into its trail |
| **Trail color** | What the flame leaves behind. `BLACK` = no trail |
| **Flicker %** | `0` = steady, `100` = strongest flicker |
| **Colors** | `FIXED_COLORS` = the colors as written. `WHEEL_COLORS` = a rainbow around the ring |

**The fades:**

| Fade | What it does |
|---|---|
| `NO_FADE` | No fade. The flame appears at once, or stays until it leaves the LED |
| `STEADY_FADE` | Changes at an even pace |
| `SMOOTH_FADE` | Gentle at the start and at the end |
| `EARLY_FADE` | Most of the change happens early |
| `LATE_FADE` | Most of the change happens late |

> 💡 **Tip:** how one flame takes over from the other is set by the arriving flame's **fade in**. A flame with `NO_FADE` (like `SOLID`) replaces the other flame's trail at once.

**Colors** are named at the top of `Flames.h`, written as `CRGB(red, green, blue)` with numbers from 0 to 255:

```cpp
const CRGB ORANGE = CRGB(255, 110, 0);
```

Change a color there, and every flame that uses it changes too. Search the web for "RGB color picker" to find the numbers you want.

**Rainbow colors** for `WHEEL_COLORS` flames are written as `wheelColor(brightness %, paleness %)`. Each LED takes its hue from its place on the ring, so there's no hue to pick:

```cpp
const CRGB WHEEL_PALE = wheelColor(100, 40);   // full brightness, a bit pale
```

To let the button pick your new flame, add its name to `FLAME_LIST` at the bottom of `Flames.h` (up to 12 flames).

### 🔁 Pairing flames

How the two flames on a ring work together depends on their trails. Some ideas for an hours ring:

| Look | Hours ring line | What happens |
|---|---|---|
| 🔥 Flame only | `{ SHOW_HOURS, FROST, EMBER, 1 }` | Only the flame is lit |
| 🌅 Fill & empty | `{ SHOW_HOURS, COOL, EMBER, 1 }` | The morning fills the ring, the afternoon empties it. This is the Sanctuary Fire Clock |
| 🔄 Always lit | `{ SHOW_HOURS, COOL, FIRE, 1 }` | The ring is never dark: each flame replaces the other's trail |
| 🔃 Empty & fill | `{ SHOW_HOURS, FROST, FIRE, 1 }` | The morning empties the ring, the afternoon fills it |

## 🧪 Testing

In **PART 2** of `FlameClock.ino`, set `BEHAVIOR` to one of these:

| Setting | What the clock does |
|---|---|
| `MODE_REGULAR` | Shows the time. The normal setting |
| `MODE_ACCELERATED` | A sped-up day, to watch a whole day go by in minutes |
| `MODE_COLOR_TEST` | Holds still and shows the colors of both flames |
| `MODE_FREEZE` | Holds still at one moment, to look closely at the colors |

Set it back to `MODE_REGULAR` when you're done.

### 🎯 Finding the top LED

`MODE_COLOR_TEST` shows every ring like this:

| 12 | 1 – 5 | 6 | 7 – 11 |
|:---:|:---:|:---:|:---:|
| 🔥 1st flame | 2nd flame's trail | 🔥 2nd flame | 1st flame's trail |

With top LED set to `1`, the single LED in the 1st flame's color should be at 12 o'clock. If it's at another hour, set top LED to **13 minus that hour**. For example, if it's at 3 o'clock, use `10`.

## 🔌 Wiring

Designed for an **Arduino Nano or Arduino Uno**.

### 💡 LED Ring

| Connection | Pin |
|---|---|
| 5V | 5V |
| GND | GND |
| DIN — Ring 1 | D6 |
| DIN — Ring 2 | D7 |
| DIN — Ring 3 | D8 |
| DIN — Ring 4 | D9 |

The project supports **1 to 4 LED rings**.

### 🔘 Push Button

- One leg → **D2**
- Other leg → **GND**

### 🕐 Optional Clock Module (RTC)

A DS3231 clock module keeps the real time, even while the clock is unplugged. Without one, the clock starts at a set time every time it's plugged in.

| RTC | Arduino |
|---|---|
| SDA | A4 |
| SCL | A5 |
| VCC | 5V |
| GND | GND |

To use it, remove the two slashes in front of `#define USE_RTC` in **PART 1**. The first time, the module is set to your computer's time when you upload, so check your computer's clock first. **PART 4** of `FlameClock.ino` explains how to change the time for daylight saving, and which battery to use.

## 📚 Required Libraries

Install these through:

**Arduino IDE → Sketch → Include Library → Manage Libraries**

- **FastLED** — always required
- **RTClib** — only required when using an RTC clock module

## 🩺 Troubleshooting

- **Nothing lights up:** check the 5V, GND and DIN wires. Press the button once, in case the clock is off.
- **Red and green look swapped:** change `COLOR_ORDER` to `RGB` in **PART 3**.
- **The first LED keeps blinking red:** the clock module wasn't found. Check its SDA and SCL wires.
- **The flame starts in the wrong place:** see [Finding the top LED](#-finding-the-top-led).
- **A flame you changed doesn't show:** the clock remembers the flame picked with the button. Uploading the changed sketch clears that.

## 🛠️ Changes from the Original Sketch

- Comments translated to English
- Replaced the original hour countdown with a **moving, flickering flame**
- Each ring can show **hours, minutes or seconds**, with its own two flames
- **10 built-in flames**, and you can make your own with named colors
- **Rainbow flames** that color each LED by its place on the ring
- Holding the button **switches flames**, and the choice is remembered when unplugged
- Can operate **without an RTC module**, using a built-in software clock
- **Test modes** for checking colors, timing and the top LED, and a sped-up day for demos
- Supports **1 to 4 LED rings**, each with its own top LED
