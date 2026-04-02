# LoR Core V3 MiniBot Bluetooth Gamepad Example

This example exists to help users get MiniBot projects running quickly on the **LoR Core V3** with as little setup work as possible. It gives a working baseline for Bluetooth gamepad control, motor output configuration, battery monitoring, and system status feedback so a user can start with a known-good project and then expand it for their own robot. Based on the provided code fileciteturn0file0

## What this code is

This is a sample Arduino program for the **Lord of Robots LoR Core V3** that connects to a Bluetooth gamepad through **Bluepad32** and uses that gamepad to control a MiniBot. It also initializes the LoR Core hardware, configures motor output slots, monitors both robot and gamepad battery state, and provides LED feedback for important system states such as startup, pairing, connected, disconnected, and low battery. fileciteturn0file0

## What this code does

At a high level, the code:

- Initializes the LoR Core V3 internal hardware features
- Starts Bluetooth gamepad support
- Supports a special pairing mode using onboard buttons
- Configures all 12 output slots as motor outputs
- Reads joystick values from the connected gamepad
- Maps joystick values into motor commands
- Drives the MiniBot in a tank-drive style layout
- Monitors robot battery voltage
- Monitors gamepad battery level
- Uses onboard LEDs to communicate system state and status
- Stops all outputs when the gamepad disconnects fileciteturn0file0

## How the MiniBot is controlled

This example uses a **tank drive** style control scheme.

- The **left joystick Y axis** controls the left side of the robot
- The **right joystick Y axis** controls the right side of the robot
- Slots **1 to 6** are driven from the right joystick
- Slots **7 to 12** are driven from the left joystick fileciteturn0file0

The code first reads the joystick values from the gamepad, applies optional inversion based on the LoR Core user switch, removes small joystick drift using a deadband, and then maps the joystick values from the Bluepad32 range of about **-512 to 512** into a motor command range of **-100 to 100**. Those mapped values are then sent to the configured motor slots using `MotorSpeed_Set()`. fileciteturn0file0

If the gamepad disconnects, the code immediately writes a neutral value to every output slot so the robot stops safely. fileciteturn0file0

## Gamepad input functions: `myController->...`

The variable `myController` is a Bluepad32 controller pointer. It gives access to all of the live gamepad data once a controller is connected. The code includes a reference block showing the main functions available. fileciteturn0file0

### Common button and axis functions

#### `myController->buttons()`
Returns a **bitmask** of the main gamepad buttons currently pressed. This is used when you want to check buttons such as A, B, X, Y, shoulder buttons, or other primary controls.

Typical use case:
- Triggering an arm action
- Starting an intake
- Firing a mechanism
- Selecting a mode

#### `myController->dpad()`
Returns a **bitmask** for the D-pad state.

Typical use case:
- Step-by-step control
- Menu navigation
- Simple directional commands
- Discrete movement commands

#### `myController->axisX()`
Left joystick horizontal axis.

Typical use case:
- Steering
- Strafing
- Turret rotation
- Menu navigation with analog feel

#### `myController->axisY()`
Left joystick vertical axis.

Typical use case:
- Left side tank drive
- Forward/backward motion input
- Lift or arm speed control

#### `myController->axisRX()`
Right joystick horizontal axis.

Typical use case:
- Turning
- Camera pan
- Second mechanism analog control

#### `myController->axisRY()`
Right joystick vertical axis.

Typical use case:
- Right side tank drive
- Secondary drive control
- Lift or arm speed control

#### `myController->throttle()`
Reads the throttle trigger, usually in the range **0 to 1023**.

Typical use case:
- Proportional speed control
- Variable servo position
- Trigger-controlled mechanism

#### `myController->brake()`
Reads the brake trigger, usually in the range **0 to 1023**.

Typical use case:
- Reverse trigger control
- Secondary proportional control
- Another analog input for mechanisms

#### `myController->miscButtons()`
Returns a bitmask of special buttons such as system, select, start, or similar controller-specific buttons.

Typical use case:
- Mode switching
- Safety enable
- Calibration actions
- Special commands

### Motion sensor functions

These are available if supported by the connected controller:

- `myController->gyroX()`
- `myController->gyroY()`
- `myController->gyroZ()`
- `myController->accelX()`
- `myController->accelY()`
- `myController->accelZ()`

Typical use case:
- Motion-based control
- Tilt control
- Gesture input
- Experimental control schemes fileciteturn0file0

## `MotorSpeed_Set()` vs `MotorPosition_Set()`

The code separates motor outputs into two categories:

- **Speed-controlled outputs**
- **Position-controlled outputs** fileciteturn0file0

This is defined by the motor type configured for each slot.

### `MotorSpeed_Set(uint8_t slot, int Speed_value)`

This function is for devices where the command represents **speed and direction**.

Expected input:
- `-100` to `100`
- `0` means stop
- Negative values reverse direction
- Positive values drive forward fileciteturn0file0

The function first checks whether the selected slot is configured as a valid speed-type motor. If not, it does nothing. If valid, it maps `-100 to 100` into the servo-style command range of `0 to 180` and writes that output. fileciteturn0file0

Use `MotorSpeed_Set()` for:
- Continuous rotation servos
- N20Plus motor controllers
- Victor SPX
- Talon SRX
- SPARK MAX
- Other speed-based motor controllers
- Drive motors
- Intake motors
- Flywheels
- Conveyors
- Any output where you want continuous motion instead of a target angle fileciteturn0file0

### `MotorPosition_Set(uint8_t slot, int position_value)`

This function is for devices where the command represents a **target position**.

Expected input:
- `0` to `180` degrees fileciteturn0file0

The function checks whether the selected slot is configured as a valid position-type output. If not, it does nothing. If valid, it writes the constrained position value directly to the output. fileciteturn0file0

Use `MotorPosition_Set()` for:
- Standard hobby servos
- Positional MG90 servos
- Mechanisms that need a specific angle
- Arms
- Grippers
- Gates
- Linkages
- Camera tilt or pan systems fileciteturn0file0

### Practical difference

Use **`MotorSpeed_Set()`** when you want something to keep spinning.

Use **`MotorPosition_Set()`** when you want something to move to and hold a specific angle.

Examples:
- Drive wheel motor controller → `MotorSpeed_Set()`
- Intake roller → `MotorSpeed_Set()`
- Arm servo → `MotorPosition_Set()`
- Claw open/close servo → `MotorPosition_Set()` fileciteturn0file0

## Battery monitor functions

This example includes two separate battery monitoring features.

### 1. Gamepad battery monitor: `GamePad_BatteryMonitor()`

This function checks the controller battery once per second using `myController->battery()`. It then changes the gamepad LED color to show battery status. fileciteturn0file0

Battery status behavior:
- `0` → unknown battery state, LED set red
- `<= 64` → low battery, LED red, rumble alert, serial warning, LoR Core LEDs flash red
- `<= 128` → medium battery, LED yellow
- `> 128` → good battery, LED green fileciteturn0file0

Why it matters:
- Warns the operator before the controller dies
- Helps avoid unexpected disconnects during use
- Gives visual feedback directly on the gamepad

### 2. Robot battery monitor: `LoRcore_BatteryMonitor()`

This function reads the robot input voltage through the `VIN_SENSE` analog input and converts the ADC reading into an estimated input voltage using the configured slope and offset values. It also compares that measured voltage to a low-voltage threshold based on the battery cell count and per-cell minimum voltage. fileciteturn0file0

In the main loop, it is called as:

```cpp
LoRcore_BatteryMonitor(1, 3.0);
```

That means:
- Battery is being treated as a **1-cell system**
- Low voltage warning threshold is **3.0 V per cell** fileciteturn0file0

If the measured voltage drops below the threshold, the code:
- Prints a low battery warning to serial
- Flashes the onboard LEDs red as a warning fileciteturn0file0

Why it matters:
- Protects the robot battery from over-discharge
- Warns the user before the robot becomes unstable
- Helps diagnose power issues during testing

## Motor slot configuration

Before the robot can be controlled, each output slot must be configured with a motor type using `ConfigureMotorOutput(slot, type)`. In this example, slots **1 through 12** are all configured as `N20Plus`. fileciteturn0file0

That configuration determines:
- What type of device is connected to the slot
- Whether the slot accepts speed or position commands
- PWM frequency
- Pulse width range
- Valid control behavior for that slot fileciteturn0file0

This is why `MotorSpeed_Set()` and `MotorPosition_Set()` can safely reject invalid commands for the wrong device type.

## Bluetooth pairing and connection flow

The code supports normal reconnect behavior and a manual pairing mode.

### Normal startup
On normal startup, Bluepad32 starts and waits for the paired controller to reconnect. fileciteturn0file0

### Pairing mode
If **User Button A** and **User Button D** are held during reset, the code:
- Clears saved Bluetooth keys
- Enables new Bluetooth pairing
- Flashes the onboard LEDs blue and white
- Waits until a controller connects
- Disables new pairing again after connection fileciteturn0file0

This gives the user a simple way to pair a new controller without changing code.

## LED status behavior

The onboard addressable LEDs are used as a status display.

Examples in this program:
- Green on successful controller connection
- Red on disconnect
- Blue/white flashing during pairing mode
- Rainbow animation while receiving live gamepad control
- Icy blue while waiting for a gamepad connection
- Red warning on low battery conditions fileciteturn0file0

## Typical MiniBot workflow

A typical use flow for this example is:

1. Upload the code to the LoR Core V3
2. Power the board and pair the Bluetooth gamepad if needed
3. Let the program initialize all configured motor slots
4. Wait for gamepad connection
5. Move the left and right joysticks to drive the MiniBot
6. Watch LEDs and serial output for system status and battery warnings
7. Expand the example by adding buttons, extra mechanisms, servos, or custom control logic fileciteturn0file0

## Summary

This example is a fast-start control program for getting a MiniBot running with the LoR Core V3 and a Bluetooth gamepad. It handles hardware initialization, Bluetooth pairing and reconnects, joystick reading, tank drive motor control, motor type safety checks, robot battery monitoring, gamepad battery monitoring, and status LEDs in one baseline project. It is intended to be used as a starting point that users can quickly upload, test, understand, and build on. fileciteturn0file0
