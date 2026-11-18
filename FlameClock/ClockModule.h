// =====================================================================
//  THE CLOCK MODULE (DS3231 RTC)
// =====================================================================
//
//  Reads and sets the time on a DS3231 clock module, through the
//  Arduino's own I2C hardware (pins A4 and A5). No library needed: it
//  only does what the clock uses, so it takes far less program space
//  than a full RTC library.
//
//  Keep this file in the same folder as FlameClock.ino. The Arduino IDE
//  shows it as a tab. You do not need to change anything here.
//
// =====================================================================

#pragma once
#include <stdint.h>

// ---- The module's registers ----
// 0 to 6 hold the time and date (second, minute, hour, weekday, day,
// month, year), as two decimal digits per byte. 0x0F is the status.
const uint8_t CLOCK_MODULE_ADDRESS = 0x68;
const uint8_t CLOCK_MODULE_STATUS = 0x0F;
const uint8_t CLOCK_MODULE_STOPPED = 0x80;   // In the status: it stopped, the time is lost

// Two decimal digits in one byte (0x59 = 59), and back
inline uint8_t fromDigits(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
inline uint8_t toDigits(uint8_t n) { return (uint8_t)((n / 10) << 4 | (n % 10)); }

// The hour, 0 to 23, from the module's hour register, kept as a 24 hour
// clock (as this sketch sets it) or as a 12 hour clock with AM and PM
inline uint8_t hourFromRegister(uint8_t r) {
  if (!(r & 0x40)) return fromDigits(r & 0x3F);   // 24 hour clock
  uint8_t hour = fromDigits(r & 0x1F) % 12;       // 12 hour clock: 12 AM is 0
  return (r & 0x20) ? hour + 12 : hour;           // PM
}

// The module's 7 time registers for the upload time, from the compiler's
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

#ifdef __AVR__
// ---- Talking to the module, through the Arduino's I2C hardware (TWI) ----
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

// Gets the I2C hardware ready: 100 kHz, with the Arduino's own pull-ups
// on SDA (A4) and SCL (A5), as the Wire library does
inline void clockModuleBegin() {
  PORTC |= _BV(4) | _BV(5);
  TWSR = 0;
  TWBR = (F_CPU / 100000UL - 16) / 2;
  TWCR = _BV(TWEN);
}

// Writes 'count' registers from 'reg' on. false = the module didn't answer.
inline bool clockModuleWrite(uint8_t reg, const uint8_t *data, uint8_t count) {
  bool ok = i2cStart() && i2cSend(CLOCK_MODULE_ADDRESS << 1, 0x18) && i2cSend(reg, 0x28);
  for (uint8_t i = 0; ok && i < count; i++) ok = i2cSend(data[i], 0x28);
  i2cStop(ok);
  return ok;
}

// Reads 'count' registers from 'reg' on. false = the module didn't answer.
inline bool clockModuleRead(uint8_t reg, uint8_t *data, uint8_t count) {
  bool ok = i2cStart() && i2cSend(CLOCK_MODULE_ADDRESS << 1, 0x18) && i2cSend(reg, 0x28) &&
            i2cStart() && i2cSend(CLOCK_MODULE_ADDRESS << 1 | 1, 0x40);
  for (uint8_t i = 0; ok && i < count; i++) {
    TWCR = _BV(TWINT) | _BV(TWEN) | (i + 1 < count ? _BV(TWEA) : 0);   // "More, please", but not after the last
    ok = i2cWait();
    data[i] = TWDR;
  }
  i2cStop(ok);
  return ok;
}

// Sets the module to the upload time, and marks its time as good again
inline bool setClockModuleToUploadTime() {
  uint8_t reg[7];
  uploadTimeRegisters(__DATE__ __TIME__, reg);
  uint8_t status;
  if (!clockModuleWrite(0, reg, 7) || !clockModuleRead(CLOCK_MODULE_STATUS, &status, 1)) return false;
  status &= ~CLOCK_MODULE_STOPPED;
  return clockModuleWrite(CLOCK_MODULE_STATUS, &status, 1);
}
#endif
