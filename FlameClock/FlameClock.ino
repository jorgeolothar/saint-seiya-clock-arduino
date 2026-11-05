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
//    Short press ...... turns the clock on or off.
//    Hold 1 second .... changes the flame you see moving to the next one.
//                       Each ring then shows the flame's number like the
//                       hours on a clock: flame 3 lights 1, 2 and 3 o'clock.
//                       Hold again for the next one. The clock remembers
//                       your choice, even when unplugged.
//
//  TEMPLE SCENES (optional, all switched off)
//  The Arduino can also light the temples of the Sanctuary, Asgard,
//  Poseidon and Hades scenes: each temple lights up in its hour. Set
//  them up in the Scenes.h tab. See SCENES.md.
//
//  WIRING (Arduino Nano or Uno)
//    LED ring ............ 5V to 5V, GND to GND, DIN to pin D6
//                          (ring 2 to D7, ring 3 to D8, ring 4 to D9)
//    Temple scenes ....... DIN to pin D10 (Sanctuary), D11 (Asgard),
//                          D12 (Poseidon), A0 (Hades). Each scene on its
//                          own 5V power supply, with its GND joined to the
//                          Arduino's GND.
//    Button .............. one leg to pin D2, the other leg to GND
//    Clock module (RTC) .. optional. SDA to A4, SCL to A5, VCC to 5V, GND to GND
//
//  LIBRARIES TO INSTALL
//  In the Arduino IDE: Sketch > Include Library > Manage Libraries
//    FastLED ....... always needed
//    RTClib ........ only if you use a clock module
//
//  FILES
//  Keep these files together in a folder called FlameClock. The Arduino
//  IDE shows them as tabs.
//    FlameClock.ino ... this file: your clocks and settings
//    Flames.h ......... the clock's flames
//    Scenes.h ......... the temple scenes
//    TempleLooks.h .... the temples' looks
//    Colors.h ......... the named colors the flames and looks use
//  The building blocks (no need to open them):
//    LookTypes.h ...... what a look (a flame) is
//    ClockTypes.h ..... what a clock (a ring) is
//    SceneTypes.h ..... what a temple and a scene are
//    Effects.h ........ how looks are drawn
//    SceneDriver.h .... how scenes are drawn
//    Power.h .......... the power limit
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

// Do you have a clock module (DS3231 RTC)? It keeps the real time, even
// while the clock is unplugged.
//   No ... leave the next line as it is. The clock starts at START_TIME
//          (PART 2) every time it is plugged in.
//   Yes .. remove the two slashes at the start of the next line, and read
//          the clock module notes in PART 4.
// #define USE_RTC

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

// The temple scenes follow these modes too: MODE_ACCELERATED runs
// through their hours faster (their looks keep their own speed),
// MODE_COLOR_TEST shows every temple's look, first color, and
// MODE_FREEZE holds them still. ALL_TEMPLES_ON in Scenes.h turns every
// temple on.

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

// Remember the flames picked with the button when unplugged?
// true = yes, false = no. Uploading a changed sketch always goes back to
// the flames in PART 1.
const bool REMEMBER_FLAME_CHOICE = true;

// The LED rings
#define COLOR_ORDER GRB     // If red and green look swapped, change to RGB
#define RING1_PIN 6         // The pin for ring 1
#define RING2_PIN 7         // The pin for ring 2
#define RING3_PIN 8         // The pin for ring 3
#define RING4_PIN 9         // The pin for ring 4

// The temple scenes' pins, chips and power limits are in Scenes.h

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
//  TROUBLESHOOTING
//  - Nothing lights up: check the 5V, GND and DIN wires (DIN goes to pin
//    D6 for ring 1). Press the button once, in case the clock is off.
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

#define MAX_RINGS 4
#define LED_PER_RING 12   // The clock face needs exactly 12 LEDs per ring

const int NUM_CLOCKS = sizeof(CLOCKS) / sizeof(CLOCKS[0]);
const int FLAME_COUNT = sizeof(FLAME_LIST) / sizeof(FLAME_LIST[0]);

static_assert(NUM_CLOCKS >= 1 && NUM_CLOCKS <= MAX_RINGS, "CLOCKS must list 1, 2, 3 or 4 rings");
static_assert(FLAME_COUNT >= 1 && FLAME_COUNT <= LED_PER_RING, "FLAME_LIST must hold 1 to 12 flames");

#ifdef USE_RTC
  #include <RTClib.h>         // RTC library
  RTC_DS3231 rtc;             // RTC module object
#endif

// ---------------------------------------------------------------------
//  Program state
// ---------------------------------------------------------------------
CRGB leds[MAX_RINGS][LED_PER_RING];         // One row of LEDs per ring

// The temple scenes switched on in Scenes.h: their LEDs, and where each
// one starts in the flicker pattern
#ifdef USE_SANCTUARY
CRGB sanctuaryLeds[ledsIn(SANCTUARY)];
const Scene sanctuary(SCENE_TEMPLES(SANCTUARY), sanctuaryLeds, SANCTUARY_LIGHTING, SANCTUARY_START, SANCTUARY_POWER, 4000);
#endif
#ifdef USE_ASGARD
CRGB asgardLeds[ledsIn(ASGARD)];
const Scene asgard(SCENE_TEMPLES(ASGARD), asgardLeds, ASGARD_LIGHTING, ASGARD_START, ASGARD_POWER, 12000);
#endif
#ifdef USE_POSEIDON
CRGB poseidonLeds[ledsIn(POSEIDON)];
const Scene poseidon(SCENE_TEMPLES(POSEIDON), poseidonLeds, POSEIDON_LIGHTING, POSEIDON_START, POSEIDON_POWER, 20000);
#endif
#ifdef USE_HADES
CRGB hadesLeds[ledsIn(HADES)];
const Scene hades(SCENE_TEMPLES(HADES), hadesLeds, HADES_LIGHTING, HADES_START, HADES_POWER, 28000);
#endif

const Scene *const SCENES[] = {
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

// The two flames each ring shows right now. They start as written in
// PART 1, and the long press changes them.
const Look *currentFlame[MAX_RINGS][2];

bool systemState = START_ON;                // Clock on (true) or off (false)

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
  DateTime now = rtc.now();
  if (now.second() != lastSecond) {
    lastSecond = now.second();
    secondStarted = millis();
  }
  unsigned long partOfSecond = millis() - secondStarted;
  if (partOfSecond > 999) partOfSecond = 999;
  return (now.hour() * 3600UL + now.minute() * 60UL + now.second()) * 1000UL + partOfSecond;
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
// steady = true turns the flicker off.
void drawClock(int ring, uint32_t ms, bool steady) {
  int pass, flamePosition;
  float progress;
  findFlame(ring, ms, pass, flamePosition, progress);
  const Look &now = *currentFlame[ring][pass];          // The flame moving now
  const Look &before = *currentFlame[ring][1 - pass];   // The flame of the last lap

  for (int position = 0; position < LED_PER_RING; position++) {
    uint16_t spot = position * 97 + ring * 523;         // The LED's place in the flicker pattern
    CRGB color;
    if (position < flamePosition) {                     // Behind: this flame's trail
      color = drawLook(now, now, 1.0f, position, LED_PER_RING, spot, steady);
    } else if (position > flamePosition) {              // Ahead: the last lap's trail
      color = drawLook(before, before, 1.0f, position, LED_PER_RING, spot, steady);
    } else {                                            // The flame, taking over from the old trail
      color = drawLook(now, before, progress, position, LED_PER_RING, spot, steady);
#ifdef PRINT_TO_SERIAL
      if (ring == 0) {
        printFlame(ms, pass, position, drawLook(now, before, progress, position, LED_PER_RING, spot, true));
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

// Draws every ring and every temple scene
void updateDisplay() {
  uint32_t ms = timeOfDay();
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    if (BEHAVIOR == MODE_COLOR_TEST) drawColorTest(ring);
    else drawClock(ring, ms, BEHAVIOR == MODE_FREEZE);   // No flicker while frozen
  }
  uint32_t speed = (BEHAVIOR == MODE_ACCELERATED) ? ACCELERATED_SPEED : 1;
  for (int i = 0; i < NUM_SCENES; i++) {
    drawScene(*SCENES[i], ms, speed, BEHAVIOR == MODE_FREEZE, BEHAVIOR == MODE_COLOR_TEST, ALL_TEMPLES_ON);
  }
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
//  REMEMBERING THE BUTTON'S FLAME CHOICE
// ---------------------------------------------------------------------
// The choice is stored in the Arduino's EEPROM memory, which keeps its
// contents without power. A number made from the upload time is stored
// with it, so a new upload starts again from the flames in PART 1.
struct SavedChoice {
  uint16_t uploadId;
  uint8_t flameNumber[MAX_RINGS][2];   // Place in FLAME_LIST, 255 = as written in PART 1
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
  if (!REMEMBER_FLAME_CHOICE) return;
  SavedChoice saved;
  saved.uploadId = uploadId();
  for (int ring = 0; ring < MAX_RINGS; ring++) {
    for (int pass = 0; pass < 2; pass++) {
      saved.flameNumber[ring][pass] = (ring < NUM_CLOCKS) ? (uint8_t)flameNumber(currentFlame[ring][pass]) : 255;
    }
  }
  EEPROM.put(0, saved);   // Only writes the bytes that changed
}

void loadChoice() {
  if (!REMEMBER_FLAME_CHOICE) return;
  SavedChoice saved;
  EEPROM.get(0, saved);
  if (saved.uploadId != uploadId()) return;   // New upload: keep the flames from PART 1
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    for (int pass = 0; pass < 2; pass++) {
      uint8_t number = saved.flameNumber[ring][pass];
      if (number < FLAME_COUNT) currentFlame[ring][pass] = FLAME_LIST[number];
    }
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
    findFlame(ring, ms, pass, position, progress);
    int next = (flameNumber(currentFlame[ring][pass]) + 1) % FLAME_COUNT;
    currentFlame[ring][pass] = FLAME_LIST[next];
    shown[ring] = FLAME_LIST[next];
    number[ring] = next + 1;
  }
  saveChoice();
  showFlameNumbers(shown, number);
}

// ---------------------------------------------------------------------
//  BUTTON: short press = on/off, long press = next flame
// ---------------------------------------------------------------------
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
    if (!systemState) clearDisplay();
  }
  else if (!pressed && buttonDown) {               // Button was released
    buttonDown = false;
    if (!longPressDone && now - pressStartTime >= DEBOUNCE_MS) {
      systemState = !systemState;                  // Short press: on/off
      if (!systemState) clearDisplay();
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
  // One data pin per temple scene
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
  if (!rtc.begin()) {
    showRtcError();                                // RTC not found
  }
#ifdef SET_RTC_NOW
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set to the upload time
#else
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // First start: set to the upload time
  }
#endif
#endif

  // Start with the flames written in PART 1, then any saved button choice
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    currentFlame[ring][0] = CLOCKS[ring].flame[0];
    currentFlame[ring][1] = CLOCKS[ring].flame[1];
  }
  loadChoice();

  pinMode(BUTTON_PIN, INPUT_PULLUP);               // Button input with internal pull-up
  lastTick = millis();
  clearDisplay();
}

void loop() {
  handleButton();

  if (systemState) {
    updateDisplay();                               // Redraw every frame so flames keep flickering
  }

  delay(FRAME_MS);
}
