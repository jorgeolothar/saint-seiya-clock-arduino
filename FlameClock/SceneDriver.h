// =====================================================================
//  SCENES: HOW THEY ARE DRAWN
// =====================================================================
//
//  Works out which groups are on at a time of day, and draws them.
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

// Is a group on at time of day 'ms'? If it is, 'onForMs' is how long
// it has been on, in clock time. allOn = true turns every group on.
inline bool groupOn(const Scene &scene, const Group &group, uint32_t ms, bool allOn,
                    uint32_t &onForMs) {
  uint32_t cycleMs = scene.hours * HOUR_MS;
  uint32_t sinceStart = (ms % DAY_MS + DAY_MS - scene.startMs % DAY_MS) % DAY_MS;
  uint32_t inCycle = sinceStart % cycleMs;
  uint8_t hour = inCycle / HOUR_MS + 1;                       // 1 = the first hour

  bool on = (group.hour == ALWAYS) ||
            (scene.lighting == SPOTLIGHT ? hour == group.hour : hour >= group.hour);
  if (on && group.hour != ALWAYS) {
    onForMs = inCycle - (group.hour - 1) * HOUR_MS;           // Since its hour started
    return true;
  }
  onForMs = inCycle;                                          // Since the cycle started
  return on || allOn;
}

// Draws one LED of a group. 'onForMs' is how long the group has been
// on, in real time. 'spot' is the LED's place in the flicker pattern, and
// 'groupSpot' the group's: a failing look fails the whole group at once.
inline CRGB drawGroup(const Group &group, bool on, uint32_t onForMs,
                      uint8_t led, uint16_t spot, uint16_t groupSpot, bool steady) {
  Look onLook = *group.on;               // The on look, with the group's own amount
  if (group.amount != LOOK_AMOUNT) onLook.amount = group.amount;
  const Look *look = &onLook;
  const Look *before = group.off;        // The first cycle fades in from the off look
  float phase = 1.0f;                    // 1.0 = the look's last color

  if (!on) {
    look = group.off;                    // Off: the off look's last color
  } else if (group.cycleMs > 0) {
    uint32_t cycles = onForMs / group.cycleMs;
    if (cycles == 0) {                   // The first cycle
      phase = (float)onForMs / (float)group.cycleMs;
    } else if (group.repeat == REPEAT) {   // Again: each cycle starts where the last one ended
      before = &onLook;
      phase = (float)(onForMs % group.cycleMs) / (float)group.cycleMs;
    } else if (group.repeat == ONCE_OFF) {
      look = group.off;
    }                                    // ONCE_HOLD: stays on the last color
  }
  return drawLook(*look, *before, phase, led, group.ledCount, spot, groupSpot, steady);
}

// Draws every group of a scene at time of day 'ms'.
//   speed ...... how much faster than real time the clock runs (1 = real
//                time), so looks keep their own speed in a sped-up day
//   steady ..... true turns the flicker and the dropouts off
//   colorTest .. true shows every group's on look, first color, steady
//   allOn ...... true turns every group on
inline void drawScene(const Scene &scene, uint32_t ms, uint32_t speed,
                      bool steady, bool colorTest, bool allOn) {
  uint16_t first = 0;                    // The group's first LED on the chain
  for (uint8_t i = 0; i < scene.groupCount; i++) {
    const Group group = scene.groups[i].get();      // Copied out of program space
    uint32_t onForMs;
    bool on = groupOn(scene, group, ms, allOn, onForMs);
    onForMs /= (speed > 0 ? speed : 1);
    uint16_t groupSpot = scene.flickerSpot + first * 97;
    for (uint8_t led = 0; led < group.ledCount; led++) {
      uint16_t spot = groupSpot + led * 97;
      scene.leds[first + led] = colorTest ? firstColor(*group.on, led, group.ledCount)
                                          : drawGroup(group, on, onForMs, led, spot, groupSpot, steady);
    }
    first += group.ledCount;
  }
}
