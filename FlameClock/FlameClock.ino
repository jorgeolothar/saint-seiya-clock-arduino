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
//  WIRING (Arduino Nano or Uno)
//    LED ring ............ 5V to 5V, GND to GND, DIN to pin D6
//                          (ring 2 to D7, ring 3 to D8, ring 4 to D9)
//    Button .............. one leg to pin D2, the other leg to GND
//    Clock module (RTC) .. optional. SDA to A4, SCL to A5, VCC to 5V, GND to GND
//
//  LIBRARIES TO INSTALL
//  In the Arduino IDE: Sketch > Include Library > Manage Libraries
//    FastLED ....... always needed
//    RTClib ........ only if you use a clock module
//
//  FILES
//  Keep these three files together in a folder called FlameClock. The
//  Arduino IDE shows them as tabs.
//    FlameClock.ino ... this file: your clocks and settings
//    Flames.h ......... the colors and flames
//    FlameTypes.h ..... the building blocks (no need to open it)
//
//  NEW IN THIS VERSION
//  Each ring has its own two flames, and you can make your own flames in
//  Flames.h. They replace the 4 fill modes: a flame with a black trail
//  works like the old "no trail" modes.
//
// =====================================================================

// Do not change these four lines
#include <FastLED.h>
#include <EEPROM.h>
#include "FlameTypes.h"
#include "Flames.h"

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
  { SHOW_HOURS,   COOL,      EMBER,         1 },

  // Ring 2, minutes: a bright spark jumps one LED every 5 minutes and
  // leaves no trail.
  // { SHOW_MINUTES, SPARK,     SPARK,         1 },

  // Ring 3, seconds: in even minutes a rainbow fills the ring, one LED
  // every 5 seconds. In odd minutes each color burns out to black. This
  // ring is mounted turned, so its top LED is 4.
  // { SHOW_SECONDS, HUE_WHEEL, RAINBOW_EMBER, 4 },
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

// Show the time and colors on the computer? Set to true, upload, and open
// Tools > Serial Monitor (set to 115200 baud). Handy when asking for help.
const bool PRINT_TO_SERIAL = false;

// =====================================================================
//  PART 3: MORE SETTINGS
//  Most people never need to change these.
// =====================================================================

// Brightness, from 0 (off) to 255 (full). Keep it low when the clock is
// powered from a computer's USB port.
#define BRIGHTNESS 60

// Flicker speed: lower = faster
const uint8_t FLICKER_TIME_DIV = 3;

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

// The two flames each ring shows right now. They start as written in
// PART 1, and the long press changes them.
const Flame *currentFlame[MAX_RINGS][2];

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
//  COLORS AND FADES
// ---------------------------------------------------------------------
// A flame's color at one place on the ring. A wheel color holds its
// brightness in red and its paleness in green (see wheelColor), and takes
// its hue from the LED's place.
CRGB paint(const Flame &flame, CRGB color, int position) {
  if (flame.colors == FIXED_COLORS) return color;
  uint8_t hue = position * 256 / LED_PER_RING;
  return CRGB(CHSV(hue, 255 - color.g, color.r));
}

// How far a fade has got (0.0 to 1.0) when 'progress' (0.0 to 1.0) of
// its time has passed
float fadeAmount(Fade fade, float progress) {
  switch (fade) {
    case SMOOTH_FADE: return progress * progress * (3.0f - 2.0f * progress);
    case EARLY_FADE:  return 1.0f - (1.0f - progress) * (1.0f - progress);
    case LATE_FADE:   return progress * progress;
    default:          return progress;   // STEADY_FADE
  }
}

// Mixes color 'from' into color 'to'. level 0 = all 'from', level 1 = all 'to'.
// Red, green and blue each move in a straight line from start to end.
CRGB fadeBetween(CRGB from, CRGB to, float level) {
  level = constrain(level, 0.0f, 1.0f);
  CRGB out;
  out.r = from.r + (int)((to.r - from.r) * level);
  out.g = from.g + (int)((to.g - from.g) * level);
  out.b = from.b + (int)((to.b - from.b) * level);
  return out;
}

// Brightness factor (255 = full) for one LED. Each LED reads its own spot
// in a smooth noise pattern, so the flames move independently.
// flicker: in %, 0 = steady, 100 = deepest dips.
uint8_t flickerScale(int ring, int position, uint8_t flicker) {
  if (flicker == 0) return 255;
  if (flicker > 100) flicker = 100;
  uint16_t x = position * 97 + ring * 523;
  uint16_t t = millis() / FLICKER_TIME_DIV;
  uint8_t n = inoise8(x, t);
  n = constrain(n, 40, 215);
  uint8_t lowest = 255 - (uint16_t)flicker * 255 / 100;
  return (uint8_t)map(n, 40, 215, lowest, 255);
}

// ---------------------------------------------------------------------
//  DRAWING ONE RING
// ---------------------------------------------------------------------
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

// One flame moves around the ring:
//   Behind the flame: its own trail.
//   Ahead of the flame: the trail the other flame left on its lap.
//   The flame itself fades in from that old trail to the flame color
//   (first half of the step), then into its own trail (second half).
// steady = true turns the flicker off.
void drawClock(int ring, uint32_t ms, bool steady) {
  int pass, flamePosition;
  float progress;
  findFlame(ring, ms, pass, flamePosition, progress);
  const Flame &now = *currentFlame[ring][pass];          // The flame moving now
  const Flame &before = *currentFlame[ring][1 - pass];   // The flame of the last lap

  for (int position = 0; position < LED_PER_RING; position++) {
    CRGB color;
    uint8_t flicker;
    if (position < flamePosition) {                      // Behind: this flame's trail
      color = paint(now, now.trailColor, position);
      flicker = now.flicker;
    } else if (position > flamePosition) {               // Ahead: the last lap's trail
      color = paint(before, before.trailColor, position);
      flicker = before.flicker;
    } else if (progress < 0.5f) {                        // Flame, 1st half: fade in
      float amount = (now.fadeIn == NO_FADE) ? 1.0f : fadeAmount(now.fadeIn, progress * 2);
      color = fadeBetween(paint(before, before.trailColor, position),
                          paint(now, now.flameColor, position), amount);
      flicker = (uint8_t)(before.flicker + (now.flicker - before.flicker) * amount);
    } else {                                             // Flame, 2nd half: fade out
      float amount = (now.fadeOut == NO_FADE) ? 0.0f : fadeAmount(now.fadeOut, progress * 2 - 1);
      color = fadeBetween(paint(now, now.flameColor, position),
                          paint(now, now.trailColor, position), amount);
      flicker = now.flicker;
    }
    if (PRINT_TO_SERIAL && ring == 0 && position == flamePosition) {
      printFlame(ms, pass, position, color);
    }
    if (!steady) color.nscale8(flickerScale(ring, position, flicker));
    leds[ring][ringLed(ring, position)] = color;
  }
}

// MODE_COLOR_TEST: 12 = 1st flame, 1-5 = 2nd trail, 6 = 2nd flame, 7-11 = 1st trail
void drawColorTest(int ring) {
  const Flame &first = *currentFlame[ring][0];
  const Flame &second = *currentFlame[ring][1];
  for (int position = 0; position < LED_PER_RING; position++) {
    CRGB color;
    if (position == 0)      color = paint(first, first.flameColor, position);
    else if (position < 6)  color = paint(second, second.trailColor, position);
    else if (position == 6) color = paint(second, second.flameColor, position);
    else                    color = paint(first, first.trailColor, position);
    leds[ring][ringLed(ring, position)] = color;
  }
}

// Draws every ring
void updateDisplay() {
  uint32_t ms = timeOfDay();
  for (int ring = 0; ring < NUM_CLOCKS; ring++) {
    if (BEHAVIOR == MODE_COLOR_TEST) drawColorTest(ring);
    else drawClock(ring, ms, BEHAVIOR == MODE_FREEZE);   // No flicker while frozen
  }
  FastLED.show();
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
    FastLED.show();
    delay(300);
    leds[0][0] = CRGB::Black;
    FastLED.show();
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
int flameNumber(const Flame *flame) {
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
void showFlameNumbers(const Flame *const flame[], const int number[]) {
  for (int blink = 0; blink < 2; blink++) {
    FastLED.clear();
    for (int ring = 0; ring < NUM_CLOCKS; ring++) {
      for (int hour = 1; hour <= number[ring]; hour++) {
        int position = hour % LED_PER_RING;
        leds[ring][ringLed(ring, position)] = paint(*flame[ring], flame[ring]->flameColor, position);
      }
    }
    FastLED.show();
    delay(400);
    clearDisplay();
    delay(200);
  }
}

// The flame moving right now on each ring becomes the next one in FLAME_LIST
void nextFlame() {
  uint32_t ms = timeOfDay();
  const Flame *shown[MAX_RINGS];
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
  if (PRINT_TO_SERIAL) Serial.begin(115200);

  // One data pin per ring
  FastLED.addLeds<WS2812B, RING1_PIN, COLOR_ORDER>(leds[0], LED_PER_RING);
  if (NUM_CLOCKS >= 2) FastLED.addLeds<WS2812B, RING2_PIN, COLOR_ORDER>(leds[1], LED_PER_RING);
  if (NUM_CLOCKS >= 3) FastLED.addLeds<WS2812B, RING3_PIN, COLOR_ORDER>(leds[2], LED_PER_RING);
  if (NUM_CLOCKS >= 4) FastLED.addLeds<WS2812B, RING4_PIN, COLOR_ORDER>(leds[3], LED_PER_RING);
  FastLED.setBrightness(BRIGHTNESS);

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
