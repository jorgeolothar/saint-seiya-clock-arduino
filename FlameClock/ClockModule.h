// =====================================================================
//  THE CLOCK MODULE (RTC)
// =====================================================================
//
//  Reads and sets the time on the clock module. The sketch only uses the
//  four functions below, whichever chip the module has:
//
//    clockModuleBegin() ............ false = the module was not found
//    clockModuleLostTime() ......... true = its time is lost (first
//                                    start, or its battery ran out)
//    setClockModuleToUploadTime() .. sets it to the upload time
//    clockModuleTime(t) ............ reads the hour, minute and second.
//                                    false = no answer: t is unchanged
//
//  Two ways to talk to the module, picked by RTC_CHIP and USE_RTCLIB
//  (PART 4 of FlameClock.ino):
//    - Built-in code, for the DS3231: no library needed, and far less
//      program space. Only on AVR boards (Uno, Nano).
//    - The RTClib library (by Adafruit), for the DS3231, DS1307, PCF8523
//      and PCF8563, on any board.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include <stdint.h>

struct ClockTime { uint8_t hour, minute, second; };

// ---------------------------------------------------------------------
//  Picking the code for the chip
// ---------------------------------------------------------------------
#define CLOCK_CHIP_DS3231 1
#define CLOCK_CHIP_DS1307 2
#define CLOCK_CHIP_PCF8523 3
#define CLOCK_CHIP_PCF8563 4
#define CLOCK_CHIP_NUMBER(chip) CLOCK_CHIP_NUMBER_OF(chip)   // Two steps, so RTC_CHIP is read first
#define CLOCK_CHIP_NUMBER_OF(chip) CLOCK_CHIP_##chip
#define RTCLIB_CLASS(chip) RTCLIB_CLASS_OF(chip)
#define RTCLIB_CLASS_OF(chip) RTC_##chip                     // DS3231 -> RTClib's RTC_DS3231

#ifdef USE_RTC
  #ifndef RTC_CHIP
    #error "Set RTC_CHIP (PART 4 of FlameClock.ino) to your clock module's chip"
  #elif CLOCK_CHIP_NUMBER(RTC_CHIP) < 1 || CLOCK_CHIP_NUMBER(RTC_CHIP) > 4
    #error "RTC_CHIP (PART 4 of FlameClock.ino) must be DS3231, DS1307, PCF8523 or PCF8563"
  #elif CLOCK_CHIP_NUMBER(RTC_CHIP) != CLOCK_CHIP_DS3231 || defined(USE_RTCLIB)
    #define CLOCK_MODULE_RTCLIB
  #elif !defined(__AVR__)
    #error "The built-in clock module code only runs on AVR boards (Uno, Nano). Turn on USE_RTCLIB (PART 4 of FlameClock.ino)"
  #endif
#endif

// ---------------------------------------------------------------------
//  The DS3231's registers (for the built-in code)
// ---------------------------------------------------------------------
// 0 to 6 hold the time and date (second, minute, hour, weekday, day,
// month, year), as two decimal digits per byte. 0x0F is the status.
const uint8_t DS3231_ADDRESS_ON_BUS = 0x68;
const uint8_t DS3231_STATUS = 0x0F;
const uint8_t DS3231_STOPPED = 0x80;   // In the status: it stopped, the time is lost

// Two decimal digits in one byte (0x59 = 59), and back
inline uint8_t fromDigits(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
inline uint8_t toDigits(uint8_t n) { return (uint8_t)((n / 10) << 4 | (n % 10)); }

// The hour, 0 to 23, from the hour register, kept as a 24 hour clock
// (as this sketch sets it) or as a 12 hour clock with AM and PM
inline uint8_t hourFromRegister(uint8_t r) {
  if (!(r & 0x40)) return fromDigits(r & 0x3F);   // 24 hour clock
  uint8_t hour = fromDigits(r & 0x1F) % 12;       // 12 hour clock: 12 AM is 0
  return (r & 0x20) ? hour + 12 : hour;           // PM
}

// The 7 time registers for the upload time, from the compiler's
// __DATE__ __TIME__, for example "Nov 18 2026" "15:42:07". The weekday
// isn't used by the clock: it is set to 1.
inline void uploadTimeRegisters(const char *dateTime, uint8_t reg[7]) {
  const char *d = dateTime, *t = dateTime + 11;
  uint8_t month = d[0] == 'J' ? (d[1] == 'a' ? 1 : d[2] == 'n' ? 6 : 7)     // Jan, Jun, Jul
                : d[0] == 'F' ? 2
                : d[0] == 'M' ? (d[2] == 'r' ? 3 : 5)                       // Mar, May
                : d[0] == 'A' ? (d[1] == 'p' ? 4 : 8)                       // Apr, Aug
                : d[0] == 'S' ? 9 : d[0] == 'O' ? 10 : d[0] == 'N' ? 11 : 12;
  uint8_t day = (d[4] == ' ' ? 0 : (d[4] - '0') * 10) + (d[5] - '0');
  uint8_t year = (d[9] - '0') * 10 + (d[10] - '0');                         // 2026 = 26
  auto two = [](const char *p) { return (uint8_t)((p[0] - '0') * 10 + (p[1] - '0')); };
  reg[0] = toDigits(two(t + 6));
  reg[1] = toDigits(two(t + 3));
  reg[2] = toDigits(two(t));                                                // 24 hour clock
  reg[3] = 1;
  reg[4] = toDigits(day);
  reg[5] = toDigits(month);
  reg[6] = toDigits(year);
}

// The hour, minute and second from registers 0 to 2. false = they make
// no sense (a garbled read).
inline bool timeFromRegisters(const uint8_t reg[3], ClockTime &t) {
  ClockTime read = { hourFromRegister(reg[2]), fromDigits(reg[1] & 0x7F), fromDigits(reg[0] & 0x7F) };
  if (read.hour > 23 || read.minute > 59 || read.second > 59) return false;
  t = read;
  return true;
}

#if defined(CLOCK_MODULE_RTCLIB)
// ---------------------------------------------------------------------
//  Through the RTClib library
// ---------------------------------------------------------------------
#include <RTClib.h>

RTCLIB_CLASS(RTC_CHIP) rtc;

// Each chip says in its own way that its time is lost
inline bool lostTime(RTC_DS3231 &chip) { return chip.lostPower(); }
inline bool lostTime(RTC_DS1307 &chip) { return !chip.isrunning(); }
inline bool lostTime(RTC_PCF8523 &chip) { return !chip.initialized() || chip.lostPower(); }
inline bool lostTime(RTC_PCF8563 &chip) { return chip.lostPower(); }

// The PCF chips need starting after being set
template <class Chip> inline void startCounting(Chip &) {}
inline void startCounting(RTC_PCF8523 &chip) { chip.start(); }
inline void startCounting(RTC_PCF8563 &chip) { chip.start(); }

inline bool clockModuleBegin() { return rtc.begin(); }
inline bool clockModuleLostTime() { return lostTime(rtc); }

inline void setClockModuleToUploadTime() {
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  startCounting(rtc);
}

inline bool clockModuleTime(ClockTime &t) {
  DateTime now = rtc.now();
  // A failed read gives impossible values, such as hour 165
  if (now.hour() > 23 || now.minute() > 59 || now.second() > 59) return false;
  t = { now.hour(), now.minute(), now.second() };
  return true;
}

#elif defined(USE_RTC)
// ---------------------------------------------------------------------
//  Built-in code for the DS3231, through the Arduino's I2C hardware (TWI)
// ---------------------------------------------------------------------
#include <avr/io.h>

// Waits for the I2C hardware to finish a step. false = it took too long:
// no module, or a loose wire.
inline bool i2cWait() {
  for (uint16_t n = 0; n < 10000; n++) {          // About 5 ms: a byte takes 0.1 ms
    if (TWCR & _BV(TWINT)) return true;
  }
  return false;
}

// Sends a start, or a byte, and checks the answer the module should give
inline bool i2cStart() {
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
  return i2cWait() && ((TWSR & 0xF8) == 0x08 || (TWSR & 0xF8) == 0x10);   // Start, or repeated start
}
inline bool i2cSend(uint8_t b, uint8_t answer) {
  TWDR = b;
  TWCR = _BV(TWINT) | _BV(TWEN);
  return i2cWait() && (TWSR & 0xF8) == answer;
}

// Ends the conversation. After a failure, restarts the I2C hardware too.
inline void i2cStop(bool ok) {
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
  for (uint16_t n = 0; n < 10000 && (TWCR & _BV(TWSTO)); n++) {}
  if (!ok) {
    TWCR = 0;
    TWCR = _BV(TWEN);
  }
}

// Writes 'count' registers from 'reg' on. false = the module didn't answer.
inline bool ds3231Write(uint8_t reg, const uint8_t *data, uint8_t count) {
  bool ok = i2cStart() && i2cSend(DS3231_ADDRESS_ON_BUS << 1, 0x18) && i2cSend(reg, 0x28);
  for (uint8_t i = 0; ok && i < count; i++) ok = i2cSend(data[i], 0x28);
  i2cStop(ok);
  return ok;
}

// Reads 'count' registers from 'reg' on. false = the module didn't answer.
inline bool ds3231Read(uint8_t reg, uint8_t *data, uint8_t count) {
  bool ok = i2cStart() && i2cSend(DS3231_ADDRESS_ON_BUS << 1, 0x18) && i2cSend(reg, 0x28) &&
            i2cStart() && i2cSend(DS3231_ADDRESS_ON_BUS << 1 | 1, 0x40);
  for (uint8_t i = 0; ok && i < count; i++) {
    TWCR = _BV(TWINT) | _BV(TWEN) | (i + 1 < count ? _BV(TWEA) : 0);   // "More, please", but not after the last
    ok = i2cWait();
    data[i] = TWDR;
  }
  i2cStop(ok);
  return ok;
}

// Gets the I2C hardware ready (100 kHz, with the Arduino's own pull-ups
// on SDA (A4) and SCL (A5), as the Wire library does), and checks that
// the module answers
inline bool clockModuleBegin() {
  PORTC |= _BV(4) | _BV(5);
  TWSR = 0;
  TWBR = (F_CPU / 100000UL - 16) / 2;
  TWCR = _BV(TWEN);
  uint8_t status;
  return ds3231Read(DS3231_STATUS, &status, 1);
}

inline bool clockModuleLostTime() {
  uint8_t status;
  return ds3231Read(DS3231_STATUS, &status, 1) && (status & DS3231_STOPPED);
}

// Sets the module to the upload time, and marks its time as good again
inline void setClockModuleToUploadTime() {
  uint8_t reg[7];
  uploadTimeRegisters(__DATE__ __TIME__, reg);
  uint8_t status;
  if (!ds3231Write(0, reg, 7) || !ds3231Read(DS3231_STATUS, &status, 1)) return;
  status &= ~DS3231_STOPPED;
  ds3231Write(DS3231_STATUS, &status, 1);
}

inline bool clockModuleTime(ClockTime &t) {
  uint8_t reg[3];
  return ds3231Read(0, reg, 3) && timeFromRegisters(reg, t);
}
#endif
