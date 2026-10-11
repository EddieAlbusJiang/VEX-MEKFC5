# Screen

   ## Introduction

 The `screen` class is a derived class of the [`controller`](index.html) class. It provides functions for displaying text, numbers, and values on the V5 Controller screen.

 A `screen` object is accessed through a `controller` object. It is not constructed directly.

 The V5 Controller screen has 3 rows and 19 columns for displaying text and numbers.

 ![A screenshot of the V5 Controller screen, showing the rows and columns in the printable area. Row 1 Column 1 starts at the upper left corner and row 3, column 19 are in the lower right corner.](../../../_images/controller-resolution.png)

   ## Access

 The `controller` class provides a `Screen` object. It is an instance of the `screen` class.

 | Object | Example Usage | Description |
| --- | --- | --- |
| `Screen` | `Controller.Screen.print(“Hello”);` | The V5 Controller screen. |

   ## Notes

 - Before using a `screen` member function, create a `controller` object.
- The Controller screen has 3 rows and 19 columns.
- The cursor position determines where the next printed value appears.

```cpp
// Create a Controller object
controller Controller = controller();
```

   ## Member Functions

 The `screen` class includes the following member functions:

 **Actions — Display text or clear the Controller screen.**

 - [`print`](#print) — Displays formatted text, numbers, or values on the Controller screen.
- [`setCursor`](#setcursor) — Moves the cursor to a specified row and column.
- [`newLine`](#newline) — Clears the rest of the current row and moves the cursor to the next row.
- [`clearScreen`](#clearscreen) — Clears the Controller screen.
- [`clearLine`](#clearline) — Clears one row on the Controller screen.

 **Getters — Read the current cursor position.**

 - [`row`](#row) — Returns the current cursor row.
- [`column`](#column) — Returns the current cursor column.

   ## Actions

  ### print

 `print` displays formatted text, numbers, or values on the V5 Controller screen at the current cursor position.

 **Available Functions**

```cpp
void print(
  const char* format,
  ...
);
```

```cpp
void print(
  char* format,
  ...
);
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `format` | `const char*` | A format string provided as a string literal or read-only C-style string, such as `“Score: %d”`. |
| `format` | `char*` | A format string provided as a modifiable character array. |
| `…` |  | Optional. One or more additional values inserted into the format string, separated by commas. |

 **Return Value**

 This function does not return a value.

 **Notes**

 - Output begins at the current cursor position on the Controller screen.
- When using a format string, the number and types of values passed must match the [format specifiers](../Logic/Custom_formatting.html#formatting-numbers) in the string.

 **Example**

```cpp
// Display a message at the starting cursor position
Controller.Screen.print("Hello, Robot!");
```

```cpp
// Display a formatted number
Controller.Screen.print("Score: %d", 25);
```

   ### setCursor

 `setCursor` moves the cursor to a specified row and column on the V5 Controller screen. The next printed value will appear at that position.

 **Available Function**

```cpp
void setCursor(
  int32_t row,
  int32_t col
);
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `row` | `int32_t` | The row number where the cursor will be placed, from `1` to `3`. |
| `col` | `int32_t` | The column number where the cursor will be placed, from `1` to `19`. |

 **Return Value**

 This function does not return a value.

 **Example**

```cpp
// Display text starting at row 2, column 3
Controller.Screen.setCursor(2, 3);
Controller.Screen.print("Row 2, Column 3");
```

   ### newLine

 `newLine` clears the rest of the current row and moves the cursor to column 1 on the next row of the V5 Controller screen.

 **Available Function**

```cpp
void newLine();
```

 **Parameters**

 This function does not have parameters.

 **Return Value**

 This function does not return a value.

 **Example**

```cpp
// Display two lines of text
Controller.Screen.print("Line 1");
Controller.Screen.newLine();
Controller.Screen.print("Line 2");
```

   ### clearScreen

 `clearScreen` clears all text from the V5 Controller screen.

 **Available Function**

```cpp
void clearScreen();
```

 **Parameters**

 This function does not have parameters.

 **Return Value**

 This function does not return a value.

 **Example**

```cpp
// Clear the Controller screen after 2 seconds
Controller.Screen.print("VEXcode");
wait(2, seconds);
Controller.Screen.clearScreen();
```

   ### clearLine

 `clearLine` clears one row on the V5 Controller screen.

 **Available Functions**

```cpp
void clearLine(
  int line
);
```

```cpp
void clearLine();
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `line` | `int` | Optional. The row to clear, from `1` to `3`. If no row is provided, the current row is cleared. |

 **Return Value**

 This function does not return a value.

 **Notes**

 - Clearing a row does not change the current cursor position.

 **Example**

```cpp
// Display text on two rows
Controller.Screen.print("This text stays");
Controller.Screen.newLine();
Controller.Screen.print("This disappears");

// Wait 3 seconds before clearing row 2
wait(3, seconds);
Controller.Screen.clearLine(2);
```

    ## Getters

  ### row

 `row` returns the current cursor row on the V5 Controller screen.

 **Available Function**

```cpp
int32_t row();
```

 **Parameters**

 This function does not have parameters.

 **Return Value**

 Returns an `int32_t` representing the current cursor row. Row numbering starts at `1`.

 **Example**

```cpp
// Display the cursor's current row
Controller.Screen.setCursor(3, 2);
Controller.Screen.print(Controller.Screen.row());
```

   ### column

 `column` returns the current cursor column on the V5 Controller screen.

 **Available Function**

```cpp
int32_t column();
```

 **Parameters**

 This function does not have parameters.

 **Return Value**

 Returns an `int32_t` representing the current cursor column. Column numbering starts at `1`.

 **Example**

```cpp
// Display the cursor's current column
Controller.Screen.setCursor(3, 2);
Controller.Screen.print(Controller.Screen.column());
```
