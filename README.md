# Simple Calculator

A simple calculator created with Qt Creator in C++.

![Screenshot](/screenshots/calculator-screenshot.jpeg?raw=true "Calculator Screenshot")

- Supports numbers up to 16 digits
- Supports keyboard presses
- Displays large finite results in scientific notation; reports overflow and division by zero

Input behavior:
- Digits and a decimal separator start a new number after `=`. An operator continues from the result.
- Both `.` and `,` are decimal separators on the keyboard; `=`, Enter and Return calculate.
- After an operator, `+/-` starts a signed second operand. `%` and Del wait until an operand is entered.
- For addition/subtraction, percentages are relative to the first operand (`200 + 10 % = 220`).
  For multiplication/division and standalone input, `%` divides the current operand by 100.
- `Error` clears the pending calculation; its tooltip explains the cause. Start again with a digit,
  a decimal separator or C. Del clears a displayed result and otherwise removes one input digit;
  keyboard Backspace is Del, and keyboard Delete is C.

Build and regression tests (Qt 5 with Qt Test and a C++11 compiler):

```text
mkdir build
cd build
qmake ../simpleCalculator.pro
make
cd ..
mkdir build-tests
cd build-tests
qmake ../tests/tests.pro
make
./calculator-tests -platform offscreen
```

For MSVC, use a developer command prompt with the Qt `bin` directory on PATH,
replace `make` with `nmake`, and run `release\calculator-tests.exe -platform offscreen`
for a release build (`qmake ../tests/tests.pro CONFIG+=release CONFIG-=debug`).
The tests cover issues #13–#25 under issue #9, including button and keyboard input,
chained operations, positive/negative overflow, and recovery after errors.

License: MIT
