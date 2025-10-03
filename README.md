# LoR Core V3 – MiniBot Example
Sample Arduino sketch for the **LoR Core V3** using a Bluetooth gamepad via **Bluepad32**, **ESP32Servo**, and **FastLED**.

- Main sketch: `LoR_Core_V3_MiniBot_Example.ino`
- Target board: **esp32_bluepad32 / ESP32 Dev Module**
- Serial baud: **115200**
- License: **Apache-2.0** (see `LICENSE`)

---

## Requirements
- **Arduino IDE 2.x**
- **LoR Core V3** controller + USB cable
- **Bluetooth gamepad**
- (If needed) **CH340** USB driver

---

## Download the code (ZIP)
1. On the repo page, click **Code ▸ Download ZIP**.  
2. Unzip to a folder.  
3. Open `LoR_Core_V3_MiniBot_Example.ino` in Arduino IDE.  
> Keep the folder name the same as the `.ino` file.

---

## Read the setup notes in the sketch
Open the `.ino` and read the header block at the top. The steps below mirror those instructions.

---

## Environment setup (matches the sketch header)

### 1) Add Boards Manager URLs
**File ▸ Preferences ▸ Additional Boards Manager URLs** → add each on its own line:
```

[https://dl.espressif.com/dl/package\_esp32\_index.json](https://dl.espressif.com/dl/package_esp32_index.json)
[https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32\_files/package\_esp32\_bluepad32\_index.json](https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json)

```

### 2) Install board packages (Boards Manager)
**Tools ▸ Board ▸ Boards Manager…**
- Install **“esp32” by Espressif Systems**
- Install **“esp32_bluepad32” by Ricardo Quesada**

### 3) Install libraries (Library Manager)
**Sketch ▸ Include Library ▸ Manage Libraries…**
- **FastLED** by Daniel Garcia — **Latest**
- **ESP32Servo** by Kevin Harrington, John K. Bennett — **Version 3.0.7**  
  *(Note: 3.0.8 does not work due to ESP32 LEDC handling changes.)*

### 4) (If needed) Install CH340 USB driver
If the board/port doesn’t appear under **Tools ▸ Port**, install:
- CH340: `https://www.wch-ic.com/download/file?id=65`  
Reconnect USB and restart IDE.

### 5) Select the **correct** target board
**Tools ▸ Board** → **USE**: `esp32_bluepad32 / ESP32 Dev Module`  
**DO NOT USE**: `esp32 / ESP32 Dev Module`

### 6) Select COM port and upload
**Tools ▸ Port** → choose the LoR Core V3 port.  
Click **Upload**. The process is automatic (BOOT not required).  
If it fails: hold **BOOT**, tap **RST**, release **BOOT** at “Connecting…”.

### 7) Pair the gamepad (first time only)
1. Power **LoR Core V3**.  
2. **Hold User Buttons A + D**, then press/release **RST**.  
3. When LEDs **flash BLUE + WHITE**, release A + D (pair mode).  
4. Put the **gamepad** into Bluetooth pairing mode.  
5. Wait for **GREEN flash** → pairing done, returns to normal mode.  
Notes: Auto-reconnect on future power-ups; BT keys are remembered across power cycles and uploads.

### 8) LED status reference
- **ICE BLUE** — Waiting / disconnected  
- **RED** — Error / disconnect event  
- **GREEN** — System OK / acknowledge  
- **RAINBOW** — Receiving gamepad data  
- **FLASH BLUE + WHITE** — Pair mode

---

## Build, upload, and run
1. Confirm **Board** = `esp32_bluepad32 / ESP32 Dev Module` and correct **Port**.  
2. **Verify** (✔) then **Upload** (→).  
3. Open **Serial Monitor** at **115200**.  
4. If not yet paired, follow **Step 7**.

---

## What the sketch does
- Initializes watchdog, LEDs (FastLED), servos (ESP32Servo), and Bluepad32.
- Drives **left** (slots 1–6) and **right** (slots 7–12) motor groups from gamepad sticks.
  - Deadband ±50 around center; map to **0–180** with **90 = stop/center**.
- **User_SW** toggles drive inversion.
- Battery monitor on **VIN_SENSE (GPIO 34)** logs VIN and scales output on low voltage.
- LED colors reflect state (see above).

---

## Troubleshooting
- **No COM Port** → different cable/USB port; install **CH340**; restart IDE.  
- **Failed to connect** → manual boot: hold **BOOT**, tap **RST**, release **BOOT** at “Connecting…”.  
- **Compile errors / missing headers** → re-check libraries (use **ESP32Servo 3.0.7**).  
- **Garbled Serial** → set **115200** baud.

---

## Repo structure
```

LoR\_Core\_V3\_MiniBot\_Example/
├─ LoR\_Core\_V3\_MiniBot\_Example.ino
├─ LICENSE            # Apache-2.0
└─ README.md

```

---


# LoR Core V3 MiniBot Example — Code Walkthrough

Target board: `esp32_bluepad32 / ESP32 Dev Module`. Serial `115200`. Watchdog `3 s`. Single controller only.

---

## 1) Environment and setup prerequisites

Mirrors the header comments. Required before compile/upload:

* Boards URLs:
  `https://dl.espressif.com/dl/package_esp32_index.json`
  `https://raw.githubusercontent.com/ricardoquesada/esp32-arduino-lib-builder/master/bluepad32_files/package_esp32_bluepad32_index.json`
* Install boards: **esp32** (Espressif) and **esp32\_bluepad32** (Ricardo Quesada).
* Libraries: **FastLED** (latest), **ESP32Servo v3.0.7** only. v3.0.8 breaks LEDC timing with current ESP32 core.
* USB driver: CH340 if needed.
* Board select: **esp32\_bluepad32 / ESP32 Dev Module**. Do **not** use the stock esp32 Dev Module.
* Upload: auto. If it fails, hold **BOOT**, tap **RST**, release **BOOT**.
* First-time pairing: hold **User A + D**, tap **RST**, release **A + D**, put gamepad in BT pair mode. Keys persist across reboots and uploads.

LED legend:

* Ice Blue = waiting/disconnected
* Red = error/disconnect
* Green = OK/ack
* Rainbow = receiving input
* Flash Blue/White = pair mode

---

## 2) Libraries

```cpp
#include <Bluepad32.h>     // Bluetooth gamepad stack and device abstraction
#include "esp_task_wdt.h"  // Watchdog control
#include <ESP32Servo.h>    // 50 Hz servo/RC PWM via LEDC
#include <FastLED.h>       // WS2812B status LEDs
```

Purpose: controller I/O, safety (WDT), motor PWM, visual status.

---

## 3) Pin maps

### AUX Port (not used in this sketch, reserved for future I/O)

```cpp
const uint8_t AUX_PINS[9] = {0, 5, 18, 23, 19, 22, 21, 1, 3}; // slot index 1..8
```

### IO Ports for motor outputs (12 servo-style channels)

```cpp
const uint8_t IO_PINS[13] = {0, 32, 25, 26, 27, 14, 12, 13, 15, 2, 4, 22, 21}; // slots 1..12
```

### User inputs

```cpp
#define User_BTN_A 35
#define User_BTN_B 39
#define User_BTN_C 38
#define User_BTN_D 37
#define User_SW   36   // direction invert switch
```

### Battery sense and LEDs

```cpp
#define VIN_SENSE 34
#define LED_PIN   33
#define LED_COUNT 4
```

---

## 4) Voltage model and calibration

```cpp
#define VOLT_SLOPE  0.0063492
#define VOLT_OFFSET 1.079
```

Reads `analogRead(VIN_SENSE)` and computes:

```
VIN_volts = (ADC_raw * VOLT_SLOPE) + VOLT_OFFSET
```

Calibrated so \~6 V ≈ 775 counts and \~12 V ≈ 1720 counts on your board. Works with the on-board divider and ADC gain.

---

## 5) Status LED config

```cpp
#define BRIGHTNESS 255
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
CRGB leds[LED_COUNT];
uint8_t rainbowHue = 0;
```

* Boot: clear and show.
* Runtime: solid colors for states, rainbow when streaming inputs.

---

## 6) Global objects and watchdog

```cpp
#define WDT_TIMEOUT 3
Servo MotorOutput[13];      // channels 1..12 used
ControllerPtr myController; // only one allowed
```

WDT is enabled with panic. `loop()` feeds it each cycle.

---

## 7) INIT\_InternalFeatures()

* Starts Serial 115200.
* Enables WDT and registers current task.
* Sets all button and sensor pins to input.
* Initializes FastLED (clears, shows).

---

## 8) Bluepad32 controller lifecycle

### onConnectedController(ctl)

* Accepts only if `myController == nullptr`. Otherwise rejects the extra device.
* Haptic “shake” and gamepad LED set to green.
* Disables **new** BT connections (`enableNewBluetoothConnections(false)`) to enforce single-device policy.
* Board LEDs → Green for 500 ms.

### onDisconnectedController(ctl)

* If it is the tracked controller, clear `myController` and show Red for 1 s.

### GamePad\_BatteryMonitor()

* Throttled to 1 Hz.
* Uses `ctl->battery()` (0–255). Thresholds:

  * 0 → unknown → gamepad LED Red.
  * ≤64 (\~25%) → Red, rumble once, board LEDs Red flash.
  * ≤128 (\~50%) → Yellow.
  * Else → Green.

### INIT\_BluetoothGamepad\_PairMode()

* If **User A + D** are held at boot:

  * `BP32.forgetBluetoothKeys()` to force clean pairing.
  * `enableNewBluetoothConnections(true)` and `BP32.setup(...)`.
  * Loop until a controller connects:

    * Feed WDT.
    * Flash board LEDs Blue ↔ White.
    * `BP32.update()`.
  * Then disable new connections.
* Else: normal `BP32.setup(...)`.
* Always: `BP32.enableVirtualDevice(false)` to block virtual HID.

**Net effect:** exactly one remembered gamepad, no multi-pairing, no virtual devices.

---

## 9) Battery monitor and power-derate

```cpp
float Low_Batt_Scaler = 0.25;  // runtime scaling factor for motor commands
unsigned long TriggerTime = 0;
bool Scaler_StepState = 0;
unsigned long Check_Period_TriggerTime = 0;

float LoRcore_BatteryMonitor(uint8_t cellCount, float perCellLowV=3.0, bool DEBUG=true)
```

Flow:

1. Read ADC → compute `vin_voltage`.
2. Threshold = `cellCount * perCellLowV` (e.g., 2S at 3.0 V/cell → 6.0 V).
3. Log every 500 ms if `DEBUG`.
4. If voltage **below** threshold:

   * Print “LOW Battery”.
   * Every 100 ms, toggle `Scaler_StepState` and set `Low_Batt_Scaler` to `0` or `0.25`.
     Also brief Red flash.
   * Result: pulsing drive inhibition to protect pack and alert user.
5. If above threshold: `Low_Batt_Scaler = 1.0`.

Return value: measured VIN (float volts).

---

## 10) Powerup\_Diagnostics\_LED()

Maps `esp_reset_reason()` to LED colors once at boot:

* Watchdog → White
* Brownout → Yellow (holds 3 s)
* Power-on → Green
* Software reset → Blue
* Panic → Red
* Unknown → Purple
  Then LEDs off.

---

## 11) Motor type system

### Enum and config table

```cpp
enum MotorType { MG90_CR, MG90_Degree, N20Plus, STD_SERVO, Victor_SPX, Talon_SRX, CUSTOM };

struct MotorTypeConfig {
  MotorType type;
  float pwmFreq;  int minPulseUs; int maxPulseUs;
  float inputMin; float inputMax; // reserved
};

MotorTypeConfig motorTypeConfigs[] = {
  { MG90_CR,   50,  500, 2500, -1,  1 },
  { MG90_Degree,50, 500, 2500,  1, 180},
  { N20Plus,    50, 1000,2000, -1,  1 },
  { Victor_SPX, 50, 1000,2000, -1,  1 },
  { Talon_SRX,  50, 1000,2000, -1,  1 },
  { STD_SERVO,  50, 1000,2000 },
};
```

* Unifies PWM period and pulse limits per device family.
* `inputMin/inputMax` fields are placeholders for future mapping logic.

### ConfigureMotorOutput(slot, type, startupPositionDeg)

1. Look up `pwmFreq`, pulse range for `type`.
2. `pinMode(IO_PINS[slot], OUTPUT)`.
3. `Servo.setPeriodHertz(pwmFreq)`, `attach(pin, min, max)`.
4. Write `startupPositionDeg` (default 90).
5. Log the full config.

In `setup()`, slots 1..12 are all configured as `N20Plus` at center (90 deg).

---

## 12) INIT\_LoRcore()

Calls, in order:

1. `INIT_InternalFeatures()`
2. `Powerup_Diagnostics_LED()`
3. `INIT_BluetoothGamepad_PairMode()`

---

## 13) setup()

* Runs `INIT_LoRcore()`.
* Logs “Motors Startup”.
* Configures all 12 motor channels as `N20Plus`, 50 Hz, 1000–2000 μs, starting at 90 deg.
* Logs “System Ready”.

---

## 14) loop()

Core cycle:

1. **WDT feed**

   ```cpp
   esp_task_wdt_reset();
   ```
2. **Bluepad32 service**

   ```cpp
   BP32.update();
   ```
3. **Battery check**

   ```cpp
   LoRcore_BatteryMonitor(2, 3.0); // 2S pack, 3.0 V/cell threshold
   ```
4. **If controller connected**

   * Run `GamePad_BatteryMonitor()` (1 Hz visual + haptic).
   * Read sticks with optional inversion:

     ```cpp
     // User_SW HIGH = normal, LOW = inverted
     int currentLeft  = (digitalRead(User_SW)==HIGH) ?  myController->axisRY() : -myController->axisRY();
     int currentRight = (digitalRead(User_SW)==HIGH) ? -myController->axisY()  :  myController->axisY(); // right stick inverted
     // Apply low-batt scaler
     currentLeft  *= Low_Batt_Scaler;
     currentRight *= Low_Batt_Scaler;
     ```
   * Deadband ±50 to remove drift.
   * Map to servo domain and clamp:

     ```cpp
     int MappedLeft  = constrain(map(currentLeft,  -512, 512, 0, 180), 0, 180);
     int MappedRight = constrain(map(currentRight, -512, 512, 0, 180), 0, 180);
     ```
   * Drive outputs:

     * Slots **1..6** = left
     * Slots **7..12** = right
     * `90` = stop, `0..90` = reverse, `90..180` = forward (per your driver’s convention).
   * LED rainbow animation while active.
   * `delay(50)` → \~20 Hz loop when connected.
5. **If controller disconnected**

   * Write `90` to all 12 channels to stop.
   * LEDs Ice Blue (waiting).
   * No blocking delay, so pairing can occur.

---

## 15) Control mapping summary

| Stick       | Axis | Sign (User\_SW=HIGH) | Channels       |
| ----------- | ---- | -------------------- | -------------- |
| Left drive  | RY   | +                    | 1,2,3,4,5,6    |
| Right drive | Y    | inverted             | 7,8,9,10,11,12 |

* Deadband: ±50 counts around center to suppress drift.
* Scaling: `map(-512..+512 → 0..180)`, then `constrain`.
* Low-battery behavior: `Low_Batt_Scaler` = `1.0` normal, toggles `0 ↔ 0.25` when below threshold, creating pulsed derate and red flashes.

---

## 16) Safety and single-controller guarantees

* Only one controller pointer (`myController`).
* New BT connections disabled after connect.
* Virtual devices disabled.
* Motors forced to neutral on disconnect.
* WDT active with 3 s timeout.
* Brownout cause flagged on boot.

---

## 17) Tuning knobs

* **WDT\_TIMEOUT**: raise if doing heavier work per loop.
* **Deadband**: change ±50 if your gamepad is driftier.
* **Map range**: keep `-512..512` unless you confirm your controller’s full scale.
* **Low-voltage**: set `perCellLowV` to 3.3 V for gentler cutoff, or 3.0 V for deeper discharge.
* **LED\_COUNT/BRIGHTNESS**: adjust to your strip.

---

## 18) Quick troubleshooting

* Port missing → install CH340 driver.
* Upload stuck → use BOOT+RST sequence.
* Motors twitch or not centered → confirm each slot configured, pulse min/max correct, and initial write at 90.
* Servo motion reversed → swap sign or swap motor leads per driver.
* No BT pairing → hold **A + D** on boot to forget keys and re-pair.
* Low-power pulsing under throttle → battery below threshold by design; charge or raise the threshold.
* ESP resets → check reset cause in boot log and the color code in `Powerup_Diagnostics_LED()`.

---

## 19) Extension points

* Add more **MotorTypeConfig** entries for other ESCs/servos.
* Use `inputMin/inputMax` fields to implement linearization or scaling per type.
* Populate **AUX\_PINS** for sensors on I²C/SPI/UART as needed.
* Gate outputs by `myController->throttle()` or buttons for modes.
* Add a persistent calibration for `VOLT_SLOPE/OFFSET` per device.


## License
**Apache-2.0** — see `LICENSE`.

Copyright 2025 Lord of Robots Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this project except in compliance with the License.
You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
