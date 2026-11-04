// =====================================================================
//  POWER LIMIT
// =====================================================================
//
//  Estimates how much current a group of LEDs will draw and, if it is
//  more than its power supply should give, dims them just enough.
//
//  This is an estimate, not a measurement. Keep the fuses and current
//  limiting in your wiring: they are the real protection.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include <FastLED.h>

// ---- The limit for one power supply ----
struct PowerLimit {
  uint16_t maxMilliamps;        // The most current the LEDs may draw from this supply
  uint8_t milliampsPerColor;    // What one color (red, green or blue) of one LED
                                // draws at full. WS2812B: about 20. Check your
                                // LEDs' datasheet.
};

// What an LED draws while it is dark
const uint8_t IDLE_MILLIAMPS_PER_LED = 1;

// Estimates the current 'count' LEDs will draw at the brightness set
// with FastLED.setBrightness(), in milliamps
inline uint32_t estimateMilliamps(const CRGB *leds, uint16_t count, const PowerLimit &limit) {
  uint32_t colorSum = 0;
  for (uint16_t i = 0; i < count; i++) {
    colorSum += leds[i].r + leds[i].g + leds[i].b;
  }
  uint32_t colorMilliamps = colorSum * limit.milliampsPerColor / 255 * FastLED.getBrightness() / 255;
  return colorMilliamps + (uint32_t)count * IDLE_MILLIAMPS_PER_LED;
}

// Dims 'count' LEDs, if needed, so they stay within the limit
inline void limitPower(CRGB *leds, uint16_t count, const PowerLimit &limit) {
  uint32_t total = estimateMilliamps(leds, count, limit);
  if (total <= limit.maxMilliamps) return;
  uint32_t idle = (uint32_t)count * IDLE_MILLIAMPS_PER_LED;
  uint32_t allowed = (limit.maxMilliamps > idle) ? limit.maxMilliamps - idle : 0;
  uint8_t scale = (uint8_t)(allowed * 255 / (total - idle));
  for (uint16_t i = 0; i < count; i++) {
    leds[i].nscale8(scale);
  }
}
