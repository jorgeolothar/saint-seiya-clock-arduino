# 🔥 Flame Clock for the Saint Seiya Zodiac Clock Tower

[🧩 **View the 3D Model on MakerWorld**](https://makerworld.com/en/models/930419-saint-seiya-clock?from=search#profileId-1965434)

## ✨ What It Does

[![Saint Seiya Clock Arduino](https://img.youtube.com/vi/aKjive5c41w/0.jpg)](https://www.youtube.com/watch?v=aKjive5c41w)

Each LED ring acts as a **12-hour clock face**, with one LED representing each hour.

A flickering **"flame"** moves around the ring, advancing one LED per hour and completing the cycle twice a day:

- 🌙 **Midnight → Noon:** Cool-colored flame
- ☀️ **Noon → Midnight:** Warm-colored flame

Depending on the selected **Fill Mode**, the flame can leave a glowing trail behind it. This allows the ring to gradually **fill up or empty throughout the day**, creating a dynamic flame-like clock effect.

## 🔘 Button Controls

- **Short press** → Turns the clock **ON/OFF**
- **Hold for 1 second** → Changes the **Fill Mode**
  - The ring flashes white **1, 2, 3, or 4 times** to indicate the selected mode.

## 🎨 The 4 Fill Modes

The day is divided into two periods:

- 🌅 **Morning:** Midnight → Noon
- 🌇 **Afternoon:** Noon → Midnight

### 🔥 Mode 1 — Flame Only

Only the moving flame is illuminated.

Everything else remains dark.

### 🌅 Mode 2 — Fill & Empty

- The morning flame leaves a glowing trail and gradually **fills the ring**.
- The afternoon flame moves around the ring and gradually **empties it again**.

### 🔄 Mode 3 — Continuous Color

The ring is **never completely dark**.

- At midnight, the ring is filled with the **afternoon trail color**.
- During the morning, the flame gradually replaces it with the **morning trail color**.
- At noon, the afternoon flame takes over and gradually fills the ring with the **afternoon trail color** again.
- By midnight, the cycle starts over.

### 🔥 Mode 4 — Fill & Empty Reverse

- At midnight, the ring starts completely filled with the **afternoon trail color**.
- The morning flame gradually **empties the ring**.
- After noon, the afternoon flame gradually **fills it back up** with the afternoon trail color.

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

### 🕐 Optional RTC Clock Module

If using an RTC module:

| RTC | Arduino |
|---|---|
| SDA | A4 |
| SCL | A5 |
| VCC | 5V |
| GND | GND |

## 📚 Required Libraries

Install these through:

**Arduino IDE → Sketch → Include Library → Manage Libraries**

- **FastLED** — always required
- **RTClib** — only required when using an RTC clock module

## 🛠️ Changes from the Original Sketch

This version includes several major changes:

- Comments translated to English
- Replaced the original hour countdown with a **moving, flickering flame**
- Added **4 different fill modes**
- Can operate **without an RTC module**, using a built-in software clock
- The built-in clock can be **sped up for testing and demonstrations**
- Supports **1 to 4 LED rings**
- Includes test options for checking **colors and timing**
