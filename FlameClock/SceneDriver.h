// =====================================================================
//  TEMPLE SCENES: HOW THEY ARE DRAWN
// =====================================================================
//
//  Works out which temples are on at a time of day, and draws them.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include "SceneTypes.h"
#include "ClockTypes.h"
#include "Effects.h"

const uint32_t HOUR_MS = 60UL * 60 * 1000;

// Is a temple on at time of day 'ms'? If it is, 'onForMs' is how long
// it has been on, in clock time. allOn = true turns every temple on.
inline bool templeOn(const Scene &scene, const Temple &temple, uint32_t ms, bool allOn,
                     uint32_t &onForMs) {
  uint32_t cycleMs = scene.hours * HOUR_MS;
  uint32_t sinceStart = (ms % DAY_MS + DAY_MS - scene.startMs % DAY_MS) % DAY_MS;
  uint32_t inCycle = sinceStart % cycleMs;
  uint8_t hour = inCycle / HOUR_MS + 1;                       // 1 = the first hour

  bool on = (temple.hour == ALWAYS) ||
            (scene.lighting == SPOTLIGHT ? hour == temple.hour : hour >= temple.hour);
  if (on && temple.hour != ALWAYS) {
    onForMs = inCycle - (temple.hour - 1) * HOUR_MS;          // Since its hour started
    return true;
  }
  onForMs = inCycle;                                          // Since the cycle started
  return on || allOn;
}

// Draws one LED of a temple. 'onForMs' is how long the temple has been
// on, in real time.
inline CRGB drawTemple(const Temple &temple, bool on, uint32_t onForMs,
                       uint8_t led, uint16_t spot, bool steady) {
  const Look *look = temple.on;
  const Look *before = temple.off;       // The first cycle fades in from the off look
  float phase = 1.0f;                    // 1.0 = the look's last color

  if (!on) {
    look = temple.off;                   // Off: the off look's last color
  } else if (temple.cycleMs > 0) {
    uint32_t cycles = onForMs / temple.cycleMs;
    if (cycles == 0) {                   // The first cycle
      phase = (float)onForMs / (float)temple.cycleMs;
    } else if (temple.repeat == REPEAT) {  // Again: each cycle starts where the last one ended
      before = temple.on;
      phase = (float)(onForMs % temple.cycleMs) / (float)temple.cycleMs;
    } else if (temple.repeat == ONCE_OFF) {
      look = temple.off;
    }                                    // ONCE_HOLD: stays on the last color
  }
  return drawLook(*look, *before, phase, led, temple.ledCount, spot, steady);
}

// Draws every temple of a scene at time of day 'ms'.
//   speed ...... how much faster than real time the clock runs (1 = real
//                time), so looks keep their own speed in a sped-up day
//   steady ..... true turns the flicker off
//   colorTest .. true shows every temple's on look, first color, steady
//   allOn ...... true turns every temple on
inline void drawScene(const Scene &scene, uint32_t ms, uint32_t speed,
                      bool steady, bool colorTest, bool allOn) {
  uint16_t first = 0;                    // The temple's first LED on the chain
  for (uint8_t i = 0; i < scene.templeCount; i++) {
    const Temple temple = scene.temples[i].get();   // Copied out of program space
    uint32_t onForMs;
    bool on = templeOn(scene, temple, ms, allOn, onForMs);
    onForMs /= (speed > 0 ? speed : 1);
    for (uint8_t led = 0; led < temple.ledCount; led++) {
      uint16_t spot = scene.flickerSpot + (first + led) * 97;
      scene.leds[first + led] = colorTest ? firstColor(*temple.on, led, temple.ledCount)
                                          : drawTemple(temple, on, onForMs, led, spot, steady);
    }
    first += temple.ledCount;
  }
}
