#include <FastLED.h>

// =====================================================================
//  FLAME CLOCK FOR THE ZODIAC CLOCK TOWER
// =====================================================================
//
//  WHAT IT DOES
//  Each LED ring is a clock face with 12 LEDs, one for every hour. A
//  flickering "flame" moves around the ring, one LED per hour, twice a
//  day: after midnight it is a cool color, after noon it is a warm one.
//  Depending on the FILL MODE, the flame can leave a glowing trail behind
//  it, so the ring slowly fills up and empties during the day.
//
//  HOW TO USE THE BUTTON
//    Short press ............ turns the clock ON or OFF.
//    Hold for 1 second ...... changes the FILL MODE. The ring blinks white
//                             once, twice, three or four times to show
//                             the mode number (1 to 4).
//
//  THE 4 FILL MODES  (morning = midnight to noon, afternoon = noon to midnight)
//    Mode 1: Only the flame is lit, everything else is dark.
//    Mode 2: The morning flame leaves a trail and fills the ring. The
//            afternoon flame moves along and empties it again.
//    Mode 3: The ring is never dark. At midnight it is full of the
//            afternoon trail color. The morning flame replaces it with
//            the morning trail color until noon, then the afternoon
//            flame fills it back up until midnight.
//    Mode 4: At midnight the ring is full of the afternoon trail color.
//            The morning flame empties it. The afternoon flame then fills
//            it back up with the afternoon trail color.
//
//  WIRING (Arduino Nano or Uno)
//    LED ring ........ 5V to 5V, GND to GND, DIN (data in) to pin D6
//                      (ring 2: D7, ring 3: D8, ring 4: D9)
//    Button .......... one leg to pin D2, the other leg to GND
//    RTC clock module (optional) ... SDA to A4, SCL to A5, VCC to 5V, GND to GND
//
//  LIBRARIES TO INSTALL (Arduino IDE: Sketch > Include Library > Manage Libraries)
//    FastLED ........ always needed
//    RTClib ......... only if you use an RTC clock module
//
//  CHANGES FROM THE ORIGINAL SKETCH
//    - Comments translated to English.
//    - The hour countdown was replaced by a moving, flickering flame
//      with 4 fill modes.
//    - Works without an RTC module, using a built-in clock that you can
//      speed up (great for testing and demos).
//    - Works with 1 to 4 rings.
//    - Test options to check colors and timing.
//
// =====================================================================

#include <FastLED.h>   // LED library. Do not change this line.

// =====================================================================
//  PART 1: YOUR MAIN OPTIONS
//  This is the part most people will change.
// =====================================================================

// How many LED rings are connected? (1, 2, 3 or 4)
#define NUM_RINGS 1

// Do you have a DS3231 RTC (real time clock) module connected?
//   NO RTC ... leave the line below as it is (with the two slashes).
//              The clock uses the built-in demo clock from PART 2.
//   HAVE RTC . remove the two slashes at the start of the line.
// See the RTC NOTES in PART 4 for how the time gets set.
// #define USE_RTC

// Which fill mode should the clock start in? (1, 2, 3 or 4, see above)
// You can change the mode later by holding the button.
const int START_MODE = 2;

// Should the lights come on by themselves when the clock is plugged in?
//   true  = yes, it starts running right away
//   false = no, press the button to turn it on
const bool START_ON = true;

// ---- COLORS ----
// Each color is written as CRGB(red, green, blue) with numbers from 0 to 255.
// Tip: search the web for "RGB color picker" to find the numbers you want.
// LEDs often look a bit different from a screen, so use COLOR_TEST in
// PART 2 to see your colors on the real LEDs.

// The moving flame in the MORNING (midnight to noon)
const CRGB MORNING_FLAME_COLOR   = CRGB(200, 245, 255);   // icy white

// The trail the morning flame leaves behind (used in modes 2 and 3)
const CRGB MORNING_TRAIL_COLOR   = CRGB(0, 191, 255);     // ice blue

// The moving flame in the AFTERNOON (noon to midnight)
const CRGB AFTERNOON_FLAME_COLOR = CRGB(255, 110, 0);     // orange

// The trail the afternoon flame leaves behind (used in modes 3 and 4)
const CRGB AFTERNOON_TRAIL_COLOR = CRGB(255, 30, 0);      // red

// =====================================================================
//  PART 2: TEST AND DEMO OPTIONS
//  Use these to check that everything works. Most people can leave
//  them as they are once the clock looks right.
// =====================================================================

// ---- DEMO CLOCK (only used when you do NOT have an RTC) ----
// Without an RTC, the clock does not know the real time. Instead it runs
// a fast "pretend" day, which is fun to watch and good for testing.
// This is how many milliseconds one pretend hour lasts:
//    60000UL = 1 minute per hour (a full day takes 24 minutes)
//     6000UL = 6 seconds per hour (a full day takes 2.4 minutes)
//     1000UL = 1 second per hour
// Do not go much below 300UL or the fades get choppy.
const unsigned long SIM_HOUR_MS = 60000UL;

// The pretend hour the clock starts at when plugged in (0 to 23).
// 0 = midnight, 12 = noon. Use 12 to see the afternoon first.
const int SIM_START_HOUR = 0;

// ---- COLOR TEST ----
// Remove the two slashes to show solid colors for 2 seconds each: pure
// red, green and blue, then your four colors. Use it to make sure the
// LEDs and your colors look right. Put the slashes back when done.
// #define COLOR_TEST

// ---- FREEZE THE CLOCK ----
// Remove the two slashes to hold the display at one moment, with no
// flicker, so you can look closely at the colors.
// (Press the button to turn the clock on.)
// #define DEBUG_FREEZE
#define DEBUG_HOUR 3          // Hour to hold (0 to 23). 0-11 = morning, 12-23 = afternoon
#define DEBUG_FRACTION 0.5f   // Position inside the hour: 0.0 = start, 0.5 = half, 1.0 = end

// ---- SHOW NUMBERS ON THE COMPUTER ----
// Remove the two slashes, upload, and open the Serial Monitor (set to
// 115200 baud) to see the numbers behind the moving flame's color.
// Useful when asking for help in the community.
// #define DEBUG_SERIAL

// =====================================================================
//  PART 3: ADVANCED OPTIONS
//  You probably do not need to change anything here.
// =====================================================================

// ---- Brightness ----
// Maximum brightness for all LEDs, from 0 (off) to 255 (full).
// Keep it low if the clock is powered from a computer's USB port.
#define BRIGHTNESS 60

// ---- Flame flicker ----
const uint8_t FLICKER_MIN = 80;       // Deepest flicker dip (0-255). Higher = calmer flames
const uint8_t FLICKER_TIME_DIV = 3;   // Flicker speed. Lower = faster flicker
const unsigned long FRAME_MS = 30;    // Pause between display updates (milliseconds)

// ---- Button ----
#define BUTTON_PIN 2                          // Button pin (other leg goes to GND)
const unsigned long LONG_PRESS_MS = 1000;     // How long to hold to change the mode
const unsigned long DEBOUNCE_MS = 40;         // Ignore very short presses (button noise)

// ---- LED rings ----
#define LED_PER_RING 12     // LEDs on each ring
#define COLOR_ORDER GRB     // If red and green look swapped, try RGB instead
#define RING1_PIN 6         // Data pin for ring 1
#define RING2_PIN 7         // Data pin for ring 2
#define RING3_PIN 8         // Data pin for ring 3
#define RING4_PIN 9         // Data pin for ring 4

// =====================================================================
//  PART 4: OTHER NOTES
// =====================================================================
//
//  RTC NOTES (only if you use an RTC module)
//  - The RTC keeps a plain clock time. It knows nothing about time zones
//    or daylight saving, so it simply shows whatever time it was set to.
//  - The first time it is powered up (or after its battery was empty),
//    this sketch sets it to the time on your computer when you uploaded
//    the sketch. So check that your computer's clock and time zone are
//    correct before uploading. It will be a few seconds behind, which does
//    not matter for this clock.
//  - After that, the RTC's little battery keeps the time while the clock
//    is unplugged. Nothing needs to be set at midnight.
//  - When daylight saving time changes, the RTC does NOT change by
//    itself. To fix the time: remove the two slashes from the line
//    "// #define SET_RTC_NOW" below, upload, then put the slashes
//    back and upload AGAIN. (If you forget the second upload, the clock
//    resets to the upload time every time it is plugged in.)
//  - Battery: the common blue DS3231 module has a small charging circuit
//    made for rechargeable LIR2032 batteries. A non-rechargeable CR2032
//    can leak or overheat unless that charging circuit is disabled. Check
//    your module's instructions or use a LIR2032.
//
// #define SET_RTC_NOW
//
//  TROUBLESHOOTING
//  - Nothing lights up: check 5V, GND and the data wire (pin D6 for ring
//    1). Make sure the clock is ON (short press), or set START_ON to true.
//  - Colors look wrong (red and green swapped): change COLOR_ORDER to RGB.
//  - The first LED blinks red forever: the RTC module was not found. Check
//    the SDA (A4) and SCL (A5) wires, or turn off USE_RTC.
//  - The mode does not change: hold the button a full second.
//
// =====================================================================
//  PART 5: THE PROGRAM
//  Nothing below this line needs to be changed.
// =====================================================================

#if NUM_RINGS < 1 || NUM_RINGS > 4
  #error "NUM_RINGS must be 1, 2, 3 or 4"
#endif

#ifdef USE_RTC
  #include <RTClib.h>         // RTC library
  RTC_DS3231 rtc;             // RTC module object
#endif

// ---------------------------------------------------------------------
//  Program state
// ---------------------------------------------------------------------
CRGB leds[NUM_RINGS][LED_PER_RING];   // One row of LEDs per ring

bool systemState = START_ON;          // Clock on (true) or off (false)
int currentMode = START_MODE;         // Fill mode 1-4

bool morningTrail = false;            // Does the morning flame leave a trail?
bool afternoonTrail = false;          // Does the afternoon flame leave a trail?

bool buttonDown = false;              // Is the button being held?
bool longPressDone = false;           // Was the mode already changed during this hold?
unsigned long pressStartTime = 0;     // When the current press started

// ---------------------------------------------------------------------
//  FILL MODES
// ---------------------------------------------------------------------
// Turns a mode number (1-4) into the two trail settings
void applyMode(int mode) {
  switch (mode) {
    case 2:  morningTrail = true;  afternoonTrail = false; break;
    case 3:  morningTrail = true;  afternoonTrail = true;  break;
    case 4:  morningTrail = false; afternoonTrail = true;  break;
    default: morningTrail = false; afternoonTrail = false; break;   // Mode 1
  }
}

// Blinks the whole ring white as many times as the mode number
void showModeNumber(int mode) {
  for (int n = 0; n < mode; n++) {
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      for (int led = 0; led < LED_PER_RING; led++) leds[ring][led] = CRGB::White;
    }
    FastLED.show();
    delay(250);
    FastLED.clear();
    FastLED.show();
    delay(250);
  }
}

// ---------------------------------------------------------------------
//  TIME: current hour and how far into the hour we are
// ---------------------------------------------------------------------
// hour: 0-23.  hourFraction: 0.0 at minute 0, up to 1.0 at the end of the hour.
void getTime(int &hour, float &hourFraction) {
#ifdef DEBUG_FREEZE
  hour = DEBUG_HOUR;
  hourFraction = DEBUG_FRACTION;
  return;
#endif
#ifdef USE_RTC
  DateTime now = rtc.now();
  hour = now.hour();
  hourFraction = (now.minute() * 60.0f + now.second()) / 3600.0f;
#else
  unsigned long elapsed = millis();
  hour = (SIM_START_HOUR + (elapsed / SIM_HOUR_MS)) % 24;
  hourFraction = (float)(elapsed % SIM_HOUR_MS) / (float)SIM_HOUR_MS;
#endif
}

// ---------------------------------------------------------------------
//  FADING
// ---------------------------------------------------------------------
// 0.0-1.0 value for the moving flame:
//   Morning (filling up):    0 at the start of the hour -> 1 at the end
//   Afternoon (burning down): 1 at the start of the hour -> 0 at the end
float activeLevel(int hour, float hourFraction) {
  return (hour < 12) ? hourFraction : 1.0f - hourFraction;
}

// Mixes color 'from' into color 'to'. level 0 = all 'from', level 1 = all 'to'.
// If 'from' is black, this is simply a fade in/out.
CRGB fadeBetween(CRGB from, CRGB to, float level) {
  level = constrain(level, 0.0f, 1.0f);
  CRGB out;
  out.r = from.r + (int)((to.r - from.r) * level);
  out.g = from.g + (int)((to.g - from.g) * level);
  out.b = from.b + (int)((to.b - from.b) * level);
  return out;
}

// ---------------------------------------------------------------------
//  FLICKER
// ---------------------------------------------------------------------
// Brightness factor (FLICKER_MIN to 255) for one LED. Each LED reads its own
// spot in a smooth noise pattern, so the flames move independently.
uint8_t flickerScale(int ring, int led) {
  uint16_t x = led * 97 + ring * 523;
  uint16_t t = millis() / FLICKER_TIME_DIV;
  uint8_t n = inoise8(x, t);
  n = constrain(n, 40, 215);
  return (uint8_t)map(n, 40, 215, FLICKER_MIN, 255);
}

// ---------------------------------------------------------------------
//  COLOR OF EACH LED
// ---------------------------------------------------------------------
// One flame moves around the ring, one LED per hour:
//   Behind the flame: its trail (or dark), depending on the fill mode.
//   Ahead of the flame: whatever the other half of the day left behind.
//   The flame itself fades between its surroundings and its flame color.
CRGB ledColor(int led, int hour, float hourFraction) {
  int active = hour % 12;                          // The LED the flame is on
  float level = activeLevel(hour, hourFraction);

  CRGB morningLeft   = morningTrail   ? MORNING_TRAIL_COLOR   : CRGB(0, 0, 0);
  CRGB afternoonLeft = afternoonTrail ? AFTERNOON_TRAIL_COLOR : CRGB(0, 0, 0);

  if (hour < 12) {                                 // Morning (midnight to noon)
    if (led < active)  return morningLeft;         // Behind: morning trail
    if (led == active) return fadeBetween(afternoonLeft, MORNING_FLAME_COLOR, level);
    return afternoonLeft;                          // Ahead: left by the afternoon
  } else {                                         // Afternoon (noon to midnight)
    if (led < active)  return afternoonLeft;       // Behind: afternoon trail
    if (led == active) return fadeBetween(afternoonLeft, AFTERNOON_FLAME_COLOR, level);
    return morningLeft;                            // Ahead: left by the morning
  }
}

// ---------------------------------------------------------------------
//  DISPLAY
// ---------------------------------------------------------------------
#ifdef DEBUG_SERIAL
// Prints the moving flame's color (before flicker) about once per second
void debugPrint(int hour, float hourFraction, CRGB c) {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 1000) return;
  lastPrint = millis();
  Serial.print(F("hour "));
  Serial.print(hour);
  Serial.print(F("  fraction "));
  Serial.print(hourFraction, 2);
  Serial.print(F("  flame RGB = "));
  Serial.print(c.r);
  Serial.print(F(", "));
  Serial.print(c.g);
  Serial.print(F(", "));
  Serial.println(c.b);
}
#endif

// Draws every ring with the current colors and flicker
void updateDisplay() {
  int hour;
  float hourFraction;
  getTime(hour, hourFraction);

  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int led = 0; led < LED_PER_RING; led++) {
      CRGB color = ledColor(led, hour, hourFraction);
#ifdef DEBUG_SERIAL
      if (ring == 0 && led == hour % 12) debugPrint(hour, hourFraction, color);
#endif
#ifndef DEBUG_FREEZE
      color.nscale8(flickerScale(ring, led));   // No flicker while frozen
#endif
      leds[ring][led] = color;
    }
  }
  FastLED.show();
}

void clearDisplay() {
  FastLED.clear();
  FastLED.show();
}

#ifdef COLOR_TEST
// Shows each test color on every LED, 2 seconds per color, no flicker
void runColorTest() {
  const CRGB testColors[] = {
    CRGB(255, 0, 0), CRGB(0, 255, 0), CRGB(0, 0, 255),            // Pure red, green, blue
    MORNING_TRAIL_COLOR, MORNING_FLAME_COLOR,                      // Your four colors
    AFTERNOON_FLAME_COLOR, AFTERNOON_TRAIL_COLOR
  };
  const int count = sizeof(testColors) / sizeof(testColors[0]);
  int index = (millis() / 2000) % count;
  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int led = 0; led < LED_PER_RING; led++) {
      leds[ring][led] = testColors[index];
    }
  }
  FastLED.show();
}
#endif

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
//  BUTTON: short press = on/off, long press = next fill mode
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
    longPressDone = true;                          // Held long enough: next mode
    currentMode = (currentMode % 4) + 1;
    applyMode(currentMode);
    showModeNumber(currentMode);
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
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
#endif

  // One data pin per ring
  FastLED.addLeds<WS2812B, RING1_PIN, COLOR_ORDER>(leds[0], LED_PER_RING);
#if NUM_RINGS >= 2
  FastLED.addLeds<WS2812B, RING2_PIN, COLOR_ORDER>(leds[1], LED_PER_RING);
#endif
#if NUM_RINGS >= 3
  FastLED.addLeds<WS2812B, RING3_PIN, COLOR_ORDER>(leds[2], LED_PER_RING);
#endif
#if NUM_RINGS >= 4
  FastLED.addLeds<WS2812B, RING4_PIN, COLOR_ORDER>(leds[3], LED_PER_RING);
#endif
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

  pinMode(BUTTON_PIN, INPUT_PULLUP);               // Button input with internal pull-up
  if (currentMode < 1 || currentMode > 4) currentMode = 1;
  applyMode(currentMode);
  clearDisplay();
}

void loop() {
#ifdef COLOR_TEST
  runColorTest();
  delay(FRAME_MS);
  return;
#endif

  handleButton();

  if (systemState) {
    updateDisplay();                               // Redraw every frame so flames keep flickering
  }

  delay(FRAME_MS);
}
