# Motor and Motor Group

   ## Introduction

 Motors and motor groups control how parts of a robot move. A `motor` object controls one configured Smart Motor, while a `motor_group` object controls multiple configured Smart Motors together so they move in tandem. Motors and motor groups can be used to raise an arm, turn a claw, spin a wheel, or move another part of a build. Two or four motors can work together as a [drivetrain](../Drivetrain/index.html) to move and turn the whole robot.

 Each motor or motor group is configured in the Devices window. Depending on the build, motor and motor group names can change. This page uses `Motor1` and `MotorGroup1` as example names. Replace them with your own configured names as needed.

 ![A VEX V5 Motor showing the forward direction indicated by a plus sign and an arrow indicating it is rotating left, or counterclockwise.](../../../_images/v5-forward.png)

 ![A VEX V5 Motor showing the reverse direction indicated by a minus sign and an arrow indicating it is rotating right, or clockwise.](../../../_images/v5-reversed.png)

 By default, `forward` spins a motor counterclockwise, and `reverse` spins a motor clockwise. If a motor is set to reverse, those directions will be switched.

   ## Motor

  ### Class Constructors

   ****1** —** Creates a `motor` object for the V5 Smart Motor connected to the specified Smart Port. The motor automatically uses the detected internal `gearSetting`.

```cpp
motor(
  int32_t index );
```

     ****2** —** Creates a `motor` object on the specified Smart Port and optionally reverses its direction. The motor automatically uses the detected internal `gearSetting`.

```cpp
motor(
 int32_t index,
 bool    reverse );
```

     ****3** —** Creates a `motor` object on the specified Smart Port with the specified `gearSetting`.

```cpp
motor(
 int32_t     index,
 gearSetting gears );
```

     ****4** —** Creates a `motor` object on the specified Smart Port with the specified `gearSetting` and optional reversed direction.

```cpp
motor(
 int32_t     index,
 gearSetting gears,
 bool        reverse );
```

     ### Class Destructor

 Destroys the `motor` object and releases associated resources.

```cpp
~motor();
```

   ### Parameters

 | Parameter | Type          | Description                                                                                                                                                                   |
 | --------- | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 | `index`   | `int32_t`     | The Smart Port that the motor is connected to, written as `PORTx`, where `x` is the port number (for example, `PORT1`).                                                       |
 | `reverse` | `bool`        | Sets whether the motor’s spin direction is reversed: `false` (default) — Does not reverse the motor’s direction.; `true` — Reverses the motor’s direction.                    |
 | `gears`   | `gearSetting` | Specifies the internal gear cartridge ratio. `ratio36_1` — The red 36:1 gear cartridge; `ratio18_1` — The green 18:1 gear cartridge; `ratio6_1` — The blue 6:1 gear cartridge |

   ### Notes

 - Gear setting should match the physical cartridge installed in the motor.

   ### Example

```cpp
// Create the motor instance in Smart Port 1
motor LeftMotor = motor(PORT1);

// Creates the motor instance with different settings
motor RightMotor = motor(
  PORT2,      // Smart Port 2
  ratio18_1,  // Gear setting is 18
  true);
```

    ## Motor Group

  ### Class Constructors

```cpp
motor_group( motor &m1, Args&... m2 );
```

   ### Class Destructor

 Destroys the `motor_group` object and releases associated resources.

```cpp
~motor_group();
```

   ### Parameters

 | Parameter | Type      | Description                                                                          |
 | --------- | --------- | ------------------------------------------------------------------------------------ |
 | `m1`      | `motor &` | The first motor added to the group.                                                  |
 | `m2`      | `motor &` | Additional motors added to the group. The maximum is four motors in one motor group. |

   ### Example

```cpp
// Create individual motor instances
motor LeftMotor  = motor(PORT1);
motor RightMotor = motor(PORT2);

// Create a motor group with both motors
motor_group DriveMotors = motor_group(LeftMotor, RightMotor);
```

    ## Member Functions

 There are many ways to code motors and motor groups. Below is a list of all `motor` and `motor_group` member functions:

 **Actions — Stop and spin motors and motor groups.**

 - [`spin`](#spin) — Spins a motor or motor group forward or reverse forever.
- [`spinFor`](#spinfor) — Spins a motor or motor group for a specific distance or amount of time.
- [`spinToPosition`](#spintoposition) — Spins a motor or motor group to a specific position.
- [`stop`](#stop) — Stops a motor or motor group from spinning.

 **Mutators — Adjust motor and motor group settings.**

 - [`setPosition`](#setposition) — Changes the motor or motor group’s current position to a new value.
- [`setVelocity`](#setvelocity) — Tells a motor or motor group how fast to spin.
- [`setStopping`](#setstopping) — Sets how a motor or motor group will stop moving: by braking, coasting, or holding.
- [`setMaxTorque`](#setmaxtorque) — Sets how hard a motor or motor group is allowed to push while spinning.
- [`setTimeout`](#settimeout) — Sets how much time a motor or motor group will try to finish a movement.

 **Getters — Check motor and motor group status.**

 - [`isDone`](#isdone) — Returns whether the motor or motor group is finished moving, as a Boolean value.
- [`isSpinning`](#isspinning) — Returns whether the motor or motor group is spinning, as a Boolean value.
- [`position`](#position) — Returns the motor or motor group’s current position.
- [`velocity`](#velocity) — Returns how fast the motor or motor group is spinning.
- [`current`](#current) — Returns how much electrical current the motor or motor group is using.
- [`power`](#power) — Returns how much power the motor or motor group is using.
- [`torque`](#torque) — Returns how much torque the motor or motor group is using.
- [`efficiency`](#efficiency) — Returns how efficiently the motor or motor group is using power.
- [`temperature`](#temperature) — Returns the temperature of the motor or motor group.
- [`voltage`](#voltage) — Returns the electrical voltage supplied to the motor or motor group.
- [`direction`](#direction) — Returns the direction the motor or motor group is spinning.
- [`installed`](#installed) — Returns whether the motor or motor group is connected to the Brain.
- [`count`](#count) — Returns the number of motors in a motor group.

 Before calling any `motor` or `motor_group` member functions, an instance must be created, as shown below:

```cpp
/* This constructor is required when using VS Code.
Motor and Motor Group configuration is generated automatically
in VEXcode using the Device Menu. Replace the values
as needed. */

// Create the motor instance in Smart Port 1
motor Motor1 = motor(PORT1);
```

   ## Actions

  ### spin

 `spin` spins a motor or motor group forward or reverse forever. The motor or motor group will continue to spin until it is given another action, like spinning in a different direction or stopping.

 **Available Functions**

   ****1** —** Spins the motor or motor group using the currently configured motor velocity.

```cpp
void spin(
 directionType dir );
```

     ****2** —** Spins the motor or motor group at the specified velocity.

```cpp
void spin(
 directionType dir,
 double        velocity,
 velocityUnits units );
```

     ****3** —** Spins the motor or motor group using the specified voltage.

```cpp
void spin(
 directionType dir,
 double        voltage,
 voltageUnits  units );
```

   **Parameters**

 | Parameter  | Type            | Description                                                                                                                |
 | ---------- | --------------- | -------------------------------------------------------------------------------------------------------------------------- |
 | `dir`      | `directionType` | The direction the motor or motor group spins: `forward` or `reverse`.                                                      |
 | `velocity` | `double`        | The velocity to spin with from 0% to 100% when using `percent`.                                                            |
 | `units`    | `velocityUnits` | The unit used to represent velocity: `percent` / `pct` — percent; `rpm` — rotations per minute; `dps` — degrees per second |
 | `voltage`  | `double`        | The voltage to spin with. A negative value spins opposite the given `dir`.                                                 |
 | `units`    | `voltageUnits`  | The unit used to represent voltage: `volt` — volts; `voltageUnits::mV` — millivolts                                        |

 **Return Values**

 This function does not return a value.

 **Notes**

 - This function is non-waiting and the project will continue executing immediately after the call.
- The motor or motor group will continue spinning until [`stop`](#stop) is called or another movement function is executed.
- Functions such as [`isDone`](#isdone) and [`isSpinning`](#isspinning) are not applicable to `spin`, since it does not use target-based movement (doesn’t have a `rotation` or `time` parameter).

 **Examples**

```cpp
// Spin forward, then stop
Motor1.spin(forward);
wait(1, seconds);
Motor1.stop();
```

   ### spinFor

 `spinFor` spins a motor or motor group for a specific distance or amount of time. A rotation-based spin is relative to the current position of the motor or motor group. By default, the project will wait until the motor or motor group is done spinning before the next line of code runs.

 **Available Functions**

   ****1** —** Spins for a specific distance at the velocity provided in the function call.

```cpp
bool spinFor(
 double        rotation,
 rotationUnits units,
 double        velocity,
 velocityUnits units_v,
 bool          waitForCompletion = true );
```

     ****2** —** Accepts a direction parameter and spins for a specific distance at the velocity provided in the function call.

```cpp
bool spinFor(
 directionType dir,
 double        rotation,
 rotationUnits units,
 double        velocity,
 velocityUnits units_v,
 bool          waitForCompletion = true );
```

     ****3** —** Spins for a specific distance using the [currently configured motor velocity](#setvelocity).

```cpp
bool spinFor(
 double        rotation,
 rotationUnits units,
 bool          waitForCompletion = true );
```

     ****4** —** Accepts a direction parameter and spins for a specific distance using the [currently configured motor velocity](#setvelocity).

```cpp
bool spinFor(
 directionType dir,
 double        rotation,
 rotationUnits units,
 bool          waitForCompletion = true );
```

     ****5** —** Spins for a specific amount of time at the velocity provided in the function call.

```cpp
bool spinFor(
 double        time,
 timeUnits     units,
 double        velocity,
 velocityUnits units_v );
```

     ****6** —** Accepts a direction parameter and spins for a specific amount of time at the velocity provided in the function call.

```cpp
bool spinFor(
 directionType dir,
 double        time,
 timeUnits     units,
 double        velocity,
 velocityUnits units_v );
```

     ****7** —** Spins for a specific amount of time using the [currently configured motor velocity](#setvelocity).

```cpp
bool spinFor(
 double    time,
 timeUnits units );
```

     ****8** —** Accepts a direction parameter and spins for a specific amount of time using the [currently configured motor velocity](#setvelocity).

```cpp
bool spinFor(
 directionType dir,
 double        time,
 timeUnits     units );
```

   **Parameters**

 | Parameter           | Type            | Description                                                                                                                                                                         |
 | ------------------- | --------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 | `dir`               | `directionType` | The direction the motor or motor group spins: `forward` or `reverse`.                                                                                                               |
 | `rotation`          | `double`        | The distance the motor or motor group spins.                                                                                                                                        |
 | `units`             | `rotationUnits` | The rotation unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions                                                                                                       |
 | `time`              | `double`        | The amount of time the motor or motor group spins for.                                                                                                                              |
 | `units`             | `timeUnits`     | The unit of time: `seconds` / `sec` — seconds; `msec` — milliseconds                                                                                                                |
 | `velocity`          | `double`        | The velocity to spin with from 0% to 100% when using `percent`.                                                                                                                     |
 | `units_v`           | `velocityUnits` | The unit used to represent `velocity`: `percent` / `pct` — percent; `rpm` — rotations per minute; `dps` — degrees per second                                                        |
 | `waitForCompletion` | `bool`          | `true` (default) — Makes the project wait until the motor or motor group is done spinning before the next line of code runs.; `false` — Makes the next line of code run right away. |

 **Return Values**

 Returns a Boolean value that shows whether the motor or motor group reached the target value.

 - `true` — The motor or motor group reached the target value.
- `false` — The motor or motor group was not able to reach the target value or `waitForCompletion` is set to `false`.

 **Notes**

 - Executing another motor movement function (such as [`spin`](#spin) or [`stop`](#stop)) while `spinFor` is in progress will interrupt the current movement.
- Because `spinFor` is target-based (uses a `rotation` or `time` parameter), functions such as [`isDone`](#isdone) and [`isSpinning`](#isspinning) work with `spinFor`.

 **Examples**

```cpp
// Spin 1 turn fast, then 1 turn slow
Motor1.spinFor(forward, 1, turns, true);
Motor1.spinFor(reverse, 1, turns, 20, velocityUnits::pct, true);
```

   ### spinToPosition

 `spinToPosition` spins a motor or motor group to a specific position.

 A motor or motor group’s position is how far it has spun, measured in `degrees` or `turns`. One turn is equal to 360 degrees. At the beginning of a project, the motor or motor group position is set to 0 degrees. The motor or motor group position can also be set using the [`setPosition`](#setposition) function.

 Position values are absolute. This means the direction of the spin depends on the motor or motor group’s current position.

 For example, if the motor or motor group starts at 0 degrees and spins to a position of 720 degrees, it will spin forward two turns. If it then spins to a position of 360 degrees, it will spin reverse one turn, because 360 is less than 720.

 **Available Functions**

   ****1** —** Spins to a specific position at the velocity provided in the function call.

```cpp
bool spinToPosition(
 double        rotation,
 rotationUnits units,
 double        velocity,
 velocityUnits units_v,
 bool          waitForCompletion = true );
```

     ****2** —** Spins to a specific position using the [currently configured motor velocity](#setvelocity).

```cpp
bool spinToPosition(
 double        rotation,
 rotationUnits units,
 bool          waitForCompletion = true );
```

   **Parameters**

 | Parameter           | Type            | Description                                                                                                                                                                         |
 | ------------------- | --------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 | `rotation`          | `double`        | The position value the motor or motor group will spin to.                                                                                                                           |
 | `units`             | `rotationUnits` | The rotation unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions                                                                                                       |
 | `velocity`          | `double`        | The velocity to spin with from 0% to 100% when using `percent`.                                                                                                                     |
 | `units_v`           | `velocityUnits` | The unit used to represent `velocity`: `pct` — percent; `rpm` — rotations per minute; `dps` — degrees per second                                                                    |
 | `waitForCompletion` | `bool`          | `true` (default) — Makes the project wait until the motor or motor group is done spinning before the next line of code runs.; `false` — Makes the next line of code run right away. |

 **Return Values**

 Returns a Boolean value that shows whether the motor or motor group started the requested target-based movement.

 - `true` — The motor or motor group begins the requested target-based movement.
- `false` — The call fails or `waitForCompletion` is set to `false`.

 **Notes**

 - Executing another motor movement function (such as [`spin`](#spin) or [`spinFor`](#spinfor)) while `spinToPosition` is in progress will interrupt the current movement.
- Because `spinToPosition` is target-based (uses a `rotation` parameter), functions such as [`isDone`](#isdone) and [`isSpinning`](#isspinning) work with `spinToPosition`.

 **Examples**

```cpp
// Spin forward, then go to 90°
Motor1.spin(forward);
wait(1, seconds);
Motor1.spinToPosition(90, degrees);
```

   ### stop

 `stop` stops a motor or motor group from spinning.

 **Available Functions**

   ****1** —** Stops the motor or motor group using the [currently configured brake mode](#setstopping).

```cpp
void stop();
```

     ****2** —** Stops the motor or motor group using the brake mode provided in the function call.

```cpp
void stop(
  brakeType mode );
```

   **Parameters**

 | Parameter | Type        | Description                                                                                                                                                                         |
 | --------- | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 | `mode`    | `brakeType` | Optional. How the motor or motor group will stop: `coast` — Slows to a stop.; `brake` — Stops immediately.; `hold` — Stops immediately and holds the motor or motor group position. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Calling `stop` immediately stops any current motor or motor group movement.
- If no brake mode is specified, the motor or motor group uses the [currently configured brake mode](#setstopping).
- `stop` interrupts any active target-based movement (such as [`spinFor`](#spinfor) or [`spinToPosition`](#spintoposition)).

 **Examples**

```cpp
// Spin and coast to a stop
Motor1.setVelocity(100, percent);
Motor1.spin(forward);
wait(1, seconds);
Motor1.stop(coast);
```

    ## Mutators

  ### setPosition

 A motor or motor group’s position is how far it has spun, measured in `degrees` or `turns`. One turn is equal to 360 degrees. `setPosition` changes the motor or motor group’s current position to a new value.

 For example, if a motor or motor group has spun to 180 degrees, setting the position to 0 degrees will reset that position from 180 to 0 degrees. Then the motor or motor group can spin to positions based on that new value.

 **Available Functions**

```cpp
void setPosition(
    double        value,
    rotationUnits units );
```

 **Parameters**

 | Parameter | Type            | Description                                                                   |
 | --------- | --------------- | ----------------------------------------------------------------------------- |
 | `value`   | `double`        | The position value to set for the motor or motor group.                       |
 | `units`   | `rotationUnits` | The position unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 This function does not return a value.

 **Notes**

 - After calling `setPosition`, subsequent calls to [`spinToPosition`](#spintoposition) will use the updated position as the reference.

   ### setVelocity

 `setVelocity` tells a motor or motor group how fast to spin. A higher percentage makes the motor or motor group spin faster and a lower percentage makes the motor or motor group spin slower.

 Every project begins with each motor or motor group spinning at 50% velocity by default.

 **Note:** A higher velocity makes the motor or motor group spin faster, but it may be less precise. A lower velocity makes the motor or motor group spin slower, but it can be more precise.

 **Available Functions**

```cpp
void setVelocity(
    double        velocity,
    velocityUnits units );
```

 **Parameters**

 | Parameter  | Type            | Description                                                                                                                  |
 | ---------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------- |
 | `velocity` | `double`        | The velocity to spin with from 0% to 100% when using `percent`.                                                              |
 | `units`    | `velocityUnits` | The unit used to represent `velocity`: `percent` / `pct` — percent; `rpm` — rotations per minute; `dps` — degrees per second |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Any subsequent motor movement function (such as [`spin`](#spin), [`spinFor`](#spinfor), or [`spinToPosition`](#spintoposition)) that does not explicitly specify a velocity will use this value.

 **Examples**

```cpp
// Try default, then slow, then fast
Motor1.spinFor(forward, 180, degrees);
wait(1, seconds);
Motor1.setVelocity(20, percent);
Motor1.spinFor(reverse, 180, degrees);
wait(1, seconds);
Motor1.setVelocity(100, percent);
Motor1.spinFor(forward, 180, degrees);
```

   ### setStopping

 `setStopping` sets how a motor or motor group will stop moving: by braking, coasting, or holding.

 **Available Functions**

```cpp
void setStopping(
  brakeType mode );
```

 **Parameters**

 | Parameter | Type        | Description                                                                                                                                                               |
 | --------- | ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 | `mode`    | `brakeType` | How the motor or motor group will stop: `coast` — Slows to a stop.; `brake` — Stops immediately.; `hold` — Stops immediately and holds the motor or motor group position. |

 **Return Values**

 This function does not return a value.

 **Notes**

 - Any subsequent call to [`stop`](#stop) without a specified brake mode will use this value.

   ### setMaxTorque

 Torque shows how hard a motor or motor group can push or pull while it spins.

 `setMaxTorque` sets the most torque a motor or motor group is allowed to use.

 A higher percentage lets the motor or motor group push harder, like when lifting a heavy object. A lower percentage limits how hard the motor or motor group can push. This can help protect the robot if the motor or motor group gets stuck or reaches the end of how far it can move.

 Every project begins with each motor or motor group’s torque at 50% by default.

 **Available Functions**

   ****1** —** Sets the maximum torque as a percentage of the motor’s or motor group’s rated torque.

```cpp
void setMaxTorque(
 double       value,
 percentUnits units );
```

     ****2** —** Sets the maximum torque using physical torque units.

```cpp
void setMaxTorque(
 double      value,
 torqueUnits units );
```

     ****3** —** Sets the maximum torque by limiting motor current.

```cpp
void setMaxTorque(
 double       value,
 currentUnits units );
```

   **Parameters**

 | Parameter | Type           | Description                                                                            |
 | --------- | -------------- | -------------------------------------------------------------------------------------- |
 | `value`   | `double`       | The max torque the motor or motor group can use.                                       |
 | `units`   | `percentUnits` | Specifies torque as a percentage: `percent` / `pct` — percent                          |
 | `units`   | `torqueUnits`  | Specifies torque in physical units such as: `Nm` — Newton-meters; `InLb` — inch-pounds |
 | `units`   | `currentUnits` | Specifies torque by limiting motor current: `amp`                                      |

 **Return Values**

 This function does not return a value.

 **Notes**

 - When using `currentUnits`, torque is limited by restricting how much electrical current the motor or motor group can use.

   ### setTimeout

 `setTimeout` sets how much time a motor or motor group will try to finish a movement. If the motor or motor group cannot finish in that time, it will stop trying and move on to the next line of code. This keeps the motor or motor group from getting stuck on a movement.

 **Available Functions**

```cpp
void setTimeout(
    int32_t   time,
    timeUnits units );
```

 **Parameters**

 | Parameter | Type        | Description                                                                                                |
 | --------- | ----------- | ---------------------------------------------------------------------------------------------------------- |
 | `time`    | `int32_t`   | The amount of time the motor or motor group can try to finish a movement. This must be a positive integer. |
 | `units`   | `timeUnits` | The unit of time: `msec` — milliseconds; `seconds` / `sec` — seconds                                       |

 **Return Values**

 This function does not return a value.

 **Notes**

 - The motor or motor group timeout is used to keep movements that do not reach their target position from stopping the execution of other commands in the stack.
- If the motor or motor group does not reach its target position before the timeout expires, it will stop trying and move on to the next line of code.
- The timeout applies to functions that have a target value such as [`spinFor`](#spinfor) and [`spinToPosition`](#spintoposition).

 **Examples**

```cpp
// Stop a long move after 1 second
Motor1.setTimeout(1, seconds);
Motor1.spinFor(forward, 2, turns);
Motor1.spinToPosition(0, degrees);
```

    ## Getters

  ### isDone

 `isDone` returns whether the motor or motor group is finished moving, as a Boolean value. This can be used to control the timing of other behaviors based on the motor or motor group’s movement.

 - `true` — The motor or motor group is finished moving.
- `false` — The motor or motor group is still moving.

 This function works together with the following Motion functions that have the `waitForCompletion` parameter: [`spinFor`](#spinfor) and [`spinToPosition`](#spintoposition).

 **Available Functions**

```cpp
bool isDone();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns whether the motor or motor group is finished moving, as a Boolean value.

 - `true` — The motor or motor group is finished moving.
- `false` — The motor or motor group is still moving.

 **Notes**

 - `isDone` works together with the following Motion functions that have the `waitForCompletion` parameter: [`spinFor`](#spinfor) and [`spinToPosition`](#spintoposition).

 **Examples**

```cpp
// Spin forward until the motor is done spinning
Motor1.spinFor(forward, 200, degrees, false);
while (true) {
  if (Motor1.isDone()) {
    Drivetrain.stop();
  } else {
    Drivetrain.drive(forward);
  }
}
```

   ### isSpinning

 `isSpinning` returns whether the motor or motor group is spinning, as a Boolean value. This can be used to control the timing of other behaviors based on the motor or motor group’s movement.

 - `true` — The motor or motor group is spinning.
- `false` — The motor or motor group is not spinning.

 This function works together with Motion movement functions such as [`spin`](#spin), [`spinFor`](#spinfor), and [`spinToPosition`](#spintoposition).

 **Available Functions**

```cpp
bool isSpinning();
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 Returns whether the motor or motor group is spinning, as a Boolean value.

 - `true` — The motor or motor group is spinning.
- `false` — The motor or motor group is not spinning.

 **Notes**

 - `isSpinning` works with Motion movement functions such as [`spin`](#spin), [`spinFor`](#spinfor), and [`spinToPosition`](#spintoposition).

 **Examples**

```cpp
// Spin forward until the motor is done spinning
Motor1.spinFor(forward, 200, degrees, false);
while (true) {
  if (Motor1.isSpinning()) {
    Drivetrain.drive(forward);
  } else {
    Drivetrain.stop();
  }
}
```

   ### position

 A motor or motor group’s position is how far it has spun, measured in `degrees` or `turns`. One turn is equal to 360 degrees. `position` returns the motor or motor group’s current position.

 At the beginning of a project, the motor or motor group position is set to 0 degrees. If the motor or motor group spins one turn forward, the position will be 360 degrees or 1 turn. If the motor or motor group spins the other direction, the position will be negative.

 **Available Functions**

```cpp
double position(
  rotationUnits units );
```

 **Parameters**

 | Parameter | Type            | Description                                                                                                         |
 | --------- | --------------- | ------------------------------------------------------------------------------------------------------------------- |
 | `units`   | `rotationUnits` | The unit to return the motor or motor group position in: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 Returns a `double` representing the motor or motor group’s current position in the specified units.

 **Notes**

 - Calling [`setPosition`](#setposition) changes the reference point for this value.
- The returned value reflects the current encoder reading and does not indicate whether the motor or motor group is actively moving.
- When called on a `motor_group`, this function returns the position of the first motor in the group.

 **Examples**

```cpp
// Spin, then show the final position
Motor1.spin(forward);
wait(1, seconds);
Motor1.stop();
Brain.Screen.print("Pos: %f", Motor1.position(degrees));
```

   ### velocity

 `velocity` returns how fast the motor or motor group is spinning.

 A positive value means the motor or motor group is spinning forward. A negative value means it is spinning in reverse.

 **Available Functions**

```cpp
double velocity(
    velocityUnits units );
```

 **Parameters**

 | Parameter | Type            | Description                                                                                                                |
 | --------- | --------------- | -------------------------------------------------------------------------------------------------------------------------- |
 | `units`   | `velocityUnits` | The unit used to represent velocity: `percent` / `pct` — percent; `rpm` — rotations per minute; `dps` — degrees per second |

 **Return Values**

 Returns a `double` representing how fast the motor or motor group is spinning in the specified units.

 **Notes**

 - When called on a `motor_group`, this function returns the velocity of the first motor in the group.

   ### current

 `current` returns how much electrical current the motor or motor group is using. Current is the amount of electricity flowing through the motor or motor group. When returned in amps, current is reported from 0.0 to 1.2 A.

 A higher current value means the motor or motor group is using more electrical current. This can happen when the motor or motor group is lifting something heavy, pushing against an object, or trying to move when it is stuck.

 This can be used to check if the motor or motor group is struggling during a movement. If current stays high, the motor may get warmer or use power less efficiently.

 **Available Functions**

   ****1** —** Returns the motor’s or motor group’s electrical current in amps.

```cpp
double current(
  currentUnits units = amp );
```

     ****2** —** Returns the motor’s or motor group’s electrical current as a percentage of maximum rated current.

```cpp
double current(
  percentUnits units );
```

   **Parameters**

 | Parameter | Type           | Description                                                                                             |
 | --------- | -------------- | ------------------------------------------------------------------------------------------------------- |
 | `units`   | `currentUnits` | The unit used to represent current:`amp` (default) — amperes                                            |
 | `units`   | `percentUnits` | Represents the current as a percentage of the motor’s maximum rated current:`percent` / `pct` — percent |

 **Return Values**

 Returns a `double` representing how much electrical current the motor or motor group is using in the specified units.

 **Notes**

 - When called on a `motor_group`, this function returns the total electrical current for all motors in the group.

   ### power

 `power` returns how much power the motor or motor group is using, measured in watts from 0.0 to 22.0 W. Power shows how quickly the motor or motor group is using energy.

 A higher power value means the motor or motor group is using energy faster. This can happen when the motor or motor group is lifting something heavy, pushing against an object, or trying to move when it is stuck.

 This can be used to compare movements or check if the motor or motor group is struggling. If power stays high, the motor may get warmer or use energy less efficiently.

 **Available Functions**

```cpp
double power(
  powerUnits units = watt );
```

 **Parameters**

 | Parameter | Type         | Description                                                          |
 | --------- | ------------ | -------------------------------------------------------------------- |
 | `units`   | `powerUnits` | The unit used to represent the motor power: `watt` (default) — watts |

 **Return Values**

 Returns a `double` representing how much power the motor or motor group is using in watts.

 **Notes**

 - When called on a `motor_group`, this function returns the electrical power of the first motor in the group.

   ### torque

 Torque shows how hard a motor or motor group is twisting, pushing, or pulling while it spins.

 `torque` returns how much torque the motor or motor group is using, measured in inch-pounds (InLb) from 0.0 to 22.0 or Newton-meters (Nm) from 0.0 to 2.1.

 A higher torque value means the motor or motor group is pushing or pulling harder. This can happen when the motor or motor group is lifting something heavy, pushing against an object, or trying to move when it is stuck.

 This can be used to check if the motor or motor group is struggling or to compare how much push different movements need.

 To set the torque of a motor or motor group, use [`setMaxTorque`](#setmaxtorque).

 **Available Functions**

```cpp
double torque(
  torqueUnits units = Nm );
```

 **Parameters**

 | Parameter | Type          | Description                                                                                       |
 | --------- | ------------- | ------------------------------------------------------------------------------------------------- |
 | `units`   | `torqueUnits` | The unit used to represent the motor torque: `Nm` (default) — Newton-meters; `InLb` — inch-pounds |

 **Return Values**

 Returns a `double` representing how much torque the motor or motor group is using in the specified units.

 **Notes**

 - When called on a `motor_group`, this function returns the mechanical output torque of the first motor in the group.

   ### efficiency

 `efficiency` returns how efficiently the motor or motor group is using power, as a percentage from 0% to 100%.

 Efficiency shows how much of the motor or motor group’s power is being used for movement. A higher efficiency value means more of the motor or motor group’s power is being used to move. A lower efficiency value can happen when the motor or motor group is working hard but not moving much, like when it is stuck or pushing against an object.

 This can be used to compare movements or check if the motor or motor group is wasting power instead of using it for movement.

 **Available Functions**

```cpp
double efficiency(
  percentUnits units = percent );
```

 **Parameters**

 | Parameter | Type           | Description                                                                                      |
 | --------- | -------------- | ------------------------------------------------------------------------------------------------ |
 | `units`   | `percentUnits` | The unit used to represent motor efficiency: `percent` / `percentUnits::pct` (default) — percent |

 **Return Values**

 Returns a `double` representing how efficiently the motor or motor group is using power as a percentage.

 **Notes**

 - When called on a `motor_group`, this function returns the efficiency of the first motor in the group.

   ### temperature

 `temperature` returns the temperature of the motor or motor group.

 Motor temperature shows how warm the motor or motor group is. A higher temperature means the motor or motor group is getting warmer while it works. The motor should stay below 55°C to keep working at full performance.

 If the motor or motor group gets too hot, it will lower its maximum current to protect itself. At 70°C, a motor will stop running until it cools down.

 This can be used to check if the motor or motor group is getting too hot during repeated movements, long runs, or when it is pushing against an object.

 **Available Functions**

   ****1** —** Returns motor or motor group temperature as a percentage of maximum operating temperature.

```cpp
double temperature(
  percentUnits units = percent );
```

     ****2** —** Returns motor or motor group temperature in physical temperature units.

```cpp
double temperature(
  temperatureUnits units );
```

   **Parameters**

 | Parameter | Type               | Description                                                                                                                            |
 | --------- | ------------------ | -------------------------------------------------------------------------------------------------------------------------------------- |
 | `units`   | `percentUnits`     | The unit used to represent motor or motor group temperature as a percentage: `percent` / `percentUnits::pct` (default) — percent       |
 | `units`   | `temperatureUnits` | The unit used to represent motor or motor group temperature in degrees: `celsius` — degrees Celsius; `fahrenheit` — degrees Fahrenheit |

 **Return Values**

 Returns a `double` representing the temperature of the motor or motor group in the specified units.

 **Notes**

 - The operating temperature range for the motor is approximately `20°C` (`68°F`) to `70°C` (`158°F`).
- When called on a `motor_group`, this function returns the temperature of the first motor in the group.

   ### voltage

 `voltage` returns the electrical voltage supplied to the motor or motor group.

 **Available Functions**

```cpp
double voltage(
  voltageUnits units = volt );
```

 **Parameters**

 | Parameter | Type           | Description                                                                          |
 | --------- | -------------- | ------------------------------------------------------------------------------------ |
 | `units`   | `voltageUnits` | The unit to represent the voltage: `volt` (default); `voltageUnits::mV` — millivolts |

 **Return Values**

 Returns a `double` representing the electrical voltage supplied to the motor or motor group in the specified units.

 **Notes**

 - When called on a `motor_group`, this function returns the electrical voltage of the first motor in the group.

   ### direction

 `direction` returns the direction that the motor or motor group is spinning.

 **Available Functions**

```cpp
directionType direction();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns a `directionType` indicating the motor’s or motor group’s current direction:

 - `forward` — The motor or motor group is spinning forward.
- `reverse` — The motor or motor group is spinning in reverse.

   ### installed

 `installed` returns whether the motor or motor group is connected to the V5 Brain.

 **Available Functions**

```cpp
bool installed();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns a `bool` indicating whether the motor or motor group is connected.

 - `true` — The motor or motor group is connected.
- `false` — The motor or motor group is not connected.

   ### count

 `count` returns the number of motors in a motor group.

 **Available Functions**

```cpp
int32_t count();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns an `int32_t` representing the number of motors in the motor group.
