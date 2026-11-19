// =====================================================================
//  FLAME CLOCK FOR THE ZODIAC CLOCK TOWER
// =====================================================================
//
//  WHAT IT DOES
//  Each ring of 12 LEDs is a clock face. A flickering "flame" moves
//  around it, one LED at a time. Each ring can show:
//    Hours ...... one LED per hour
//    Minutes .... one LED per 5 minutes
//    Seconds .... one LED per 5 seconds
//
//  The flame goes around twice, with a different flame each time:
//    Hours ...... one flame from midnight to noon, another from noon to midnight
//    Minutes .... one flame in even hours (12, 2, 4 ...), another in odd hours
//    Seconds .... one flame in even minutes, another in odd minutes
//
//  As the flame reaches an LED, it slowly lights up. Then it fades into
//  its "trail", the color it leaves behind, and moves on. A black trail
//  leaves the ring dark behind the flame.
//
//  THE BUTTON
//    Short press ...... turns the clock on or off. With scenes, it moves
//                       to the next of these, then back to the first:
//                         Normal ...... the clock, and the scenes in
//                                       their hours
//                         Tour ........ a whole day, midnight to
//                                       midnight, in 2 minutes: hours
//                                       rings and every scene. Then back
//                                       to Normal by itself.
//                         Showcase .... every group of every scene on
//                         Clock only .. the scenes off
//                         Off ......... everything off
//    Hold 1 second .... changes the flame you see moving to the next one.
//                       Each ring then shows the flame's number like the
//                       hours on a clock: flame 3 lights 1, 2 and 3 o'clock.
//                       Hold again for the next one.
//    The clock remembers the flame and the Normal, Showcase or Clock only
//    choice, even when unplugged.
//
//  SCENES (optional, all switched off)
//  The Arduino can also light scenes: groups of LEDs, like the temples
//  of the Sanctuary, Asgard, Poseidon and Hades, that each light up in
//  their hour. Set them up in the Scenes.h tab. See SCENES.md.
//
//  WIRING (Arduino Nano or Uno)
//    LED ring ............ 5V to 5V, GND to GND, DIN to pin D6
//                          (ring 2 to D7, ring 3 to D8, ring 4 to D9)
//    Scenes .............. DIN to pin D10 (Sanctuary), D11 (Asgard),
//                          D12 (Poseidon), A0 (Hades). Each scene on its
//                          own 5V power supply, with its GND joined to the
//                          Arduino's GND.
//    Button .............. one leg to pin D2, the other leg to GND
//    Clock module (RTC) .. optional. SDA to A4, SCL to A5, VCC to 5V, GND to GND
//
//  LIBRARIES TO INSTALL
//  In the Arduino IDE: Sketch > Include Library > Manage Libraries
//    FastLED ....... always needed
//    RTClib ........ only for a clock module without a DS3231 chip (see
//                    the clock module notes in PART 4)
//
//  FILES
//  Keep these files together in a folder called FlameClock. The Arduino
//  IDE shows them as tabs.
//    FlameClock.ino ... this file: your clocks and settings
//    Flames.h ......... the clock's flames
//    Scenes.h ......... the scenes
//    SceneLooks.h ..... the scenes' looks
//    Colors.h ......... the named colors the flames and looks use
//  The building blocks (no need to open them):
//    LookTypes.h ...... what a look (a flame) is
//    ClockTypes.h ..... what a clock (a ring) is
//    SceneTypes.h ..... what a group and a scene are
//    Effects.h ........ how looks are drawn
//    SceneDriver.h .... how scenes are drawn
//    Power.h .......... the power limit
//    ClockModule.h .... reads and sets the clock module's time
//
// =====================================================================

// Do not change these lines
#include <FastLED.h>
#include <EEPROM.h>
#include "ClockTypes.h"
#include "Flames.h"
#include "Effects.h"
#include "Power.h"
#include "Scenes.h"
#include "SceneDriver.h"

// =====================================================================
//  PART 1: YOUR CLOCKS
//  Write one line for each LED ring (1 to 4 rings), ring 1 first:
//
//    { what it shows, 1st flame, 2nd flame, top LED },
//
//    what it shows .. SHOW_HOURS, SHOW_MINUTES or SHOW_SECONDS
//    1st flame ...... the flame for midnight to noon, even hours, or even
//                     minutes. Use any flame name from the Flames.h tab.
//    2nd flame ...... the flame for the other half. It can be the same one.
//    top LED ........ which LED is at 12 o'clock. Usually 1. If the ring
//                     is turned, see "Finding the top LED" in PART 2.
//
//  The two lines with slashes in front `//` are extra rings, just for show. To
//  use one, remove its two slashes and wire that ring (ring 2 to D7,
//  ring 3 to D8).
// =====================================================================

const Clock CLOCKS[] = {
  // Ring 1, hours: the Sanctuary fire clock. In the morning a white flame
  // lights the ring blue, one LED per hour. In the afternoon the blue
  // flames burn out, one per hour, like the countdown through the twelve
  // zodiac temples.
  { SHOW_HOURS,   COOL,         EMBER,         1 },

  // Ring 2, minutes: a bright spark jumps one LED every 5 minutes and
  // leaves no trail.
  // { SHOW_MINUTES, SPARK,        SPARK,         1 },

  // Ring 3, seconds: in even minutes a rainbow fills the ring, one LED
  // every 5 seconds. In odd minutes each color burns out to black. This
  // ring is mounted turned, so its top LED is 4.
  // { SHOW_SECONDS, RAINBOW_GLOW, RAINBOW_EMBER, 4 },
};

// Turn on by itself when plugged in? true = yes, false = wait for the button
const bool START_ON = true;

// Do you have a clock module (RTC)? It keeps the real time, even
// while the clock is unplugged.
//   No ... leave the next line as it is. The clock starts at START_TIME
//          (PART 2) every time it is plugged in.
//   Yes .. remove the two slashes at the start of the next line, and read
//          the clock module notes in PART 4.
// #define USE_RTC

// Dim the LEDs at night? From NIGHT_START to MORNING_START, the rings
// and scenes dim to this % of their brightness (BRIGHTNESS_PERCENT, in
// PART 3), over half an hour.
//   100 ... no dimming (the night setting is off)
//   30 .... 30% of the brightness at night
const uint8_t NIGHT_PERCENT = 100;
const uint32_t NIGHT_START = clockTime(19, 0, 0);     // 7 in the evening
const uint32_t MORNING_START = clockTime(7, 0, 0);    // 7 in the morning

// =====================================================================
//  PART 2: TESTING
//  Handy for checking colors and timing. Set BEHAVIOR back to
//  MODE_REGULAR when you are done.
// =====================================================================

// What should the clock do?
//   MODE_REGULAR ....... show the time (the normal setting)
//   MODE_ACCELERATED ... a sped-up day starting at START_TIME, to watch a
//                        whole day go by in minutes
//   MODE_COLOR_TEST .... hold still and show the colors of both flames:
//                          12 o'clock ....... 1st flame
//                          1 to 5 o'clock ... 2nd flame's trail
//                          6 o'clock ........ 2nd flame
//                          7 to 11 o'clock .. 1st flame's trail
//   MODE_FREEZE ........ hold still at FREEZE_TIME, to look closely at
//                        the colors
const Behavior BEHAVIOR = MODE_REGULAR;

// Finding the top LED: use MODE_COLOR_TEST with top LED 1. The single
// LED in the 1st flame's color should be at 12 o'clock. If it is at
// another hour, set top LED to 13 minus that hour (at 3 o'clock: 10).

// Times are written clockTime(hour, minute, second), on a 24 hour clock:
//   clockTime(0, 0, 0) ....... midnight
//   clockTime(15, 45, 0) ..... 3:45 in the afternoon

// Where the clock starts when there is no clock module, and where
// MODE_ACCELERATED starts
const uint32_t START_TIME = clockTime(0, 0, 0);

// Where MODE_FREEZE stops. This time shows every kind of ring with its
// flame at full color.
const uint32_t FREEZE_TIME = clockTime(3, 32, 32.5);

// How much faster MODE_ACCELERATED runs:
//     60 = a whole day in 24 minutes
//    600 = a whole day in 2.4 minutes
// Minutes and seconds rings are a blur when sped up a lot; try 10.
const uint32_t ACCELERATED_SPEED = 60;

// Show the time and colors on the computer? Remove the two slashes,
// upload, and open Tools > Serial Monitor (set to 115200 baud). Handy
// when asking for help. Leave it off otherwise: it takes memory.
// #define PRINT_TO_SERIAL

// The scenes follow these modes too: MODE_ACCELERATED runs through
// their hours faster (their looks keep their own speed),
// MODE_COLOR_TEST shows every group's look, first color, and
// MODE_FREEZE holds them still. ALL_GROUPS_ON in Scenes.h turns every
// group on.

// =====================================================================
//  PART 3: MORE SETTINGS
//  Most people never need to change these.
// =====================================================================

// Brightness of all the LEDs, in %: 0 = off, 100 = full. Keep it low when
// the clock is powered from a computer's USB port. Every color is dimmed
// by it, so a dim color dims even more.
const uint8_t BRIGHTNESS_PERCENT = 25;
static_assert(BRIGHTNESS_PERCENT <= 100, "BRIGHTNESS_PERCENT goes up to 100");

// The most current the rings may draw, so the clock dims itself instead
// of asking too much from its power supply:
//   { most milliamps, milliamps per color }
//   most milliamps ........ the rings share the Arduino's 5V. From a
//                           computer's USB port, keep it at 400 or less.
//   milliamps per color ... what red, green or blue of one LED draws at
//                           full. WS2812B: about 20.
// This is an estimate, not a measurement: keep your fuses.
const PowerLimit RING_POWER = { 400, 20 };

// How fast flames flicker, in %: 100 = normal, 200 = twice as fast,
// 50 = half as fast
const uint16_t FLICKER_SPEED_PERCENT = 100;
static_assert(FLICKER_SPEED_PERCENT >= 1 && FLICKER_SPEED_PERCENT <= 1000,
              "FLICKER_SPEED_PERCENT goes from 1 to 1000");

// How often the LEDs are updated, in thousandths of a second
const unsigned long FRAME_MS = 30;

// The button
#define BUTTON_PIN 2                          // The pin the button is on
const unsigned long LONG_PRESS_MS = 1000;     // How long to hold it (1000 = 1 second)
const unsigned long DEBOUNCE_MS = 40;         // Ignore shorter presses (button noise)

// How long the button's tour of a whole day takes, in seconds
const uint16_t TOUR_SECONDS = 120;

// Remember the flames and lights picked with the button when unplugged?
// true = yes, false = no. Uploading a changed sketch always goes back to
// the flames in PART 1, and to Normal.
const bool REMEMBER_BUTTON_CHOICES = true;

// The LED rings
#define COLOR_ORDER GRB     // If red and green look swapped, change to RGB
#define RING1_PIN 6         // The pin for ring 1
#define RING2_PIN 7         // The pin for ring 2
#define RING3_PIN 8         // The pin for ring 3
#define RING4_PIN 9         // The pin for ring 4

// The scenes' pins, chips and power limits are in Scenes.h

// =====================================================================
//  PART 4: NOTES
// =====================================================================
//
//  CLOCK MODULE NOTES (only if you use one)
//  - The first time it is used, the clock module is set to your
//    computer's time at the moment you upload. Check your computer's
//    clock first.
//  - Its battery keeps the time while the clock is unplugged.
//  - It does not change for daylight saving time. To set the time again:
//    remove the two slashes from "// #define SET_RTC_NOW" below, upload,
//    then put the slashes back and upload again. (Without the second
//    upload, it goes back to the upload time every time it is plugged in.)
//  - Battery: most blue DS3231 modules are made for a rechargeable
//    LIR2032 battery. A regular CR2032 can leak or overheat in them.
//    Check your module's instructions.
//
// #define SET_RTC_NOW
//
//  - The chip: its name is printed on the module's biggest chip.
//    DS3231 (the most common), DS1307, PCF8523 or PCF8563.
//
#define RTC_CHIP DS3231
//
//  - The DS3231 needs no library: the sketch has its own code for it,
//    which takes far less program space. The other chips need the
//    RTClib library (by Adafruit), installed from the Library Manager.
//    To use RTClib for a DS3231 too (for example on a board that isn't
//    an Uno or a Nano), remove the two slashes:
//
// #define USE_RTCLIB
//
//  TROUBLESHOOTING
//  - Nothing lights up: check the 5V, GND and DIN wires (DIN goes to pin
//    D6 for ring 1). Press the button, in case the clock is off.
//  - The scenes stay dark: press the button until they come back. Clock
//    only and Off turn them off.
//  - Red and green look swapped: change COLOR_ORDER to RGB (PART 3).
//  - The first LED keeps blinking red: the clock module was not found.
//    Check its SDA (A4) and SCL (A5) wires, or put the two slashes back
//    in front of "#define USE_RTC".
//  - The flame starts in the wrong place: see "Finding the top LED"
//    (PART 2).
//  - Holding the button does nothing: hold it a full second.
//  - A flame you changed in PART 1 does not show: the clock remembers
//    the flame picked with the button. Uploading the changed sketch
//    clears that.
//
// =====================================================================
//  PART 5: THE PROGRAM
//  Nothing below this line needs to be changed.
// =====================================================================

#include "ClockModule.h"

#define MAX_RINGS 4
#define LED_PER_RING 12   // The clock face needs exactly 12 LEDs per ring

const int NUM_CLOCKS = sizeof(CLOCKS) / sizeof(CLOCKS[0]);
const int FLAME_COUNT = sizeof(FLAME_LIST) / sizeof(FLAME_LIST[0]);

static_assert(NUM_CLOCKS >= 1 && NUM_CLOCKS <= MAX_RINGS, "CLOCKS must list 1, 2, 3 or 4 rings");
static_assert(FLAME_COUNT >= 1 && FLAME_COUNT <= LED_PER_RING, "FLAME_LIST must hold 1 to 12 flames");


// ---------------------------------------------------------------------
//  Program state
// ---------------------------------------------------------------------
CRGB leds[MAX_RINGS][LED_PER_RING];         // One row of LEDs per ring

// The scenes switched on in Scenes.h: their LEDs, and where each
// one starts in the flicker pattern
#ifdef USE_DEMO
CRGB demoLeds[ledsIn(DEMO)];
const Scene demo(SCENE_GROUPS(DEMO), demoLeds, DEMO_LIGHTING, DEMO_START, DEMO_POWER, 36000);
static_assert(cycleFitsADay(DEMO, DEMO_LIGHTING), "DEMO (Scenes.h): a cycle can be up to 24 hours. With FILL_AND_EMPTY, the highest hour can be up to 12");
#endif
#ifdef USE_SANCTUARY
CRGB sanctuaryLeds[ledsIn(SANCTUARY)];
const Scene sanctuary(SCENE_GROUPS(SANCTUARY), sanctuaryLeds, SANCTUARY_LIGHTING, SANCTUARY_START, SANCTUARY_POWER, 4000);
static_assert(cycleFitsADay(SANCTUARY, SANCTUARY_LIGHTING), "SANCTUARY (Scenes.h): a cycle can be up to 24 hours. With FILL_AND_EMPTY, the highest hour can be up to 12");
#endif
#ifdef USE_ASGARD
CRGB asgardLeds[ledsIn(ASGARD)];
const Scene asgard(SCENE_GROUPS(ASGARD), asgardLeds, ASGARD_LIGHTING, ASGARD_START, ASGARD_POWER, 12000);
static_assert(cycleFitsADay(ASGARD, ASGARD_LIGHTING), "ASGARD (Scenes.h): a cycle can be up to 24 hours. With FILL_AND_EMPTY, the highest hour can be up to 12");
#endif
#ifdef USE_POSEIDON
CRGB poseidonLeds[ledsIn(POSEIDON)];
const Scene poseidon(SCENE_GROUPS(POSEIDON), poseidonLeds, POSEIDON_LIGHTING, POSEIDON_START, POSEIDON_POWER, 20000);
static_assert(cycleFitsADay(POSEIDON, POSEIDON_LIGHTING), "POSEIDON (Scenes.h): a cycle can be up to 24 hours. With FILL_AND_EMPTY, the highest hour can be up to 12");
#endif
#ifdef USE_HADES
CRGB hadesLeds[ledsIn(HADES)];
const Scene hades(SCENE_GROUPS(HADES), hadesLeds, HADES_LIGHTING, HADES_START, HADES_POWER, 28000);
static_assert(cycleFitsADay(HADES, HADES_LIGHTING), "HADES (Scenes.h): a cycle can be up to 24 hours. With FILL_AND_EMPTY, the highest hour can be up to 12");
#endif

const Scene *const SCENES[] = {
#ifdef USE_DEMO
  &demo,
#endif
#ifdef USE_SANCTUARY
  &sanctuary,
#endif
#ifdef USE_ASGARD
  &asgard,
#endif
#ifdef USE_POSEIDON
  &poseidon,
#endif
#ifdef USE_HADES
  &hades,
#endif
  nullptr   // The end of the list
};
const int NUM_SCENES = sizeof(SCENES) / sizeof(SCENES[0]) - 1;

// Each pin can do only one job. With two chains on one pin, only one of
// them would light, so the sketch doesn't compile if two share a pin.
const uint8_t UNUSED_PIN = 255;    // A ring that isn't in CLOCKS
constexpr uint8_t PINS_IN_USE[] = {
  BUTTON_PIN,
  RING1_PIN,
  NUM_CLOCKS >= 2 ? RING2_PIN : UNUSED_PIN,
  NUM_CLOCKS >= 3 ? RING3_PIN : UNUSED_PIN,
  NUM_CLOCKS >= 4 ? RING4_PIN : UNUSED_PIN,
#ifdef USE_RTC
  A4, A5,           // The clock module's SDA and SCL
#endif
#ifdef USE_DEMO
  DEMO_PIN,
#endif
#ifdef USE_SANCTUARY
  SANCTUARY_PIN,
#endif
#ifdef USE_ASGARD
  ASGARD_PIN,
#endif
#ifdef USE_POSEIDON
  POSEIDON_PIN,
#endif
#ifdef USE_HADES
  HADES_PIN,
#endif
};
constexpr int timesUsed(uint8_t pin, size_t from = 0) {
  return from == sizeof(PINS_IN_USE) ? 0 : (PINS_IN_USE[from] == pin) + timesUsed(pin, from + 1);
}
static_assert(timesUsed(RING1_PIN) == 1, "RING1_PIN (PART 3) shares its pin with the button, another ring or a scene");
static_assert(NUM_CLOCKS < 2 || timesUsed(RING2_PIN) == 1, "RING2_PIN (PART 3) shares its pin with the button, another ring or a scene");
static_assert(NUM_CLOCKS < 3 || timesUsed(RING3_PIN) == 1, "RING3_PIN (PART 3) shares its pin with the button, another ring or a scene");
static_assert(NUM_CLOCKS < 4 || timesUsed(RING4_PIN) == 1, "RING4_PIN (PART 3) shares its pin with the button, another ring or a scene");
#ifdef USE_DEMO
static_assert(timesUsed(DEMO_PIN) == 1, "DEMO_PIN (Scenes.h) shares its pin with a ring, the button, the clock module or another scene");
#endif
#ifdef USE_SANCTUARY
static_assert(timesUsed(SANCTUARY_PIN) == 1, "SANCTUARY_PIN (Scenes.h) shares its pin with a ring, the button, the clock module or another scene");
#endif
#ifdef USE_ASGARD
static_assert(timesUsed(ASGARD_PIN) == 1, "ASGARD_PIN (Scenes.h) shares its pin with a ring, the button, the clock module or another scene");
#endif
#ifdef USE_POSEIDON
static_assert(timesUsed(POSEIDON_PIN) == 1, "POSEIDON_PIN (Scenes.h) shares its pin with a ring, the button, the clock module or another scene");
#endif
#ifdef USE_HADES
static_assert(timesUsed(HADES_PIN) == 1, "HADES_PIN (Scenes.h) shares its pin with a ring, the button, the clock module or another scene");
#endif

static_assert(NIGHT_PERCENT <= 100, "NIGHT_PERCENT (PART 1) goes up to 100");
static_assert(TOUR_SECONDS >= 1, "TOUR_SECONDS (PART 3) must be at least 1");
const uint32_t TOUR_MS = TOUR_SECONDS * 1000UL;

// The two flames each ring shows right now. They start as written in
// PART 1, and the long press changes them.
const Look *currentFlame[MAX_RINGS][2];

Lights lights = LIGHTS_OFF;                 // What the button picked (setup picks the first)
Lights savedLights = LIGHTS_NORMAL;         // The last Normal, Showcase or Clock only
unsigned long tourStart = 0;                // When the tour started (millis)

uint32_t countedTime = START_TIME % DAY_MS; // Time counted without an RTC (ms since midnight)
unsigned long lastTick = 0;                 // When countedTime was last moved on

bool buttonDown = false;                    // Is the button being held?
bool longPressDone = false;                 // Was the flame already changed during this hold?
unsigned long pressStartTime = 0;           // When the current press started

// ---------------------------------------------------------------------
//  TIME: milliseconds since midnight
// ---------------------------------------------------------------------
#ifdef USE_RTC
// Real time from the RTC. The RTC only counts whole seconds, so the
// Arduino's own timer fills in the part of a second to keep fades smooth.
uint32_t rtcTimeOfDay() {
  static uint8_t lastSecond = 255;
  static unsigned long secondStarted = 0;
  static ClockTime t = { 0, 0, 0 };
  clockModuleTime(t);                              // A failed read keeps the last time
  if (t.second != lastSecond) {
    lastSecond = t.second;
    secondStarted = millis();
  }
  unsigned long partOfSecond = millis() - secondStarted;
  if (partOfSecond > 999) partOfSecond = 999;
  return (t.hour * 3600UL + t.minute * 60UL + t.second) * 1000UL + partOfSecond;
}
#endif

uint32_t timeOfDay() {
  if (BEHAVIOR == MODE_FREEZE) return FREEZE_TIME % DAY_MS;
#ifdef USE_RTC
  if (BEHAVIOR != MODE_ACCELERATED) return rtcTimeOfDay();
#endif
  // No RTC (or accelerated): move the counted time on by the time passed
  unsigned long now = millis();
  uint32_t speed = (BEHAVIOR == MODE_ACCELERATED) ? ACCELERATED_SPEED : 1;
  countedTime = (countedTime + (now - lastTick) * speed) % DAY_MS;
  lastTick = now;
  return countedTime;
}

// Length of one step (one LED) in milliseconds
uint32_t stepLength(Unit unit) {
  switch (unit) {
    case SHOW_MINUTES: return 5UL * 60 * 1000;    // 5 minutes
    case SHOW_SECONDS: return 5UL * 1000;         // 5 seconds
    default:           return 60UL * 60 * 1000;   // 1 hour
  }
}

// Where a ring's flame is at a time of day:
//   pass: 0 = 1st flame, 1 = 2nd flame
//   position: 0 to 11, the place on the clock face (0 = 12 o'clock)
//   progress: 0.0 when the flame arrives on the LED, up to 1.0 when it leaves
void findFlame(int ring, uint32_t ms, int &pass, int &position, float &progress) {
  uint32_t step = stepLength(CLOCKS[ring].unit);
  uint32_t inTwoLaps = ms % (step * 2 * LED_PER_RING);
  int stepNumber = inTwoLaps / step;
  pass = stepNumber / LED_PER_RING;
  position = stepNumber % LED_PER_RING;
  progress = (float)(inTwoLaps % step) / (float)step;
}

// Turns a place on the clock face (0 = 12 o'clock) into the LED's number
// on the ring, starting from the ring's top LED
int ringLed(int ring, int position) {
  return (CLOCKS[ring].topLed + LED_PER_RING - 1 + position) % LED_PER_RING;
}

// ---------------------------------------------------------------------
//  SHOWING THE LEDS
// ---------------------------------------------------------------------
// Sends the colors to the LEDs, dimmed if needed to stay within each
// power supply's limit (PART 3)
void showLeds() {
  limitPower(leds[0], NUM_CLOCKS * LED_PER_RING, RING_POWER);
  for (int i = 0; i < NUM_SCENES; i++) {
    limitPower(SCENES[i]->leds, SCENES[i]->ledCount, SCENES[i]->power);
  }
  FastLED.show();
}

// ---------------------------------------------------------------------
//  DRAWING ONE RING
// ---------------------------------------------------------------------
#ifdef PRINT_TO_SERIAL
// Prints the time and ring 1's flame color (before flicker) about once per second
void printFlame(uint32_t ms, int pass, int position, CRGB c) {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 1000) return;
  lastPrint = millis();
  uint32_t seconds = ms / 1000;
  int parts[3] = { (int)(seconds / 3600), (int)(seconds / 60 % 60), (int)(seconds % 60) };
  Serial.print(F("time "));
  for (int i = 0; i < 3; i++) {
    if (i > 0) Serial.print(':');
    if (parts[i] < 10) Serial.print('0');
    Serial.print(parts[i]);
  }
  Serial.print(pass == 0 ? F("  1st flame") : F("  2nd flame"));
  Serial.print(F(" at "));
  Serial.print(position == 0 ? 12 : position);
  Serial.print(F(" o'clock  RGB = "));
  Serial.print(c.r);
  Serial.print(F(", "));
  Serial.print(c.g);
  Serial.print(F(", "));
  Serial.println(c.b);
}
#endif

// One flame moves around the ring:
//   Behind the flame: its own trail (the last color of its list).
//   Ahead of the flame: the trail the other flame left on its lap.
//   The flame itself fades in from that old trail through its color
//   list, and ends on its own trail.
// steady = true turns the flicker (and a failing look's dropouts) off.
void drawClock(int ring, uint32_t ms, bool steady) {
  int pass, flamePosition;
  float progress;
  findFlame(ring, ms, pass, flamePosition, progress);
  const Look &now = *currentFlame[ring][pass];          // The flame moving now
  const Look &before = *currentFlame[ring][1 - pass];   // The flame of the last lap

  uint16_t ringSpot = ring * 523;                       // A failing flame fails the whole ring at once
  for (int position = 0; position < LED_PER_RING; position++) {
    uint16_t spot = position * 97 + ringSpot;           // The LED's place in the flicker pattern
    CRGB color;
    if (position < flamePosition) {                     // Behind: this flame's trail
      color = drawLook(now, now, 1.0f, position, LED_PER_RING, spot, ringSpot, steady);
    } else if (position > flamePosition) {              // Ahead: the last lap's trail
      color = drawLook(before, before, 1.0f, position, LED_PER_RING, spot, ringSpot, steady);
    } else {                                            // The flame, taking over from the old trail
      color = drawLook(now, before, progress, position, LED_PER_RING, spot, ringSpot, steady);
#ifdef PRINT_TO_SERIAL
      if (ring == 0) {
        printFlame(ms, pass, position, drawLook(now, before, progress, position, LED_PER_RING, spot, ringSpot, true));
      }
#endif
    }
    leds[ring][ringLed(ring, position)] = color;
  }
}

// MODE_COLOR_TEST: 12 = 1st flame, 1-5 = 2nd trail, 6 = 2nd flame, 7-11 = 1st trail
// (the flame is the first color of its list, the trail the last)
void drawColorTest(int ring) {
  const Look &first = *currentFlame[ring][0];
  const Look &second = *currentFlame[ring][1];
  for (int position = 0; position < LED_PER_RING; position++) {
    CRGB color;
    if (position == 0)      color = firstColor(first, position, LED_PER_RING);
    else if (position < 6)  color = endColor(second, position, LED_PER_RING);
    else if (position == 6) color = firstColor(second, position, LED_PER_RING);
    else                    color = endColor(first, position, LED_PER_RING);
    leds[ring][ringLed(ring, position)] = color;
  }
}

// ---------------------------------------------------------------------
//  THE TOUR: a whole day, from midnight, in TOUR_SECONDS
// ---------------------------------------------------------------------
const uint32_t TOUR_SPEED = DAY_MS / TOUR_MS;   // How much faster than real time (720 for 2 minutes)

void startTour() {
  tourStart = millis();
}

// The time of day the tour shows
uint32_t tourTime() {
  uint32_t into = millis() - tourStart;
  if (into >= TOUR_MS) into = TOUR_MS - 1;      // The last moment, until the tour ends
  return into * TOUR_SPEED;
}

// The time a ring shows: during the tour, hours rings go round the
// day with the scenes. Minutes and seconds rings keep the time.
uint32_t ringTime(int ring, uint32_t ms) {
  if (lights != LIGHTS_TOUR || CLOCKS[ring].unit != SHOW_HOURS) return ms;
  return tourTime();
}

// The time a scene shows, and how much faster than real time it runs
uint32_t sceneTime(const Scene &, uint32_t ms, uint32_t &speed) {
  if (lights != LIGHTS_TOUR) return ms;
  speed = TOUR_SPEED;
  return tourTime();
}

// ---------------------------------------------------------------------
//  NIGHT: dimmer from NIGHT_START to MORNING_START (PART 1)
// ---------------------------------------------------------------------
const uint32_t NIGHT_FADE_MS = 30UL * 60 * 1000;   // Half an hour to dim, and to brighten

// How much of the night it is at time of day 'ms', from 0 (day) to 255
// (night): it dims over the half hour after NIGHT_START, and brightens
// over the half hour before MORNING_START
uint8_t nightLevel(uint32_t ms) {
  uint32_t length = (MORNING_START % DAY_MS + DAY_MS - NIGHT_START % DAY_MS) % DAY_MS;
  uint32_t since = (ms % DAY_MS + DAY_MS - NIGHT_START % DAY_MS) % DAY_MS;
  if (since >= length) return 0;                   // Day
  uint32_t edge = (since < length - since) ? since : length - since;
  return edge >= NIGHT_FADE_MS ? 255 : edge * 255 / NIGHT_FADE_MS;
}

// The brightness for a night level (0 = day, 255 = night)
uint8_t nightBrightness(uint8_t level) {
  uint16_t full = BRIGHTNESS_PERCENT * 255 / 100;
  uint16_t night = (full * NIGHT_PERCENT + 50) / 100;
  return full - (uint32_t)(full - night) * level / 255;
}

// Draws every ring and every scene
void updateDisplay() {
  if (lights == LIGHTS_TOUR && millis() - tourStart >= TOUR_MS) lights = LIGHTS_NORMAL;   // The tour is over
  uint32_t ms = timeOfDay();
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    if (BEHAVIOR == MODE_COLOR_TEST) drawColorTest(ring);
    else drawClock(ring, ringTime(ring, ms), BEHAVIOR == MODE_FREEZE);   // No flicker while frozen
  }
  for (int i = 0; i < NUM_SCENES; i++) {
    const Scene &scene = *SCENES[i];
    if (lights == LIGHTS_CLOCK_ONLY) {
      for (uint16_t led = 0; led < scene.ledCount; led++) scene.leds[led] = CRGB(0, 0, 0);
      continue;
    }
    uint32_t speed = (BEHAVIOR == MODE_ACCELERATED) ? ACCELERATED_SPEED : 1;
    uint32_t at = sceneTime(scene, ms, speed);
    drawScene(scene, at, speed, BEHAVIOR == MODE_FREEZE, BEHAVIOR == MODE_COLOR_TEST,
              ALL_GROUPS_ON || lights == LIGHTS_SHOWCASE);
  }
  bool colorCheck = (BEHAVIOR == MODE_COLOR_TEST || BEHAVIOR == MODE_FREEZE);   // Never dimmed
  FastLED.setBrightness(nightBrightness(colorCheck ? 0 : nightLevel(ms)));
  showLeds();
}

void clearDisplay() {
  FastLED.clear();
  FastLED.show();
}

#ifdef USE_RTC
// Blinks the first LED red forever, to tell you the RTC was not found
void showRtcError() {
  while (true) {
    leds[0][0] = CRGB::Red;
    showLeds();
    delay(300);
    leds[0][0] = CRGB::Black;
    showLeds();
    delay(300);
  }
}
#endif

// ---------------------------------------------------------------------
//  REMEMBERING THE BUTTON'S CHOICES
// ---------------------------------------------------------------------
// The choices are stored in the Arduino's EEPROM memory, which keeps its
// contents without power. A number made from the upload time is stored
// with them, so a new upload starts again from the flames in PART 1, and
// from Normal.
struct SavedChoice {
  uint16_t uploadId;
  uint8_t flameNumber[MAX_RINGS][2];   // Place in FLAME_LIST, 255 = as written in PART 1
  uint8_t lights;                      // LIGHTS_NORMAL, LIGHTS_SHOWCASE or LIGHTS_CLOCK_ONLY
};

uint16_t uploadId() {
  const char *text = __DATE__ __TIME__;
  uint16_t id = 0;
  while (*text) id = id * 31 + *text++;
  return id;
}

// A flame's place in FLAME_LIST (0 = first), or -1 if it is not in the list
int flameNumber(const Look *flame) {
  for (int i = 0; i < FLAME_COUNT; i++) {
    if (FLAME_LIST[i] == flame) return i;
  }
  return -1;
}

void saveChoice() {
  if (!REMEMBER_BUTTON_CHOICES) return;
  SavedChoice saved;
  saved.uploadId = uploadId();
  for (int ring = 0; ring < MAX_RINGS; ring++) {
    for (int pass = 0; pass < 2; pass++) {
      saved.flameNumber[ring][pass] = (ring < NUM_CLOCKS) ? (uint8_t)flameNumber(currentFlame[ring][pass]) : 255;
    }
  }
  saved.lights = savedLights;
  EEPROM.put(0, saved);   // Only writes the bytes that changed
}

void loadChoice() {
  if (!REMEMBER_BUTTON_CHOICES) return;
  SavedChoice saved;
  EEPROM.get(0, saved);
  if (saved.uploadId != uploadId()) return;   // New upload: keep the flames from PART 1, and Normal
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    for (int pass = 0; pass < 2; pass++) {
      uint8_t number = saved.flameNumber[ring][pass];
      if (number < FLAME_COUNT) currentFlame[ring][pass] = FLAME_LIST[number];
    }
  }
  if (NUM_SCENES > 0 && (saved.lights == LIGHTS_SHOWCASE || saved.lights == LIGHTS_CLOCK_ONLY)) {
    savedLights = (Lights)saved.lights;
  }
}

// ---------------------------------------------------------------------
//  LONG PRESS: next flame
// ---------------------------------------------------------------------
// Blinks each ring's flame number twice, like the hours on a clock:
// flame 3 lights the LEDs at 1, 2 and 3 o'clock, in that flame's color.
void showFlameNumbers(const Look *const flame[], const int number[]) {
  for (int blink = 0; blink < 2; blink++) {
    FastLED.clear();
    for (int ring = 0; ring < NUM_CLOCKS; ring++) {
      for (int hour = 1; hour <= number[ring]; hour++) {
        int position = hour % LED_PER_RING;
        leds[ring][ringLed(ring, position)] = firstColor(*flame[ring], position, LED_PER_RING);
      }
    }
    showLeds();
    delay(400);
    clearDisplay();
    delay(200);
  }
}

// The flame moving right now on each ring becomes the next one in FLAME_LIST
void nextFlame() {
  uint32_t ms = timeOfDay();
  const Look *shown[MAX_RINGS];
  int number[MAX_RINGS];
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    int pass, position;
    float progress;
    findFlame(ring, ringTime(ring, ms), pass, position, progress);   // The flame on show, in the tour too
    int next = (flameNumber(currentFlame[ring][pass]) + 1) % FLAME_COUNT;
    currentFlame[ring][pass] = FLAME_LIST[next];
    shown[ring] = FLAME_LIST[next];
    number[ring] = next + 1;
  }
  saveChoice();
  showFlameNumbers(shown, number);
}

// ---------------------------------------------------------------------
//  BUTTON: short press = next lights, long press = next flame
// ---------------------------------------------------------------------
void nextLightsPressed() {
  lights = nextLights(lights, NUM_SCENES > 0);
  if (lights == LIGHTS_TOUR) startTour();
  if (lights == LIGHTS_OFF) clearDisplay();
  if (lights == LIGHTS_NORMAL || lights == LIGHTS_SHOWCASE || lights == LIGHTS_CLOCK_ONLY) {
    savedLights = lights;                          // Remembered when unplugged
    saveChoice();
  }
}

void handleButton() {
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();

  if (pressed && !buttonDown) {                    // Button was just pressed
    buttonDown = true;
    longPressDone = false;
    pressStartTime = now;
  }
  else if (pressed && buttonDown && !longPressDone && now - pressStartTime >= LONG_PRESS_MS) {
    longPressDone = true;                          // Held long enough: next flame
    nextFlame();
    if (lights == LIGHTS_OFF) clearDisplay();
  }
  else if (!pressed && buttonDown) {               // Button was released
    buttonDown = false;
    if (!longPressDone && now - pressStartTime >= DEBOUNCE_MS) {
      nextLightsPressed();                         // Short press: the next lights
    }
  }
}

// ---------------------------------------------------------------------
//  ARDUINO SETUP AND LOOP
// ---------------------------------------------------------------------
void setup() {
#ifdef PRINT_TO_SERIAL
  Serial.begin(115200);
#endif

  // One data pin per ring
  FastLED.addLeds<WS2812B, RING1_PIN, COLOR_ORDER>(leds[0], LED_PER_RING);
  if (NUM_CLOCKS >= 2) FastLED.addLeds<WS2812B, RING2_PIN, COLOR_ORDER>(leds[1], LED_PER_RING);
  if (NUM_CLOCKS >= 3) FastLED.addLeds<WS2812B, RING3_PIN, COLOR_ORDER>(leds[2], LED_PER_RING);
  if (NUM_CLOCKS >= 4) FastLED.addLeds<WS2812B, RING4_PIN, COLOR_ORDER>(leds[3], LED_PER_RING);
  // One data pin per scene
#ifdef USE_DEMO
  FastLED.addLeds<DEMO_CHIP, DEMO_PIN, DEMO_ORDER>(demoLeds, demo.ledCount);
#endif
#ifdef USE_SANCTUARY
  FastLED.addLeds<SANCTUARY_CHIP, SANCTUARY_PIN, SANCTUARY_ORDER>(sanctuaryLeds, sanctuary.ledCount);
#endif
#ifdef USE_ASGARD
  FastLED.addLeds<ASGARD_CHIP, ASGARD_PIN, ASGARD_ORDER>(asgardLeds, asgard.ledCount);
#endif
#ifdef USE_POSEIDON
  FastLED.addLeds<POSEIDON_CHIP, POSEIDON_PIN, POSEIDON_ORDER>(poseidonLeds, poseidon.ledCount);
#endif
#ifdef USE_HADES
  FastLED.addLeds<HADES_CHIP, HADES_PIN, HADES_ORDER>(hadesLeds, hades.ledCount);
#endif
  FastLED.setBrightness(BRIGHTNESS_PERCENT * 255 / 100);

#ifdef USE_RTC
  if (!clockModuleBegin()) {
    showRtcError();                                // RTC not found
  }
#ifdef SET_RTC_NOW
  setClockModuleToUploadTime();                    // Set to the upload time
#else
  if (clockModuleLostTime()) {
    setClockModuleToUploadTime();                  // First start, or its battery ran out: set to the upload time
  }
#endif
#endif

  // Start with the flames written in PART 1, then any saved button choice
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    currentFlame[ring][0] = CLOCKS[ring].flame[0];
    currentFlame[ring][1] = CLOCKS[ring].flame[1];
  }
  loadChoice();
  lights = START_ON ? savedLights : LIGHTS_OFF;

  pinMode(BUTTON_PIN, INPUT_PULLUP);               // Button input with internal pull-up
  lastTick = millis();
  clearDisplay();
}

void loop() {
  handleButton();

  // A new frame every FRAME_MS, however long drawing it takes. If drawing
  // takes longer than that, the next frame starts right away.
  static unsigned long frameStart = 0;
  unsigned long now = millis();
  if (now - frameStart < FRAME_MS) return;         // Not time yet
  frameStart = (now - frameStart < 2 * FRAME_MS) ? frameStart + FRAME_MS : now;

  if (lights != LIGHTS_OFF) {
    updateDisplay();                               // Redraw every frame so flames keep flickering
  }
}
