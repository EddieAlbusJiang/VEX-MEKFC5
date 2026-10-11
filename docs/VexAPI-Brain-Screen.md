# Screen

   ## Introduction

 The `screen` class is derived from the [`brain`](index.html) base class. It controls how the V5 Brain shows text, numbers, and graphics on its touchscreen, and how it responds to touch input.

 By default, the font for printing to the Brain is **monospaced small** which has 12 rows and 48 columns.

 For drawing, the Brain’s resolution is 480 x 240 pixels.

 ![The V5 Brain screen with red grid lines showing the layout of 12 rows and 48 columns in total. The pixel dimensions are 479 wide by 239 tall. The upper left corner begins at (0,0) pixels and row 1, column 1, while the lower right corner ends at (479, 239) pixels and row 12, column 48.](../../../_images/brain-resolution.png)

   ## Access

 The `screen` class can be accessed by:

```cpp
Brain.Screen
```

   ## Notes

 - The `screen` object is provided by the Brain. It is not constructed directly.
- Projects use a single global `Brain` object, so screen functions are called using `Brain.Screen`.

   ## Example

```cpp
/* This constructor is required when using VS Code.
A Brain is generated automatically at the start of
VEXcode projects. */

// Create the V5 Brain
brain Brain = brain();

// Print text to the Brain screen
Brain.Screen.print("Hello!");
```

   ## Member Functions

 The `screen` class includes the following member functions:

 - [`print`](#print) — Prints formatted text, numbers, or Boolean values to the Brain screen.
- [`setCursor`](#setcursor) — Moves the text cursor to a specified row and column.
- [`newLine`](#newline) — Clears the remainder of the current line and moves the cursor to the next line.
- [`clearLine`](#clearline) — Clears the remainder of the current line or clears a specified line.
- [`row`](#row) — Returns the row where text will be printed.
- [`column`](#column) — Returns the column where text will be printed.
- [`getStringHeight`](#getstringheight) — Returns the height of a string in pixels using the current font.
- [`getStringWidth`](#getstringwidth) — Returns the width of a string in pixels using the current font.
- [`printAt`](#printat) — Prints formatted text at an x, y pixel location.
- [`clearScreen`](#clearscreen) — Clears all drawings and text from the Brain screen.
- [`setFont`](#setfont) — Sets the font used for printing on the screen.
- [`setPenWidth`](#setpenwidth) — Sets the thickness of drawn lines and shape outlines.
- [`setPenColor`](#setpencolor) — Sets the color of text, pixels, lines, and shape outlines.
- [`setFillColor`](#setfillcolor) — Sets the fill color for drawn shapes and printed text backgrounds.
- [`setOrigin`](#setorigin) — Sets the origin used for screen coordinates.
- [`setClipRegion`](#setclipregion) — Restricts screen output to a rectangular region.
- [`drawPixel`](#drawpixel) — Draws one pixel at a coordinate.
- [`drawLine`](#drawline) — Draws a line between two points.
- [`drawRectangle`](#drawrectangle) — Draws a rectangle at an x, y location with a specified size.
- [`drawCircle`](#drawcircle) — Draws a circle at an x, y location with a specified radius.
- [`drawImageFromFile`](#drawimagefromfile) — Draws an image from an SD card file onto the screen.
- [`render`](#render) — Enables double buffering or renders the back buffer to the screen.
- [`pressing`](#pressing) — Returns whether the screen is currently being pressed.
- [`xPosition`](#xposition) — Returns the x-coordinate of the most recent screen press.
- [`yPosition`](#yposition) — Returns the y-coordinate of the most recent screen press.
- [`pressed`](#pressed) — Registers a callback function to run when the screen is pressed.
- [`released`](#released) — Registers a callback function to run when the screen is released after being pressed.

 Before calling any `screen` member functions, a `brain` instance must be created, as shown below:

```cpp
// Create the V5 Brain
brain Brain = brain();
```

  ### print

 Displays text, numbers, or Boolean values on the V5 Brain screen.

 **Available Functions**

   ****1** —** Prints formatted text using string literals or read-only C-style strings.

```cpp
void print(
    const char* format,
    ... );
```

     ****2** —** Prints formatted text using modifiable character arrays.

```cpp
void print(
    char* format,
    ... );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `format` | `const char*` | A format string provided as a string literal or read-only C-style string (for example, `“Score: %d”`). |
| `format` | `char*` | A format string provided as a modifiable character array. |
| `…` |  | One or more additional values that are inserted into the format string, separated by commas. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Output begins at the [current cursor position](#setcursor) on the screen.
- Printed text uses the [current screen font](#setfont).
- Printed text uses the [current pen color](#setpencolor).
- Text printed with `print` is always printed with a background, and the background color is determined by the [current fill color](#setfillcolor).
- When using a format string, the number and types of values passed must match the [format specifiers](../Logic/Custom_formatting.html#formatting-numbers) in the string.

 **Examples**

```cpp
// Display a message at the starting cursor
Brain.Screen.print("Hello, Robot!");
```

 ![A screenshot of the V5 Brain showing Hello, Robot! printed on the screen in the upper left corner in white text.](../../../_images/print-on-screen.png)

   ### setCursor

 Moves the cursor to a specific row and column on the V5 Brain screen. The next [`print`](#print) call will start printing at that location.

 **Available Functions**

```cpp
void setCursor(
  int32_t row,
  int32_t col );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `row` | `int32_t` | The row to move the cursor to. |
| `col` | `int32_t` | The column to move the cursor to. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Affects where subsequent text output begins when using [`print`](#print).
- Rows and columns are based on the [currently set screen font](#setfont).
- The default font (`mono20`) uses 12 rows and 48 columns.
- Calling `setCursor` affects the position of subsequent text output only.

 **Examples**

```cpp
// Display text starting at Row 3 Column 12
Brain.Screen.setCursor(3, 12);
Brain.Screen.print("Row 3, Column 12");
```

 ![A screenshot of the V5 Brain showing Row 3 Column 12 printed in the upper portion near the center of screen, at that position.](../../../_images/set-cursor.png)

   ### newLine

 Moves the cursor to the beginning of the next row on the Brain screen.

 **Available Functions**

```cpp
void newLine();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 This function does not return a value.

 **Notes**

 - If the cursor is in the middle of a sentence, `newLine` will clear the rest of the row after the cursor before moving to the next row.
- After calling `newLine`, the cursor is positioned at the first column of the next line.
- The cursor position and row numbering are set using [`setCursor`](#setcursor).

 **Examples**

```cpp
// Display two lines of text
Brain.Screen.print("Line 1");
Brain.Screen.newLine();
Brain.Screen.print("Line 2");
```

 ![A screenshot of the V5 Brain screen showing two lines of white text in the upper left corner. The top line reads Line 1, and the second line directly below it reads Line 2.](../../../_images/next-row.png)

   ### clearLine

 Clears text from a row on the V5 Brain screen.

 **Available Functions**

   ****1** —** Clears the rest of the current line from the cursor position.

```cpp
void clearLine();
```

     ****2** —** Clears the specified row using the currently configured [fill color](#setfillcolor).

```cpp
void clearLine(
    int number );
```

     ****3** —** Clears the specified row using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void clearLine(
    int          number,
    const color& color );
```

     ****4** —** Clears the specified row using a hexadecimal color value.

```cpp
void clearLine(
    int         number,
    const char* color );
```

     ****5** —** Clears the specified row using a hue value.

```cpp
void clearLine(
    int number,
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `number` | `int` | The row number to clear. Row numbering starts at `1`. |
| `color` | `const color&` | Clears the row using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent`This can also use a [custom color object](../Logic/Custom_color.html). |
| `color` | `const char*` | Clears the row using a hexadecimal color value (for example, `“#000000”`). |
| `hue` | `int` | Clears the row using an integer hue value in the range `0`–`359`, representing degrees around a color wheel. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Clearing a line does not change the current cursor position.
- The cursor position and row numbering are set using [`setCursor`](#setcursor).
- The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Display text on two rows
Brain.Screen.print("This text stays");
Brain.Screen.newLine();
Brain.Screen.print("This disappears");

// Wait 3 seconds before clearing row 2
wait(3, seconds);
Brain.Screen.clearLine(2);
```

   ### row

 Returns the row where text will be printed on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t row();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns an `int32_t` representing the row where text will be printed.

 Row numbering starts at 1.

 **Notes**

 - The cursor position is based on the [current screen font](#setfont).
- The returned value indicates where the next printed output will appear.
- The row value changes when text is printed or when [`setCursor`](#setcursor) is called.

 **Examples**

```cpp
// Display the cursor's current row
Brain.Screen.setCursor(3, 12);
Brain.Screen.print(Brain.Screen.row());
```

 ![A screenshot of the V5 Brain screen showing the text 3 in white in the upper left quadrant of the screen, at the row 3 column 12 position.](../../../_images/row.png)

   ### column

 Returns the column where text will be printed on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t column();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns an `int32_t` representing the column where text will be printed.

 Column numbering starts at 1.

 **Notes**

 - The cursor position is based on the [current screen font](#setfont).
- The returned value indicates where the next printed output will appear.
- The column value changes when text is printed or when [`setCursor`](#setcursor) is called.

 **Examples**

```cpp
// Display the cursor's current column
Brain.Screen.setCursor(5, 15);
Brain.Screen.print(Brain.Screen.column());
```

 ![A screenshot of the V5 Brain screen showing the text 15 in white in the upper left quadrant of the screen, at the row 3 column 15 position.](../../../_images/column.png)

   ### getStringHeight

 Returns the height, in pixels, of a string when rendered on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t getStringHeight(
    const char* cstr );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `cstr` | `const char*` | The text to be measured, provided as a [C-style string](../Logic/Custom_formatting.html#strings) (for example, `“Hello”`). |

 **Return Values**

 Returns an `int32_t` representing the height of the string in pixels when rendered on the Brain screen.

 **Notes**

 - The returned height depends on the [currently selected screen font](#setfont).
- This function measures the string without drawing it to the screen.
- The returned height reflects how the string would appear when printed.

   ### getStringWidth

 Returns the width, in pixels, of a string when rendered on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t getStringWidth(
    const char* cstr );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `cstr` | `const char*` | The text to be measured, provided as a [C-style string](../Logic/Custom_formatting.html#strings) (for example, `“Hello”`). |

 **Return Values**

 Returns an `int32_t` representing the width of the string in pixels when rendered on the Brain screen.

 **Notes**

 - The returned width depends on the [currently selected screen font](#setfont).
- This function measures the string without drawing it to the screen.
- The returned width reflects how the string would appear when printed.

   ### printAt

 Prints text, numbers, or Boolean values at a specific coordinate on the V5 Brain screen.

 **Available Functions**

   ****1** —** Prints formatted output at the specified pixel location.

```cpp
void printAt(
    int32_t    x,
    int32_t    y,
    const char* format,
    ... );
```

     ****2** —** Prints formatted output at the specified pixel location and sets whether the text background is opaque or transparent.

```cpp
void printAt(
    int32_t    x,
    int32_t    y,
    bool       bOpaque,
    const char* format,
    ... );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int32_t` | The x-position to print at, referenced to the screen origin. |
| `y` | `int32_t` | The y-position to print at, referenced to the screen origin. |
| `bOpaque` | `bool` | Controls whether the printed text is drawn opaquely or transparently:<br> `true` — The background of the printed text is filled using the [current fill color](#setfillcolor).; `false` — The text is drawn transparently and the background is not filled. |
| `format` | `const char*` | A format string that controls what is printed on the screen (for example, `“Score: %d”`). |
| `…` |  | One or more additional values that are inserted into the format string, separated by commas. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The `x` and `y` coordinates are relative to the current screen origin, which can be changed using [`setOrigin`](#setorigin).
- Printed text uses the [current screen font](#setfont).
- Printed text uses the [current pen color](#setpencolor).
- When using a format string, the number and types of values passed must match the [format specifiers](../Logic/Custom_formatting.html#formatting-numbers) in the string.

 **Examples**

```cpp
// Print "Center" at the center of the screen
Brain.Screen.printAt(220, 120, "Center");
```

 ![A screenshot of the V5 Brain screen with white text that reads Center, centered on the screen.](../../../_images/print-at.png)

   ### clearScreen

 Erases all drawings and text from the Brain screen and resets the cursor position to row 1, column 1.

 **Available Functions**

   ****1** —** Clears the screen to black.

```cpp
void clearScreen();
```

     ****2** —** Clears the screen using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void clearScreen(
    const color& color );
```

     ****3** —** Clears the screen using a hexadecimal color value.

```cpp
void clearScreen(
    const char* color );
```

     ****4** —** Clears the screen using a hue value.

```cpp
void clearScreen(
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `color` | `const color&` | Clears the screen using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent`This can also use a [custom color object](../Logic/Custom_color.html). |
| `color` | `const char*` | Clears the screen using a hexadecimal color value represented as a string (for example, `“#000000”`). |
| `hue` | `int` | Clears the screen using an integer hue value in the range `0`–`359`, representing degrees around a color wheel. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Clear screen after 2 seconds
Brain.Screen.print("VEXcode");
wait(2, seconds);
Brain.Screen.clearScreen();
```

```cpp
// Clear screen to blue after 2 seconds
Brain.Screen.print("VEXcode");
wait(2, seconds);
Brain.Screen.clearScreen(blue);
```

 ![A screenshot of the V5 Brain screen with the entire printable area bright blue.](../../../_images/clear-screen-color.png)

   ### setFont

 Sets the font used for text displayed on the V5 Brain screen.

 This controls the font applied to subsequent text output when using screen text functions such as [`print`](#print).

 **Available Functions**

```cpp
void setFont(
  fontType font );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `font` | `fontType` | Sets the font to one of the following:<br> `mono12`; `mono15`; `mono20`; `mono30`; `mono40`; `mono60`; `prop20`; `prop30`; `prop40`; `prop60`; `fontType::cjk16` — Can print Chinese, Japanese, and Korean characters. Simplified Chinese — Can be seen up to column 28.; Traditional Chinese — Can be seen up to column 28.; Japanese (Kanji) — Can be seen up to column 28.; Korean (Hangul) — Can be seen up to column 30.These options are shown below. |

 | ![TA screenshot of the V5 Brain screen with numbers and letters printed in white in the upper left corner in MONO 12 font. It shows A-Z on one line, across about a quarter of the width of screen. In the bottom left corner it reads 80 columns and 20 rows.](../../../_images/mono12.png)<br>`mono12` | ![The same image as the one previous, with Mono 15 font. It shows A-Z on one line, across nearly half width of the screen. In the bottom left corner it reads 68 columns and 16 rows.](../../../_images/mono15.png)<br>`mono15` | ![The same image as the one previous, with Mono 20 font. It shows A-Z on one line, across nearly two thirds of the width of the screen. In the bottom left corner it reads 48 columns and 12 rows.](../../../_images/mono20.png)<br>`mono20` |
| --- | --- | --- |
| ![The same image as the one previous, with Mono 30 font. It shows A-Z on one line, across nearly the whole width of the screen. In the bottom left corner it reads 32 columns and 8 rows.](../../../_images/mono30.png)<br>`mono30` | ![The same image as the one previous, with Mono 40 font. It shows A-X on one line, spanning the width the screen. In the bottom left corner it reads 24 columns and 6 rows.](../../../_images/mono40.png)<br>`mono40` | ![The same image as the one previous, with Mono 60 font. It shows A-P on one line, spanning the width the screen. On the bottom it indicates 16 columns and 4 rows.](../../../_images/mono60.png)<br>`mono60` |
| ![The same image as the one previous, with Prop 20 font. It shows A-Z on one line, across nearly two thirds of the width of the screen. In the bottom left corner it reads 48 columns and 12 rows.](../../../_images/prop20.png)<br>`prop20` | ![The same image as the one previous, with Prop 30 font. It shows A-Z on one line, spanning nearly the width of the screen. In the bottom left corner it reads 32 columns and 8 rows.](../../../_images/prop30.png)<br>`prop30` | ![The same image as the one previous, with Prop 40 font. It shows A-U on one line, spanning the width of the screen. In the bottom left corner it reads 24 columns and 6 rows.](../../../_images/prop40.png)<br>`prop40` |
| ![The same image as the one previous, with Prop 60 font. It shows A-N on one line, spanning the width of the screen. On the bottom it reads 15 columns and 4 rows.](../../../_images/prop60.png)<br>`prop60` | ![The same image as the one previous, with CJK 16 font. It shows A-Z on one line, spanning about two thirds of the screen. The subsequent lines read VEXcode in simplified Chinese, traditional Chinese, Korean, and Japanese. On the bottom it reads 43 columns and 12 rows.](../../../_images/cjk16.png)<br>`fontType::cjk16` |  |

 **Return Values**

 This function does not return a value.

 **Examples**

```cpp
// Print larger text
Brain.Screen.setFont(mono40);
Brain.Screen.print("VEX");
```

 ![A screenshot of the V5 Brain screen showing white text that reads VEX in larger font than the default, in the upper left corner.](../../../_images/set-font.png)

```cpp
// Print in Chinese
Brain.Screen.setFont(fontType::cjk16);
Brain.Screen.print("VEX机器人");
```

 ![A screenshot of the V5 Brain screen showing the text "VEX机器人" in Chinese font in the upper left corner.](../../../_images/cjk16-example.png)

   ### setPenWidth

 Sets the thickness of drawn lines and shape outlines.

 **Available Functions**

```cpp
void setPenWidth(
  uint32_t width );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `width` | `uint32_t` | The pen width, in pixels, from 0 to 32. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The pen width applies to drawing functions such as [`drawLine`](#drawline), [`drawRectangle`](#drawrectangle), and [`drawCircle`](#drawcircle).

 **Examples**

```cpp
// Draw a rectangle with a pen width of 10
Brain.Screen.setPenWidth(10);
Brain.Screen.drawRectangle(50, 50, 130, 60);
```

 ![A screenshot of the V5 Brain screen showing a rectangle with thick borders printed in the upper left quadrant of the screen.](../../../_images/set-pen-width.png)

   ### setPenColor

 Sets the color of text, pixels, lines, and shape outlines.

 **Available Functions**

   ****1** —** Sets the pen color using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void setPenColor(
    const color& color );
```

     ****2** —** Sets the pen color using a hexadecimal color value.

```cpp
void setPenColor(
    const char* color );
```

     ****3** —** Sets the pen color using a hue value.

```cpp
void setPenColor(
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `color` | `const color&` | Sets the pen color using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent` |
| `color` | `const char*` | Sets the pen color using a hexadecimal color value represented as a string (for example, `“#FF0000”`). |
| `hue` | `int` | Sets the pen color using an integer hue value in the range `0`–`359`, representing degrees around a color wheel (for example, `0` is red, `120` is green, and `240` is blue). |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The default pen color at the start of a project is `white`.
- The pen color is used for outlines of shapes (such as with [`drawCircle`](#drawcircle) or [`drawRectangle`](#drawrectangle)), text and numeric output (with [`print`](#print) and [`printAt`](#printat)), and individual pixels drawn with [`drawPixel`](#drawpixel).
- The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Draw a rectangle with orange borders
Brain.Screen.setPenColor(orange);
Brain.Screen.drawRectangle(50, 50, 130, 60);
```

 ![A screenshot of the V5 Brain screen showing a rectangle drawn in orange in the upper left quadrant of the screen.](../../../_images/set-pen-color.png)

   ### setFillColor

 Sets the fill color used when shapes are drawn.

 **Available Functions**

   ****1** —** Sets the fill color using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void setFillColor(
    const color& color );
```

     ****2** —** Sets the fill color using a hexadecimal color value.

```cpp
void setFillColor(
    const char* color );
```

     ****3** —** Sets the fill color using a hue value.

```cpp
void setFillColor(
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `color` | `const color&` | Sets the fill color using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent` |
| `color` | `const char*` | Sets the fill color using a hexadecimal color value represented as a string (for example, `“#00FF00”`). |
| `hue` | `int` | Sets the fill color using an integer hue value in the range `0`–`359`, representing degrees around a color wheel (for example, `0` is red, `120` is green, and `240` is blue). |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The default fill color at the start of a project is `black`.
- The fill color is used for interior of shapes (such as with [`drawCircle`](#drawcircle) or [`drawRectangle`](#drawrectangle)).
- The fill color is used as the background color for printed text (such as with [`print`](#print) and [`printAt`](#printat)).  - If `transparent` is used with printed text, the background will be `black`.
- The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Draw a rectangle filled with purple
  Brain.Screen.setFillColor(purple);
  Brain.Screen.drawRectangle(50, 100, 130, 60);
```

 ![A screenshot of the V5 Brain screen showing a rectangle filled with purple in the lower left quadrant of the screen.](../../../_images/set-fill-color.png)

   ### setOrigin

 Sets the origin used for drawing and coordinate-based printing on the Brain screen.

 **Available Functions**

```cpp
void setOrigin(
  int32_t x,
  int32_t y );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int32_t` | The origin’s x-position relative to the top-left corner of the Brain screen from 0 to 479. |
| `y` | `int32_t` | The origin’s y-position relative to the top-left corner of the Brain screen from 0 to 239. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The default origin is the top-left corner of the screen at coordinate (0, 0).
- The origin applies to coordinate functions such as [`drawLine`](#drawline), [`drawRectangle`](#drawrectangle), [`drawCircle`](#drawcircle), and [`printAt`](#printat).
- The origin remains in effect until changed again or the project is reset.

  <span style="font-size: 1.5rem; "> Examples </span> ```cpp /* This constructor is required when using VS Code. A Brain is generated automatically at the start of VEXcode projects. */ // Create the V5 Brain brain Brain = brain(); ``` <hr> ```cpp // Draw a rectangle with a pen width of 10 Brain.Screen.setPenWidth(10); Brain.Screen.drawRectangle(50, 50, 130, 60); ```    ### setClipRegion

 Defines a rectangular area on the screen where all drawings and text will be confined. Any content outside this region will not be displayed.

 **Available Functions**

```cpp
void setClipRegion(
  int x,
  int y,
  int width,
  int height );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int` | The x-coordinate of the top-left corner of the clip region, given as an integer from 0 to 479. |
| `y` | `int` | The y-coordinate of the top-left corner of the clip region, given as an integer from 0 to 239. |
| `width` | `int` | The width of the clip region in pixels, given as an integer from 0 to 479. |
| `height` | `int` | The height of the clip region in pixels, given as an integer from 0 to 239. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Drawing functions are still executed even if they fall outside the clip region, but only the portion inside the region is visible.
- The `x` and `y` coordinates are relative to the current screen origin, which can be changed using [`setOrigin`](#setorigin).
- The clip region applies only to the current thread.
- Setting a new clip region replaces any previously set clip region for the current thread.

 **Examples**

```cpp
// Restrict text and drawings to a specific region
Brain.Screen.setClipRegion(0, 0, 120, 120);
Brain.Screen.drawRectangle(60, 60, 100, 100, red);
Brain.Screen.printAt(60, 60, "Cut off!");
```

 ![A screenshot of the V5 Brain's screen with white text that reads Cut of with a solid red square directly below it in the upper left quadrant, reflecting what is printed within the clip region.](../../../_images/set_clip_region.png)

   ### drawPixel

 Draws one pixel on the V5 Brain screen at the selected x and y pixel location.

 **Available Functions**

```cpp
void drawPixel(
  int x,
  int y );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int` | The x-coordinate where the pixel will be drawn, given as an integer from 0 to 479. |
| `y` | `int` | The y-coordinate where the pixel will be drawn, given as an integer from 0 to 239. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Pixels are drawn using the current pen color set with [`setPenColor`](#setpencolor).
- The `x` and `y` coordinates are relative to the current screen origin, which can be set using [`setOrigin`](#setorigin).
- If the specified pixel location is outside the screen bounds, the pixel is not drawn.

 **Examples**

```cpp
// Draw the pixels marking the corners of a square
Brain.Screen.drawPixel(250, 100);
Brain.Screen.drawPixel(275, 100);
Brain.Screen.drawPixel(250, 125);
Brain.Screen.drawPixel(275, 125);
```

 ![A screenshot of the V5 Brain screen showing an assortment of pixels showing the corners of a square near the center of the screen.](../../../_images/draw-pixel.png)

   ### drawLine

 Draws a straight line connecting two points on the V5 Brain screen.

 **Available Functions**

```cpp
void drawLine(
  int x1,
  int y1,
  int x2,
  int y2 );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x1` | `int` | The starting x-coordinate of the line, given as an integer from 0 to 479. |
| `y1` | `int` | The starting y-coordinate of the line, given as an integer from 0 to 239. |
| `x2` | `int` | The ending x-coordinate of the line, given as an integer from 0 to 479. |
| `y2` | `int` | The ending y-coordinate of the line, given as an integer from 0 to 239. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Lines are drawn using the current pen color set with [`setPenColor`](#setpencolor).
- The width of the line is determined by the current pen width set with [`setPenWidth`](#setpenwidth).
- The `x` and `y` coordinates are relative to the current screen origin, which can be set using [`setOrigin`](#setorigin).
- If any portion of the line lies outside the screen bounds, only the visible portion is drawn.

 **Examples**

```cpp
// Draw a line from the top left to bottom right of the screen
Brain.Screen.drawLine(0, 0, 479, 239);
```

 ![A screenshot of the V5 Brain screen shows a thin diagonal line across the center, from the upper left corner to the lower right corner.](../../../_images/draw-line.png)

   ### drawRectangle

 Draws a rectangle on the V5 Brain screen at the specified position and size.

 **Available Functions**

   ****1** —** Draws and fills a rectangle using the currently configured [fill color](#setfillcolor).

```cpp
void drawRectangle(
    int x,
    int y,
    int width,
    int height );
```

     ****2** —** Draws and fills a rectangle using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void drawRectangle(
    int           x,
    int           y,
    int           width,
    int           height,
    const color&  color );
```

     ****3** —** Draws and fills a rectangle using a hexadecimal color value.

```cpp
void drawRectangle(
    int           x,
    int           y,
    int           width,
    int           height,
    const char*   color );
```

     ****4** —** Draws and fills a rectangle using a hue value.

```cpp
void drawRectangle(
    int x,
    int y,
    int width,
    int height,
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int` | The x-coordinate of the top-left corner of the rectangle, given as an integer from 0 to 479. |
| `y` | `int` | The y-coordinate of the top-left corner of the rectangle, given as an integer from 0 to 239. |
| `width` | `int` | The width of the rectangle, given as an integer from 0 to 479. |
| `height` | `int` | The height of the rectangle, given as an integer from 0 to 239. |
| `color` | `const color&` | Fills the rectangle using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent`This can also use a [custom color object](../Logic/Custom_color.html). |
| `color` | `const char*` | Fills the rectangle using a hexadecimal color value (for example, `“#FF0000”`). |
| `hue` | `int` | Fills the rectangle using a hue value in the range `0`–`359`. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The rectangle outline is drawn using the current pen color set with [`setPenColor`](#setpencolor).
- The outline thickness is determined by the current pen width set with [`setPenWidth`](#setpenwidth).
- When no `color` parameter is provided, the rectangle interior is filled using the current fill color set with [`setFillColor`](#setfillcolor).
- The `x` and `y` coordinates are relative to the current screen origin, which can be changed using [`setOrigin`](#setorigin).
- If any portion of the rectangle lies outside the screen bounds, only the visible portion is drawn.
- The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Draw a rectangle on the screen
Brain.Screen.drawRectangle(50, 50, 130, 60);
```

 ![A screenshot of the V5 Brain's screen shows a rectangle with a thin white border printed in the upper left quadrant.](../../../_images/draw-rectangle.png)

   ### drawCircle

 Draws a circle on the V5 Brain screen at the specified position and radius.

 **Available Functions**

   ****1** —** Draws and fills a circle using the currently configured [fill color](#setfillcolor).

```cpp
void drawCircle(
    int x,
    int y,
    int radius );
```

     ****2** —** Draws and fills a circle using a predefined or [custom color](../Logic/Custom_color.html) object.

```cpp
void drawCircle(
    int           x,
    int           y,
    int           radius,
    const color&  color );
```

     ****3** —** Draws and fills a circle using a hexadecimal color value.

```cpp
void drawCircle(
    int           x,
    int           y,
    int           radius,
    const char*   color );
```

     ****4** —** Draws and fills a circle using a hue value.

```cpp
void drawCircle(
    int x,
    int y,
    int radius,
    int hue );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `x` | `int` | The x-coordinate of the center of the circle, given as an integer from 0 to 479. |
| `y` | `int` | The y-coordinate of the center of the circle, given as an integer from 0 to 239. |
| `radius` | `int` | The radius of the circle, given as an integer from 0 to 239 pixels. |
| `color` | `const color&` | Fills the circle using a `color` object. Predefined colors include: `black`; `white`; `red`; `green`; `blue`; `yellow`; `orange`; `purple`; `cyan`; `transparent` This can also use a [custom color object](../Logic/Custom_color.html). |
| `color` | `const char*` | Fills the circle using a hexadecimal color value (for example, `“#FF0000”`). |
| `hue` | `int` | Fills the circle using a hue value in the range `0`–`359`. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The circle outline is drawn using the current pen color set with [`setPenColor`](#setpencolor).
- The outline thickness is determined by the current pen width set with [`setPenWidth`](#setpenwidth).
- When no `color` parameter is provided, the circle interior is filled using the current fill color set with [`setFillColor`](#setfillcolor).
- The `x` and `y` coordinates are relative to the current screen origin, which can be changed using [`setOrigin`](#setorigin).
- The `hue` parameter value is based on a circular color wheel, as shown below, where values represent degrees around the wheel (for example, red near 0°, green near 120°, and blue near 240°).

 ![A color wheel showing the range of colors from 0 to 359, where red is 0, green is 120, and blue is 240.](../../../_images/hue_range.png)

 **Examples**

```cpp
// Draw a circle on the screen
Brain.Screen.drawCircle(240, 120, 40);
```

 ![A screenshot of the V5 Brain screen shows a circle with a thin white border drawn in the center.](../../../_images/draw-circle.png)

   ### drawImageFromFile

 Draws an image on the V5 Brain screen using an image file stored on the [SD card](SDcard.html).

 **Available Functions**

```cpp
bool drawImageFromFile(
  const char* name,
  int x,
  int y );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `name` | `const char*` | The filename of the image on the SD card. The file must have a `.bmp` or `.png` extension. |
| `x` | `int` | The x-coordinate, in pixels from 0 to 479, at which the left edge of the image will be drawn. |
| `y` | `int` | The y-coordinate, in pixels from 0 to 239, at which the top edge of the image will be drawn. |

 **Return Values**

 Returns a Boolean indicating whether the image was successfully drawn:

 - `true` — The image was successfully loaded and drawn on the screen.
- `false` — The image could not be drawn (for example, if the file does not exist or is invalid).

  **Notes**

 - Supported image formats are `.bmp` and `.png`.
- The image file size must not exceed `512 KB`.  - 8-bit RLE encoding is recommended to minimize file size.
- The maximum supported image dimensions are the size of the Brain screen (approximately 479 x 239 pixels).
- The `x` and `y` coordinates are relative to the current screen origin, which can be changed using [`setOrigin`](#setorigin).

 **Examples**

```cpp
// Draw a bmp file on the Brain's screen at coordinate 0, 0
Brain.Screen.drawImageFromFile("test_image.bmp", 0, 0);
```

   ### render

 Enables double buffering for the Brain’s screen. Once called, any drawing functions (like text, shapes, or images) will no longer appear immediately for the rest of the project. Instead, updates will only be shown when `render` is used again. This allows for smoother and more controlled screen updates, but means nothing will be visible until `render` is used.

 **Available Functions**

   ****1** —** Enables double buffering or renders the back buffer to the screen.

```cpp
bool render();
```

     ****2** —** Renders the back buffer to the screen with control over vertical sync behavior and optional scheduler execution.

```cpp
bool render(
    bool bVsyncWait,
    bool bRunScheduler = true );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `bVsyncWait` | `bool` | `true` — Wait for the screen’s vertical refresh before rendering to reduce visual tearing.; `false` — Do not wait for the screen’s vertical refresh before rendering. |
| `bRunScheduler` | `bool` | Optional. `true` (default) — Allow background tasks to run while waiting to render.; `false` — Do not allow background tasks to run while waiting to render. |

 **Return Values**

 Returns a Boolean indicating whether the saved drawings in the buffer were successfully rendered to the screen:

 - `true` — The drawings were successfully rendered on the screen.
- `false` — The drawings could not be rendered on the screen.

   ### pressing

 Reports whether the V5 Brain screen is currently being pressed.

 **Available Functions**

```cpp
bool pressing();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns a Boolean indicating the current pressed state of the screen:`true` — The screen is currently being pressed. or `false` — The screen is not being pressed.

 **Examples**

```cpp
// Change the screen's color after it's pressed
while (Brain.Screen.pressing() == false) {
  wait(5, msec);
}

Brain.Screen.setFillColor(green);
Brain.Screen.drawRectangle(0, 0, 479, 239);
```

```cpp
// Display different messages after the screen is pressed
while (!Brain.Screen.pressing()) {
  wait(5, msec);
}
Brain.Screen.print("First message!");
Brain.Screen.newLine();

// Lift finger to press the screen again
while (Brain.Screen.pressing()) {
  wait(5, msec);
}
while (Brain.Screen.pressing() == false) {
  wait(5, msec);
}
Brain.Screen.print("Second message!");
Brain.Screen.newLine();
```

   ### xPosition

 Returns the x-coordinate of the last press on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t xPosition();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns an `int32_t` representing the x-coordinate, in pixels from 0 to 479, of the last screen press.

 **Examples**

```cpp
// Display a circle where the screen is pressed
while (Brain.Screen.pressing() == false) {
  wait(5, msec);
}

Brain.Screen.drawCircle(Brain.Screen.xPosition(), Brain.Screen.yPosition(), 20, white);
```

   ### yPosition

 Returns the y-coordinate of the last press on the V5 Brain screen.

 **Available Functions**

```cpp
int32_t yPosition();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns an `int32_t` representing the y-coordinate, in pixels from 0 to 239, of the last screen press.

 **Examples**

```cpp
// Display a circle where the screen is pressed
while (Brain.Screen.pressing() == false) {
  wait(5, msec);
}

Brain.Screen.drawCircle(Brain.Screen.xPosition(), Brain.Screen.yPosition(), 20, white);
```

   ### pressed

 Registers a function to be called when the V5 Brain screen is pressed.

 **Available Functions**

   ****1** —** Calls a function when the screen is pressed.

```cpp
void pressed(
    void (*callback)(void)
);
```

     ****2** —** Calls a function when the screen is pressed and passes a user-defined argument to the callback function.

```cpp
void pressed(
    void (*callback)(void*),
    void* arg );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A function that is called when the screen is pressed. |
| `callback` | `void ()(void)` | A function that is called when the screen is pressed and receives a user-defined argument. |
| `arg` | `void*` | A user-defined value that is passed to the callback function when the screen is pressed. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The callback function is executed each time the Brain screen is pressed.
- Only one callback function can be registered at a time; calling `pressed` again replaces the previous callback.
- The callback function must return `void`.

 **Examples**

   Define the callback function (outside of `int main()`)

```cpp
// Tracks how many times the screen is pressed.
void onScreenPressed(void* arg) {
  int* count = (int*)arg;
  (*count)++;

  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print("Pressed %d times", *count);
}
```

 Register the callback inside `int main()`

```cpp
int main() {
  int pressCount = 0;

  // Register the callback and pass a value to update.
  Brain.Screen.pressed(onScreenPressed, &pressCount);

  while (true) {
    wait(20, msec);
  }
}
```

     ### released

 Registers a function to be called when the V5 Brain screen is released after being pressed.

 **Available Functions**

   ****1** —** Calls a function when the screen is released after being pressed.

```cpp
void released(
    void (*callback)(void)
);
```

     ****2** —** Calls a function when the screen is released after being pressed and passes a user-defined argument to the callback function.

```cpp
void released(
    void (*callback)(void*),
    void* arg );
```

   **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A function that is called when the screen is released after being pressed. |
| `callback` | `void ()(void)` | A function that is called when the screen is released after being pressed and receives a user-defined argument. |
| `arg` | `void*` | A user-defined value that is passed to the callback function when the screen is released after being pressed. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The callback function is executed each time the Brain screen is released after being pressed.
- Only one callback function can be registered at a time; calling `released` again replaces the previous callback.
- The callback function must return `void`.

 **Examples**

   Define the callback function (outside of `int main()`)

```cpp
// Tracks how many times the screen is released.
void onScreenReleased(void* arg) {
  int* count = (int*)arg;
  (*count)++;

  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print("Released %d times", *count);
}
```

 Register the callback inside `int main()`

```cpp
int main() {
  int releaseCount = 0;

  // Register the callback and pass a value to update.
  Brain.Screen.released(onScreenReleased, &releaseCount);

  while (true) {
    wait(20, msec);
  }
}
```
