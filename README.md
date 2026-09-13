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

Build with CMake (recommended):

Requirements: CMake 3.16+, Qt 5.12+ with Widgets, and a C++11 compiler.
Use a Qt kit matching the compiler and architecture, for example MSVC x64 with MSVC x64 Qt.

Run these commands from the repository root; replace the Qt path with your installation:

```text
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

On Linux/macOS, set `CMAKE_PREFIX_PATH` to the appropriate Qt installation, or omit it
if CMake already finds Qt. For Ninja with MSVC, run in a Visual Studio developer
command prompt and add `-G Ninja` to the configure command. Qt Creator can open
`CMakeLists.txt` directly; select a Qt 5 kit.

The `simpleCalculator` executable is in `build` for single-configuration generators
such as Ninja, or `build/Release` for Visual Studio. On macOS it is an app bundle.
On Windows, put the Qt `bin` directory on PATH before launching the application.
For a distributable Windows application, use the matching Qt `windeployqt` tool on the executable.

The existing qmake build remains available:

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
