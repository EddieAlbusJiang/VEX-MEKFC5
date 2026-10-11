# Inertial Sensor

   ## Introduction

 The `inertial` class is used to control and access data from a V5 Inertial Sensor connected to a Smart Port.

 The V5 Inertial Sensor measures how the robot is moving and turning. The gyroscope measures turning, such as heading, rotation, gyro rate, and orientation. The accelerometer measures changes in motion, such as speeding up or slowing down along the x-, y-, and z-axes.

   ## Class Constructors

```cpp
inertial(
  int32_t  index,
  turnType dir = turnType::right );
```

   ## Class Destructor

 Destroys the `inertial` object and releases associated resources.

```cpp
~inertial();
```

   ## Parameters

 | Parameter | Type | Description |
| --- | --- | --- |
| `index` | `int32_t` | The Smart Port that the Inertial Sensor is connected to, written as `PORTx`, where `x` is the port number (for example, `PORT1`). |
| `dir` | `turnType` | Optional. The direction that returns positive values: `right` (default) — Right turns return positive values.; `left` — Left turns return positive values. |

   ## Examples

```cpp
// Create an inertial instance for the V5 Inertial Sensor
inertial myInertial = inertial(
    PORT1,      // Smart Port 1
    right);     // dir
```

   ## Member Functions

 The `inertial` class includes the following member functions:

 - [`heading`](#heading) — Returns the direction the sensor is facing.
- [`rotation`](#rotation) — Returns how far the sensor has turned.
- [`setHeading`](#setheading) — Sets the sensor’s current heading to a new heading value.
- [`setRotation`](#setrotation) — Sets the sensor’s current rotation to a new rotation value.
- [`calibrate`](#calibrate) — Calibrates the Inertial Sensor.
- [`resetHeading`](#resetheading) — Resets the sensor’s current heading to 0 degrees.
- [`resetRotation`](#resetrotation) — Resets the sensor’s current rotation to 0 degrees.
- [`isCalibrating`](#iscalibrating) — Returns whether the Inertial Sensor is currently calibrating.
- [`changed`](#changed) — Registers a callback function that runs when the heading changes.
- [`installed`](#installed) — Returns whether the Inertial Sensor is installed.
- [`acceleration`](#acceleration) — Returns how quickly the sensor is speeding up or slowing down on the selected axis.
- [`gyroRate`](#gyrorate) — Returns how fast the sensor is rotating on the selected axis.
- [`orientation`](#orientation) — Returns the Inertial Sensor’s roll, pitch, or yaw angle.
- [`collision`](#collision) — Registers a callback function that runs when a collision is detected.
- [`setTurnType`](#setturntype) — Sets which turn direction returns positive values.
- [`getTurnType`](#getturntype) — Returns which turn direction produces positive values.

 Before calling any `inertial` member functions, an `inertial` instance must be created, as shown below:

```cpp
/* This constructor is required when using VS Code.
Inertial Sensor configuration is generated automatically
in VEXcode using the Device Menu. Replace the values
as needed. */

// Create an inertial instance for the V5 Inertial Sensor connected to Port 1.
inertial myInertial = inertial(PORT1);
```

  ### heading

 A heading is the direction the sensor is facing, measured in degrees from 0 to 359.99. This function returns the sensor’s current heading.

 The starting heading is 0 degrees. If the sensor turns past 359.99 degrees, the heading wraps back to 0 degrees.

 **Available Functions**

```cpp
double heading( rotationUnits units = degrees );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `units` | `rotationUnits` | The heading unit: `deg` / `degrees` (default) — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 Returns a `double` representing the current heading of the sensor in the specified units.

 **Notes**

 - The heading value is wrapped between 0 and 359.99 degrees. Use [`rotation`](#rotation) to access the continuous rotation value.

 **Examples**

```cpp
// Turn, then show the current heading
Drivetrain.turnFor(right, 450, degrees);
Brain.Screen.print("%f", myInertial.heading(degrees));
```

   ### rotation

 Returns the current rotation of the Inertial Sensor.

 Rotation is how much the sensor has turned. Unlike heading, rotation can increase past 359.99 degrees or decrease below 0 degrees. For example, making two full turns to the right returns a rotation of 720 degrees. Turning one full turn to the left from 0 degrees returns a rotation of -360 degrees.

 **Available Functions**

```cpp
double rotation( rotationUnits units = degrees );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `units` | `rotationUnits` | The rotation unit: `deg` / `degrees` (default) — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 Returns a `double` representing the current rotation in the specified units.

 **Notes**

 - The rotation value is continuous and does not wrap between 0 and 359.99 degrees. Use [`heading`](#heading) to access the wrapped heading value.

 **Examples**

```cpp
// Turn, then show the current rotation
Drivetrain.turnFor(right, 450, degrees);
Brain.Screen.print("%f", myInertial.rotation(degrees));
```

   ### setHeading

 A heading is the direction the sensor is facing, measured in degrees from 0 to 359.99. This function changes the Inertial Sensor’s current heading to a new heading value.

 For example, if the sensor has turned to face right, setting the heading to 0 degrees makes that right-facing position the new 0 degrees. Then the sensor can track other headings based on that new direction.

 **Available Functions**

```cpp
void setHeading( double value, rotationUnits units );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `value` | `double` | The heading value, in degrees, to set for the sensor. This can be a value from 0 to 359.99. |
| `units` | `rotationUnits` | The heading unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 This function does not return a value.

 **Examples**

```cpp
// Turn the robot around
myInertial.setHeading(180, degrees);
Drivetrain.turnToHeading(0, degrees);
```

   ### setRotation

 Rotation is how much the sensor has turned, measured in degrees. Unlike heading, rotation can increase past 359.99 degrees or decrease below 0 degrees. At the beginning of a project, the rotation value is set to 0 degrees. This function changes the Inertial Sensor’s current rotation to a new value.

 For example, if the sensor has made two full turns to the right, its rotation value will be 720 degrees. Setting the rotation to 0 degrees will reset that rotation from 720 to 0 degrees. Then the sensor can track rotations based on that new value.

 **Available Functions**

```cpp
void setRotation( double value, rotationUnits units );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `value` | `double` | The rotation value, in degrees, to set for the sensor. This can be an integer or a decimal. |
| `units` | `rotationUnits` | The rotation unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 This function does not return a value.

 **Examples**

```cpp
// Turn the robot around
myInertial.setRotation(-180, degrees);
Drivetrain.turnToRotation(0, degrees);
```

   ### calibrate

 Calibrates the Inertial Sensor.

 Calibration helps the sensor measure turns correctly. Keep the sensor still during calibration. If the sensor moves during calibration, heading, rotation, gyro rate, and orientation values may not measure correctly.

 **Available Functions**

```cpp
void calibrate( );
```

 **Parameters**

 This function does not accept any parameters.

 **Return Values**

 This function does not return a value.

 **Notes**

 - The sensor should remain stationary while calibration is in progress.
- Use [`isCalibrating`](#iscalibrating) to determine when calibration has completed.

   ### resetHeading

 Resets the Inertial Sensor’s current heading to 0 degrees.

 After this function is used, the sensor’s current direction becomes the new 0 degree heading.

 **Available Functions**

```cpp
void resetHeading();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 This function does not return a value.

   ### resetRotation

 Resets the Inertial Sensor’s current rotation to 0 degrees.

 After this function is used, the sensor tracks future turns from the new 0 degree rotation value.

 **Available Functions**

```cpp
void resetRotation();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 This function does not return a value.

   ### isCalibrating

 Returns whether the Inertial Sensor is currently calibrating.

 **Available Functions**

```cpp
bool isCalibrating();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns a Boolean indicating whether the Inertial Sensor is calibrating:

 - `true` — The sensor is currently calibrating.
- `false` — The sensor is not currently calibrating.

   ### changed

 Registers a callback function that runs when the heading value of the Inertial Sensor changes.

 **Available Functions**

```cpp
void changed( void (* callback)(void) );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(void)` | A pointer to a function that will be called when the heading value changes. The function must take no parameters and return `void`. |

 **Return Values**

 This function does not return a value.

 **Examples**

   Define the callback function (outside of `int main()`)

```cpp
// Display the heading when the Inertial Sensor's heading value changes.
void onHeadingChanged() {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print("Heading: %.2f", myInertial.heading());
}
```

 Register the callback inside `int main()`

```cpp
int main() {
  // Run onHeadingChanged when the Inertial Sensor's heading value changes.
  myInertial.changed(onHeadingChanged);
}
```

     ### installed

 Returns whether the Inertial Sensor is installed and connected.

 **Available Functions**

```cpp
bool installed();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns a Boolean indicating whether the Inertial Sensor is installed and connected:

 - `true` — The sensor is connected and responding.
- `false` — The sensor is not connected or not detected.

   ### acceleration

 Acceleration is how quickly the sensor is speeding up or slowing down. This function returns the acceleration of the Inertial Sensor on the selected axis, from -4.0 G to 4.0 G.

 A `G` is a unit used to measure acceleration. 1 G is about the acceleration you feel from gravity while sitting still. The value can be positive or negative depending on the direction of acceleration on the selected axis.

 **Available Functions**

```cpp
double acceleration( axisType axis );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `axis` | `axisType` | The axis to measure acceleration on: `xaxis`; `yaxis`; `zaxis` |

 **Return Values**

 Returns a `double` representing the acceleration along the specified axis in G (gravitational units).

 **Notes**

 - Acceleration values are reported in units of `G`.

   ### gyroRate

 Gyro rate is how fast the Inertial Sensor is rotating. This function returns the current rotation speed of the Inertial Sensor on the selected axis.

 The value can be positive or negative depending on the direction the sensor is rotating on that axis.

 **Available Functions**

```cpp
double gyroRate( axisType axis, velocityUnits units );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `axis` | `axisType` | The axis to return the gyro rate from: `xaxis`; `yaxis`; `zaxis` |
| `units` | `velocityUnits` | The gyro rate unit: `dps` — degrees per second; `rpm` — revolutions per minute |

 **Return Values**

 Returns a `double` representing the current rotation speed along the specified axis in the selected units.

 **Examples**

```cpp
// Display the rate of change of rotation while turning in dps
Drivetrain.turn(right);
wait(1, seconds);

Brain.Screen.print(myInertial.gyroRate(zaxis, dps));

Drivetrain.stop();
```

   ### orientation

 Orientation is the Inertial Sensor’s current angle on a selected axis. This function returns the current orientation of the Inertial Sensor for the specified axis.

 The selected x-, y-, or z-axis corresponds to one of the sensor’s orientation axes.

 **Available Functions**

```cpp
double orientation(
  axisType      axis,
  rotationUnits units );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `axis` | `axisType` | The axis for which orientation is returned: `xaxis`; `yaxis`; `zaxis` |
| `units` | `rotationUnits` | The orientation unit: `deg` / `degrees` — degrees; `turns` / `rev` — revolutions |

 **Return Values**

 Returns a `double` representing the selected orientation angle in the specified units.

 **Examples**

```cpp
while (true) {
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1,1);
  // Show the Inertial Sensor's orientation value
  // for roll in degrees
  double orient = myInertial.orientation(xaxis, deg);
  Brain.Screen.print("%f", orient);
  wait(0.1, seconds);
}
```

   ### collision

 Registers a callback function that runs when the Inertial Sensor detects a sudden impact or collision.

 **Available Functions**

```cpp
void collision( void (* callback)(axisType, double, double, double) );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `callback` | `void (*)(axisType, double, double, double)` | A pointer to a function that will be called when a collision is detected. The callback function must accept the following parameters: `axisType` — The axis on which the collision was detected.; Acceleration values at the time of impact, in order: `double` — Acceleration along the x-axis (in G).; `double` — Acceleration along the y-axis (in G).; `double` — Acceleration along the z-axis (in G).The function must return `void`. |

 **Return Values**

 This function does not return a value.

 **Examples**

   Define the callback function (outside of `int main()`)

```cpp
// Display a message when the Inertial Sensor detects a collision.
void onCollision(axisType axis, double x, double y, double z) {
  Brain.Screen.print("Collision detected!");
}
```

 Register the callback inside `int main()`

```cpp
int main() {
  // Run onCollision when the Inertial Sensor detects a collision.
  myInertial.collision(onCollision);
}
```

     ### setTurnType

 Sets which turn direction returns positive values.

 **Available Functions**

```cpp
void setTurnType( turnType dir );
```

 **Parameters**

 | Parameter | Type | Description |
| --- | --- | --- |
| `dir` | `turnType` | The direction that returns positive values: `right`; `left` |

 **Return Values**

 This function does not return a value.

 **Notes**

 - This setting affects how [`heading`](#heading) and [`rotation`](#rotation) report positive and negative values.
- Calling this function overrides the `dir` value specified when [constructing the `inertial` object](#class-constructors).

   ### getTurnType

 Returns which turn direction produces positive values.

 **Available Functions**

```cpp
turnType getTurnType();
```

 **Parameters**

 This function does not take any parameters.

 **Return Values**

 Returns a `turnType` indicating which direction produces positive values:

 - `right` — Clockwise rotation returns positive values.
- `left` — Counterclockwise rotation returns positive values.

 **Notes**

 - This value is set during [construction of the `inertial` object](#class-constructors) or by calling [`setTurnType`](#setturntype).
