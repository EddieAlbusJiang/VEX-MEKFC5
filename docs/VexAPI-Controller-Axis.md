# Axis

  ## Introduction

 The `axis` class is a derived class of the [`controller`](index.html) class. It provides access to Controller joystick axis values and joystick axis change callbacks.

 An `axis` object is accessed through a `controller` object. It is not constructed directly.

 ![A VEX V5 Controller with the axes around the joysticks labeled. Axis 1 and 2 are around the right joystick, and Axis 3 and 4 are around the left.](../../../_images/v5-controller-stick-axis-new-highlight.png)

   ## Access

 The `controller` class provides the following axis objects. Each object is an instance of the `axis` class.

 | Axis Object | Example Usage | Description |
| --- | --- | --- |
| `Axis1` | `Controller.Axis1.position()` | Right joystick horizontal axis. |
| `Axis2` | `Controller.Axis2.position()` | Right joystick vertical axis. |
| `Axis3` | `Controller.Axis3.position()` | Left joystick vertical axis. |
| `Axis4` | `Controller.Axis4.position()` | Left joystick horizontal axis. |

   ## Notes

 - Before using an `axis` member function, create a `controller` object.
- `position` returns a value from `-100` to `100`.

```cpp
// Create a Controller object
controller Controller = controller();
```

   ## Member Functions

 The `axis` class includes the following member functions:

 **Getter — Read joystick axis position.**

 - [`position`](#position) — Returns the position of the joystick axis.

 **Callback — Run a function when the joystick’s position changes along an axis.**

 - [`changed`](#changed) — Runs a function when the joystick’s position changes along that axis.

   ## Getter

  ### position

 `position` returns the position of the joystick axis as a value from `-100` to `100`.

 **Available Function**

```cpp
int32_t position(percentUnits units = percentUnits::pct) const;
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `units` | `percentUnits` | Optional. The unit used for the returned axis value. The default is `percentUnits::pct`. |

 **Return Value**

 Returns an `int32_t` value from `-100` to `100`.

 - `100` — The joystick axis is fully moved in the positive direction.
- `0` — The joystick axis is centered.
- `-100` — The joystick axis is fully moved in the negative direction.

 **Example**

```cpp
// Turn with the left joystick
while (true) {
  if (Controller.Axis4.position() > 10) {
    Drivetrain.turn(right);
  }
  else if (Controller.Axis4.position() < -10) {
    Drivetrain.turn(left);
  }
  else {
    Drivetrain.stop();
  }

  wait(20, msec);
}
```

    ## Callback

  ### changed

 `changed` runs a function when the joystick’s position changes along that axis.

 **Available Function**

```cpp
void changed(void (* callback)(void)) const;
```

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A function to run each time the joystick’s position changes along that axis. |

 **Return Value**

 This function does not return a value.

 **Example**

 Define the callback function outside of `main`.

```cpp
// Rumble when the left joystick moves
void controllerRumble() {
  Controller.rumble(".");
}
```

 Register the callback inside `main`.

```cpp
int main() {
  vexcodeInit();

  // Run controllerRumble when the joystick's position changes along Axis4
  Controller.Axis4.changed(controllerRumble);
}
```
