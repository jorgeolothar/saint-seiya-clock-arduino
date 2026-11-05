// =====================================================================
//  TEMPLE SCENES: THE BUILDING BLOCKS
// =====================================================================
//
//  This file describes what a "temple" and a "scene" are, so that
//  Scenes.h can list your temples.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include "LookTypes.h"
#include "Power.h"

// ---- As a temple's hour: on all the time ----
const uint8_t ALWAYS = 0;

// ---- Which temples are on ----
enum Lighting : uint8_t {
  SPOTLIGHT,      // Only the temple of the current hour (and ALWAYS temples)
  FILL            // Every temple from its hour until the end of the scene's cycle
};

// ---- What a temple's look does once it has played one cycle ----
enum Repeat : uint8_t {
  REPEAT,         // Plays again, and again
  ONCE_HOLD,      // Stays on the look's last color
  ONCE_OFF        // Turns to the temple's off look
};

// ---- A temple: a few LEDs in a row on the scene's chain ----
struct Temple {
  uint8_t hour;           // Which hour of the scene's cycle it is on (1 = the first), or ALWAYS
  uint8_t ledCount;       // How many LEDs
  const Look *on;         // The look while it is on
  const Look *off;        // The look while it is off
  uint16_t cycleMs;       // One cycle of the on look, in thousandths of a second (0 = holds its last color)
  Repeat repeat;          // REPEAT, ONCE_HOLD or ONCE_OFF

  constexpr Temple(uint8_t onHour, uint8_t leds, const Look &onLook, const Look &offLook,
                   uint16_t cycle, Repeat again)
    : hour(onHour), ledCount(leds), on(&onLook), off(&offLook), cycleMs(cycle), repeat(again) {}
  Temple() = default;     // An empty temple, to copy one out of program space into
};

// ---- Something kept in program space (flash) instead of memory ----
// The Uno has 32 KB of program space but only 2 KB of memory, and
// keeps even fixed tables in memory unless they are marked PROGMEM.
// What is in program space has to be copied out to be read: get() does
// that. The value itself is private, so it can't be read by mistake
// without get(), which would give garbage on the Arduino.
template <typename T> class InFlash {
  T stored;
 public:
  constexpr InFlash(const T &value) : stored(value) {}
  T get() const {
    T copy;
    memcpy_P(&copy, &stored, sizeof(T));
    return copy;
  }
};

// ---- A program-space copy of a list of temples, made when the sketch
// is compiled. The list in Scenes.h is then only read by the compiler,
// so it takes no memory. ----
template <size_t... I> struct Indexes {};
template <size_t N, size_t... I> struct CountTo : CountTo<N - 1, N - 1, I...> {};
template <size_t... I> struct CountTo<0, I...> { typedef Indexes<I...> List; };

template <size_t N, const Temple (&LIST)[N], typename = typename CountTo<N>::List>
struct StoredTemples;
template <size_t N, const Temple (&LIST)[N], size_t... I>
struct StoredTemples<N, LIST, Indexes<I...>> {
  static constexpr InFlash<Temple> temples[N] PROGMEM = { LIST[I]... };
};
template <size_t N, const Temple (&LIST)[N], size_t... I>
constexpr InFlash<Temple> StoredTemples<N, LIST, Indexes<I...>>::temples[N];

// A list of temples as a Scene takes it: the list itself, for counting
// its temples, LEDs and hours, and its program-space copy, for drawing
#define SCENE_TEMPLES(list) list, StoredTemples<sizeof(list) / sizeof(Temple), list>::temples

// ---- Counting a list of temples (worked out when the sketch is compiled) ----
constexpr uint16_t ledsFrom(const Temple *temples, size_t count) {
  return count == 0 ? 0 : temples->ledCount + ledsFrom(temples + 1, count - 1);
}
template <size_t N> constexpr uint16_t ledsIn(const Temple (&temples)[N]) {
  return ledsFrom(temples, N);
}
constexpr uint8_t lastHourFrom(const Temple *temples, size_t count, uint8_t highest) {
  return count == 0 ? highest
                    : lastHourFrom(temples + 1, count - 1, temples->hour > highest ? temples->hour : highest);
}

// ---- A scene: a chain of temples on one data pin ----
struct Scene {
  const InFlash<Temple> *temples;   // The temples, in the order they are wired, in program space
  uint8_t templeCount;
  uint8_t hours;            // Hours in one cycle: the highest temple hour
  Lighting lighting;        // SPOTLIGHT or FILL
  uint32_t startMs;         // When hour 1 starts, in milliseconds since midnight
  CRGB *leds;               // The scene's LEDs
  uint16_t ledCount;
  PowerLimit power;         // The scene's power supply
  uint16_t flickerSpot;     // Where the scene's LEDs start in the flicker pattern

  // Written as Scene(SCENE_TEMPLES(list), leds, ...)
  template <size_t N>
  constexpr Scene(const Temple (&list)[N], const InFlash<Temple> (&stored)[N], CRGB *sceneLeds,
                  Lighting light, uint32_t start, PowerLimit limit, uint16_t spot)
    : temples(stored), templeCount(N), hours(lastHourFrom(list, N, 1)), lighting(light),
      startMs(start), leds(sceneLeds), ledCount(ledsFrom(list, N)), power(limit), flickerSpot(spot) {
    static_assert(N <= 255, "A scene can hold up to 255 temples");
  }
};
