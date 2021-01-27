---
sort: 8
---
## Guider

The best and simplest example of a working `GuiderInterface` is the `ArduinoST4` driver.
This class is clean and minimal and adding an explanation here would be redundant.

The only thing implemented in this class not covered by the tutorials so far is the
timer functions `IEAddTimer` and `IERmTimer`, but they should be pretty straight forward
to understand. They are just adding/removing delayed callbacks to functions.

If you want to know how to implement a guider driver, look at `arduino_st4.h` and
`arduino_st4.cpp` in the [main INDI library](https://github.com/indilib/indi).
