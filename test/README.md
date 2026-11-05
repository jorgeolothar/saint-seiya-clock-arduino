# 🧪 Testing on a Computer

These tests build the sketch on a computer instead of the Arduino, so you can check a change without uploading it. They're for changing the program itself. You don't need them to set up your clocks or make your own flames.

## Running them

You need `bash`, `git` and `g++`: Linux, macOS, or WSL on Windows. From the top folder of the repo:

```bash
test/run.sh            # compare with the last commit
test/run.sh main~2     # compare with another commit, branch or tag
```

## What they do

**Regression.** Builds the sketch twice, once as it is in the last commit and once as it is in your folder, and compares the LED colors. Both copies get rings 2 and 3 switched on, so hours, minutes and seconds rings are all tested. Each copy draws:

- every pair of flames in `FLAME_LIST`, on every ring,
- at the start of each step and each eighth of a step, over two laps,
- at 300 random times of day,
- with and without flicker,
- and the color test.

`IDENTICAL` means the change didn't change a single LED color. `DIFFERENT` is expected when you change a look or how flames are drawn on purpose.

**Checks.** Tests the parts that have nothing to compare with: color lists of other lengths, `SNAP` and `HOLD`, the power limit, and the temple scenes: which temples are on at which hour (`SPOTLIGHT`, `FILL`, 8 and 12 hour cycles, `ALL_TEMPLES_ON`), where each temple's LEDs are on the chain, and what `REPEAT`, `ONCE_HOLD` and `ONCE_OFF` do. The scene checks use a test scene of their own, so editing `Scenes.h` doesn't break them. All four scenes in `Scenes.h` are switched on for this, so they're all compiled.

## What they don't check

- **The real Arduino.** They don't compile for the Uno or Nano, so still click Verify in the Arduino IDE to see that it fits, and look at the real LEDs.
- **FastLED's own math.** The `stub/` folder holds a small stand-in for FastLED, the Arduino and the EEPROM. Its rainbow colors and flicker pattern are not FastLED's, so the printed colors of `RAINBOW_COLORS` flames and the flicker are not what the LEDs show. That's fine for comparing two versions, which both use the same stand-in.
- **All of FastLED's names.** The real FastLED defines macros, and a sketch name that matches one breaks only in the Arduino IDE. The stand-in defines the ones known to clash (`LOOP`, `LEDS`, at the end of `stub/FastLED.h`). If the IDE shows an error the tests didn't, add the name there.
- **Program space.** On the Arduino, the temple lists are kept in program space and copied out with `memcpy_P`. A computer has only one kind of memory, so there it's a plain copy, and the tests can't tell a wrong read from a right one. `InFlash` makes a read without `get()` a compile error, but only the real LEDs prove it works.
- **Timing.** Time is set by the test, not counted, so it says nothing about speed or smoothness.

## Files

| File | What it is |
|---|---|
| [run.sh](run.sh) | Builds and runs everything |
| [regression.cpp](regression.cpp) | Draws every flame pair at many times and prints the colors |
| [checks.cpp](checks.cpp) | The checks |
| [stub/](stub/) | The stand-ins for FastLED, the Arduino and the EEPROM |
