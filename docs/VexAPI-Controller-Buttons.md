# Button

  ## Introduction

 The `button` class is a derived class of the [`controller`](index.html) class. It provides access to Controller button states and button event callbacks.

 A `button` object is accessed through a `controller` object. It is not constructed directly.

 ![The front and back side of the V5 Controller with the buttons highlighted in yellow. On the surface of the controller are up, down, left, and right arrow buttons on the left, and X, A, B, Y buttons clockwise from 12 o'clock on the right. On the back side of the controller are L1, L2, R1, and R2 on the left and right sides respectively.](../../../_images/v5-controller-buttons-sticks-callout.png)

   ## Access

 The `controller` class provides the following button objects. Each object is an instance of the `button` class.

 | Object | Example Usage | Description |
| --- | --- | --- |
| `ButtonA` | `Controller.ButtonA.pressing()` | The **A** button. |
| `ButtonB` | `Controller.ButtonB.pressing()` | The **B** button. |
| `ButtonX` | `Controller.ButtonX.pressing()` | The **X** button. |
| `ButtonY` | `Controller.ButtonY.pressing()` | The **Y** button. |
| `ButtonDown` | `Controller.ButtonDown.pressing()` | The **Down** button. |
| `ButtonUp` | `Controller.ButtonUp.pressing()` | The **Up** button. |
| `ButtonLeft` | `Controller.ButtonLeft.pressing()` | The **Left** button. |
| `ButtonRight` | `Controller.ButtonRight.pressing()` | The **Right** button. |
| `ButtonL1` | `Controller.ButtonL1.pressing()` | The **L1** button. |
| `ButtonL2` | `Controller.ButtonL2.pressing()` | The **L2** button. |
| `ButtonR1` | `Controller.ButtonR1.pressing()` | The **R1** button. |
| `ButtonR2` | `Controller.ButtonR2.pressing()` | The **R2** button. |

   ## Notes

 - Before using a `button` member function, create a `controller` object.

```cpp
// Create a Controller object
controller Controller = controller();
```

   ## Member Functions

 The `button` class includes the following member functions:

 **Getter — Read button state.**

 - [`pressing`](#pressing) — Returns whether the button is being pressed.

 **Callbacks — Run functions when button events occur.**

 - [`pressed`](#pressed) — Registers a function to run when the button is pressed.
- [`released`](#released) — Registers a function to run when the button is released.

   ## Getter

  ### pressing

 `pressing` returns whether the button is currently being pressed.

 **Available Function**

```cpp
bool pressing() const;
```

 **Parameters**

 This function does not have parameters.

 **Return Value**

 Returns a Boolean value.

 - `true` — The button is being pressed.
- `false` — The button is not being pressed.

 **Example**

```cpp
// Turn right while L1 is pressed
while (true) {
  if (Controller.ButtonL1.pressing()) {
    Drivetrain.turn(right);
  }
  else {
    Drivetrain.stop();
  }

  wait(20, msec);
}
```

    ## Callbacks

  ### pressed

 `pressed` registers a function that runs when the button is pressed.

 **Available Function**

```cpp
void pressed(void (* callback)(void)) const;
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A function that runs when the button is pressed. |

 **Return Value**

 This function does not return a value.

 **Example**

 Define the callback function outside of `main`.

```cpp
// Drive forward when A is pressed
void driveForward() {
  Drivetrain.driveFor(forward, 100, mm);
}
```

 Register the callback inside `main`.

```cpp
int main() {
  vexcodeInit();

  // Run driveForward when A is pressed
  Controller.ButtonA.pressed(driveForward);
}
```

   ### released

 `released` registers a function that runs when the button is released.

 **Available Function**

```cpp
void released(void (* callback)(void)) const;
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A function that runs when the button is released. |

 **Return Value**

 This function does not return a value.

 **Example**

 Define the callback function outside of `main`.

```cpp
// Stop the drivetrain when A is released
void stopDriving() {
  Drivetrain.stop();
}
```

 Register the callback inside `main`.

```cpp
int main() {
  vexcodeInit();

  // Run stopDriving when A is released
  Controller.ButtonA.released(stopDriving);
}
```
