# Simple Calculator

A desktop calculator built with C++ and Qt Widgets.

## Features

- **Expression editor:** type or paste expressions with parentheses and operator precedence.
- **History:** the latest 100 successful calculations are saved between launches. Click a result
  to reuse it, or use Clear history. Date, time and angle mode appear in the item's tooltip.
- **Memory:** MC clears memory, MR recalls it, M+ adds the current expression's value,
  and M- subtracts it. The M indicator's tooltip shows the stored number. Memory lasts for the session.
- **Light and dark themes:** the choice is saved automatically.
- **Scientific mode:** sin, cos, tan, sqrt, ln, log (base 10), abs, square, reciprocal,
  arbitrary powers, pi and e. The angle selector supports degrees and radians.

## Input

The expression line supports normal cursor movement, selection, copy and paste. Press Enter or `=`
to calculate, Escape or C to clear. Backspace/Delete edit the expression normally; the Del button
removes text before the cursor, or clears a completed result. Invalid expressions remain editable,
with an explanatory message below the result.

Digits start a new expression after a result; operators continue from it. Both `.` and `,` are
accepted decimal separators. The display uses up to 16 significant digits, while recalled values
and the expression line use 17 to preserve double precision. Input is limited to 512 characters
and 64 nested unary/group/function levels. Arithmetic uses `double`, not exact decimal arithmetic.

| Expression | Meaning/result |
| --- | --- |
| `2 + 3 * (4 - 1)` | `11` |
| `2 + 3 * 4` | `14` (multiplication has priority) |
| `2^3^2` | `512` (powers associate to the right) |
| `-2^2` | `-4`; use `(-2)^2` for `4` |
| `200 + 10%` | `220` |
| `200 - 10%` | `180` |
| `200 * 10%` | `20` |
| `sqrt(81) + log(100)` | `11` |
| `sin(30)` in Degrees | approximately `0.5` |
| `sin(pi/2)` in Radians | approximately `1` |
| `1e-3` | scientific notation for `0.001` |

A standalone percentage to the right of `+` or `-` is relative to the left value. In products,
quotients and powers, `%` is the operand divided by 100: `200 + 10% * 2` therefore gives `200.2`.
Write multiplication explicitly (`2*pi`, `2*(3+1)`). The on-screen parenthesis and constant buttons
insert `*` after a completed operand when needed.

Scientific function buttons wrap selected text, or the entire current expression when it ends in
an operand. Otherwise they insert a function template and put the cursor inside its parentheses.
`+/-` negates the selection or the current expression; after an operator it starts a negative operand.
MR and history recall fill a pending operand; otherwise they replace the expression with the saved value.

Division by zero, non-finite results, negative square roots, non-positive logarithms and undefined
tangents produce messages. Complex numbers are not supported. Very small finite values may underflow
or round to zero with floating-point arithmetic.

History, theme, angle mode and panel visibility use QSettings under organization
`QtSimpleCalculatorTeam`, application `Calculator` (on Windows, the current user's registry).
Calculations are stored locally; there is no network service.

## Build with CMake

Requirements: CMake 3.16+, Qt 5.12+ with Widgets, and a C++11 compiler. Use a Qt kit matching
the compiler and architecture. Replace the Qt path with your installation:

```text
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

For Ninja with MSVC, use a Visual Studio developer command prompt and add `-G Ninja` when configuring.
Qt Creator can open `CMakeLists.txt` directly with a Qt 5 kit. On Linux/macOS, point `CMAKE_PREFIX_PATH`
to the relevant Qt installation, or omit it if Qt is discoverable.

The executable is in `build` for Ninja or `build/Release` for Visual Studio. On macOS it is an app
bundle. On Windows, put the matching Qt `bin` directory on PATH to launch it, or use `windeployqt`
on the executable to prepare a distributable application.

## Build with qmake

```text
mkdir build-qmake
cd build-qmake
qmake ../simpleCalculator.pro
make
```

For MSVC, use `nmake` in a developer command prompt with Qt on PATH.

CMake builds the application only. Existing regression sources in `tests/` refer to the earlier
calculator behavior; they have not been adapted or run for this feature update.

License: MIT
